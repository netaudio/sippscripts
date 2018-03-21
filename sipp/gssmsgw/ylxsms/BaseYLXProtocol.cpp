#include "stdafx.h"
#include <boost/lexical_cast.hpp>
#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"

#include "baseylxProtocol.h"


#include "ylxprotocol.h"
#include "ylx.h"
#include "ylxserver.h"

class YLXServer;

#include "ylxCacheControl.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW



BaseYLXProtocol::BaseYLXProtocol(const ServiceIdentifier& id,
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseProtocol(id, configFile, handler)
,m_logger(Logger::getInstance("msggw"))
,m_msgLogger(Logger::getInstance("sigp"))
,m_cacheList(8192, 60)
,m_cycleTimes(0)
,m_longSms(0)
,m_splitSize(140),m_tested(false),m_seqGen(1,999999)
,m_maxSendWindowSize(100)
{
		
	setStatus(ST_NONE);
}

BaseYLXProtocol::~BaseYLXProtocol()
{
	
}


bool	BaseYLXProtocol::open()
{

	return true;
}


bool	BaseYLXProtocol::close()
{
	if(getStatus() == ST_READY)
	{
		sendExit();
	}

	setStatus(ST_ERROR);
	return true;
}

bool	BaseYLXProtocol::activeTest()
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



int	BaseYLXProtocol::procResport(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	BYTE buf[2000];
	char sn[100];
	char mtsn[100];
	YLXReport pkg;
	string errorDesc;
	string results = (*utm)[_TagResult];
	
	pkg.state = atoi(results.c_str());

 //   sprintf(sn,"%s",pkg.getPacketSequence());  //(*utm)[_TagSeqNo];
	//sprintf(mtsn,"%s",pkg.getSubmitSequenceNumber());  //_(*utm)[_TagSequenceNumber];

    sprintf(sn,"%s",(*utm)[_TagSeqNo]);  //(*utm)[_TagSeqNo];pkg.getPacketSequence
	sprintf(mtsn,"%s",(*utm)[_TagSequenceNumber]);  //_(*utm)[_TagSequenceNumber];pkg.getSubmitSequenceNumber()


	int len = MakeSubmitRespPacket(buf,pkg.state,(BYTE*)sn,(BYTE *)errorDesc.c_str(),(BYTE*)mtsn);

	sendPackage((char *)&buf,len);

	//bool ret = m_server->sendStream(pkg->content);
	//if(ret)
	//{
	//	m_statPackets.totalSendPackets ++;
	//}
	//else
	//{
	//	m_statPackets.sendErrorPackets ++;
	//}
	//return ret;


	return true;
}


int	BaseYLXProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
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
	case _ReqDispatchSMSReport:

		if(getStatus() != ST_READY)
		{
			respMessage(source, utm, -1);
			return -1;
		}

		return procResport(source, utm);

	//case _ReqQuerySendWindowSize:
	//	return handleQuerySendWindowSize(source,utm);
	//case _ReqSMSSendBatch:
	//	return handleSendSubmit(source, utm);
	//case _ReqGWService:
	//	break;

	default:
		m_logger.error("sendMessage msgid=[%02d] error, nothing to do\n", utm->getMessageId());
		break;
	}
	return -1;
}


void	BaseYLXProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
	//cacti::IniFile iniFile;
	//if(!iniFile.open(m_configFile.c_str()))
	//	return;
}

void	BaseYLXProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
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

