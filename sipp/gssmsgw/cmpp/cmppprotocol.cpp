#include "stdafx.h"


#include "boost/lexical_cast.hpp"
#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"
#include "cmppprotocol.h"
#include "cmppclient.h"
#include "CMPPCacheControl.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW


CMPPProtocol::CMPPProtocol(const ServiceIdentifier& id,
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
	//m_cacheControl = NULL;
	m_clientMo = NULL;
	m_clientMt = NULL;
	m_connectNum = 0;
	setStatus(ST_NONE);
}

CMPPProtocol::~CMPPProtocol()
{
	close();
	if(m_clientMo)
	{
		delete m_clientMo;
		m_clientMo = NULL;
	}
	if(m_clientMt)
	{
		delete m_clientMt;
		m_clientMt = NULL;
	}
}

bool	CMPPProtocol::open()
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

	if(NULL == m_clientMo)
	{
		m_clientMo = new CMPPClientMo( m_myID.m_appref, this);
		m_clientMo->setReconnectInterval(120);
#pragma message("to do ...")
		
	}
	if(NULL == m_clientMt)
	{
		m_clientMt = new CMPPClientMt(m_myID.m_appref, this);
		m_clientMt->setReconnectInterval(120);
	}
	assert(m_clientMt);
	assert(m_clientMo);
	setStatus(ST_INIT);
	m_logger.info("[%02d] cmpp client (0x%x) (%s, %d) init, support long sms(%d)\n",
		m_myID.m_appref, 
		m_version,
		m_serverIP.c_str(), m_serverPort,
		m_longSms);

	m_clientMo->setLocalAddress(m_clientPort,m_clientIP.c_str());
	m_clientMo->m_moPort = m_serverPort;
	bool ret = m_clientMo->start(m_serverPort, m_serverIP.c_str(), false);
	if(!ret)
	{
		setStatus(ST_ERROR);
	}
	m_logger.info("[%02d] cmpp client connect to server %s\n",
		m_myID.m_appref, ret == true ? "ok" : "failed");
	
	m_logger.info("[%02d] cmpp  clientMt (0x%x) (%s, %d)init, support long sms(%d)\n",
		m_myID.m_appref, 
		m_version,
		m_serverIP.c_str(), m_serverPortMt,
		m_longSms);
	m_clientMt->setLocalAddress(m_clientPort,m_clientIP.c_str());
	m_clientMt->m_mtPort = m_serverPortMt;
	bool retMt = m_clientMt->start(m_serverPortMt, m_serverIP.c_str(), false);
	if(!retMt)
	{
		setStatus(ST_ERROR);
	}
	m_logger.info("[%02d] cmpp clientMt connect to server %s\n",
		m_myID.m_appref, retMt == true ? "ok" : "failed");

	return ret;
}

bool	CMPPProtocol::close()
{
	if(getStatus() == ST_READY)
	{
		sendExit();
	}

	setStatus(ST_ERROR);
	return true;
}

bool	CMPPProtocol::activeTest()
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

