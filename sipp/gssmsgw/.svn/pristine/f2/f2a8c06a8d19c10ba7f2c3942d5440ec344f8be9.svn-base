#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cngpprotocol.h"
#include "cngpclient.h"
#include "cacti/util/IniFile.h"
#include "cacti/util/DailyFile.h"
#include "startup/Service.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW

cacti::DailyFile g_cngpReport("cngpreport");

unsigned int CNGPProtocol::m_msgIdSeq = 0;

CNGPProtocol::CNGPProtocol(const ServiceIdentifier& id,
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseProtocol(id, configFile, handler)
,m_logger(Logger::getInstance("msggw"))
,m_msgLogger(Logger::getInstance("cngp"))
{
	m_client = NULL;
	setStatus(ST_NONE);
}

CNGPProtocol::~CNGPProtocol()
{
	close();
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}
}

bool	CNGPProtocol::open()
{
	loadConfig();

	if(!bootupSocket())
	{
		m_logger.info("[%02d] %s\n",m_myID.m_appref, "Socket stack failed");
		return false;
	}

	if(NULL == m_client)
	{
		m_client = new CNGPClient( m_myID.m_appref, this );

#pragma message("to do ...")
		m_client->setReconnectInterval(120);
	}

	assert(m_client);
	setStatus(ST_INIT);
	m_logger.info("[%02d] %s\n",m_myID.m_appref, "CNGP client init");

	return m_client->start(m_serverPort, m_serverIP.c_str(), false);;
}

bool	CNGPProtocol::close()
{
	sendExit();
	setStatus(ST_ERROR);
	return true;
}

bool	CNGPProtocol::activeTest()
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

	return sendActiveTest();
}

int	CNGPProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{ 
	switch(utm->getMessageId())
	{
	case _ReqSendSMS:
		return sendSubmit(utm);

	case _ReqGWService:
		{
			string command = (*utm)[_TagCommand];
			if( "QueryUserStatus" == command )
			{
				sendQueryUserStatus(source,utm);
			}
			else  if ( "PaymentRequest" == command )
			{
				sendPaymentRequest(source,utm);
			}
			else if ("PaymentAffirm" == command )
			{
				sendPaymentAffirm(source,utm);
			}
			else 
			{
				utm->setReturn(1);
				utm->setRes((*utm).getReq());
				utm->setReq(m_myID);
				utm->setMessageId(_ResGWService);
				postMessage(source,*utm );
				return 0 ;
			}
		}
		break;

	default:
		break;
	}
	return -1;
}


void	CNGPProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
	cacti::IniFile iniFile;
	if(!iniFile.open(m_configFile.c_str()))
		return;

	char key[256] = {0};
	sprintf(key, "client.%d.SmgwNo", m_myID.m_appref);
	m_SmgwNo     = iniFile.readString("gateway", key, "000000");

	sprintf(key, "client.%d.FwdSmgwNo", m_myID.m_appref);
	m_FwdSmgwNo  = iniFile.readString("gateway",key,"000000");

	sprintf(key, "client.%d.SmscNo", m_myID.m_appref);
	m_SmscNo  = iniFile.readString("gateway",key,"000000");

	sprintf(key, "client.%d.spID", m_myID.m_appref);
	m_spId	= iniFile.readString("gateway",key,"168168");

	sprintf(key, "client.%d.version", m_myID.m_appref);
	m_version	= (u8) iniFile.readInt("gateway",key,CNGP::Version_20);
	
	return;


}

void	CNGPProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
{
	switch(utm->getMessageId())
	{
	case _EvtServerConnected:
		sendLogin();
		sendActiveTest();
		m_logger.info("[%02d] Connected to SMGW Server OK!\n", m_myID.m_appref);
		Service::printConsole("[%02d] Connected to SMGW Server OK\n", m_myID.m_appref);
		break;

	case _EvtServerDisconnected:
		m_logger.info("[%02d] Disconnected from server\n", m_myID.m_appref);
		Service::printConsole(" !!! [%02d] Disconnected with SMGW server !!!\n", m_myID.m_appref);

		setStatus(ST_ERROR);
		break;

	default:
		break;
	}

}

