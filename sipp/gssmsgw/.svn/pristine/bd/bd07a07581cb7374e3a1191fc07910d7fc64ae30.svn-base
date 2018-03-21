#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "smppprotocol.h"
#include "smppserver.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"

#define new DEBUG_NEW


SMPPProtocol::SMPPProtocol(const ServiceIdentifier& id,
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseProtocol(id, configFile, handler)
,m_logger(Logger::getInstance("msggw"))
,m_msgLogger(Logger::getInstance("SMPP"))
{
	m_client = NULL;
}

SMPPProtocol::~SMPPProtocol()
{
	close();
	if(m_client)
	{
		delete m_client;
	}
}

bool	SMPPProtocol::open()
{
	loadConfig();

	if(!bootupSocket())
	{
		m_logger.info("[%02d] %s\n",m_myID.m_appref, "Socket stack failed");
		return false;
	}

	if(NULL == m_client)
	{
		m_client = new SMPPServer( m_myID.m_appref, this );
	}

	assert(m_client);
	
	bool ret = m_client->start(m_serverPort, m_serverIP.c_str());
	m_logger.info("[%02d] %s %s\n",m_myID.m_appref, "SMPP Server init", ret ? "OK" : "Error");

	return  ret;
}

bool	SMPPProtocol::close()
{
		return true;
}

bool	SMPPProtocol::activeTest()
{
	return true;
}

int	SMPPProtocol::sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)
{ 
	switch(utm->getMessageId())
	{
	case _ReqSendSMS:
		return sendSubmit(utm);

	case _ReqGWService:
		break;

	default:
		break;
	}
	return -1;
}


void	SMPPProtocol::loadConfig()
{
	BaseProtocol::loadConfig();
}

void	SMPPProtocol::handleClientEvt(const ServiceIdentifier& receiver, const UserTransferMessagePtr& utm)
{
	switch(utm->getMessageId())
	{
	case _EvtClientConnected:
		setStatus(ST_READY);
		m_logger.info("[%02d] Client Connected to me\n", m_myID.m_appref);
		break;

	case _EvtClientDisconnected:
		m_logger.info("[%02d] Client disconnected\n", m_myID.m_appref);
		break;

	default:
		break;
	}

}

void	SMPPProtocol::handleClientMsg(cacti::Stream& content)
{
	SMPPHeader header(0);
	content.reset();
	header.fromStream(content);
	SMPPPacketPtr packet = SMPPPacketFactory::createPacket(header.command);
	packet->setHeader(header);
	packet->fromStream(content, true);

	switch(header.command)
	{
	case SMPP_BindRecv:
	case SMPP_BindTrn:
		{
			SMPPLogin* pkg = static_cast<SMPPLogin*>(packet.get());
			handleLogin(pkg);
		}
		break;

	case SMPP_BindTrn_Resp:
		{
			SMPPLoginResp* pkg = static_cast<SMPPLoginResp*>(packet.get());
			m_logger.info("[%02d] RECV SMPP_BindTrn_Resp SystemId(%s) \n", m_myID.m_appref, pkg->systemId.c_str());
		}
		break;

	case SMPP_Submit:
		{
			SMPPSubmit* pkg = static_cast<SMPPSubmit*>(packet.get());
			handleSubmit(pkg);
		}
		break;

	case SMPP_Submit_Resp:
		{
			SMPPSubmitResp* pkg = static_cast<SMPPSubmitResp*>(packet.get());
			handleSubmitResp(pkg);
		}
		break;
	case SMPP_Enquire:
		{
			handleEnquire(packet.get());
		}
		break;

	case SMPP_Deliver:
		{
			SMPPDeliver* pkg = static_cast<SMPPDeliver*>(packet.get());
			handleDeliver(pkg);
		}
		break;


	case SMPP_Unbind:
		{
			handleUnbind(packet.get());
		}
		break;

	case SMPP_Unbind_Resp:
		{
			m_logger.info("[%02d] RECV SMPP_Unbind_Resp\n", m_myID.m_appref);
		}
		break;

	default:
		m_logger.info("[%02d] unhandle cmd %d\n", m_myID.m_appref, header.command);
		break;
	}

}