int	CMPPProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{ 
	switch(utm->getMessageId())
	{
	case _ReqSendSMS:

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


void	CMPPProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
	cacti::IniFile iniFile;
	if(!iniFile.open(m_configFile.c_str()))
		return;

	char key[256] = {0};

	sprintf(key, "client.%d.version", m_myID.m_appref);
	m_version	= (u8) iniFile.readInt("gateway", key, CMPP::Version_20);

	sprintf(key, "client.%d.mo.flowID", m_myID.m_appref);
	m_moSieId=iniFile.readInt("gateway", key,0);

	sprintf(key, "client.%d.report.flowID", m_myID.m_appref);
	m_reportSieId=iniFile.readInt("gateway", key,0);

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

void	CMPPProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
{
	switch(utm->getMessageId())
	{
	case _EvtServerConnected:
		{			
			//m_logger.info("[%02d] client connected \n", m_myID.m_appref);
			m_connectNum = (*utm)[_TagData];
			if (m_connectNum == PortIsMo)
			{
				m_logger.info("[%02d] Mo port client connected,m_connectNum=%d\n", m_myID.m_appref,m_connectNum);
			} 
			else if (m_connectNum == PortIsMt)
			{
				m_logger.info("[%02d] Mt port client connected,m_connectNum=%d\n", m_myID.m_appref,m_connectNum);
			}
			sendLogin();
		}
		break;

	case _EvtServerDisconnected:
		{
			//m_logger.info("[%02d] client disconnected \n", m_myID.m_appref);
			
			u32 connectNum = (*utm)[_TagData];
			//printf("connectNum=%d\n",connectNum);
			if (connectNum == PortIsMo)
			{
				m_logger.info("[%02d] mo port client disconnected\n", m_myID.m_appref);
				//m_clientMo->m_moActiveTest = false;
			} 
			else if (connectNum == PortIsMt)
			{
				m_logger.info("[%02d] mt port client disconnected\n", m_myID.m_appref);
				//m_clientMt->m_mtActiveTest = false;
			}
			setStatus(ST_ERROR);
		}
		break;
	default:
		break;
	}

}

void	CMPPProtocol::handleClientMsg(cacti::Stream& content)
{
	CMPPHeader header(0);
	content.reset();
	if(false == header.fromStream(content))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}
	CMPPPacketPtr packet = CMPPPacketFactory::createPacket(header.command, m_version);
	packet->setHeader(header);
	if(false == packet->fromStream(content, true))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}

	m_statPackets.totalRecvPackets++;

	switch(header.command)
	{
	case CMPP_Login_Resp:
		{
			CMPPLoginResp* pkg = static_cast<CMPPLoginResp*>(packet.get());
			
			if (m_connectNum== PortIsMo)
			{
				m_logger.info("[%02d] [%08x] <- Mo port login resp %d,m_connectNum=%d\n", 
					m_myID.m_appref, pkg->getSequence(), pkg->status,m_connectNum);
			} 
			else if (m_connectNum == PortIsMt)
			{
				m_logger.info("[%02d] [%08x] <- Mt port login resp %d,m_connectNum=%d\n", 
					m_myID.m_appref, pkg->getSequence(), pkg->status,m_connectNum);
			}

			if(pkg->status == CMPP::Status_OK)
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

	case CMPP_Submit_Resp:
		{
			CMPPSubmitResp* pkg = static_cast<CMPPSubmitResp*>(packet.get());
			handleSubmitResp(pkg);
		}
		break;

	case CMPP_Deliver:
		{
			CMPPDeliver* pkg = static_cast<CMPPDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;

	case CMPP_Active_Test:
		{
			sendActiveTestResp(packet->getSequence());
		}
		break;

	case CMPP_Active_Test_Resp:
		{
			m_logger.debug("[%02d] [%08x] <- activeTest resp\n", 
				m_myID.m_appref, packet->getSequence());
		}
		break;

	case CMPP_Exit:
		{
			m_logger.info("[%02d] [%08x] <- exit\n", 
				m_myID.m_appref, packet->getSequence());
		}
		break;

	case CMPP_Exit_Resp:
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

bool CMPPProtocol::sendPackage(CMPPPacket& pkg)
{
	Stream stream;
	stream.reserve(CMPP::Max_Packet_Size);
	if(!pkg.toStream(stream))
	{
		return false;
	}

	bool ret = m_clientMo->sendStream(stream);
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

bool CMPPProtocol::sendMtPackage(CMPPPacket& pkg)
{
	Stream stream;
	stream.reserve(CMPP::Max_Packet_Size);
	if(!pkg.toStream(stream))
	{
		return false;
	}

	bool ret = m_clientMt->sendStream(stream);

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
bool CMPPProtocol::sendPackage(CMPPPacketCachePtr pkg)
{
	bool ret = m_clientMo->sendStream(pkg->content);
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
bool CMPPProtocol::sendSubmitPackage(CMPPSubmit& pkg)
{
	return sendMtPackage(pkg);
}

bool	CMPPProtocol::sendLogin()
{
	//MD5 md5;
	CMPPLogin packet;
	packet.setSequence(m_seqGen.next());

	Timestamp timest =cacti::Timestamp::getNowTime();
	string authenCode;
	char md5[16+1] = {0};
	char fillcha[9+1]={0};
	char msg[256]  = {0};
	char *ptr = msg;
	strncpy(ptr,m_userName.c_str(),m_userName.size());
	ptr+=m_spId.size();
	strncpy(ptr,fillcha,9);
	ptr+=9;
	strncpy(ptr,m_userPassword.c_str(),m_userPassword.size());
	ptr+=m_userPassword.size();
	snprintf(ptr,10,"%10s",timest.getFormattedTime("%m%d%H%M%S").c_str());
	ptr+=10;

	md5digest(md5,msg,ptr-msg);
	authenCode=md5;
	//authenCode=md5.MD5Encode(msg);
	packet.sourceAddr.assign(m_spId.begin(),m_spId.end());
	packet.authenticatorSource.assign(authenCode.begin(),authenCode.end());
	//packet.version=0;
	packet.version = m_version;
	packet.timeStamp=(u32)atoi(timest.getFormattedTime("%m%d%H%M%S").c_str());
	bool ret = false;
	if (m_connectNum == PortIsMo)
	{
		ret = sendPackage(packet);
		m_logger.info("[%02d] [%08x] -> Mo port login (%s, %s), spid(%s), status: %s,%s,%d\n", 
			m_myID.m_appref, packet.getSequence(),
			m_userName.c_str(), m_userPassword.c_str(),
			m_spId.c_str(),
			ret == true ? "ok" : "failed",msg,m_connectNum);
	} 
	else if (m_connectNum == PortIsMt)
	{
		ret = sendMtPackage(packet);
		m_logger.info("[%02d] [%08x] -> Mt port login (%s, %s), spid(%s), status: %s,%s,%d\n", 
			m_myID.m_appref, packet.getSequence(),
			m_userName.c_str(), m_userPassword.c_str(),
			m_spId.c_str(),
			ret == true ? "ok" : "failed",msg,m_connectNum);
	}
	return ret;

}

bool	CMPPProtocol::sendExit()
{
	CMPPExit packet;
	packet.setSequence(m_seqGen.next());

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> exit, status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");

	return ret;	
}

bool	CMPPProtocol::sendActiveTest()
{
	CMPPActiveTest packet;
	packet.setSequence(m_seqGen.next());

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> activeTest status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");
	
	return true;
}

bool	CMPPProtocol::sendActiveTestResp(u32 seq)
{
	CMPPActiveTestResp packet;
	packet.setSequence(seq);
	//bool ret = sendPackage(packet);
	bool ret = false;
	if (m_clientMo->m_moActiveTest == true)
	{
		ret = sendPackage(packet);
		m_clientMo->m_moActiveTest = false;
		m_logger.info("[%02d] [%08x] -> Mo port activeTestResp status: %s\n", 
			m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");
	} 
	else if(m_clientMt->m_mtActiveTest == true)
	{
		ret = sendMtPackage(packet);
		m_clientMt->m_mtActiveTest = false;
		m_logger.info("[%02d] [%08x] -> Mt port activeTestResp status: %s\n", 
			m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");
	}

	return true;
}

int CMPPProtocol::handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	
	return  -1;
}
int	CMPPProtocol::sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	CMPPSubmit packet;
	packet.setSequence(m_seqGen.next());
	
	m_flowSequence = "10000";
	if(!getParameter(m_flowSequence,"FlowSequence",*utm))
	{
		m_flowSequence = "10000";
	}
	char msgId[65]={0};
	u32 stamp=makeTimestamp();
	sprintf(msgId,"%d%d",stamp,packet.getSequence());
	packet.Msg_Id=boost::lexical_cast<u64>(stamp);//信息标识
	packet.Pk_total=1;//相同Msg_Id的信息总条数，从1开始
	packet.Pk_number=1;//相同Msg_Id的信息序号，从1开始

	u8 needReport=Report_Need;
	if(!getParameter(needReport,"NeeReport",*utm))
	{
		needReport=Report_Need;
	}
	packet.Registered_Delivery=needReport;//	是否要求返回状态确认报告

	u8 priority=0;
	if(!getParameter(priority,"Priorty",*utm)){
		priority=0;
	}
	packet.Msg_level=priority;//信息级别


	string serviceCode=m_serviceCode;
	if(!getParameter(serviceCode,"ServiceCode",*utm))
	{
		 serviceCode=m_serviceCode;
	}
	packet.Service_Id.assign(serviceCode.begin(),serviceCode.end());//业务标识

	u8 freeUserType=2;
	if(!getParameter(freeUserType,"FreeUserType",*utm))
	{
		freeUserType=2;
	}
	packet.Fee_UserType=freeUserType;//计费用户类型
	string freeUser="";
	if(getParameter(freeUser,"FreeUser",*utm))
	{
		packet.Fee_terminal_Id.assign(freeUser.begin(),freeUser.end());//被计费用户的号码
	}

	u8 terminalType=0;
	packet.Fee_terminal_type=terminalType;//被计费用户的号码类型


	u8 messageCoding=Format_GB;
	if(!getParameter(needReport,"MsgFormat",*utm))
	{
		messageCoding=Format_GB;
	}

	packet.Msg_Fmt=messageCoding;//信息格式

	packet.Msg_src.assign(m_spId.begin(),m_spId.end());//信息内容来源


	string feeType="01";
	if(!getParameter(feeType,"FeeType",*utm))
	{
		feeType="01";
	}
	packet.FeeType.assign(feeType.begin(),feeType.end());//资费类别

	string feeCode="0";
	if(!getParameter(feeCode,"FeeCode",*utm))
	{
		feeCode="0";
	}
	packet.FeeCode.assign(feeCode.begin(),feeCode.end());//资费

	string calling=(*utm)[_TagCalling];
	packet.Src_Id.assign(calling.begin(),calling.end());//源号码

	string callee=(*utm)[_TagCallee];
	packet.Dest_Terminal_Id.assign(callee.begin(),callee.end());//接收短信的MSISDN号码

	//packet.Dest_terminal_Id.assign(callee.begin(),callee.end());
	//packet.Dest_terminal_type=0;

	string message=(*utm)[_TagMessage];
	//string newMessage;
	//EncodeUcs2(message,newMessage);

	packet.Msg_Content.assign(message.begin(),message.end());//信息内容
	u32 messageSize=(u32)packet.Msg_Content.size();;
	packet.Msg_Length=messageSize;//信息长度
	string linkId="";
	if(!getParameter(linkId,"LinkID",*utm))
	{
		linkId="";
	}
	packet.LinkID.assign(linkId.begin(),linkId.end());//点播业务使用的LinkID，非点播类业务的MT流程不使用该字段

	if(messageSize<=m_splitSize)
	{
		bool ret=sendSubmitPackage(packet);
		m_logger.debug("[%02d] [%08x] cmpp sendSubmit(%s) status:%d\n",m_myID.m_appref,
			packet.getSequence(),packet.toString("|"),ret);
		return ret;
	}
	else
	{
		u8array contentHeader;
		contentHeader.resize(6);
		packet.Msg_Fmt=CMPP::Format_UCS2;
		string newMessage;
		int len=EncodeUcs2(message,newMessage);
		if(len>0)
		{
			message.clear();
			message=newMessage;
		}
		int splitSize = m_splitSize;

		packet.TP_udhi = (u8)0x01;

		contentHeader[0] = 0x05;
		contentHeader[1] = 0x00;
		contentHeader[2] = 0x03;
		contentHeader[3] = 0x08;
		splitSize = m_splitSize - 6;
		int cycleCount = (message.length() / splitSize);
		int lastSize = message.length() % splitSize;
		cycleCount +=  (lastSize > 0) ? 1 : 0;
		packet.Pk_total = (u8)cycleCount;
		contentHeader[4] = (u8)cycleCount;
		string buf;
		int iPos = 0;
		for(int i = 0; i < cycleCount; i++)
		{
		
			packet.Pk_number = (u8)(i+1);
			contentHeader[5] = (u8)(i+1);
			packet.setSequence(m_seqGen.next());

			buf.clear();
			iPos = i * splitSize;
			if( lastSize > 0 && ((i+1) == cycleCount) )
			{
				buf.assign(message.end()-lastSize, message.end());
			}
			else
			{
				buf.assign(message.begin()+iPos, message.begin()+(i+1)*splitSize);
			}
			packet.Msg_Content.assign(contentHeader.begin(),contentHeader.end());
			packet.Msg_Content.insert(packet.Msg_Content.end(),buf.begin(),buf.end());
			packet.Msg_Length=(u32)packet.Msg_Content.size();
			bool ret=sendSubmitPackage(packet);
			m_logger.debug("[%02d] [%08x] cmpp long sms sendSubmit(%s) status:%d\n",m_myID.m_appref,
			packet.getSequence(),packet.toString("|"),ret);
		}

		return 0;
	}
}

bool	CMPPProtocol::handleSubmitResp(CMPPSubmitResp* pkg)
{
	#pragma message("to do ...")
	
	m_logger.info("[%d] handleSubmitResp (%I64x-%d)\n",
		m_myID.m_appref,pkg->Msg_Id,pkg->Result);
	char msgId[17];
	snprintf(msgId,17,"%I64x",pkg->Msg_Id);
	addFlowSequence(msgId,m_flowSequence);
	m_logger.debug("[%02d] insert sequence(%s)---(%s)\n",
		m_myID.m_appref,msgId,m_flowSequence.c_str());
	return true;
}

bool	CMPPProtocol::sendDeliverResp(u64 msgId,u32 status,u32 seq)
{
	CMPPDeliverResp resp;
	resp.setSequence(seq);
	resp.Msg_Id=msgId;
	resp.Result=status;
	m_logger.info("[%d] sendDeliverResp (%I64x)\n",m_myID.m_appref,resp.Msg_Id);
	return sendPackage(resp);
}

bool	CMPPProtocol::handleDeliver(CMPPDeliver* pkg)
{
	string userPhone;
	string content;
	string linkId;
	string spNo;
	//string msgId;
	string stat;
	u32 seq = pkg->getSequence();
	linkId.assign(pkg->LinkID.begin(),pkg->LinkID.end());
	if(pkg->isReport())
	{
		stat.assign(pkg->report.Stat.begin(),pkg->report.Stat.end());
		userPhone.assign(pkg->report.Dest_terminal_Id.begin(),pkg->report.Dest_terminal_Id.end());
		m_logger.info("[%d]handleReport(%I64x:%I64x-%s-%s-%s)\n",
			m_myID.m_appref,pkg->Msg_Id,pkg->report.Msg_Id,userPhone.c_str(),stat.c_str(),linkId.c_str());
		
		//sendDeliverResp(pkg->report.Msg_Id,0,seq);
		sendDeliverResp(pkg->Msg_Id,0,seq);

		ServiceIdentifier res(0,0,0);
		UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);
		(*utm)[_TagGatewayName]=m_serviceName.c_str();

		(*utm)[_TagCallee]  =  userPhone.c_str();
		(*utm)[_TagMessage] = stat.c_str();

		(*utm)[_TagOriCalling] =(u32)pkg->report.Msg_Id;
		(*utm)[_TagServiceId] = "report";
		(*utm)[_TagFlowId] =m_reportSieId;
		char msgId[17];
		snprintf(msgId,17,"%I64x",pkg->report.Msg_Id);
		string flowSequence = getFlowSequence(msgId);
		(*utm)[_TagSequenceNumber] = flowSequence;
		postMessage(m_myID, *utm);
		return true;
	}
	else
	{
		//linkId.assign(pkg->LinkID.begin(),pkg->LinkID.end());
		spNo.assign(pkg->Dest_Id.begin(),pkg->Dest_Id.end());
		content.assign(pkg->Msg_Content.begin(),pkg->Msg_Content.end());
		//msgId.assign(pkg->Msg_Id.begin(),pkg->Msg_Id.end());
		userPhone.assign(pkg->Src_terminal_Id.begin(),pkg->Src_terminal_Id.end());
		m_logger.info("[%d] handleDeliver (%s-%s-%s-%s-%I64x)\n",
			m_myID.m_appref,userPhone.c_str(),spNo.c_str(),content.c_str(),linkId.c_str(),pkg->Msg_Id);
		
		sendDeliverResp(pkg->Msg_Id,0,seq);

		string gwname=spNo.substr(0,8);
		
		ServiceIdentifier res(0,0,0);
		UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);


		(*utm)[_TagGatewayName]=m_serviceName.c_str();
		(*utm)[_TagCalling] =userPhone.c_str();
		(*utm)[_TagCallee]  =  spNo.c_str();
		(*utm)[_TagMessage] = content.c_str();

		(*utm)[_TagOriCalling] = (u32)pkg->Msg_Id;
		(*utm)[_TagOriCallee] = linkId.c_str();
		(*utm)[_TagServiceId] = (u8)0;
		(*utm)[_TagFlowId] =m_moSieId;
		(*utm)[_TagLinkId] = linkId;
		postMessage(m_myID, *utm);

	}
	m_logger.debug("handler deliver post to client %s\n",spNo.c_str());
	return true;
}
int CMPPProtocol::respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,CMPPSubmitResp& pkg)
{
	utm->setMessageId(_ResSMSSendBatch);
	utm->setRes(utm->getReq());
	utm->setReq(m_myID);

	utm->setReturn(ret);



	return CMPP::Status_OK;
}
int	CMPPProtocol::respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret)
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

	return CMPP::Status_OK;
}
/*
void CMPPProtocol::runSubmitSend()
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
void CMPPProtocol::snapshot()
{
	m_logger.info("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
		m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
		m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);

	Service::printConsole("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
		m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
		m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);
}

void CMPPProtocol::handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
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
		ret.push_back("CMPP1.3");
		ret.push_back(StringUtil::format("%u", m_statPackets.totalSendPackets));
		ret.push_back(StringUtil::format("%u", m_statPackets.totalRecvPackets));
		ret.push_back(StringUtil::format("%u", m_statPackets.lostPackets));

		alarm->send(ServiceIdentifier(), "res module", 0xFF, ret);
	}	
}
bool CMPPProtocol::handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	
	
	return true;
	
}
bool CMPPProtocol::transferReport(string caller,string called,string reportcontent)
{
	UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSReport);
	(*utm)[_TagCallee] = string(caller.c_str());
	(*utm)[_TagCalling] =  string(called.c_str());
	(*utm)[_TagUserData] = reportcontent;
	 postMessage(m_smshttpgw,*utm);
	 return true;
}
bool CMPPProtocol::transferDeliver(string caller,string called,string msgcontent)
{
	UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSDeliver);
	(*utm)[_TagCallee] = string(called.c_str());
	(*utm)[_TagCalling] = string(caller.c_str());
	(*utm)[_TagUserData] = msgcontent;
	postMessage(m_smshttpgw,*utm);
	return true;
}
void CMPPProtocol::parseReportMsgContent(const u8array& msgContent,int msglength, std::string& newMsgConent)
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