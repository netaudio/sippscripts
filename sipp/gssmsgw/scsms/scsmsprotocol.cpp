#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"
#include "cacti/util/Timestamp.h"

#include "scsmsprotocol.h"
#include "scsmsclient.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"


#define new DEBUG_NEW


SCSMSProtocol::SCSMSProtocol(const ServiceIdentifier& id, 
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseProtocol(id, configFile, handler)
,m_logger(Logger::getInstance("msggw"))
,m_msgLogger(Logger::getInstance("scsms"))
,m_cacheList(8192, 60)
,m_cycleTimes(0)
{
	m_client = NULL;
	setStatus(ST_NONE);
}

SCSMSProtocol::~SCSMSProtocol()
{
	close();
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}
}

bool	SCSMSProtocol::open()
{
	loadConfig();

	if(!bootupSocket())
	{
		m_logger.info("[%02d] %s\n", m_myID.m_appref, "Socket stack failed");
		return false;
	}

	if(NULL == m_client)
	{
		m_client = new SCSMSClient( m_myID.m_appref, this );

#pragma message("to do ...")
		m_client->setReconnectInterval(120);
	}

	assert(m_client);
	setStatus(ST_INIT);
	m_logger.info("[%02d] scsms client (0x%x) init\n",
		m_myID.m_appref, m_version);

	return m_client->start(m_serverPort, m_serverIP.c_str(), false);
}

bool	SCSMSProtocol::close()
{
	m_client->stop();

	setStatus(ST_ERROR);
	return true;
}

bool	SCSMSProtocol::activeTest()
{
	switch(getStatus())
	{
	case ST_NONE:
		return open();

	case ST_INIT:
		return true;

	case ST_READY:
		break;

	case ST_BUSY:
		break;

	case ST_ERROR:
		return false;

	default:
		break;
	}

	m_cycleTimes++;
	if(m_cycleTimes >= m_checkTimes)
	{

		m_cycleTimes = 0;
		//m_logger.debug("[%02d] cache packets total(%d)\n",m_myID.m_appref,m_cacheList.getSize());
		int i = m_cacheList.deleteExpirePacket();
		
		if(i > 0)
		{
			m_logger.debug("[%02d] del expire packet, total(%d)\n", m_myID.m_appref, i);
		}
	}

	return sendActiveTest();
}

int	SCSMSProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{ 
	switch(utm->getMessageId())
	{
	case _ReqSendSMS:
		return sendSubmit(source, utm);

	case _ReqGWService:
		break;

	default:
		break;
	}
	return -1;
}

void	SCSMSProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
	cacti::IniFile iniFile;
	if(!iniFile.open(m_configFile.c_str()))
		return;

	char key[256] = {0};

	sprintf(key, "client.%d.version", m_myID.m_appref);
	m_version	= (u8) iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.spID", m_myID.m_appref);
	m_spId	= iniFile.readString("gateway",key,"123456");

	m_cacheList.setLiveTime(m_recvTimeout);

	//test(2);
}



void	SCSMSProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
{
	switch(utm->getMessageId())
	{
	case _EvtServerConnected:
		{
			m_logger.info("[%02d] client connected\n", m_myID.m_appref);
			{
				//setStatus(ST_READY);
				sendLogin();
			}
		}
		break;

	case _EvtServerDisconnected:
		{
			m_logger.info("[%02d] client disconnected \n", m_myID.m_appref);
			//disconnect();
			//m_client->stop();
			setStatus(ST_ERROR);
		}
		break;

	default:
		break;
	}

}