void	BaseYLXProtocol::handleClientMsg(cacti::Stream& content)
{
	YLXHeader header(0);
	content.reset();
	if(false == header.fromStream(content))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}

	YLXPacketPtr packet = YLXPacketFactory::createPacket(header.command);
	packet->setHeader(header);
	if(false == packet->fromStream(content, true))
	{
		m_statPackets.recvErrorPackets++;
		return;
	}

	m_statPackets.totalRecvPackets++;

	switch(header.command)
	{
//	case YLX_Login:
//		{
//			string loginName;
//			string pwd;
//			YLXLogin* pkg = static_cast<YLXLogin*>(packet.get());
//			loginName.assign(pkg->loginName.begin(),pkg->loginName.end());
//			pwd.assign(pkg->password.begin(),pkg->password.end());
//			m_logger.info("[%02d] [%s] <- login loginName:%s,logtype:%d,pwd:%s\n", 
//				m_myID.m_appref, pkg->getPacketSequence(), loginName.c_str(),pkg->loginType,pwd.c_str());
//			sendLoginResp(pkg->getSouceCode(),pkg->getSendDate(),pkg->getSequence());
//		}
//		break;
//	case YLX_Login_Resp:
//		{
//			YLXLoginResp* pkg = static_cast<YLXLoginResp*>(packet.get());
//			m_logger.info("[%02d] [%s] <- login resp %d\n", 
//				m_myID.m_appref,pkg->getPacketSequence(), pkg->status);
//
//			if(pkg->status == YLX::Status_OK)
//			{
//				setStatus(ST_READY);
//#ifdef _TEST
//				if(!m_tested){
//				
//					//test(2);
//				}
//				
//#endif	
//			}
//			else
//				setStatus(ST_ERROR);
//		}
//		break;
//
	case YLX_Stat:
		{
			handleStat(*packet);
		}
		break;
	case YLX_Submit:
		{
			handleSubmit(content);
		}
		break;
	case YLX_Submit_Resp:
		{
			YLXSubmitResp* pkg = static_cast<YLXSubmitResp*>(packet.get());
			handleSubmitResp(pkg);
		}
		break;

	case YLX_Deliver:
		{
			YLXDeliver* pkg = static_cast<YLXDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;
	case YLX_Report:
		{
			YLXReport* pkg = static_cast<YLXReport*>(packet.get());
			handleReport(pkg);
		}
		break;

	case YLX_Active_Test:
		{
			sendActiveTestResp(packet->getSequence());
		}
		break;

	case YLX_Active_Test_Resp:
		{
			m_logger.debug("[%02d] [%s] <- activeTest resp\n", 
				m_myID.m_appref,packet->getPacketSequence());
		}
		break;

	//case YLX_Exit:
	//	{
	//		m_logger.info("[%02d] [%s] <- exit\n", 
	//			m_myID.m_appref, packet->getPacketSequence());
	//	}
	//	break;

	//case YLX_Exit_Resp:
	//	{
	//		m_logger.info("[%02d] [%s] <- recv exit resp\n", 
	//			m_myID.m_appref, packet->getPacketSequence());
	//	}
	//	break;

	default:
		m_statPackets.recvErrorPackets++;

		m_logger.info("[%02d] [%s] <- unknown cmd %d\n", 
			m_myID.m_appref, packet->getPacketSequence(), header.command);
		break;
	}

}

bool BaseYLXProtocol::sendPackage(YLXPacket& pkg)
{
	return true;
}


bool BaseYLXProtocol::sendPackage(YLXPacketCachePtr pkg)
{
	return true;
}


bool BaseYLXProtocol::sendPackage(char *buf,int len )
{
	return true;
	//return 	sendPackagebuf((char *)&buf,len);
}


bool BaseYLXProtocol::sendSubmitPackage(YLXSubmit& pkg)
{
	return sendPackage(pkg);
}


bool	BaseYLXProtocol::sendLogin()
{
	//YLXLogin packet;
	//packet.setSourceCode(getSourceId());
	////packet.getHeader().sourceCode=1234;
	////packet.getHeader().sendDate=GetTimeFormat(time(NULL),"%m%d%H%M%S");
	//packet.setSequence(m_seqGen.next());

	//packet.loginName.assign(m_userName.begin(), m_userName.end());
	//packet.password.assign(m_userPassword.begin(),m_userPassword.end());
	//
	//bool ret = sendPackage(packet);
	//m_logger.info("[%02d] [%s] -> login (%s, %s), spid(%s), status: %s\n", 
	//	m_myID.m_appref, packet.getPacketSequence(), 
	//	m_userName.c_str(), m_userPassword.c_str(),
	//	m_spId.c_str(),
	//	ret == true ? "ok" : "failed");

	return 1;

}

bool	BaseYLXProtocol::sendLoginResp(u32 sourceCode,u32 sendDate,u32 sequence)
{
	//YLXLoginResp packet;
	//packet.setSourceCode(getSourceId());
	//packet.getHeader().sourceCode=sourceCode;
	//packet.getHeader().sendDate=sendDate;
	//packet.setSequence(sequence);

	//packet.status=0;

	//bool ret = sendPackage(packet);
	//m_logger.info("[%02d] [%s] -> login_resp (%s, %s), spid(%s), status: %s\n", 
	//	m_myID.m_appref, packet.getPacketSequence(), 
	//	m_userName.c_str(), m_userPassword.c_str(),
	//	m_spId.c_str(),
	//	ret == true ? "ok" : "failed");

	return 1;

}

bool	BaseYLXProtocol::sendExit()
{
	//YLXExit packet;
	//packet.setSourceCode(getSourceId());
	//packet.setSequence(m_seqGen.next());

	//bool ret = sendPackage(packet);
	//m_logger.info("[%02d] [%s] -> exit, status: %s\n", 
	//	m_myID.m_appref, packet.getPacketSequence(), ret == true ? "ok" : "failed");

	return 0;	
}

