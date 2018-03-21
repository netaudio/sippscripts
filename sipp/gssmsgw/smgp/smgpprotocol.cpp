#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"
#include "smgpprotocol.h"
#include "smgpclient.h"
#include "SMGPCacheControl.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW
#define _TEST


SMGPProtocol::SMGPProtocol(const ServiceIdentifier& id,
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseProtocol(id, configFile, handler)
,m_logger(Logger::getInstance("msggw"))
,m_msgLogger(Logger::getInstance("smgp"))
,m_cacheList(8192, 60)
,m_cycleTimes(0)
,m_longSms(0)
,m_splitSize(140)
,m_maxSendWindowSize(100)
{
	m_cacheControl = NULL;
	m_client = NULL;
	setStatus(ST_NONE);
}

SMGPProtocol::~SMGPProtocol()
{
	close();
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}
	if(m_cacheControl)
	{
		delete m_cacheControl;
		m_cacheControl = NULL;
	}
}

bool	SMGPProtocol::open()
{
	char month[10] = {0};
	int day;
	int year;
	sscanf(__DATE__, "%s %d %d", &month, &day, &year);
	int imonth = 0;
	if (strcmp(month, "Jan") == 0 )
		imonth = 1;
	else if (strcmp(month, "Feb") == 0 )
		imonth = 2;
	else if (strcmp(month, "Mar") == 0 )
		imonth = 3;
	else if (strcmp(month, "Apr") == 0 )
		imonth = 4;
	else if (strcmp(month, "May") == 0 )
		imonth = 5;
	else if (strcmp(month, "Jun") == 0 )
		imonth = 6;
	else if (strcmp(month, "Jul") == 0 )
		imonth = 7;
	else if (strcmp(month, "Aug") == 0 )
		imonth = 8;
	else if (strcmp(month, "Sep") == 0 )
		imonth = 9;
	else if (strcmp(month, "Oct") == 0 )
		imonth = 10;
	else if (strcmp(month, "Nov") == 0 )
		imonth = 11;
	else if (strcmp(month, "Dec") == 0 )
		imonth = 12;


	m_logger.info("[%02d] Start Process, build(%04d-%02d-%02d %s)\n",  
		m_myID.m_appref, year, imonth, day, __TIME__);

	m_startTime = time(0);
	loadConfig();

	if(!bootupSocket())
	{
		m_logger.info("[%02d] %s\n",m_myID.m_appref, "Socket stack failed");
		return false;
	}

	if(NULL == m_client)
	{
		m_client = new SMGPClient( m_myID.m_appref, this );
		

		m_client->setReconnectInterval(120);
#pragma message("to do ...")
		
	}

	assert(m_client);
	setStatus(ST_INIT);
	m_logger.info("[%02d] smgp client (0x%x) (%s, %d) init, support long sms(%d)\n",
		m_myID.m_appref, 
		m_version,
		m_serverIP.c_str(), m_serverPort,
		m_longSms);

	//m_sendThread.start(this, &SMGPProtocol::runSubmitSend);
//	m_stopSendThread = false;	
	if(NULL == m_cacheControl)
	{
		m_cacheControl = new SMGPCacheControl(this,m_maxSendWindowSize,m_recvTimeout);
		if(!m_cacheControl->open())
		{
			m_logger.error("[%02d] smgp control init failed %\n",
				m_myID.m_appref);
		}

	}


	bool ret = m_client->start(m_serverPort, m_serverIP.c_str(), false);
	if(!ret)
	{
		setStatus(ST_ERROR);
	}
	m_logger.info("[%02d] smgp client connect to server %s\n",
		m_myID.m_appref, ret == true ? "ok" : "failed");
	

	return ret;
}

bool	SMGPProtocol::close()
{
	if(getStatus() == ST_READY)
	{
		sendExit();
	}

	setStatus(ST_ERROR);
	return true;
}

bool	SMGPProtocol::activeTest()
{
// 	static bool hastTest = false;
#ifdef _TEST
	test(2);
#endif
// 	hastTest = true;




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
		return open();

	default:
		break;
	}

	m_cycleTimes++;
	if(m_cycleTimes >= m_checkTimes)
	{

		m_cycleTimes = 0;
		int i = m_cacheList.deleteExpirePacket();
		if(i > 0)
		{
			m_statPackets.lostPackets++;

			m_logger.debug("[%02d] del expire packet, total(%d)\n",
				m_myID.m_appref, i);
		}
	}
	/*
	if( (time(0) - m_lastRecvTime) > (m_cycleTimes * 60) )
	{
		if(m_cycleTimes >= m_checkTimes)
		{
			m_cycleTimes = 0;
		}

		m_logger.error("[%02d] error, recv nothing, %d seconds \n", m_myID.m_appref, m_cycleTimes * 60);
		m_client->stop();
		setStatus(ST_ERROR);
		return true;
	}
	*/

	m_logger.debug("[%02d] recv(%ld), send(%ld), recv error(%ld), send error(%ld)\n", 
		m_myID.m_appref, m_statPackets.totalRecvPackets, m_statPackets.totalSendPackets,
		m_statPackets.recvErrorPackets, m_statPackets.sendErrorPackets);

	return sendActiveTest();
}

int	SMGPProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{ 
	switch(utm->getMessageId())
	{
	case _ReqSendSMS:
	case _ReqDispatchSMS:

		if(getStatus() != ST_READY)
		{
			respMessage(source, utm, -1);
			return -1;
		}

		return sendSubmit(source, utm);

	case _ReqQuerySendWindowSize:
		return handleQuerySendWindowSize(source,utm);
	case _ReqSMSSendBatch:
		return handleSendSubmit(source, utm);
	case _ReqGWService:
		break;

	default:
		break;
	}
	return -1;
}


void	SMGPProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
	cacti::IniFile iniFile;
	if(!iniFile.open(m_configFile.c_str()))
		return;

	char key[256] = {0};

	sprintf(key, "client.%d.version", m_myID.m_appref);
	m_version	= (u8) iniFile.readInt("gateway", key, SMGP::Version_20);

	sprintf(key, "client.%d.spID", m_myID.m_appref);
	m_spId	= iniFile.readString("gateway",key,"118100");

	m_longSms = iniFile.readInt("smgp", "longSms", 0);
	m_splitSize = iniFile.readInt("smgp", "splitSize", 140);

	m_maxSendWindowSize = iniFile.readInt("smgp", "maxSendWindowSize", 100);
	int t_transferid ; 
	t_transferid = iniFile.readInt("smgp", "smshttpgw",0);
	if(t_transferid==0)
	{
		setStatus(ST_ERROR);
		return;
	}
	m_smshttpgw.m_appid   = t_transferid;
	m_smshttpgw.m_appport = AppPort::_apGateway;
	m_smshttpgw.m_appref  = 0;

	m_cacheList.setLiveTime(m_recvTimeout);
}

void	SMGPProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
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
			//m_client->stop();  //此处执行此代码导致线程死锁
		//	m_client->exit();
			setStatus(ST_ERROR);
		}
		break;

	default:
		break;
	}

}