void	SCSMSProtocol::handleClientMsg(cacti::Stream& content)
{
	SCSMSHeader header(0);
	content.reset();
	header.fromStream(content);
	SCSMSPacketPtr packet = SCSMSPacketFactory::createPacket(header.command);
	packet->setHeader(header);
	packet->fromStream(content, true);

	switch(header.command)
	{
	case SCSMS_Login_Resp:
		{
			SCSMSLoginResp* pkg = static_cast<SCSMSLoginResp*>(packet.get());
			m_logger.info("[%02d] login resp %d\n", m_myID.m_appref, pkg->status);

			if(pkg->status == SCSMS::Status_OK)
				setStatus(ST_READY);
			else
				setStatus(ST_ERROR);
		}
		break;

	case SCSMS_Deliver:
		{
			SCSMSDeliver* pkg = static_cast<SCSMSDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;

	case SCSMS_Active_Test:
		{
			sendActiveTestResp();
		}
		break;

	case SCSMS_Active_Test_Resp:
		{
			m_logger.debug("[%02d] activeTest resp\n", m_myID.m_appref);
		}
		break;

	case SCSMS_GW_REPORT:
		{
			SCSMSGWReport* pkg = static_cast<SCSMSGWReport*>(packet.get());
			handleGwReport(pkg);
		}
		break;

	case SCSMS_TERM_REPORT:
		{
			SCSMSTermReport* pkg = static_cast<SCSMSTermReport*>(packet.get());
			handleTermReport(pkg);
		}
		break;

	default:
		m_logger.info("[%02d] unknown cmd %d\n", m_myID.m_appref, header.command);
		break;
	}

}

bool	SCSMSProtocol::sendLogin()
{
	SCSMSLogin packet;

	packet.clientID.assign(m_userName.begin(), m_userName.end());
	packet.password.assign(m_userPassword.begin(), m_userPassword.end());

	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] send login status: %d\n", m_myID.m_appref, ret);

	return ret;
}

bool	SCSMSProtocol::sendActiveTest()
{
	SCSMSActiveTest packet;
	packet.status = 0;

	return m_client->sendPackage(packet);
}

bool	SCSMSProtocol::sendActiveTestResp()
{
	SCSMSActiveTestResp packet;
	packet.status = 0;

	return m_client->sendPackage(packet);
}

// UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, 0);
// (*utm)[_TagServiceCode] = "1181177";
// (*utm)[_TagCalling] = "1181177";
// (*utm)[_TagCallee] = "059522327651";
// (*utm)[_TagMessage] = "测试123456";
// sendSubmit(utm);