bool	BaseYLXProtocol::sendExitResp(u32 sourceCode,u32 sendDate,u32 sequence)
{
	//YLXExitResp packet;
	//packet.setSourceCode(getSourceId());
	//packet.setSequence(m_seqGen.next());

	//bool ret = sendPackage(packet);
	//m_logger.info("[%02d] [%s] -> exit, status: %s\n", 
	//	m_myID.m_appref, packet.getPacketSequence(), ret == true ? "ok" : "failed");

	return 1;	
}

bool	BaseYLXProtocol::sendActiveTest()
{
	/*
	YLXActiveTest packet;
	packet.setSequence(m_seqGen.next());

	bool ret = sendPackage(packet);
	m_logger.info("[%02d] [%08x] -> activeTest status: %s\n", 
		m_myID.m_appref, packet.getSequence(), ret == true ? "ok" : "failed");
	*/
	return true;
}

bool	BaseYLXProtocol::sendActiveTestResp(u32 seq)
{
	//YLXActiveTestResp packet;
	//packet.setSourceCode(getSourceId());
	//packet.setSequence(seq);
	//
	//bool ret = sendPackage(packet);
	//m_logger.info("[%02d] [%s] -> activeTestResp status: %s\n", 
	//	m_myID.m_appref, packet.getPacketSequence(), ret == true ? "ok" : "failed");

	return true;
}

int BaseYLXProtocol::handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	//YLXSubmit packet;

	
	return   -1;
}


int	BaseYLXProtocol::sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{	
//	YLXSubmit packet;
//	packet.setSequence(m_seqGen.next());	
//	//packet.setSourceCode(m_myID.m_appref);	
//	packet.setSourceCode(getSourceId());
//	string flowSequence;
//	if(getParameter(flowSequence,"FlowSequence",*utm)){
//		string pkgSequence = packet.getPacketSequence();
//		addFlowSequence(pkgSequence,flowSequence);
//		m_logger.debug("[%02d] insert sequence(%s)---(%s)\n",
//			m_myID.m_appref,pkgSequence.c_str(),flowSequence.c_str());
//	}
//
//	string calling=(*utm)[_TagCalling];
//	string callee=(*utm)[_TagCallee];
//	packet.spNumber.assign(calling.begin(),calling.end());
//
//	string chargeNumber;
//	if(!getParameter(chargeNumber,"ChargeTermID",*utm)){
//		chargeNumber="000000000000000000000";
//	}
//	packet.chargeNumber.assign(chargeNumber.begin(),chargeNumber.end());	
//	packet.userNumber.assign(callee.begin(),callee.end());
//	packet.corpId.assign(m_spId.begin(),m_spId.end());
//	packet.serviceType.assign(m_serviceCode.begin(),m_serviceCode.end());
//
//	u8 freeType=2;
//	if(!getParameter(freeType,"FreeType",*utm)){
//		freeType=2;
//	}
//	packet.feeType=freeType;
//
//	string freeValue="0";
//	if(!getParameter(freeValue,"FreeCode",*utm)){
//		freeValue="0";
//	}
//	packet.feeValue.assign(freeValue.begin(),freeValue.end());
//
//	string givenValue="0";
//	if(!getParameter(givenValue,"GivenValue",*utm)){
//		givenValue="0";
//	}
//	packet.givenValue.assign(givenValue.begin(),givenValue.end());
//
//
//	u8 agentFlag=0;
//	if(getParameter(agentFlag,"AgentFlag",*utm)){
//		packet.agentFlag=agentFlag;
//	}
//
//	u8 mtFlag=MO_Order;
//	if(!getParameter(mtFlag,"SubmitMsgType",*utm)){
//		mtFlag=MO_Order;
//	}
//	packet.morelatetoMTFlag=mtFlag;
//
//
//	u8 priority=0;
//	if(!getParameter(priority,"Priorty",*utm)){
//		priority=0;
//	}
//	packet.priority=priority;
//
//	//string expireTime="1212210032032+";
//	//packet.expireTime.assign(expireTime.begin(),expireTime.end());
//
//	//string scheduleTime="1212210032032+";//(*utm)[_TagScheduleTime];
//	//packet.scheduleTime.assign(scheduleTime.begin(),scheduleTime.end());
//
//	u8 needReport=Report_Need;
//	if(!getParameter(needReport,"NeeReport",*utm)){
//		needReport=Report_Need;
//	}
//	packet.reportFlag=needReport;
//
//	u8 messageCoding=Format_UCS2;
//	if(!getParameter(messageCoding,"MsgFormat",*utm)){
//		messageCoding=Format_UCS2;
//	}
//	packet.messageCoding=messageCoding;
//
//
//	string message=(*utm)[_TagMessage];
//	string newMessage;
//	EncodeUcs2(message,newMessage);
//
//	packet.messageContent.assign(newMessage.begin(),newMessage.end());
//	packet.messageLength=(u32)packet.messageContent.size();
//
//
//
//	string linkId="";
//	if(!getParameter(linkId,"LinkID",*utm)){
//		linkId="";
//	}
//
//	packet.reserve.assign(linkId.begin(),linkId.end());
//	bool ret=sendSubmitPackage(packet);
//
//	m_logger.debug("[%02d] [%s] sigp sendSubmit(%s) status:%d\n",m_myID.m_appref,
//	packet.getPacketSequence(),packet.toString("|"),ret);
//
	return 1;
}