void	CNGPProtocol::handleClientMsg(cacti::Stream& content)
{
	CNGPHeader header(0);
	content.reset();
	header.fromStream(content);
	CNGPPacketPtr packet = CNGPPacketFactory::createPacket(header.command);
	packet->setHeader(header);
	packet->fromStream(content, true);

	switch(header.command)
	{
	case CNGP_Login_Resp:
		{
			CNGPLoginResp* pkg = static_cast<CNGPLoginResp*>(packet.get());
			m_logger.info("[%02d] RECV CNGP_Login_Resp  %d\n", m_myID.m_appref, pkg->getStatus());

			if(pkg->getStatus() == CNGP::Status_OK)
				setStatus(ST_READY);
			else
				setStatus(ST_ERROR);
		}
		break;

	case CNGP_Submit_Resp:
		{
			CNGPSubmitResp* pkg = static_cast<CNGPSubmitResp*>(packet.get());
			handleSubmitResp(pkg);
		}
		break;

	case CNGP_Deliver:
		{
			CNGPDeliver* pkg = static_cast<CNGPDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;

	case CNGP_Active_Test:
		{
			sendActiveTestResp(packet->getSequence());
		}
		break;

	case CNGP_Active_Test_Resp:
		{
			m_logger.debug("[%02d] RECV CNGP_ActiveTest_Resp\n", m_myID.m_appref);
		}
		break;

	case CNGP_Exit:
		{
			m_logger.info("[%02d] RECV CNGP_Exit!\n", m_myID.m_appref);
		}
		break;

	case CNGP_Exit_Resp:
		{
			m_logger.info("[%02d] RECV CNGP_Exit_Resp\n", m_myID.m_appref);
		}
		break;
	case CNGP_Query_UserState_Resp:
		{
			m_logger.info("[%02d] RECV CNGP_Query_UserState_Resp \n", m_myID.m_appref);
			CNGPQueryUserStateResp* pkg = static_cast<CNGPQueryUserStateResp*>(packet.get());
			handleQueryUserStatusResp(pkg);
		}
		break;
	case CNGP_Payment_Request_Resp:
		{
			m_logger.info("[%02d] RECV CNGP_Query_UserState_Resp \n", m_myID.m_appref);\
			CNGPPaymentRequestResp* pkg = static_cast<CNGPPaymentRequestResp*>(packet.get());
			handlePaymentRequestResp(pkg);
		}
		break;

	case CNGP_Payment_Affirm_Resp:
		{
			m_logger.info("[%02d] RECV CNGP_Payment_Affirm_Resp \n", m_myID.m_appref);
			CNGPPaymentAffirmResp* pkg = static_cast<CNGPPaymentAffirmResp*>(packet.get());
			handlePaymentAffirmResp(pkg);
		}
		break;

	default:
		m_logger.info("[%02d] RECV unhandle cmd %d\n", m_myID.m_appref, header.command);
		break;
	}

}

bool	CNGPProtocol::sendLogin()
{
	CNGPLogin packet;
	packet.setSequence(m_seqGen.next());

	packet.clientID.assign(m_userName.begin(), m_userName.end());

	u32 stamp = makeTimestamp();
	string authenCode;
	makeAuthCode(authenCode, 
		m_userName.c_str(), 
		m_userPassword.c_str(), 
		stamp,SMS_CNGP);
	packet.authenticatorClient.assign(authenCode.begin(), authenCode.end());

	packet.loginMode     = CNGP::Mode_Transmit;
	packet.timeStamp     = stamp;
	packet.clientVersion = m_version;


	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND CNGP_Login %s\n", m_myID.m_appref, ret ? "Success" : "!!!Failed!!!");

	return ret;

}

bool	CNGPProtocol::sendExit()
{
	CNGPExit packet;
	packet.setSequence(m_seqGen.next());

	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] send exit status: %d\n", m_myID.m_appref, ret);

	return ret;	
}

bool	CNGPProtocol::sendActiveTest()
{
	CNGPActiveTest packet;
	packet.setSequence(m_seqGen.next());
	m_logger.info("[%02d] SEND CNGP_ActiveTest\n",m_myID.m_appref);
	return m_client->sendPackage(packet);
}

bool	CNGPProtocol::sendActiveTestResp(u32 seq)
{
	CNGPActiveTestResp packet;
	packet.setSequence(seq);

	return m_client->sendPackage(packet);
}

// UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, 0);
// (*utm)[_TagServiceCode] = "1181177";
// (*utm)[_TagCalling] = "1181177";
// (*utm)[_TagCallee] = "059522327651";
// (*utm)[_TagMessage] = "测试123456";
// sendSubmit(utm);