int	SCSMSProtocol::sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	SCSMSSubmit packet;

	//移动CMPP，电信SMGP，联通SGIP，网通SNGP
	if( !getParameter(packet.Msg_Kind, "Msg_Kind", *utm) )
	{
		m_logger.info("[%02d] submit failed, Msg_Kind is empty\n", m_myID.m_appref);
		return -1;
	}

	if(m_spId.length() > 6)
	{
		m_logger.info("[%02d] submit failed, length of spId > 6\n", m_myID.m_appref);
		return -1;
	}
	char acSeq[11] = {0};

	std::string LineNo = "1024" + m_spId;
	sprintf(acSeq, "%010d", atoi(LineNo.c_str()));
	LineNo = acSeq;

	LineNo += Timestamp::getNowTime().getFormattedTime("%Y%m%d%H%M%S");
	
	sprintf(acSeq, "%010d", m_seqGen.next());
	LineNo += acSeq;
	packet.LineNo.assign(LineNo.begin(), LineNo.end());
	
	packet.Pk_total = 1;
	packet.Pk_number = 1;

	//计费用户类型字段，默认为0
	//0：对目的终端MSISDN计费；
	//1：对源终端MSISDN计费；
	//2：对SP计费;
	//3：表示本字段无效，对谁计费参见Fee_terminal_Id字段。
	if( !getParameter(packet.Fee_UserType, "Fee_UserType", *utm) )
	{
		m_logger.info("[%02d] submit failed, Fee_UserType is empty\n", m_myID.m_appref);
		return -1;
	}

	//业务代码
	string serviceCode = (*utm)[_TagServiceCode];
	if("" == serviceCode)
	{
		serviceCode = m_serviceCode;
	}
	packet.Service_Code.assign(serviceCode.begin(), serviceCode.end());

	//计费类型
	if( !getParameter(packet.FeeType, "FeeType", *utm) )
	{
		m_logger.info("[%02d] submit failed, FeeType is empty\n", m_myID.m_appref);
		return -1;
	}

	//费用代码
	if( !getParameter(packet.FeeCode, "FeeCode", *utm) )
	{
		m_logger.info("[%02d] submit failed, FeeCode is empty\n", m_myID.m_appref);
		return -1;
	}
	
	//包月费/封顶费（联通无该字段）
	if( !getParameter(packet.FixedFee, "FixedFee", *utm) )
	{
		m_logger.info("[%02d] submit failed, FixedFee is empty\n", m_myID.m_appref);
		return -1;
	}

	//赠送费用（联通才有该字段）
	if( !getParameter(packet.GivenValue, "GivenValue", *utm) )
	{
		m_logger.info("[%02d] submit warning, GivenValue is empty\n", m_myID.m_appref);
	}

	//收费用户号码
	if( !getParameter(packet.Fee_terminal_Id, "Fee_terminal_Id", *utm) )
	{
		m_logger.info("[%02d] submit failed, Fee_terminal_Id is empty\n", m_myID.m_appref);
		return -1;
	}

	//短消息格式
	packet.Msg_Fmt = SCSMS::Format_GB;

	//发送号码
	if( !getParameter(packet.Src_Id, "Src_Id", *utm) )
	{
		m_logger.info("[%02d] submit failed, Src_Id is empty\n", m_myID.m_appref);
		return -1;
	}

	//接收号码
	if( !getParameter(packet.Dest_Id, "Dest_Id", *utm) )
	{
		m_logger.info("[%02d] submit failed, Dest_Id is empty\n", m_myID.m_appref);
		return -1;
	}

	packet.Dest_Id_type = 0;

	//短消息内容
	string msgContent = (*utm)[_TagMessage];//"test ok!";
	if(msgContent.length() <= 0)
	{
		m_logger.info("[%02d] submit failed, Msg_Content is empty\n", m_myID.m_appref);
		return -1;
	}
	else
	{
		m_logger.debug("[%02d] submit Msg_Content :%s\n",m_myID.m_appref,msgContent.c_str());
	}

	
	//上行信息的LINKID，无上行不填
	if( !getParameter(packet.LinkID, "LinkID", *utm) )
	{
		m_logger.info("[%02d] submit warning, LinkID is empty\n", m_myID.m_appref);
	}

	//短信类型
	if( !getParameter(packet.MsgType, "MsgType", *utm) )
	{
		m_logger.info("[%02d] submit failed, MsgType is empty\n", m_myID.m_appref);
		return -1;
	}

	//SpCode
	if( !getParameter(packet.SpCode, "SpCode", *utm) )
	{
		m_logger.info("[%02d] submit warning, SpCode is empty\n", m_myID.m_appref);
	}

	//ServiceID
	if( !getParameter(packet.ServiceID, "ServiceID", *utm) )
	{
		m_logger.info("[%02d] submit warning, ServiceID is empty\n", m_myID.m_appref);

	}

	//[21]短信息发送方号码
//	string Src_Id = (*utm)[_TagCalling];
//	packet.Src_Id.assign(Src_Id.begin(), Src_Id.end());

	string Src_Id;
	Src_Id.assign(packet.Src_Id.begin(), packet.Src_Id.end());
	StringUtil::trim(Src_Id);


	//[21*DestTermCount]短消息接收号码