bool	BaseYLXProtocol::handleStat(YLXPacket pkg)
{
	return sendPackage(pkg);
}



int	BaseYLXProtocol::MakeSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
//	YSMSSubmit packet;
//
//	//移动CMPP，电信SMGP，联通SGIP，网通SNGP
//	if( !getParameter(packet.Msg_Kind, "Msg_Kind", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, Msg_Kind is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	if(m_spId.length() > 6)
//	{
//		m_logger.info("[%02d] submit failed, length of spId > 6\n", m_myID.m_appref);
//		return -1;
//	}
//	char acSeq[11] = {0};
//
//	std::string LineNo = "1024" + m_spId;
//	sprintf(acSeq, "%010d", atoi(LineNo.c_str()));
//	LineNo = acSeq;
//
//	LineNo += Timestamp::getNowTime().getFormattedTime("%Y%m%d%H%M%S");
//	
//	sprintf(acSeq, "%010d", m_seqGen.next());
//	LineNo += acSeq;
//	packet.LineNo.assign(LineNo.begin(), LineNo.end());
//	
//	packet.Pk_total = 1;
//	packet.Pk_number = 1;
//
//	//计费用户类型字段，默认为0
//	//0：对目的终端MSISDN计费；
//	//1：对源终端MSISDN计费；
//	//2：对SP计费;
//	//3：表示本字段无效，对谁计费参见Fee_terminal_Id字段。
//	if( !getParameter(packet.Fee_UserType, "Fee_UserType", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, Fee_UserType is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//业务代码
//	string serviceCode = (*utm)[_TagServiceCode];
//	if("" == serviceCode)
//	{
//		serviceCode = m_serviceCode;
//	}
//	packet.Service_Code.assign(serviceCode.begin(), serviceCode.end());
//
//	//计费类型
//	if( !getParameter(packet.FeeType, "FeeType", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, FeeType is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//费用代码
//	if( !getParameter(packet.FeeCode, "FeeCode", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, FeeCode is empty\n", m_myID.m_appref);
//		return -1;
//	}
//	
//	//包月费/封顶费（联通无该字段）
//	if( !getParameter(packet.FixedFee, "FixedFee", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, FixedFee is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//赠送费用（联通才有该字段）
//	if( !getParameter(packet.GivenValue, "GivenValue", *utm) )
//	{
//		m_logger.info("[%02d] submit warning, GivenValue is empty\n", m_myID.m_appref);
//	}
//
//	//收费用户号码
//	if( !getParameter(packet.Fee_terminal_Id, "Fee_terminal_Id", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, Fee_terminal_Id is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//短消息格式
//	packet.Msg_Fmt = YSMS::Format_GB;
//
//	//发送号码
//	if( !getParameter(packet.Src_Id, "Src_Id", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, Src_Id is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//接收号码
//	if( !getParameter(packet.Dest_Id, "Dest_Id", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, Dest_Id is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	packet.Dest_Id_type = 0;
//
//	//短消息内容
//	string msgContent = (*utm)[_TagMessage];//"test ok!";
//	if(msgContent.length() <= 0)
//	{
//		m_logger.info("[%02d] submit failed, Msg_Content is empty\n", m_myID.m_appref);
//		return -1;
//	}
//	else
//	{
//		m_logger.debug("[%02d] submit Msg_Content :%s\n",m_myID.m_appref,msgContent.c_str());
//	}
//
//	
//	//上行信息的LINKID，无上行不填
//	if( !getParameter(packet.LinkID, "LinkID", *utm) )
//	{
//		m_logger.info("[%02d] submit warning, LinkID is empty\n", m_myID.m_appref);
//	}
//
//	//短信类型
//	if( !getParameter(packet.MsgType, "MsgType", *utm) )
//	{
//		m_logger.info("[%02d] submit failed, MsgType is empty\n", m_myID.m_appref);
//		return -1;
//	}
//
//	//SpCode
//	if( !getParameter(packet.SpCode, "SpCode", *utm) )
//	{
//		m_logger.info("[%02d] submit warning, SpCode is empty\n", m_myID.m_appref);
//	}
//
//	//ServiceID
//	if( !getParameter(packet.ServiceID, "ServiceID", *utm) )
//	{
//		m_logger.info("[%02d] submit warning, ServiceID is empty\n", m_myID.m_appref);
//
//	}
//
//	//[21]短信息发送方号码
////	string Src_Id = (*utm)[_TagCalling];
////	packet.Src_Id.assign(Src_Id.begin(), Src_Id.end());
//
//	string Src_Id;
//	Src_Id.assign(packet.Src_Id.begin(), packet.Src_Id.end());
//	StringUtil::trim(Src_Id);
//
//
//	//[21*DestTermCount]短消息接收号码
////	string Dest_Id = (*utm)[_TagCallee];
////	packet.Dest_Id.assign(Dest_Id.begin(), Dest_Id.end());
//
//	string Dest_Id;
//	Dest_Id.assign(packet.Dest_Id.begin(), packet.Dest_Id.end());
//	StringUtil::trim(Dest_Id);
//
//	bool bRet = false;
//	string buf;
//
//	if(msgContent.length() <= YSMS::Max_Message_Size)
//	{
//		packet.Msg_Content.assign(msgContent.begin(), msgContent.end());
//
//		bRet = m_server->sendPackage(packet);
//		
//		buf.assign(msgContent.begin(), msgContent.end());
//		cacti::StringUtil::replace(buf, "\n", " ");
//		m_msgLogger.info("^^%02d^^MT^^%d^^%s^^%s^^%s^^%s\n",
//			m_myID.m_appref,
//			(bRet == true ? 1 : 0),
//			LineNo.c_str(),
//			Src_Id.c_str(),
//			Dest_Id.c_str(),
//			msgContent.c_str());
//	}
//	else
//	{
//		//ceil(msgContent.length() / YSMS::Max_Message_Size); //c func
//		int cycleCount = (msgContent.length() / YSMS::Max_Message_Size);//拆词 段落数
//		int lastSize = msgContent.length() % YSMS::Max_Message_Size;
//		cycleCount +=  (lastSize > 0) ? 1 : 0;
//
//		packet.Pk_total = (u8)cycleCount;
//
//		int iPos = 0;
//		int prePos = 0;
//		bool needSended;
//		for(int i = 0; i < cycleCount; i++)
//		{
//			needSended =false;
//			packet.Pk_number = (u8)(i+1);			
//
//			iPos = i * YSMS::Max_Message_Size + prePos;
//			//改变LineNo ??
//			if( lastSize > 0 && ((i+1) == cycleCount) )
//			{
//				if (lastSize - prePos >0)
//				{					
//					packet.Msg_Content.assign(msgContent.end()-lastSize+prePos , msgContent.end());
//					buf.assign(msgContent.end()-lastSize+prePos, msgContent.end());
//					needSended = true;
//				}
//				
//			}
//			else
//			{		
//
//				int iEnd = (i+1)*YSMS::Max_Message_Size;
//
//				char little[10] ;
//				string sMsg = "";
//				for (int j=0;j<YSMS::Max_Message_Size;)
//				{
//					if (iPos+j < msgContent.length())
//					{
//						int tmpPos= iPos+j ;
//						memset(little,0,sizeof(little));
//						strncpy(little,msgContent.c_str()+tmpPos,1);
//						//判断是否双字节字符
//						if(little[0]<0)
//						{
//							strncpy(little,msgContent.c_str()+tmpPos,2);			
//							j = j+2;
//							if (j==YSMS::Max_Message_Size+1)
//							{
//								prePos ++;	//遇双字节字符，往前进一个BYTE，
//							}
//						}else{
//							j = j+1;
//						}
//						sMsg = sMsg + little;
//					}else
//						break;
//
//				}	
//				
//				packet.Msg_Content.assign(sMsg.begin(), sMsg.end());
//				buf.assign(sMsg.begin(), sMsg.end());
//				needSended =true;
//				//packet.Msg_Content.assign(msgContent.begin()+iPos, msgContent.begin()+iEnd);
//				//buf.assign(msgContent.begin()+iPos, msgContent.begin()+iEnd);
//			}
//
//			if (needSended)
//			{
//				/*	debug break */
//				bRet = m_server->sendPackage(packet); //only judge last send
//				
//				cacti::StringUtil::replace(buf, "\n", " ");
//				m_msgLogger.info("^^%02d^^MT^^%d^^%s^^%s^^%s^^%s\n",
//					m_myID.m_appref,
//					(bRet == true ? 1 : 0),
//					LineNo.c_str(),
//					Src_Id.c_str(),
//					Dest_Id.c_str(),
//					buf.c_str());
//			}
//			
//		}
//	}
//
//	if(bRet == true)
//	{
//		CachedPacketPtr pkg = CachedPacketPtr(new CachedPacket());
//		pkg->source = source;
//		pkg->exchangeID = LineNo;
//		pkg->utm = utm;
//		pkg->packetType = 0;
//		//m_logger.debug("[%02d] cache Package (%s)\n",m_myID.m_appref,LineNo.c_str());
//		m_cacheList.addPacket(LineNo.c_str(), pkg);
//
//		CachedPacket* cachePkg = m_cacheList.getPacket(LineNo.c_str());
//		//m_logger.debug("[%02d] try get cache Package (%s)\n",m_myID.m_appref,cachePkg->exchangeID.c_str());
//	}

	bool bRet = true;
	return bRet == true ? -2 : -1;
}