void	SMGPProtocol::handleClientMsg(cacti::Stream& content)
{
	SMGPHeader header(0);
	content.reset();
	if(false == header.fromStream(content))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}

	SMGPPacketPtr packet = SMGPPacketFactory::createPacket(header.command, m_version);
	packet->setHeader(header);
	if(false == packet->fromStream(content, true))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}

	m_statPackets.totalRecvPackets++;

	switch(header.command)
	{
	case SMGP_Login_Resp:
		{
			SMGPLoginResp* pkg = static_cast<SMGPLoginResp*>(packet.get());
			m_logger.info("[%02d] [%08x] <- login resp %d\n", 
				m_myID.m_appref, pkg->getSequence(), pkg->status);

			if(pkg->status == SMGP::Status_OK)
			{
				setStatus(ST_READY);
#ifdef _TEST
				//test(2);
#endif	
			}
			else
				setStatus(ST_ERROR);
		}
		break;

	case SMGP_Submit_Resp:
		{
			SMGPSubmitResp* pkg = static_cast<SMGPSubmitResp*>(packet.get());
			handleSubmitResp(pkg);
		}
		break;

	case SMGP_Deliver:
		{
			SMGPDeliver* pkg = static_cast<SMGPDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;

	case SMGP_Active_Test:
		{
			sendActiveTestResp(packet->getSequence());
		}
		break;

	case SMGP_Active_Test_Resp:
		{
			m_logger.debug("[%02d] [%08x] <- activeTest resp\n", 
				m_myID.m_appref, packet->getSequence());
		}
		break;

	case SMGP_Exit:
		{
			m_logger.info("[%02d] [%08x] <- exit\n", 
				m_myID.m_appref, packet->getSequence());
		}
		break;

	case SMGP_Exit_Resp:
		{
			m_logger.info("[%02d] [%08x] <- recv exit resp\n", 
				m_myID.m_appref, packet->getSequence());
		}
		break;

	default:
		m_statPackets.recvErrorPackets++;

		m_logger.info("[%02d] [%08x] <- unknown cmd %d\n", 
			m_myID.m_appref, packet->getSequence(), header.command);
		break;
	}

}

bool SMGPProtocol::sendPackage(SMGPPacket& pkg)
{
	Stream stream;
	stream.reserve(SMGP::Max_Packet_Size);
	if(!pkg.toStream(stream))
	{
		return false;
	}

	bool ret = m_client->sendStream(stream);
	if(ret)
	{
		m_statPackets.totalSendPackets ++;
	}
	else
	{
		m_statPackets.sendErrorPackets ++;
	}

	return ret;
}
bool SMGPProtocol::sendPackage(SMGPPacketCachePtr pkg)
{
	bool ret = m_client->sendStream(pkg->content);
	if(ret)
	{
		m_statPackets.totalSendPackets ++;
	}
	else
	{
		m_statPackets.sendErrorPackets ++;
	}
	return ret;
}
bool SMGPProtocol::sendSubmitPackage(SMGPSubmit& pkg)
{
	Stream stream;
	stream.reserve(SMGP::Max_Packet_Size);
	//only for check
	if(!pkg.toStream(stream))
	{
		return false;
	}
	return m_cacheControl->put(pkg);
}

bool	SMGPProtocol::sendLogin()
{
	SMGPLogin packet;
	packet.setSequence(m_seqGen.next());

	packet.clientID.assign(m_userName.begin(), m_userName.end());

	u32 stamp = makeTimestamp();
	string authenCode;
	makeAuthCode(authenCode, 
		m_userName.c_str(), 
		m_userPassword.c_str(), 
		stamp,SMS_SMGP);
	packet.authenticatorClient.assign(authenCode.begin(), authenCode.end());

	packet.loginMode     = SMGP::Mode_Transmit;
	packet.timeStamp     = stamp;
	packet.clientVersion = m_version;//SMGP::Version_20;

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> login (%s, %s), spid(%s), status: %s\n", 
		m_myID.m_appref, packet.getSequence(), 
		m_userName.c_str(), m_userPassword.c_str(),
		m_spId.c_str(),
		ret == true ? "ok" : "failed");

	return ret;

}

bool	SMGPProtocol::sendExit()
{
	SMGPExit packet;
	packet.setSequence(m_seqGen.next());

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> exit, status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");

	return ret;	
}

bool	SMGPProtocol::sendActiveTest()
{
	SMGPActiveTest packet;
	packet.setSequence(m_seqGen.next());

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> activeTest status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");
	
	return true;
}

bool	SMGPProtocol::sendActiveTestResp(u32 seq)
{
	SMGPActiveTestResp packet;
	packet.setSequence(seq);
	
	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> activeTestResp status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");

	return true;
}

int SMGPProtocol::handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	SMGPSubmit packet(m_version);

	//是否内部发送的群发短信
	u8 smsType = SMGP::Type_NormalMsg;
	if( getParameter(smsType, "groupSms", *utm) )
	{
		packet.setSmsType(smsType);
	}

	//短消息类型
	packet.msgType = SMGP::MsgType_MT;

	//SP是否要求返回状态报告
	u8 needReport = (*utm)[_TagNeedReport];
	packet.needReport = needReport;

	//短消息发送优先级
	u8 proirity = (*utm)[_TagPriority];
	packet.priority = proirity;

	//[10]业务代码
	string serviceID = (*utm)[_TagServiceCode];//"1181177";
	if("" == serviceID)
	{
		serviceID = m_serviceCode;
	}
	packet.serviceID.assign(serviceID.begin(), serviceID.end());

	//[2]收费类型
	string feeType = "00";
	if( !getParameter(feeType, "FeeType", *utm) )
	{
		feeType = "00";
	}
	packet.feeType.assign(feeType.begin(), feeType.end());

	//[6]资费代码
	string feeCode = "0";
	if( getParameter(feeCode, "FeeCode", *utm) )
	{
		packet.feeCode.assign(feeCode.begin(), feeCode.end());
	}

	//[6]包月费/封顶费
	string fixedFee = "0";
	if( getParameter(fixedFee, "FixedFee", *utm) )
	{
		packet.fixedFee.assign(fixedFee.begin(), fixedFee.end());
	}

	//短消息格式
	u8 msgFormat = SMGP::Format_GB;
	packet.msgFormat = msgFormat;
	if( getParameter(msgFormat, "MsgFormat", *utm) )
	{
		packet.msgFormat = msgFormat;
	}

	//[17]短消息有效时间
	//u8array		validTime;
	//[17]短消息定时发送时间
	//u8array		atTime;
	//string sendtime=(*utm)[_TagTimeValue];
	//packet.atTime.assign(sendtime.begin(),sendtime.end());

	//[21]短信息发送方号码
	string srcTermID = (*utm)[_TagCalling];//"1181177";
	packet.srcTermID.assign(srcTermID.begin(), srcTermID.end());

	
	//[21*DestTermCount]短消息接收号码
	string destTermID="";
	string destTermIDlist = (*utm)[_TagCallee];//"059522327651";
	vector<string> calledlist;
	StringUtil::split(calledlist,destTermIDlist,",");
	//短消息接收号码总数
	packet.destTermIDCount = calledlist.size();