//	string Dest_Id = (*utm)[_TagCallee];
//	packet.Dest_Id.assign(Dest_Id.begin(), Dest_Id.end());

	string Dest_Id;
	Dest_Id.assign(packet.Dest_Id.begin(), packet.Dest_Id.end());
	StringUtil::trim(Dest_Id);

	bool bRet = false;
	string buf;

	if(msgContent.length() <= SCSMS::Max_Message_Size)
	{
		packet.Msg_Content.assign(msgContent.begin(), msgContent.end());

		bRet = m_client->sendPackage(packet);
		
		buf.assign(msgContent.begin(), msgContent.end());
		cacti::StringUtil::replace(buf, "\n", " ");
		m_msgLogger.info("^^%02d^^MT^^%d^^%s^^%s^^%s^^%s\n",
			m_myID.m_appref,
			(bRet == true ? 1 : 0),
			LineNo.c_str(),
			Src_Id.c_str(),
			Dest_Id.c_str(),
			msgContent.c_str());
	}
	else
	{
		//ceil(msgContent.length() / SCSMS::Max_Message_Size); //c func
		int cycleCount = (msgContent.length() / SCSMS::Max_Message_Size);//拆词 段落数
		int lastSize = msgContent.length() % SCSMS::Max_Message_Size;
		cycleCount +=  (lastSize > 0) ? 1 : 0;

		packet.Pk_total = (u8)cycleCount;

		int iPos = 0;
		int prePos = 0;
		bool needSended;
		for(int i = 0; i < cycleCount; i++)
		{
			needSended =false;
			packet.Pk_number = (u8)(i+1);			

			iPos = i * SCSMS::Max_Message_Size + prePos;
			//改变LineNo ??
			if( lastSize > 0 && ((i+1) == cycleCount) )
			{
				if (lastSize - prePos >0)
				{					
					packet.Msg_Content.assign(msgContent.end()-lastSize+prePos , msgContent.end());
					buf.assign(msgContent.end()-lastSize+prePos, msgContent.end());
					needSended = true;
				}
				
			}
			else
			{		

				int iEnd = (i+1)*SCSMS::Max_Message_Size;

				char little[10] ;
				string sMsg = "";
				for (int j=0;j<SCSMS::Max_Message_Size;)
				{
					if (iPos+j < msgContent.length())
					{
						int tmpPos= iPos+j ;
						memset(little,0,sizeof(little));
						strncpy(little,msgContent.c_str()+tmpPos,1);
						//判断是否双字节字符
						if(little[0]<0)
						{
							strncpy(little,msgContent.c_str()+tmpPos,2);			
							j = j+2;
							if (j==SCSMS::Max_Message_Size+1)
							{
								prePos ++;	//遇双字节字符，往前进一个BYTE，
							}
						}else{
							j = j+1;
						}
						sMsg = sMsg + little;
					}else
						break;

				}	
				
				packet.Msg_Content.assign(sMsg.begin(), sMsg.end());
				buf.assign(sMsg.begin(), sMsg.end());
				needSended =true;
				//packet.Msg_Content.assign(msgContent.begin()+iPos, msgContent.begin()+iEnd);
				//buf.assign(msgContent.begin()+iPos, msgContent.begin()+iEnd);
			}

			if (needSended)
			{
				/*	debug break */
				bRet = m_client->sendPackage(packet); //only judge last send
				
				cacti::StringUtil::replace(buf, "\n", " ");
				m_msgLogger.info("^^%02d^^MT^^%d^^%s^^%s^^%s^^%s\n",
					m_myID.m_appref,
					(bRet == true ? 1 : 0),
					LineNo.c_str(),
					Src_Id.c_str(),
					Dest_Id.c_str(),
					buf.c_str());
			}
			
		}
	}

	if(bRet == true)
	{
		CachedPacketPtr pkg = CachedPacketPtr(new CachedPacket());
		pkg->source = source;
		pkg->exchangeID = LineNo;
		pkg->utm = utm;
		pkg->packetType = 0;
		//m_logger.debug("[%02d] cache Package (%s)\n",m_myID.m_appref,LineNo.c_str());
		m_cacheList.addPacket(LineNo.c_str(), pkg);

		CachedPacket* cachePkg = m_cacheList.getPacket(LineNo.c_str());
		//m_logger.debug("[%02d] try get cache Package (%s)\n",m_myID.m_appref,cachePkg->exchangeID.c_str());
	}

	return bRet == true ? -2 : -1;
}