bool	BaseYLXProtocol::handleSubmit(cacti::Stream& content)
{
	char *pbuf = content.getWriteBuffer();
	S_Msg msg = ParseCMDID_Submit((BYTE *)pbuf);

	BYTE respbuf[2048];
	BYTE errdesc[] = "I don't known!";
	BYTE mtsn[] = "12345678";



	int len = MakeSubmitRespPacket(respbuf, 0, msg.CMDID_Submit.SerialID, errdesc, mtsn);
	sendPackage((char *)respbuf, len);

	u8array messagecontent;

	if(msg.CMDID_Submit.MsgContentLength > 0){
		messagecontent.resize(msg.CMDID_Submit.MsgContentLength);
		for(int i = 0;i<msg.CMDID_Submit.MsgContentLength;i++)
			messagecontent[i] = msg.CMDID_Submit.MsgContent[i];
	}else{
		return false;
	}

	dispatch_sms(m_myID,msg);
	//dispatch_sms(m_myID, (const char *)msg.CMDID_Submit.Callee, (const char *)msg.CMDID_Submit.Originnumber, messagecontent);

	return true;

}



bool	BaseYLXProtocol::handleSubmitResp(YLXSubmitResp* pkg)
{
#pragma message("to do ...")
	

	
	//m_logger.debug("[%02d] [%s] <- YLX submit resp %d\n", 
	//m_myID.m_appref, pkg->getPacketSequence(), pkg->status);
	
	//CachedPacket* cachePkg = m_cacheList.getPacket(pkg->getSequence());
	//if(cachePkg != NULL)
	//{
	//	m_cacheList.deletePacket(pkg->getSequence());
	//}
	//}


	return true;
}