//短信网关不支持群发则去掉此功能
	/*
	for(int i=0;i<calledlist.size()&&i<100;i++)
	{
		destTermID.append(21-calledlist[i].size(),'0');
		destTermID.append(calledlist[i]);

	}
	*/
	destTermID = calledlist[0];
	packet.destTermID.assign(destTermID.begin(), destTermID.end());

	//[21]计费用户号码
	string chargeTermID = "";
	if(getParameter(chargeTermID, "ChargeTermID", *utm))
	{
		packet.chargeTermID.assign(chargeTermID.begin(), chargeTermID.end());
	}
	else
	{
		packet.chargeTermID.assign(srcTermID.begin(), srcTermID.end());
		//packet.chargeTermID.assign(destTermID.begin(), destTermID.end());
	}


	//[MsgLength]短消息内容
	string msgContent = (*utm)[_TagMessage];//"test ok!";


	if(msgContent.length() <= 0)
	{
		m_logger.debug("[%02d] submit msg length = 0\n",
			m_myID.m_appref);

		respMessage(source, utm, -1);
		return -1;
	}

	bool bSyncOrder = false; //是否由网关自动处理同步订购关系,用于反向业务操作
	string syncID = "DG " + serviceID + "  " + destTermID;

	string submitMsgType = "0";
	string spDealResult = "0";
	string linkID = "0";
	string mServiceID = "0";

	if(m_version == SMGP::Version_30)
	{
		

		getParameter(submitMsgType, "SubmitMsgType", *utm);
		getParameter(spDealResult, "SPDealResult", *utm);
		getParameter(linkID, "LinkID", *utm);
		getParameter(mServiceID, "MServiceID", *utm);

		linkID.resize(20);
		packet.LinkID = (string)linkID;

		u8array acSpID;
		acSpID.assign(m_spId.begin(), m_spId.end());
		acSpID.resize(8);
		packet.MsgSrc = (u8array)acSpID;

		packet.ChargeUserType = (u8)0;
		packet.ChargeTermType = (u8)0;
		packet.DestTermType = (u8)0;

		//packet.ChargeTermPseudo
		//packet.DestTermPseudo

		packet.SubmitMsgType = u8(atoi(submitMsgType.c_str()));
		packet.SPDealResult = u8(atoi(spDealResult.c_str()));

		u8array sid;
		sid.assign(mServiceID.begin(), mServiceID.end());
		sid.resize(21);
		packet.MServiceID = (u8array)sid;
	}

	// 	if(packet.msgFormat == SMGP::Format_UCS2)
	// 	{
	string newMsgContent = "";
	int len = EncodeUcs2(msgContent, newMsgContent);
	if(len > 0)
	{
		msgContent.clear();
		msgContent = newMsgContent;
	}
	// 	}

	bool bRet = false;
	string buf    = "";
	string buf_gb = "";

	if(m_version == SMGP::Version_30)
	{
		packet.PkTotal = (u8)1;
		packet.PkNumber = (u8)1;
	}

	if(msgContent.length() <= m_splitSize)  //SMGP::Max_Message_Size
	{
		packet.setSequence(m_seqGen.next());

		if(packet.msgFormat == SMGP::Format_GB)
		{
			DecodeUcs2(msgContent, buf);
			packet.msgContent.assign(buf.begin(), buf.end());
		}
		else
		{
			packet.msgContent.assign(msgContent.begin(), msgContent.end());
			buf = (*utm)[_TagMessage];
		}

		packet.msgLength = (u8)packet.msgContent.size();

		//[8]保留 
		//u8array		reserve;

		bRet = sendSubmitPackage(packet);
		
		if(bRet == true)
		{
			CachedPacketPtr pkg = CachedPacketPtr(new CachedPacket());
			pkg->source = source;
			pkg->packetID = packet.getSequence();
			pkg->utm = utm;
			pkg->exchangeID = syncID;
			pkg->packetType = 3;//submit ok   waiting sumitresp

			m_cacheList.addPacket(packet.getSequence(), pkg);			
		}

		// 		if(packet.msgFormat == SMGP::Format_UCS2)
		// 		{
		//			buf = (*utm)[_TagMessage];
		// 		}
		// 		else
		// 		{
		// 			buf.assign(msgContent.begin(), msgContent.end());
		// 		}


		cacti::StringUtil::replace(buf, "\n", " ");
		cacti::StringUtil::replace(buf, "\r", " ");
		/*	m_msgLogger.info("^^%02d^^MT^^%d^^%02d^^%08x^^%s^^%s^^%s^^%s\n",
		m_myID.m_appref,
		(bRet == true ? 0 : 1),
		(u8)packet.SubmitMsgType,
		packet.getSequence(),
		linkID.c_str(),
		srcTermID.c_str(),
		destTermID.c_str(),
		buf.c_str());
		*/
		m_msgLogger.info("|%02d|%s|%02d|%s|MT|%08x|CMD:%08x|%d|%d|%s|%d|%s|%s|%s|%d|%s\n",
			source.m_appref,
			"",
			m_myID.m_appref,
			m_serviceName.c_str(),
			packet.getSequence(),
			SMGP::SMGP_Submit,
			(bRet == true ? 0 : 1),
			(u8)packet.SubmitMsgType,
			linkID.c_str(),
			packet.priority,
			chargeTermID.c_str(),
			srcTermID.c_str(),
			buf.c_str(),
			packet.destTermIDCount,
			destTermID.c_str());

	
		m_logger.debug("[%02d] [%08x] -> submit, submitMsgType(%d), linkId(%s), serviceId(%s), mServiceID(%s), srcTermID(%s), dstTermID(%s), msg(%s)\n",
		m_myID.m_appref, packet.getSequence(), 
		(u8)packet.SubmitMsgType,
		linkID.c_str(),
		serviceID.c_str(),
		mServiceID.c_str(),
		srcTermID.c_str(),
		destTermID.c_str(),
		buf.c_str());
		

	
	
	}
	else
	{
		u8array contentHeader;
		contentHeader.resize(6);

		int splitSize = m_splitSize;

		if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
		{
			packet.TP_udhi = (u8)0x01;

			contentHeader[0] = 0x05;
			contentHeader[1] = 0x00;
			contentHeader[2] = 0x03;
			contentHeader[3] = 0x08;

			splitSize = m_splitSize - 6;
		}

		//ceil(msgContent.length() / SMGP::Max_Message_Size); //c func
		int cycleCount = (msgContent.length() / splitSize);
		int lastSize = msgContent.length() % splitSize;
		cycleCount +=  (lastSize > 0) ? 1 : 0;

		if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
		{
			packet.PkTotal = (u8)cycleCount;
			contentHeader[4] = (u8)cycleCount;
		}

		int iPos = 0;
		for(int i = 0; i < cycleCount; i++)
		{
			if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
			{
				packet.PkNumber = (u8)(i+1);
				contentHeader[5] = (u8)(i+1);
			}

			packet.setSequence(m_seqGen.next());

			buf.clear();
			buf_gb.clear();
			// 			if(packet.msgFormat == Format_GB)
			// 			{
			// 				iPos += split_gbk_string(msgContent, buf, iPos, SMGP::Max_Message_Size);
			// 				packet.msgContent.assign(buf.begin(), buf.end());
			// 			}
			// 			else
			// 			{
			// 				iPos = i * SMGP::Max_Message_Size;
			// 				if( lastSize > 0 && ((i+1) == cycleCount) )
			// 				{
			// 					packet.msgContent.assign(msgContent.end()-lastSize, msgContent.end());
			// 					buf.assign(msgContent.end()-lastSize, msgContent.end());
			// 				}
			// 				else
			// 				{
			// 					packet.msgContent.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*SMGP::Max_Message_Size);
			// 					buf.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*SMGP::Max_Message_Size);
			// 				}
			// 			}

			iPos = i * splitSize;
			if( lastSize > 0 && ((i+1) == cycleCount) )
			{
				buf.assign(msgContent.end()-lastSize, msgContent.end());
			}
			else
			{
				buf.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*splitSize);
			}

			DecodeUcs2(buf, buf_gb);
			if(packet.msgFormat == Format_GB)
			{
				packet.msgContent.assign(buf_gb.begin(), buf_gb.end());
			}
			else
			{
				if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
				{
					packet.msgContent.assign(contentHeader.begin(), contentHeader.end());
					packet.msgContent.insert(packet.msgContent.end(), buf.begin(), buf.end());
				}
				else
				{
					packet.msgContent.assign(buf.begin(), buf.end());
				}
			}

			// 			if(packet.msgFormat == SMGP::Format_UCS2)
			// 			{
			// 				DecodeUcs2(buf, buf);
			// 			}

			packet.msgLength = (u8)packet.msgContent.size();
			bRet = sendSubmitPackage(packet); //only judge last send
			if(bRet == true)
			{
				CachedPacketPtr pkg = CachedPacketPtr(new CachedPacket());
				pkg->source = source;
				pkg->packetID = packet.getSequence();
				pkg->utm = utm;
				pkg->exchangeID = syncID;
				pkg->packetType = 3;//submit ok   waiting sumitresp

				m_cacheList.addPacket(packet.getSequence(), pkg);			
			}

			cacti::StringUtil::replace(buf, "\n", " ");
			cacti::StringUtil::replace(buf, "\r", " ");
			/*m_msgLogger.info("^^%02d^^MT^^%d^^%02d^^%08x^^%s^^%s^^%s^^%s\n",
			m_myID.m_appref,
			(bRet == true ? 0 : 1),
			(u8)packet.SubmitMsgType,
			packet.getSequence(),
			linkID.c_str(),
			srcTermID.c_str(),
			destTermID.c_str(),
			buf_gb.c_str());
			*/


			m_msgLogger.info("|%02d|%s|%02d|%s|MT|%08x|CMD:%08x|%d|%d|%s|%d|%s|%s|%s|%d|%s\n",
				source.m_appref,
				" ",
				m_myID.m_appref,
				m_serviceName.c_str(),
				packet.getSequence(),
				SMGP::SMGP_Submit,
				(bRet == true ? 0 : 1),
				(u8)packet.SubmitMsgType,
				linkID.c_str(),
				packet.priority,
				chargeTermID.c_str(),
				srcTermID.c_str(),
				buf_gb.c_str(),
				packet.destTermIDCount,
				destTermID.c_str());


			m_logger.debug("[%02d] [%08x] -> submit, submitMsgType(%d), linkId(%s), serviceId(%s), mServiceID(%s), srcTermID(%s), dstTermID(%s), msg(%s)\n",
				m_myID.m_appref, packet.getSequence(), 
				(u8)packet.SubmitMsgType,
				linkID.c_str(),
				serviceID.c_str(),
				mServiceID.c_str(),
				srcTermID.c_str(),
				destTermID.c_str(),
				buf_gb.c_str());
		}

	}

	if(bSyncOrder == true)
		return -2;

	return bRet == true ? 0 : -1;
}
int	SMGPProtocol::sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	SMGPSubmit packet(m_version);

	//是否内部发送的群发短信
	u8 smsType = SMGP::Type_NormalMsg;
	if( getParameter(smsType, "groupSms", *utm) )
	{
		packet.setSmsType(smsType);
	}

	//短消息类型
	packet.msgType = SMGP::MsgType_MT;

	//SP是否要求返回状态报告
	u8 needReport = SMGP::Report_Need;
	if( !getParameter(needReport, "NeedReport", *utm) )
	{
		needReport = SMGP::Report_Nothing;
	}
	packet.needReport = needReport;

	//短消息发送优先级
	u8 proirity = SMGP::Priority_Normal;
	if( !getParameter(proirity, "Priority", *utm) )
	{
		packet.priority = SMGP::Priority_Normal;
	}
	packet.priority = proirity;

	//[10]业务代码
	string serviceID = (*utm)[_TagServiceCode];//"1181177";
	if("" == serviceID)
	{
		serviceID = m_serviceCode;
	}
	packet.serviceID.assign(serviceID.begin(), serviceID.end());

	//[2]收费类型
	string feeType = "0";
	if( !getParameter(feeType, "FeeType", *utm) )
	{
		feeType = "0";
	}
	packet.feeType.assign(feeType.begin(), feeType.end());

	//[6]资费代码
	string feeCode = "0";
	if( getParameter(feeCode, "FeeCode", *utm) )
	{
		packet.feeCode.assign(feeCode.begin(), feeCode.end());
	}

	//[6]包月费/封顶费
	string fixedFee = "0";
	if( getParameter(fixedFee, "FixedFee", *utm) )
	{
		packet.fixedFee.assign(fixedFee.begin(), fixedFee.end());
	}

	//短消息格式
	u8 msgFormat = SMGP::Format_GB;
	packet.msgFormat = msgFormat;
	if( getParameter(msgFormat, "MsgFormat", *utm) )
	{
		packet.msgFormat = msgFormat;
	}

	//[17]短消息有效时间
	//u8array		validTime;
	//[17]短消息定时发送时间
	//u8array		atTime;

	//[21]短信息发送方号码
	string srcTermID = (*utm)[_TagCalling];//"1181177";
	packet.srcTermID.assign(srcTermID.begin(), srcTermID.end());

 
	//短消息接收号码总数
	packet.destTermIDCount = 1;
	//[21*DestTermCount]短消息接收号码
	string destTermID = (*utm)[_TagCallee];//"059522327651";
	packet.destTermID.assign(destTermID.begin(), destTermID.end());

	//[21]计费用户号码
	string chargeTermID = "";
	if(getParameter(chargeTermID, "ChargeTermID", *utm))
	{
		packet.chargeTermID.assign(chargeTermID.begin(), chargeTermID.end());
	}
	else
	{
		packet.chargeTermID.assign(srcTermID.begin(), srcTermID.end());
		//packet.chargeTermID.assign(destTermID.begin(), destTermID.end());
	}


	//[MsgLength]短消息内容
	string msgContent = (*utm)[_TagMessage];//"test ok!";


	if(msgContent.length() <= 0)
	{
		m_logger.debug("[%02d] submit msg length = 0\n",
			m_myID.m_appref);

		respMessage(source, utm, -1);
		return -1;
	}

	bool bSyncOrder = false; //是否由网关自动处理同步订购关系,用于反向业务操作
	string syncID = "DG " + serviceID + "  " + destTermID;

	string submitMsgType = "0";
	string spDealResult = "0";
	string linkID = "0";
	string mServiceID = "0";

	if(m_version == SMGP::Version_30)
	{
// 		int TP_pid = 0;  //To do: change to struct later
// 		__asm
// 		{
// 			push eax
// 			mov eax, 0100000b
// 			mov TP_pid,eax
// 			pop eax
// 		}
// 		packet.TP_pid = (u8)TP_pid;
// 		packet.TP_udhi = (u8)0;

		getParameter(submitMsgType, "SubmitMsgType", *utm);
		getParameter(spDealResult, "SPDealResult", *utm);
		getParameter(linkID, "LinkID", *utm);
		getParameter(mServiceID, "MServiceID", *utm);


		u32 iSyncOrder = 1;
		getParameter(iSyncOrder, "SyncOrder", *utm);  //是否由网关自动进行同步关系
		if( iSyncOrder != 0 )
		{
			bSyncOrder = true;
			submitMsgType = "13";
			spDealResult = "0";
			linkID = "0";
	
			msgContent = syncID;
		}

		linkID.resize(20);
		packet.LinkID = (string)linkID;

		u8array acSpID;
		acSpID.assign(m_spId.begin(), m_spId.end());
		acSpID.resize(8);
		packet.MsgSrc = (u8array)acSpID;
		
		packet.ChargeUserType = (u8)0;
		packet.ChargeTermType = (u8)0;
		packet.DestTermType = (u8)0;

		//packet.ChargeTermPseudo
		//packet.DestTermPseudo

		packet.SubmitMsgType = u8(atoi(submitMsgType.c_str()));
		packet.SPDealResult = u8(atoi(spDealResult.c_str()));

		u8array sid;
		sid.assign(mServiceID.begin(), mServiceID.end());
		sid.resize(21);
		packet.MServiceID = (u8array)sid;
	}