bool	SCSMSProtocol::handleGwReport(SCSMSGWReport* pkg)
{
#pragma message("to do ...")

	std::string LineNo;
	LineNo.assign(pkg->LineNo.begin(), pkg->LineNo.end());
	StringUtil::trim(LineNo);

	std::string ReportFlag;
	ReportFlag.assign(pkg->ReportFlag.begin(), pkg->ReportFlag.end());
	StringUtil::trim(ReportFlag);

	std::string ReportFlagTime;
	ReportFlagTime.assign(pkg->ReportFlagTime.begin(), pkg->ReportFlagTime.end());
	StringUtil::trim(ReportFlagTime);

	m_logger.info("[%02d] gw report %s,%s,%s\n", m_myID.m_appref, 
		LineNo.c_str(), ReportFlag.c_str(), ReportFlagTime.c_str());

	
	//if (!strcmp(ReportFlag.c_str(),std::string("0").c_str()))
	if (ReportFlag[0] != '0')
	{
		m_logger.debug("[%02d] gw report failed! To delete packet (%s)\n",m_myID.m_appref,LineNo.c_str());
		CachedPacket* cachePkg = m_cacheList.getPacket(LineNo.c_str());

		if(cachePkg != NULL)
		{			
			respMessage(cachePkg->source, cachePkg->utm, 1);
			m_cacheList.deletePacket(LineNo.c_str());
		}
	}	
	
	return true;
}

bool	SCSMSProtocol::handleTermReport(SCSMSTermReport* pkg)
{
#pragma message("to do ...")

	std::string LineNo;
	LineNo.assign(pkg->LineNo.begin(), pkg->LineNo.end());
	StringUtil::trim(LineNo);

	m_logger.debug("[%02d] term report get lineno (%s)\n",m_myID.m_appref,LineNo.c_str());
	CachedPacket* cachePkg = m_cacheList.getPacket(LineNo.c_str());

	std::string ReportFlag;
	ReportFlag.assign(pkg->ReportFlag.begin(), pkg->ReportFlag.end());
	StringUtil::trim(ReportFlag);

	std::string ReportFlagTime;
	ReportFlagTime.assign(pkg->ReportFlagTime.begin(), pkg->ReportFlagTime.end());
	StringUtil::trim(ReportFlagTime);

	m_logger.info("[%02d] term report %s,%s,%s\n", m_myID.m_appref, 
		LineNo.c_str(), ReportFlag.c_str(), ReportFlagTime.c_str());


	if(cachePkg != NULL)
	{
		u32 SendSuccess = 1;
		if (ReportFlag[0] == '0' || !strcmp(ReportFlag.c_str(),"DELIVRD"))
		{
			SendSuccess = 0;
		}
		//m_logger.debug("[%02d] before deletePacket total(%d)\n",m_myID.m_appref,m_cacheList.getSize());
		respMessage(cachePkg->source, cachePkg->utm, SendSuccess);
		
		m_cacheList.deletePacket(LineNo.c_str());
		//m_logger.debug("[%02d] after deletePacket total(%d)\n",m_myID.m_appref,m_cacheList.getSize());
	}

	
	return true;
}