bool	BaseYLXProtocol::sendDeliverResp(u32 souceCode,u32 sendDate,u32 seq, u32 status)
{
	//YLXDeliverResp pkg;
	//pkg.setSourceCode(souceCode);
	//pkg.setSendDate(sendDate);
	//pkg.setSequence(seq);
	//pkg.status = status;

	//return sendPackage(pkg);
	return true;
}

bool	BaseYLXProtocol::sendReportResp(u32 souceCode,u32 sendDate,u32 seq, u32 status)
{
	//YLXDeliverResp pkg;
	//pkg.setSourceCode(souceCode);
	//pkg.setSendDate(sendDate);
	//pkg.setSequence(seq);
	//pkg.status = status;

	//return sendPackage(pkg);
	return true;
}


void	BaseYLXProtocol::dispatch_sms(ServiceIdentifier id,S_Msg msg)
{
	char buf[100];
	UserTransferMessagePtr utm = UTM::create(id,id, _ReqDispatchSMS);
	(*utm)[_TagGatewayName]=m_exName;			
	(*utm)[_TagCalling] = (char *)msg.CMDID_Submit.Originnumber;
	(*utm)[_TagCallee] = (char *)msg.CMDID_Submit.Callee;

	(*utm)[_TagMessage] = (char *)msg.CMDID_Submit.MsgContent;
	std::vector<std::string> m_paramName;
	std::vector<std::string> m_paramValues;
	m_paramName.push_back("Protocol");
	m_paramValues.push_back("YLX");
	m_paramName.push_back("SubmitMsgType");
	m_paramValues.push_back("8");
	m_paramName.push_back("MsgFormat");

	sprintf(buf,"%d",msg.CMDID_Submit.Msgcoding);
	m_paramValues.push_back(buf);

	m_paramName.push_back("SPDealResult");
	m_paramValues.push_back("0");
	m_paramName.push_back("SPDealResult");
	m_paramValues.push_back("0");
	(*utm)[_TagParameterName] = m_paramName;
	(*utm)[_TagParameterValue] = m_paramValues;

	postMessage(id,*utm);
}