// 	if(packet.msgFormat == SMGP::Format_UCS2)
// 	{
		string newMsgContent = "";
		int len = EncodeUcs2(msgContent, newMsgContent);
		if(len > 0)
		{
			msgContent.clear();
			msgContent = newMsgContent;
		}
// 	}

	bool bRet = false;
	string buf    = "";
	string buf_gb = "";

	if(m_version == SMGP::Version_30)
	{
		packet.PkTotal = (u8)1;
		packet.PkNumber = (u8)1;
	}

	if(msgContent.length() <= m_splitSize)  //SMGP::Max_Message_Size
	{
		packet.setSequence(m_seqGen.next());

		if(packet.msgFormat == SMGP::Format_GB)
		{
			DecodeUcs2(msgContent, buf);
			packet.msgContent.assign(buf.begin(), buf.end());
		}
		else
		{
			packet.msgContent.assign(msgContent.begin(), msgContent.end());
			buf = (*utm)[_TagMessage];
		}

		packet.msgLength = (u8)packet.msgContent.size();

		//[8]保留 
		//u8array		reserve;

		bRet = sendSubmitPackage(packet);
		
// 		if(packet.msgFormat == SMGP::Format_UCS2)
// 		{
//			buf = (*utm)[_TagMessage];
// 		}
// 		else
// 		{
// 			buf.assign(msgContent.begin(), msgContent.end());
// 		}
		

		cacti::StringUtil::replace(buf, "\n", " ");
		cacti::StringUtil::replace(buf, "\r", " ");
	/*	m_msgLogger.info("^^%02d^^MT^^%d^^%02d^^%08x^^%s^^%s^^%s^^%s\n",
			m_myID.m_appref,
			(bRet == true ? 0 : 1),
			(u8)packet.SubmitMsgType,
			packet.getSequence(),
			linkID.c_str(),
			srcTermID.c_str(),
			destTermID.c_str(),
			buf.c_str());
*/
		m_msgLogger.info("|%02d|%s|%02d|%s|MT|%08x|CMD:%08x|%d|%d|%s|%d|%s|%s|%s|%d|%s\n",
			source.m_appref,
			"",
			m_myID.m_appref,
			m_serviceName.c_str(),
			packet.getSequence(),
			SMGP::SMGP_Submit,
			(bRet == true ? 0 : 1),
			(u8)packet.SubmitMsgType,
			linkID.c_str(),
			packet.priority,
			chargeTermID.c_str(),
			srcTermID.c_str(),
			buf.c_str(),
			packet.destTermIDCount,
			destTermID.c_str());

		
		m_logger.debug("[%02d] [%08x] -> submit, submitMsgType(%d), linkId(%s), serviceId(%s), mServiceID(%s), srcTermID(%s), dstTermID(%s), msg(%s)\n",
			m_myID.m_appref, packet.getSequence(), 
			(u8)packet.SubmitMsgType,
			linkID.c_str(),
			serviceID.c_str(),
			mServiceID.c_str(),
			srcTermID.c_str(),
			destTermID.c_str(),
			buf.c_str());
	
		if(m_version == SMGP::Version_30)
		{

			if(bRet == true && bSyncOrder == true)
			{
				CachedPacketPtr pkg = CachedPacketPtr(new CachedPacket());
				pkg->source = source;
				pkg->packetID = packet.getSequence();
				pkg->utm = utm;
				pkg->exchangeID = syncID;
				pkg->packetType = 0;

				m_cacheList.addPacket(packet.getSequence(), pkg);			
			
				/*m_logger.debug("[%02d] [%08x] -- msg cached, sync relation, linkid(%s), msg(%s)\n",
					m_myID.m_appref, packet.getSequence(), linkID.c_str(), syncID.c_str());
					*/
					
			}

			if(bRet == false)
			{
				CachedPacket* cachePkg = m_cacheList.getPacket(syncID);
				if(cachePkg != NULL)
				{
					respMessage(cachePkg->source, cachePkg->utm, -1);
					m_cacheList.deletePacket(syncID);
				}
			}

			if(bRet == true && bSyncOrder == false)
			{
				m_cacheList.changePacketID(syncID, packet.getSequence());
			}

		}

	}
	else
	{
		u8array contentHeader;
		contentHeader.resize(6);

		int splitSize = m_splitSize;

		if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
		{
			packet.TP_udhi = (u8)0x01;

			contentHeader[0] = 0x05;
			contentHeader[1] = 0x00;
			contentHeader[2] = 0x03;
			contentHeader[3] = 0x08;

			splitSize = m_splitSize - 6;
		}

		//ceil(msgContent.length() / SMGP::Max_Message_Size); //c func
		int cycleCount = (msgContent.length() / splitSize);
		int lastSize = msgContent.length() % splitSize;
		cycleCount +=  (lastSize > 0) ? 1 : 0;

		if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
		{
			packet.PkTotal = (u8)cycleCount;
			contentHeader[4] = (u8)cycleCount;
		}

		int iPos = 0;
		for(int i = 0; i < cycleCount; i++)
		{
			if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
			{
				packet.PkNumber = (u8)(i+1);
				contentHeader[5] = (u8)(i+1);
			}

			packet.setSequence(m_seqGen.next());

			buf.clear();
			buf_gb.clear();
// 			if(packet.msgFormat == Format_GB)
// 			{
// 				iPos += split_gbk_string(msgContent, buf, iPos, SMGP::Max_Message_Size);
// 				packet.msgContent.assign(buf.begin(), buf.end());
// 			}
// 			else
// 			{
// 				iPos = i * SMGP::Max_Message_Size;
// 				if( lastSize > 0 && ((i+1) == cycleCount) )
// 				{
// 					packet.msgContent.assign(msgContent.end()-lastSize, msgContent.end());
// 					buf.assign(msgContent.end()-lastSize, msgContent.end());
// 				}
// 				else
// 				{
// 					packet.msgContent.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*SMGP::Max_Message_Size);
// 					buf.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*SMGP::Max_Message_Size);
// 				}
// 			}

			iPos = i * splitSize;
			if( lastSize > 0 && ((i+1) == cycleCount) )
			{
				buf.assign(msgContent.end()-lastSize, msgContent.end());
			}
			else
			{
				buf.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*splitSize);
			}

			DecodeUcs2(buf, buf_gb);
			if(packet.msgFormat == Format_GB)
			{
				packet.msgContent.assign(buf_gb.begin(), buf_gb.end());
			}
			else
			{
				if(m_longSms > 0 && packet.msgFormat == Format_UCS2)
				{
					packet.msgContent.assign(contentHeader.begin(), contentHeader.end());
					packet.msgContent.insert(packet.msgContent.end(), buf.begin(), buf.end());
				}
				else
				{
					packet.msgContent.assign(buf.begin(), buf.end());
				}
			}

// 			if(packet.msgFormat == SMGP::Format_UCS2)
// 			{
// 				DecodeUcs2(buf, buf);
// 			}

			packet.msgLength = (u8)packet.msgContent.size();
			bRet = sendSubmitPackage(packet); //only judge last send
			
			cacti::StringUtil::replace(buf, "\n", " ");
			cacti::StringUtil::replace(buf, "\r", " ");
			/*m_msgLogger.info("^^%02d^^MT^^%d^^%02d^^%08x^^%s^^%s^^%s^^%s\n",
				m_myID.m_appref,
				(bRet == true ? 0 : 1),
				(u8)packet.SubmitMsgType,
				packet.getSequence(),
				linkID.c_str(),
				srcTermID.c_str(),
				destTermID.c_str(),
				buf_gb.c_str());
				*/
				

			m_msgLogger.info("|%02d|%s|%02d|%s|MT|%08x|CMD:%08x|%d|%d|%s|%d|%s|%s|%s|%d|%s\n",
				source.m_appref,
				" ",
				m_myID.m_appref,
				m_serviceName.c_str(),
				packet.getSequence(),
				SMGP::SMGP_Submit,
				(bRet == true ? 0 : 1),
				(u8)packet.SubmitMsgType,
				linkID.c_str(),
				packet.priority,
				chargeTermID.c_str(),
				srcTermID.c_str(),
				buf_gb.c_str(),
				packet.destTermIDCount,
				destTermID.c_str());


			m_logger.debug("[%02d] [%08x] -> submit, submitMsgType(%d), linkId(%s), serviceId(%s), mServiceID(%s), srcTermID(%s), dstTermID(%s), msg(%s)\n",
				m_myID.m_appref, packet.getSequence(), 
				(u8)packet.SubmitMsgType,
				linkID.c_str(),
				serviceID.c_str(),
				mServiceID.c_str(),
				srcTermID.c_str(),
				destTermID.c_str(),
				buf_gb.c_str());
		}
		
	}
	
	if(bSyncOrder == true)
		return -2;

	return bRet == true ? 0 : -1;
}