int	CNGPProtocol::sendSubmit(UserTransferMessagePtr utm)
{
	CNGPSubmit packet;
	//SPID
	string spid = (*utm)[_TagSPID];
	if( "" == spid )
	{
		spid =m_userName;
	}
	packet.clientID.assign(spid.begin(),spid.end());

	//短消息类型
	packet.subType = CNGP::MsgType_OrderSend;

	u32 needReport = (*utm)[_TagNeedReport];
	//SP是否要求返回状态报告
	//packet.needReport = CNGP::Report_Nothing;
	packet.needReport = (u8)needReport;

	//短消息发送优先级
	packet.priority = CNGP::Priority_Normal;

	//[10]业务代码
	string serviceID = (*utm)[_TagServiceCode];//"1181177";
	if("" == serviceID)
	{
		serviceID = m_serviceCode;
	}
	packet.serviceID.assign(serviceID.begin(), serviceID.end());

	//[2]收费类型
	string feeType = "00";
	if ((*utm).hasKey(_TagFeeType,TLV::TYPE_STRING))
			feeType = (*utm)[_TagFeeType];

	packet.feeType.assign(feeType.begin(), feeType.end());
	
	//[1]收费对象类型
	packet.feeUserType = CNGP::FeeUserType_BillTel;

	//[6]资费
	string		feeCode = "0";
	if ((*utm).hasKey(_TagFeeCode,TLV::TYPE_STRING))
		feeCode = (*utm)[_TagFeeCode];
		
	packet.feeCode.assign(feeCode.begin(), feeCode.end());

	//短消息格式
	packet.msgFormat = CNGP::Format_GB;

	//[17]短消息有效时间
	//u8array		validTime;
	//[17]短消息定时发送时间
	//u8array		atTime;

	//[21]短信息发送方号码
	string srcTermID = (*utm)[_TagCalling];//"1181177";
	
	//河南的需求，源号码一定要填serviceid
	srcTermID = serviceID;

	packet.srcTermID.assign(srcTermID.begin(), srcTermID.end());

	//[21]计费用户号码
	string chargeTermID = (*utm)[_TagBillPhone];
	if( "" == chargeTermID )
		chargeTermID = srcTermID;

	packet.chargeTermID.assign(chargeTermID.begin(), chargeTermID.end());

	//短消息接收号码总数
	packet.destTermIDCount = 1;
	//[21*DestTermCount]短消息接收号码
	string destTermID = (*utm)[_TagCallee];//"059522327651";
	packet.destTermID.assign(destTermID.begin(), destTermID.end());

	//[MsgLength]短消息内容
	string msgContent = (*utm)[_TagMessage];//"test ok!";
	if(msgContent.length() <= 0)
	{
		return -1;
	}

	bool bRet = false;
	string buf;

	if(msgContent.length() <= CNGP::Max_Message_Size)
	{
		packet.setSequence(m_seqGen.next());
		//短消息长度
		packet.msgLength = (u8)msgContent.length();
		packet.msgContent.assign(msgContent.begin(), msgContent.end());


		bRet = m_client->sendPackage(packet);
		buf.assign(msgContent.begin(), msgContent.end());
		cacti::StringUtil::replace(buf, "\n", " ");
		m_msgLogger.info("^^%02d^^MT^^%d^^%08x^^%s^^%s^^%s^^%s\n",
			m_myID.m_appref,
			(bRet == true ? 1 : 0),
			packet.getSequence(),
			srcTermID.c_str(),
			destTermID.c_str(),
			msgContent.c_str(),
			feeType.c_str());
	}
	else
	{
		//ceil(msgContent.length() / CNGP::Max_Message_Size); //c func
		int cycleCount = (msgContent.length() / CNGP::Max_Message_Size);
		int lastSize = msgContent.length() % CNGP::Max_Message_Size;
		cycleCount +=  (lastSize > 0) ? 1 : 0;

		int iPos = 0;

		for(int i = 0; i < cycleCount; i++)
		{
			iPos = i * CNGP::Max_Message_Size;
			packet.setSequence(m_seqGen.next());
			if( lastSize > 0 && ((i+1) == cycleCount) )
			{
				packet.msgContent.assign(msgContent.end()-lastSize, msgContent.end());
				buf.assign(msgContent.end()-lastSize, msgContent.end());
			}
			else
			{
				packet.msgContent.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*CNGP::Max_Message_Size);
				buf.assign(msgContent.begin()+iPos, msgContent.begin()+(i+1)*CNGP::Max_Message_Size);
			}
			packet.msgLength = (u8)packet.msgContent.size();
			bRet = m_client->sendPackage(packet); //only judge last send
			
			cacti::StringUtil::replace(buf, "\n", " ");
			m_msgLogger.info("^^%02d^^MT^^%d^^%08x^^%s^^%s^^%s^^%s\n",
				m_myID.m_appref,
				(bRet == true ? 1 : 0),
				packet.getSequence(),
				srcTermID.c_str(),
				destTermID.c_str(),
				buf.c_str(),
				feeType.c_str());
		}
	}
	m_logger.info("[%02d] SEND CNGP_Submit Caller(%s) Called(%s) ServiceCode(%s) FeeType(%s) FeeCode(%s) Message(%s)\n",
		m_myID.m_appref,srcTermID.c_str(),destTermID.c_str(),serviceID.c_str(),feeType.c_str(),feeCode.c_str(),msgContent.c_str());	
	return bRet == true ? 0 : -1;
}