void BaseYLXProtocol::dispatch_sms(ServiceIdentifier id,const char *userPhone,const char* caller,u8array message)
{
	UserTransferMessagePtr utm = UTM::create(id,id, _ReqDispatchSMS);
	(*utm)[_TagGatewayName]=m_exName;			
			(*utm)[_TagCalling] = caller;
			(*utm)[_TagCallee] = userPhone;
	
			(*utm)[_TagMessage] = message;

			std::vector<std::string> m_paramName;
			std::vector<std::string> m_paramValues;

			m_paramName.push_back("Protocol");
			m_paramValues.push_back("YLX");
			m_paramName.push_back("SubmitMsgType");
			m_paramValues.push_back("8");
			m_paramName.push_back("MsgFormat");
			m_paramValues.push_back("15");
			m_paramName.push_back("SPDealResult");
			m_paramValues.push_back("0");
			
/*		
			if(i % 3 == 0)
			{
				m_paramName.push_back("groupSms");
				m_paramValues.push_back("15");
			}
			*/

			(*utm)[_TagParameterName] = m_paramName;
			(*utm)[_TagParameterValue] = m_paramValues;

			ServiceIdentifier si;
			si.m_appid=2;
			si.m_appport=70;
		postMessage(id,*utm);
//	postToOwner(id,utm);

}


bool	BaseYLXProtocol::handleDeliver(YLXDeliver* pkg)
{
	//
	//ServiceIdentifier res(0,0,0);
	//u8array cc;
	//pkg->toArray(cc);
	//BYTE buf[2000];

	//int len = MakeRecvSms(buf, 
	//	m_myID.m_appref,
	//	pkg->getPacketSequence(), 
	//	userPhone.c_str(),
	//	newContent.c_str(),
	//	linkId.c_str(),
	//	spNo.c_str(),
	//	gwname.c_str(),
	//	pkg->messageCoding);

	//SendPacket((char *)buf,len);

	//UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);	
	//(*utm)[_TagGatewayName]=gwname.c_str();
	//(*utm)[_TagCalling] =userPhone.c_str();
	//(*utm)[_TagCallee]  =  spNo.c_str();
	//(*utm)[_TagMessage] = newContent.c_str();
	//(*utm)[_TagLinkId] = linkId;
	//postMessage(m_myID, *utm);
	//m_logger.debug("[%02d] handler deliver post to client %s\n",m_myID.m_appref,spNo.c_str());
	return true;
}

bool	BaseYLXProtocol::handleReport(YLXReport* pkg)
{
	
	//string userPhone;
	//
	//userPhone.assign(pkg->userNumber.begin(),pkg->userNumber.end());

	//m_logger.debug("[%02d][%s][%s] handleReport (%s-%d-%d)\n",
	//	m_myID.m_appref,
	//	pkg->getPacketSequence(),pkg->getSubmitSequenceNumber(),userPhone.c_str(),pkg->state,
	//	pkg->errorCode);
	//sendReportResp(pkg->getSouceCode(),pkg->getSendDate(),pkg->getSequence(),0);

	////if (strcmp(userPhone.c_str(),"8600000000000") == 0 && pkg->errorCode == 23)
	////if(getTestNum() == userPhone)
	////if (strcmp(userPhone.c_str(),getTestNum().c_str()) == 0 )
	//if (strcmp(userPhone.c_str(),"8600000000000") == 0)
	//{
	//	return true;
	//}
	//BYTE buf[2000];
	//char s[10];
	//itoa(pkg->errorCode,s,10);
	//int len = MakeSubmitRespPacket(buf,pkg->errorCode,(BYTE*)pkg->getPacketSequence(),(BYTE *)s,(BYTE*)pkg->getSubmitSequenceNumber());
	//SendPacket((char *)buf,len);


	//UserTransferMessage utm(m_myID,NullSid,_EvtReceiveMessage,pkg->errorCode);
	//utm[_TagServiceId] = m_serviceName;
	//utm[_TagCallee] = userPhone.c_str();
	//utm[_TagGatewayName] = m_serviceName;

	//string flowSequence = getFlowSequence(pkg->getSubmitSequenceNumber());
	//utm[_TagSequenceNumber] = flowSequence;
	//postMessage(m_myID,utm);
	//m_logger.debug("[%02d] handler report(%s)----(%s)\n",m_myID.m_appref,pkg->getSubmitSequenceNumber(),flowSequence.c_str());

	return true;
}


int BaseYLXProtocol::respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,YLXSubmitResp& pkg)
{
	//utm->setMessageId(_ResSMSSendBatch);
	//utm->setRes(utm->getReq());
	//utm->setReq(m_myID);

	//utm->setReturn(ret);



	//(*utm)[_TagStatus] = pkg.status;


	//postMessage(source, *utm);

	//m_logger.debug("[%02d] respSubmit  _ResSMSSendBatch to (%s), ret(%d),msgstatus(%d)\n",
	//	m_myID.m_appref, source.toString().c_str() ,ret,pkg.status);

	return YLX::Status_OK;
}