bool	SMGPProtocol::handleSubmitResp(SMGPSubmitResp* pkg)
{
#pragma message("to do ...")
	
	unsigned char* smsgwid = &pkg->msgID[0];

	string msgid = StringUtil::BCD2ASC((const unsigned char*)(smsgwid),20,0);
	
	m_logger.debug("[%02d] [%08x] <- submit resp %d ,msgid(%s)\n", 
	m_myID.m_appref, pkg->getSequence(), pkg->status,msgid.c_str());
	
	/*
	m_msgLogger.info("|||%02d|%s|MT|%08x|CMD:%08x|%s|%d\n",
		m_myID.m_appref,
		m_serviceName.c_str(),
		pkg->getSequence(),
		SMGP::SMGP_Submit_Resp,
		msgid.c_str(),
		pkg->status
		);
*/
	m_msgLogger.info("|SMS|%d|%02d|MTResp|CMD:%08x|%s|%s|%s|%d|%s|%d|%s\n",
		m_myID.m_appid,
		m_myID.m_appref,

		SMGP::SMGP_Submit_Resp,
		"SMGP_Submit_Resp",
		"",
		m_serviceName.c_str(),
		pkg->getSequence(),
		msgid.c_str(),
		pkg->status,
		""
		);

	if(m_version != SMGP::Version_30)
	{
		return true;
	}

	/*if(pkg->status != SMGP::Status_OK)
	{
		CachedPacket* cachePkg = m_cacheList.getPacket(pkg->getSequence());

		if(cachePkg != NULL)
		{
			respMessage(cachePkg->source, cachePkg->utm, -1);
			m_cacheList.deletePacket(pkg->getSequence());
		}
	}
	else
	{
	*/
	CachedPacket* cachePkg = m_cacheList.getPacket(pkg->getSequence());
	if(cachePkg != NULL)
	{

		if(cachePkg->packetType == 0)//发送订购关系请求成功
		{
			cachePkg->packetType = 1;//等待同步订购关系LinkID
			m_cacheList.changePacketID(pkg->getSequence(), cachePkg->exchangeID);
		}
		else if(cachePkg->packetType == 2)//正式下发短信成功
		{
			respMessage(cachePkg->source, cachePkg->utm, pkg->status);
			m_cacheList.deletePacket(pkg->getSequence());
		}else if(cachePkg->packetType == 3)//普通下行短信回复。
		{
			respSubmitMessage(cachePkg->source, cachePkg->utm, pkg->status,*pkg);
			m_cacheList.deletePacket(pkg->getSequence());
		}
	}
	//}
	m_cacheControl->response(pkg->getSequence());

	return true;
}