bool	SMPPProtocol::sendLogin()
{
	SMPPLogin packet;
	packet.setSequence(m_seqGen.next());

	return true;

}

bool	SMPPProtocol::sendExit()
{
	SMPPPacket packet(SMPP_Unbind);
	packet.setSequence(m_seqGen.next());

	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND  SMPP_Unbind  %d\n", m_myID.m_appref, ret);

	return ret;	
}

void SMPPProtocol::handleLogin(SMPPLogin *pkg)
{
	m_logger.info("[%02d] RECV SMPP_Login Systemid(%s) Password(%s) Version(%d) AddressRand(%s) \n",
		m_myID.m_appref,pkg->systemId.c_str(),pkg->password.c_str() , (int)pkg->version,pkg->addressRange.c_str());
	
	SMPPLoginResp packet;
	packet.getHeader().command = pkg->getHeader().command + 0x80000000;
	packet.setSequence( pkg->getHeader().sequence);
	packet.systemId = pkg->systemId;

	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND SMPP_Login_Resp  : %d\n", m_myID.m_appref, ret);
	return ;
}

void SMPPProtocol::handleSubmit(SMPPSubmit *pkg)
{
	m_logger.info("[%02d] RECV SMPP_Submit Caller(%s) Called(%s) Message(%s)\n",
		m_myID.m_appref,pkg->source_address.c_str(),pkg->destination_address.c_str(),pkg->short_message_text.c_str());
	
	SMPPSubmitResp packet;
	packet.setSequence( pkg->getHeader().sequence);
	char buf[32];
	sprintf(buf,"%08X",m_seqGen.next());
	packet.Message_id = buf;

	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND SMPP_Submit_Resp : MessageId(%s)\n", m_myID.m_appref,buf);

	ServiceIdentifier res(0,0,0);
	UserTransferMessagePtr utm = UTM::create(m_myID, res, _EvtReceiveMessage);
	(*utm)[_TagCalling] = pkg->source_address.c_str();
	(*utm)[_TagCallee]  = pkg->destination_address.c_str();
	(*utm)[_TagMessage] = pkg->short_message_text.c_str();
	postToOwner(m_myID, utm);

	return ;
}

void SMPPProtocol::handleEnquire(SMPPPacket *pkg)
{
	m_logger.info("[%02d] RECV SMPP_Enquire \n",m_myID.m_appref);

	SMPPPacket packet(SMPP_Enquire_Resp);
	packet.setSequence(pkg->getSequence());
	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND SMPP_Enquire_Resp\n", m_myID.m_appref);
	return;
}

void SMPPProtocol::handleUnbind(SMPPPacket *pkg)
{
	m_logger.info("[%02d] RECV SMPP_Unbind\n",m_myID.m_appref);

	SMPPPacket packet(SMPP_Unbind_Resp);
	packet.setSequence(pkg->getSequence());
	bool ret = m_client->sendPackage(packet);
	m_logger.info("[%02d] SEND SMPP_Unbind_Resp\n", m_myID.m_appref);
	return;
}
int	SMPPProtocol::sendSubmit(UserTransferMessagePtr utm)
{
	return true;
}

bool	SMPPProtocol::handleSubmitResp(SMPPSubmitResp* pkg)
{
#pragma message("to do ...")
	m_logger.info("[%02d] RECV SMPP_Submit_Resp MessageId(%s)\n",m_myID.m_appref,pkg->Message_id.c_str());	
	return true;
}

bool	SMPPProtocol::sendDeliverResp(u32 seq, u8array msgID, u32 status)
{
	SMPPDeliverResp pkg;
	pkg.setSequence(seq);
	pkg.msgID = msgID;
	pkg.status = status;

	return m_client->sendPackage(pkg);
}

bool	SMPPProtocol::handleDeliver(SMPPDeliver* pkg)
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
		(pkg->isReport == SMPP::Report_Need) ? "is report" : content.c_str());

	bool bRet = sendDeliverResp(pkg->getSequence(), pkg->msgID, SMPP::Status_OK);

	m_logger.info("[%02d] SEND deliverResp %s \n",
		m_myID.m_appref,
		(bRet == 0 ? "failed" : "ok"));

	if(pkg->isReport == SMPP::Report_Need)
	{

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