bool	SCSMSProtocol::handleDeliver(SCSMSDeliver* pkg)
{
	string LineNo;
	LineNo.assign(pkg->LineNo.begin(), pkg->LineNo.end());
	StringUtil::trim(LineNo);

	string Msg_Id;
	Msg_Id.assign(pkg->Msg_Id.begin(), pkg->Msg_Id.end());
	StringUtil::trim(Msg_Id);
	
	string Dest_Id;
	Dest_Id.assign(pkg->Dest_Id.begin(), pkg->Dest_Id.end());
	StringUtil::trim(Dest_Id);
	
	//Dest_Id_Type

	string Src_Id;
	Src_Id.assign(pkg->Src_Id.begin(), pkg->Src_Id.end());
	StringUtil::trim(Src_Id);

	//Src_Id_Type

	string Service_Code;
	Service_Code.assign(pkg->Service_Code.begin(), pkg->Service_Code.end());
	StringUtil::trim(Service_Code);

	//Msg_Fmt

	string Msg_Content;
	Msg_Content.assign(pkg->Msg_Content.begin(), pkg->Msg_Content.end());
	StringUtil::trim(Msg_Content);
	
	string LinkId;
	LinkId.assign(pkg->LinkId.begin(), pkg->LinkId.end());
	StringUtil::trim(LinkId);


	ServiceIdentifier res(0,0,0);
	UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);
	(*utm)[_TagCalling] = Src_Id.c_str();
	(*utm)[_TagCallee]  = Dest_Id.c_str();
	(*utm)[_TagMessage] = Msg_Content.c_str();
	(*utm)[_TagOriCalling] = "";
	(*utm)[_TagOriCallee] = LinkId.c_str();
	(*utm)[_TagServiceId] = Service_Code.c_str();

	postToOwner(m_myID, utm);

	cacti::StringUtil::replace(Msg_Content, "\n", " ");
	m_msgLogger.info("^^%02d^^MO^^%d^^%s^^%s^^%s^^%s\n",
		m_myID.m_appref,
		1,
		LineNo.c_str(),
		Src_Id.c_str(),
		Dest_Id.c_str(),
		Msg_Content.c_str());

	return true;
}


int	SCSMSProtocol::respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret)
{
	utm->setMessageId(_ResSendSMS);
	utm->setRes(utm->getReq());
	utm->setReq(m_myID);

	utm->setReturn(ret);

	std::string message = "";
	char buf[20] = {0};

	sprintf(buf, "%d", ret);
	message = buf;
	(*utm)[_TagMessage] = message;

	//transfer message need return
	(*utm)[_TagGatewayName] = (*utm)[_TagController];


	postMessage(source, *utm);

	m_logger.debug("[%02d] resp to (%s), status(%d)\n",
		m_myID.m_appref, source.toString().c_str() ,ret);

	return SCSMS::Status_OK;
}