bool	SMGPProtocol::sendDeliverResp(u32 seq, u8array msgID, u32 status)
{
	SMGPDeliverResp pkg;
	pkg.setSequence(seq);
	pkg.msgID = msgID;
	pkg.status = status;

	return sendPackage(pkg);
}

bool	SMGPProtocol::handleDeliver(SMGPDeliver* pkg)
{
	string dstTermID;
	dstTermID.assign(pkg->destTermID.begin(), pkg->destTermID.end());
	string srcTermID;
	srcTermID.assign(pkg->srcTermID.begin(), pkg->srcTermID.end());
	string linkID = (string)pkg->LinkID;
	string content;
	content.assign(pkg->msgContent.begin(), pkg->msgContent.end());
	string msgid;
	unsigned char* t_msgid = &pkg->msgID[0];
	msgid = StringUtil::BCD2ASC((const unsigned char*)t_msgid,20);
	
//TODO：不同的编码格式应该给予不同的处理
	if(pkg->msgFormat == SMGP::Format_UCS2)
	{
		string newContent;
		int len = DecodeUcs2(content, newContent);
		if(len > 0)
		{
			content = newContent;
		}
	}
	/*
	else if(pkg->msgFormat == SMGP::Format_Ascii)
	{
		
	}
	*/

	
	string recvtime;
	recvtime.assign(pkg->recvTime.begin(),pkg->recvTime.end());



	if(pkg->isReport == SMGP::Report_Need)
	{	
		string newContent;

		parseReportMsgContent(pkg->msgContent,pkg->msgLength,newContent);

		if(newContent.size()>0)
			content = newContent;
		
		transferReport(srcTermID,dstTermID,content);

	}else{

		transferDeliver(srcTermID,dstTermID,content);

	}
	
	

	m_logger.debug("[%02d] [%08x] <- deliver, %d, report=%d, submitMsgType(%d), linkid(%s),msgid(%s) srcTermID(%s), dstTermID(%s), msg(%s), spDealResult(%d)\n",
		m_myID.m_appref,
		pkg->getSequence(),
		(u8)pkg->SPDealResult,
		pkg->isReport,
		(u8)pkg->SubmitMsgType,
		linkID.c_str(),
		msgid.c_str(),
		srcTermID.c_str(),
		dstTermID.c_str(),
		content.c_str(),
		(u8)pkg->SPDealResult
		);

	bool bRet = sendDeliverResp(pkg->getSequence(), pkg->msgID, SMGP::Status_OK);
	m_logger.info("[%02d] [%08x] -> deliverResp %s \n",
		m_myID.m_appref,
		pkg->getSequence(),
		(bRet == false ? "failed" : "ok"));
/*

	if(pkg->isReport == SMGP::Report_Need)
	{
#pragma message("to do ...")	

		m_msgLogger.info("^^%02d^^MO^^%d^^%08x^^%s^^%s^^%s\n",
			m_myID.m_appref,
			1,
			pkg->getSequence(),
			srcTermID.c_str(),
			dstTermID.c_str(),
			"is report");

		return true;
	}

	m_msgLogger.info("^^%02d^^MO^^%d^^%02d^^%08x^^%s^^%s^^%s^^%s\n",
		m_myID.m_appref,
		(u8)pkg->SPDealResult,
		(u8)pkg->SubmitMsgType,
		pkg->getSequence(),
		linkID.c_str(),
		srcTermID.c_str(),
		dstTermID.c_str(),
		content.c_str());

		*/


	if(pkg->isReport == SMGP::Report_Need)
	{
#pragma message("to do ...")	


		m_msgLogger.info("|||%02d|%s|MO|%08x|CMD:%08x|%s|%s|%d|%s|%s|%s|%s\n",
		m_myID.m_appref,
		m_serviceName.c_str(),
		pkg->getSequence(),
		SMGP::SMGP_Deliver,
		msgid.c_str(),
		linkID.c_str(),
		pkg->isReport,
		recvtime.c_str(),
		srcTermID.c_str(),
		dstTermID.c_str(),
		content.c_str());
		return true;
	}
	m_msgLogger.info("|||%02d|%s|MO|%08x|CMD:%08x|%s|%s|%d|%s|%s|%s|%s|%d|%d\n",
		m_myID.m_appref,
		m_serviceName.c_str(),
		pkg->getSequence(),
		SMGP::SMGP_Deliver,
		msgid.c_str(),
		linkID.c_str(),
		pkg->isReport,
		recvtime.c_str(),
		srcTermID.c_str(),
		dstTermID.c_str(),
		content.c_str(),
		(u8)pkg->SPDealResult,
		(u8)pkg->SubmitMsgType);

	
	if(m_version == SMGP::Version_30 && (u8)pkg->SubmitMsgType == 15)
	{
		CachedPacket* cache = m_cacheList.getPacket(content);
		if(cache != NULL)
		{
			u8 result = (u8)pkg->SPDealResult;
			if(result == 0 && linkID.length() > 0)
			{
				std::vector<std::string> m_paramName;
				std::vector<std::string> m_paramValues;

				m_paramName.push_back("SubmitMsgType");
				m_paramValues.push_back("0");

				m_paramName.push_back("SPDealResult");
				m_paramValues.push_back("0");

				m_paramName.push_back("LinkID");
				m_paramValues.push_back(linkID);

				m_paramName.push_back("ChargeTermID");
				m_paramValues.push_back(srcTermID.c_str());

				m_paramName.push_back("SyncOrder");
				m_paramValues.push_back("0");       //是否已同步标记

				(*(cache->utm))[_TagParameterName] = m_paramName;
				(*(cache->utm))[_TagParameterValue] = m_paramValues;

				cache->packetType = 2;
				sendSubmit(cache->source, cache->utm);
			}
			else
			{
				respMessage(cache->source, cache->utm, result);
				m_cacheList.deletePacket(content);
			}

			return true;
		}

	}

	ServiceIdentifier res(0,0,0);
	UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);
	(*utm)[_TagCalling] = srcTermID.c_str();
	(*utm)[_TagCallee]  = dstTermID.c_str();
	(*utm)[_TagMessage] = content.c_str();

	if(m_version == SMGP::Version_30)
	{
		char acTemp[32] = {0};
		sprintf_s(acTemp, "%d", (u8)pkg->SubmitMsgType);

		(*utm)[_TagOriCalling] = string(acTemp);
		(*utm)[_TagOriCallee] = (string)pkg->LinkID;
		(*utm)[_TagServiceId] = (u8)pkg->SPDealResult;
	}
	else
	{
		(*utm)[_TagOriCalling] = "";
		(*utm)[_TagOriCallee] = "";
		(*utm)[_TagServiceId] = (u8)0;
	}

	postToOwner(m_myID, utm);
	return true;
}
int SMGPProtocol::respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,SMGPSubmitResp& pkg)
{
	utm->setMessageId(_ResSMSSendBatch);
	utm->setRes(utm->getReq());
	utm->setReq(m_myID);

	utm->setReturn(ret);

	unsigned char* smsgwid = &pkg.msgID[0];

	string msgid = StringUtil::BCD2ASC((const unsigned char*)(smsgwid),20,0);

	(*utm)[_TagStatus] = pkg.status;
	(*utm)[_TagMessageId]  =msgid;

	postMessage(source, *utm);

	m_logger.debug("[%02d] respSubmit  _ResSMSSendBatch to (%s), ret(%d),msgstatus(%d)\n",
		m_myID.m_appref, source.toString().c_str() ,ret,pkg.status);

	return SMGP::Status_OK;
}
int	SMGPProtocol::respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret)
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


		string srcTermID = (*utm)[_TagCalling];//"118100";
		string destTermID = (*utm)[_TagCallee];//"059522327651";
		postMessage(source, *utm);

		m_logger.debug("[%02d] resp to (%s), status(%d), srcTermID(%s), dstTermID(%s)\n",
			m_myID.m_appref, source.toString().c_str() ,ret,
			srcTermID.c_str(), destTermID.c_str());

	return SMGP::Status_OK;
}
/*
void SMGPProtocol::runSubmitSend()
{
	Stream pkg;
	bool ret = true;

	//example: if sleeptime=50ms, then the thread will send 20 packets per second when send only 1 packet once time, but ignore the total time of decode the each packet;
	//must be large than 10 (windows limit to 10ms, not rigidly accurate)
	//m_maxWindowSize >= 20, ignore less then 20
	//TODO:哎,还是要抽空优化
	int sleepTime = 50;

	//default 20 sections
	int section = 1000/sleepTime; 
	
	if(m_maxWindowSize < section)
	{
		m_logger.info("[%02d] adjust speed from %d to %d \n", m_myID.m_appref, m_maxWindowSize, section);
		m_maxWindowSize  = section;
	}

	int countPerSection = m_maxWindowSize/section;
// 	if(countPerSection < 0)
// 	{
// 		countPerSection = 1;
// 	}

	int adjustPerSection = m_maxWindowSize%section;
	
	int hasSendPerSection = 0;
	int sectionFinished = 0;
	int hasSendTotalAdjust = 0;
	bool hasSendAdjustPerSection = false;

	//1s = 1000ms = sleepTime * section
	//total(per second) = countPerSection*section + adjustPerSection;

	m_logger.info("[%02d] smgp client send thread started\n", m_myID.m_appref);
	m_logger.info("[%02d] sms send, max(%d) = section(%d) * avg(%d) + adjust(%d), interval(%dms)\n", 
		m_myID.m_appref, m_maxWindowSize, section, countPerSection, adjustPerSection, sleepTime);

	
	int sendPerSecond = 0;
	time_t nowTime = time(0);

	while(!m_stopSendThread)
	{
		pkg.clear();
		pkg.reset();

		if(m_userSmsQueue.tryGet(pkg))
		{
			if(nowTime != time(0))
			{
				m_logger.info("[%02d] pkg speed: %d/s\n", m_myID.m_appref, sendPerSecond);
				nowTime = time(0);
				sendPerSecond = 1;
			}
			else
			{
				sendPerSecond ++;
			}

			ret = m_client->sendStream(pkg);
			if(ret)
			{
				m_statPackets.totalSendPackets ++;
			}
			else
			{
				m_statPackets.sendErrorPackets ++;
			}

			hasSendPerSection ++;

			if(hasSendPerSection < countPerSection)
			{
				continue;
			}
			else
			{
				if( adjustPerSection > 0 && hasSendTotalAdjust < adjustPerSection && hasSendAdjustPerSection == false )
				{
					hasSendTotalAdjust++;
					hasSendAdjustPerSection = true;
					continue;
				}
			}

			sectionFinished++;
			if(sectionFinished > section)
			{
				hasSendTotalAdjust = 0;
				sectionFinished = 0; //next second will begin
			}

			hasSendPerSection = 0;
			hasSendAdjustPerSection = false;

			cacti::Thread::sleep(Timestamp(0, sleepTime*1000));
			continue;
		}

		if(m_batchSmsQueue.tryGet(pkg))
		{

			if(nowTime != time(0))
			{
				m_logger.info("[%02d] pkg speed: %d/s\n", m_myID.m_appref, sendPerSecond);
				nowTime = time(0);
				sendPerSecond = 1;
			}
			else
			{
				sendPerSecond ++;
			}

			ret = m_client->sendStream(pkg);
			if(ret)
			{
				m_statPackets.totalSendPackets ++;
			}
			else
			{
				m_statPackets.sendErrorPackets ++;
			}

			hasSendPerSection ++;

			if(hasSendPerSection < countPerSection)
			{
				continue;
			}
			else
			{
				if( adjustPerSection > 0 && hasSendTotalAdjust < adjustPerSection && hasSendAdjustPerSection == false )
				{
					hasSendTotalAdjust++;
					hasSendAdjustPerSection = true;
					continue;
				}
			}

			sectionFinished++;
			if(sectionFinished > section)
			{
				hasSendTotalAdjust = 0;
				sectionFinished = 0; //next second will begin
			}

			hasSendPerSection = 0;
			hasSendAdjustPerSection = false;

			cacti::Thread::sleep(Timestamp(0, sleepTime*1000));
			continue;
		}

		cacti::Thread::sleep(Timestamp(0, sleepTime*1000));
	}

	//m_stopSendEvent.signal();
}
*/
void SMGPProtocol::snapshot()
{
	m_logger.info("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
		m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
		m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);

	Service::printConsole("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
		m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
		m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);
}