int	BaseYLXProtocol::respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret)
{

		//utm->setMessageId(_ResSendSMS);
		//utm->setRes(utm->getReq());
		//utm->setReq(m_myID);

		//utm->setReturn(ret);

		//std::string message = "";
		//char buf[20] = {0};

		//sprintf(buf, "%d", ret);
		//message = buf;
		//(*utm)[_TagMessage] = message;

		////transfer message need return
		//(*utm)[_TagGatewayName] = (*utm)[_TagController];


		//string srcTermID = (*utm)[_TagCalling];//"118100";
		//string destTermID = (*utm)[_TagCallee];//"059522327651";
		//postMessage(source, *utm);

		//m_logger.debug("[%02d] resp to (%s), status(%d), srcTermID(%s), dstTermID(%s)\n",
		//	m_myID.m_appref, source.toString().c_str() ,ret,
		//	srcTermID.c_str(), destTermID.c_str());

	return YLX::Status_OK;
}

void BaseYLXProtocol::snapshot()
{
	//m_logger.info("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
	//	m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
	//	m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);

	//Service::printConsole("[%02d] snapshot: start(%s),status(%d),send(%d),recv(%d),lost(%d)\n",
	//	m_myID.m_appref, getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S").c_str(), getStatus(),
	//	m_statPackets.totalSendPackets, m_statPackets.totalRecvPackets, m_statPackets.lostPackets);
}

void BaseYLXProtocol::handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
//	strarray  vec = (*utm)[_TagCommand];
//	AlarmSender* alarm = AlarmSender::getInstance();
//	strarray ret;
//
//#ifdef _DEBUG
//	alarm->send(ServiceIdentifier(), "recv cmd", 0, vec);
//	m_logger.debug("[%02d] recv cmd: (%s)\n", m_myID.m_appref, vec[0].c_str());
//#endif
//
//	if(vec.size() <= 0)
//	{
//		return;
//	}
//
//	if(vec[0] == "req")
//	{
//		//res module 
//		//index,modulename,type,status,time,version,send,recv,lost
//		ret.push_back(StringUtil::format("%d", m_myID.m_appref));
//		ret.push_back(getServiceName());
//		ret.push_back("smsgateway");
//		ret.push_back(StringUtil::format("%d", getStatus()));
//		ret.push_back(getStrTime(m_startTime, "%Y-%m-%d %H:%M:%S"));
//		ret.push_back("YLX1.3");
//		ret.push_back(StringUtil::format("%u", m_statPackets.totalSendPackets));
//		ret.push_back(StringUtil::format("%u", m_statPackets.totalRecvPackets));
//		ret.push_back(StringUtil::format("%u", m_statPackets.lostPackets));
//
//		alarm->send(ServiceIdentifier(), "res module", 0xFF, ret);
//	}	
}
bool BaseYLXProtocol::handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{
	
	
	return true;
	
}
bool BaseYLXProtocol::transferReport(string caller,string called,string reportcontent)
{
	//UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSReport);
	//(*utm)[_TagCallee] = string(caller.c_str());
	//(*utm)[_TagCalling] =  string(called.c_str());
	//(*utm)[_TagUserData] = reportcontent;
	// postMessage(m_smshttpgw,*utm);
	 return true;
}
bool BaseYLXProtocol::transferDeliver(string caller,string called,string msgcontent)
{
	//UserTransferMessagePtr utm = UTM::create(m_myID,m_smshttpgw,_ReqSMSDeliver);
	//(*utm)[_TagCallee] = string(called.c_str());
	//(*utm)[_TagCalling] = string(caller.c_str());
	//(*utm)[_TagUserData] = msgcontent;
	//postMessage(m_smshttpgw,*utm);
	return true;
}

void BaseYLXProtocol::parseReportMsgContent(const u8array& msgContent,int msglength, std::string& newMsgConent)
{
//
//	string contmsgid = StringUtil::BCD2ASC((const unsigned char*)(&msgContent[3]),20);
//	string contentother = string((const char*)(&msgContent[13]),msglength-13-17);
////	string contextlen= string((const char*)(&msgContent[msglength-20]),3);//20  is  textlength   5 is "Text:"
//	//TODO：应判断最后一个字符是否为中文，如果是的话则取16位，如果不是则取17位
//	//懒下，不处理了
//	//string context= string((const char*)(&msgContent[msglength-17]),16);
////	string conttext=   StringUtil::BCD2ASC((const unsigned char*)(&msgContent[textpos]),34);
//
//	newMsgConent.append("Id:");
//	newMsgConent.append(contmsgid);
//	newMsgConent.append(contentother);
//	//newMsgConent.append(context);
	
}