bool	CNGPProtocol::handleSubmitResp(CNGPSubmitResp* pkg)
{
#pragma message("to do ...")
	m_logger.info("[%02d] RECV CNGP_Submit_Resp %d\n",m_myID.m_appref,pkg->getStatus());	
	return true;
}

bool	CNGPProtocol::sendDeliverResp(u32 seq, u8array msgID, u32 status)
{
	CNGPDeliverResp pkg;
	pkg.setSequence(seq);
	pkg.msgID = msgID;
	pkg.setStatus(status);

	return m_client->sendPackage(pkg);
}

bool	CNGPProtocol::handleDeliver(CNGPDeliver* pkg)
{
	string content;
	content.assign(pkg->msgContent.begin(), pkg->msgContent.end());
	string dstTermID;
	dstTermID.assign(pkg->destTermID.begin(), pkg->destTermID.end());
	string srcTermID;
	srcTermID.assign(pkg->srcTermID.begin(), pkg->srcTermID.end());

	cacti::StringUtil::replace(content, "\n", " ");
	m_msgLogger.info("^^%02d^^MO^^%d^^%08x^^%s^^%s^^%s\n",
		m_myID.m_appref,
		1,
		pkg->getSequence(),
		srcTermID.c_str(),
		dstTermID.c_str(),
		(pkg->isReport == CNGP::Report_Need) ? "is report" : content.c_str());

	bool bRet = sendDeliverResp(pkg->getSequence(), pkg->msgID, CNGP::Status_OK);

	m_logger.info("[%02d] send deliverResp %s \n",
		m_myID.m_appref,
		(bRet == 0 ? "failed" : "ok"));

	if(pkg->isReport == CNGP::Report_Need)
	{
		string stat="stat:XXXXXXX", err="err:XXX";
		size_t pos = content.find("stat:");
		if( pos != string::npos)
			stat = content.substr(pos,12);
		pos = content.find("err:");
		if( pos != string::npos)
			err = content.substr(pos,7);
		string now = Timestamp::getNowTime().getFormattedTime("%Y-%m-%d %H:%M:%S");

		g_cngpReport.print("%s^^%s^^%s^^%s^^%s\n",
			now.c_str(),
			srcTermID.c_str(),
			dstTermID.c_str(),
			err.c_str(),
			stat.c_str());
#pragma message("to do ...")	
		return true;
	}

	ServiceIdentifier res(0,0,0);
	UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);
	(*utm)[_TagCalling] = srcTermID.c_str();
	(*utm)[_TagCallee]  = dstTermID.c_str();
	(*utm)[_TagMessage] = content.c_str();

	postToOwner(m_myID, utm);
	return true;
}


bool CNGPProtocol::sendQueryUserStatus(const ServiceIdentifier& sender,UserTransferMessagePtr utm)
{
	CNGPQueryUserState packet;
	packet.setSequence(m_seqGen.next());
	m_requestList.Put(packet.getSequence(),Requester(sender,utm->getReq()));

	string caller = (*utm)[_TagMessage];
	m_logger.info("[%02d] SEND CNGP_Query_UserStatus Caller(%s) \n", m_myID.m_appref,caller.c_str());

	string2Vector(packet.userAddr,caller);
	string2Vector(packet.smgwNo,m_SmgwNo);
	return m_client->sendPackage(packet);
}

bool CNGPProtocol::handleQueryUserStatusResp(CNGPQueryUserStateResp *pkg)
{
	string userStatus;
	userStatus.assign(pkg->userStatus.begin(),pkg->userStatus.end());
	
	char buf[32];
	sprintf(buf,"%u",pkg->count);
	string content =  userStatus + "$" +buf;

	Requester requester;
	if( !m_requestList.Get(pkg->getSequence(), requester) )
		return false;

	UserTransferMessagePtr utm = UTM::create(m_myID, requester.m_session, _ResGWService);
	(*utm)[_TagMessage] = content.c_str();
	(*utm).setReturn(pkg->getStatus());
	postMessage(requester.m_sender, *utm);
	return true;
}