void SMGPProtocol::handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	strarray  vec = (*utm)[_TagCommand];
	AlarmSender* alarm = AlarmSender::getInstance();
	strarray ret;

#ifdef _DEBUG
	alarm->send(ServiceIdentifier(), "recv cmd", 0, vec);
	m_logger.debug("[%02d] recv cmd: (%s)\n", m_myID.m_appref, vec[0].c_str());
#endif

	if(vec.size() <= 0)
	{
		return;
	}

	if(vec[0] == "req")
	{
		//res module 
		//index,modulename,type,status,time,version,send,recv,lost
		ret.push_back(StringUtil::format("%d", m_myID.m_appref));
		ret.push_back(getServiceName());
		ret.push_back("smsgateway");
		ret.push_back(StringUtil::format("%d", getStatus()));
		ret.push_back(getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S"));
		ret.push_back("SMGP1.3");
		ret.push_back(StringUtil::format("%u", m_statPackets.totalSendPackets));
		ret.push_back(StringUtil::format("%u", m_statPackets.totalRecvPackets));
		ret.push_back(StringUtil::format("%u", m_statPackets.lostPackets));

		alarm->send(ServiceIdentifier(), "res module", 0xFF, ret);
	}	
}
bool SMGPProtocol::handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	
	UserTransferMessagePtr resutm = UTM::create(m_myID,utm->getReq(),_ResQuerySendWindowSize);
	
	if(getStatus() != ST_READY)
	{
			resutm->setReturn(1);
			postMessage(source,*resutm);
			return true;
	}
		
	u8array prioritys = (*utm)[_TagParameterName];

	u32array psizes(prioritys.size());
	for(int i=0;i<prioritys.size();i++)
	{
		psizes[i]=m_cacheControl->getCanPutMsgSize(prioritys[i]);
	}
	//utm->setMessageId(_ResQuerySendWindowSize);
	(*resutm)[_TagParameterName] = prioritys;
	(*resutm)[_TagParameterValue] = psizes;
	//source.m_appref=2;//batch  sms gw 