void SCSMSProtocol::test(int unit)
{
	if(unit == 1)
	{
		//01 3c 00 39 39 38 38 30 32 00 00 00 00 31 32 33 34 35 36 37 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 01 01 01 01 
		//02 01 00 30 01 01 01 01 01 

		std::string deliver = "00000";
		u8array bin;
		char tmp[3];
		const char* hex=deliver.c_str();
		for(size_t i=0; i<deliver.length(); i+=2)
		{
			strncpy(tmp, hex+i, 2);
			bin.push_back((u8)strtol(tmp, NULL, 16));
		}

		cacti::Stream content;
		content.putStream(&bin[0], deliver.length()/2);
		handleClientMsg(content);
	}
	
	if (unit == 2)
		{
			UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, 0);
			(*utm)[_TagServiceCode] = "1181177";
			(*utm)[_TagCalling] = "1181177";
			(*utm)[_TagCallee] = "059522327651";
			//(*utm)[_TagMessage] = "错爱歌词为:已刻了久躺的笔迹,揣摩完美无缺的轮廓,是我挑剔的手在颤抖,画不出一个结果,看窗外秋起的风景,挣脱夏日束缚的暖和,树叶飘落景色衬托,笑看姿体的婀娜,,谁把爱带走又留下把心紧握了又释放,此情无法接受的下次成了别人的美景才懂得珍惜,,毛笔绘出的诗句也绘出遗憾的经历,错失原有的爱情受得痛苦是自己,断断续续的恋情就没有幸福的结局,强求完美却遗失原本快乐的自己,,看窗外秋起的风景,挣脱夏日束缚的暖和,看树叶飘落把景色,衬托笑看姿体的婀娜,,谁把爱带走又留下把心紧握了又释放,此情无法接受的下次成了别人的美景才懂得珍惜,毛笔绘出的诗句也绘出遗憾的经历,错失原有的爱情受得痛苦是自己,断断续续的恋情就没有幸福的结局,强求完美却遗失原本快乐的自己,,毛笔绘出的诗句也绘出遗憾的经历,错失原有的爱情受得痛苦是自己,断断续续的恋情就没有幸福的结局,强求完美却遗失原本快乐的自己,强求完美遗失原本的快乐的自己";
			//有汉字拆词后仍有余字
			//(*utm)[_TagMessage] = "残留;是爱慢慢变成伤害;我们都被挫败;终于只剩残骸;无奈一切多麽精彩;越快乐越悲哀;都将回归尘埃;剩多少没能回报;是欠你的煎熬;如果错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从此后醒不来;迷惑不明白些什麽;不甘心的很多;太清醒也是错;有错错在陷得太深;错在想要太多;可错不在你我;还有多少没能回报;是欠你煎熬;如果有错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它缺少了个未来;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从此伏息苦海;";
			//有汉字拆词后无余字
			//(*utm)[_TagMessage] = "残留;是爱慢慢变成伤害;我们都被挫败;终于只剩残骸;无奈一切多麽精彩;越快乐越悲哀;都将回归尘埃;剩多少没能回报;是欠你的煎熬;如果错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从此后醒不来;迷惑不明白些什麽;不甘心的很多;太清醒也是错;有错错在陷得太深;错在想要太多;可错不在你我;还有多少没能回报;是欠你煎熬;如果有错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它缺少了个未来;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从此伏";
			//有汉字拆词后少余字
			(*utm)[_TagMessage] = "残留;是爱慢慢变成伤害;我们都被挫败;终于只剩残骸;无奈一切多麽精彩;越快乐越悲哀;都将回归尘埃;剩多少没能回报;是欠你的煎熬;如果错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从此后醒不来;迷惑不明白些什麽;不甘心的很多;太清醒也是错;有错错在陷得太深;错在想要太多;可错不在你我;还有多少没能回报;是欠你煎熬;如果有错请赦免我;别怪我太懦弱;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它缺少了个未来;给不到你要的生活;忘了那结不成的果;至少我们曾爱过;我还残留着你的爱;多痛它都存在;没有什么是能将它掩埋;只可惜它从";
			std::vector<std::string> m_paramName;
			std::vector<std::string> m_paramValues;

			m_paramName.push_back("Msg_Kind");
			m_paramValues.push_back("SMGP");
			m_paramName.push_back("Fee_UserType");
			m_paramValues.push_back("0");

			m_paramName.push_back("FeeType");
			m_paramValues.push_back("00");
			m_paramName.push_back("FeeCode");
			m_paramValues.push_back("000000");
			m_paramName.push_back("FixedFee");
			m_paramValues.push_back("000000");

			m_paramName.push_back("GivenValue");
			m_paramValues.push_back("000000");
			m_paramName.push_back("Fee_terminal_Id");
			m_paramValues.push_back("");
			m_paramName.push_back("Src_Id");
			m_paramValues.push_back("998802");
			m_paramName.push_back("Dest_Id");
			m_paramValues.push_back("");
			m_paramName.push_back("MsgType");
			m_paramValues.push_back("2");

			m_paramName.push_back("SpCode");
			m_paramValues.push_back("02");
			m_paramName.push_back("ServiceID");
			m_paramValues.push_back("998802");

			(*utm)[_TagParameterName] = m_paramName;
			(*utm)[_TagParameterValue] = m_paramValues;

			ServiceIdentifier* sender = new ServiceIdentifier();
			sendSubmit(*sender,utm);
		}
}