bool CNGPProtocol::sendPaymentRequest(const ServiceIdentifier& sender,UserTransferMessagePtr utm)
{
	
	// 主叫号码$扣费金额$被叫号码$费率类型$开始时间$结束时间
	vector<string> param;
	string message = (*utm)[_TagMessage];
	int paramCount = StringUtil::split(param,message,"$");

	if (paramCount != 6 )
	{
		UserTransferMessagePtr utm = UTM::create(m_myID,utm->getReq(), _ResGWService);
		(*utm)[_TagMessage] = "incorrect param";
		(*utm).setReturn(-1);
		postMessage(sender, *utm);
		return false;
	}
	CNGPPaymentRequest packet;
	packet.setSequence(m_seqGen.next());
	m_requestList.Put(packet.getSequence(),Requester(sender,utm->getReq()));


	int index = 0;
	string caller = param[index++];
	string feecode = param[index++];
	string called = param[index++];
	string feetype = param[index++];
	string starttime = param[index++];
	string endtime = param[index++];

	m_logger.info("[%02d] SEND CNGP_Payment_Request caller(%s) feecode(%s) feetype(%s) \n", 
				m_myID.m_appref,caller.c_str(),feecode.c_str(), feetype.c_str());

	// if user use "1" means "01"
	if( 1 == feetype.length())
		feetype = "0"+feetype;

	Timestamp now = Timestamp::getNowTime();
	char buf[32];
	sprintf( buf, "%06u",m_msgIdSeq++ );
	string messageId = m_SmgwNo +  now.getFormattedTime("%d%H%M%S")+buf;
	packet.messagId = StringUtil::getBCD(messageId);
	packet.smType = 2;
	packet.subType = 2;
	
	string2Vector(packet.chargeTermId, caller);
	string2Vector(packet.spId,m_spId);
	string2Vector(packet.destTermId,caller);
	string2Vector(packet.serviceId,called);
	string2Vector(packet.feeType,feetype);
	string2Vector(packet.feeCode,feecode);
	packet.priority = 3;
	packet.msgLength = 0x45;
	//packet.areaCode
	if(caller.length() > 4 && '0' == caller[0]  )
		string2Vector(packet.areaCode,caller.substr(0,4));
	string2Vector(packet.smgwNo,m_SmgwNo);
	string2Vector(packet.fwdSmgwNo,m_FwdSmgwNo);
	string2Vector(packet.smscNo,m_SmscNo);
	string2Vector(packet.recvTime,starttime);
	string2Vector(packet.doneTime,endtime);
	
	
	return m_client->sendPackage(packet);
}

bool CNGPProtocol::handlePaymentRequestResp(CNGPPaymentRequestResp* pkg)
{

	Requester requester;
	if( !m_requestList.Get(pkg->getSequence(), requester) )
		return false;

	UserTransferMessagePtr utm = UTM::create(m_myID, requester.m_session, _ResGWService);

	char buf[32];
	sprintf(buf,"%d",pkg->resultNotifyCode);
	(*utm)[_TagMessage] = string(buf);
	(*utm).setReturn(pkg->getStatus());
	postMessage(requester.m_sender, *utm);
	return true;
}
bool CNGPProtocol::sendPaymentAffirm(const ServiceIdentifier& sender,UserTransferMessagePtr utm)
{
	CNGPPaymentAffirm packet;
	packet.setSequence(m_seqGen.next());
	m_requestList.Put(packet.getSequence(),Requester(sender,utm->getReq()));

	Timestamp now = Timestamp::getNowTime();
	char buf[32];
	sprintf( buf, "%06u",m_msgIdSeq++ );
	string messageId = m_SmgwNo +  now.getFormattedTime("%d%H%M%S")+buf;
	packet.messagId = StringUtil::getBCD(messageId);

	packet.responseStatus = 0;
	//packet.feeAddr
	m_logger.info("[%02d] SEND CNGP_Payment_Affirm\n", m_myID.m_appref);
	return m_client->sendPackage(packet);
}

bool CNGPProtocol::handlePaymentAffirmResp(CNGPPaymentAffirmResp* pkg)
{
	Requester requester;
	if( !m_requestList.Get(pkg->getSequence(), requester) )
		return false;

	UserTransferMessagePtr utm = UTM::create(m_myID, requester.m_session, _ResGWService);
	(*utm)[_TagMessage] = "";
	postMessage(requester.m_sender, *utm);
	return true;
}