//	resutm->getRes().m_appref=2;
	resutm->setReturn(0);
	postMessage(source,*resutm);
	return true;
	
}
bool SMGPProtocol::transferReport(string caller,string called,string reportcontent)
{
	UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSReport);
	(*utm)[_TagCallee] = string(caller.c_str());
	(*utm)[_TagCalling] =  string(called.c_str());
	(*utm)[_TagUserData] = reportcontent;
	 postMessage(m_smshttpgw,*utm);
	 return true;
}
bool SMGPProtocol::transferDeliver(string caller,string called,string msgcontent)
{
	UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSDeliver);
	(*utm)[_TagCallee] = string(called.c_str());
	(*utm)[_TagCalling] = string(caller.c_str());
	(*utm)[_TagUserData] = msgcontent;
	postMessage(m_smshttpgw,*utm);
	return true;
}
void SMGPProtocol::parseReportMsgContent(const u8array& msgContent,int msglength, std::string& newMsgConent)
{

	string contmsgid = StringUtil::BCD2ASC((const unsigned char*)(&msgContent[3]),20);
	string contentother = string((const char*)(&msgContent[13]),msglength-13-17);
//	string contextlen= string((const char*)(&msgContent[msglength-20]),3);//20  is  textlength   5 is "Text:"
	//TODO：应判断最后一个字符是否为中文，如果是的话则取16位，如果不是则取17位
	//懒下，不处理了
	//string context= string((const char*)(&msgContent[msglength-17]),16);
//	string conttext=   StringUtil::BCD2ASC((const unsigned char*)(&msgContent[textpos]),34);

	newMsgConent.append("Id:");
	newMsgConent.append(contmsgid);
	newMsgConent.append(contentother);
	//newMsgConent.append(context);
	
}