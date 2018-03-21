#include "stdafx.h"

#include "YSMSServer.h"
#include "YSMSpacket.h"
#include "base/baseprotocol.h"

#include "evtmsg.h"

#define new DEBUG_NEW

YSMSServer::YSMSServer(int clientID, ServerInterface* owner): m_owner(owner)
{
	m_logger.info("[%04d]  Server init\n", clientID);
}


bool YSMSServer::sendPackage(YSMSPacket& pkg)
{
	Stream stream;
	stream.reserve(YSMS::Max_Packet_Size);
	pkg.toStream(stream);
	return send(m_clientId.getId(),stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}


void YSMSServer::onBroken(ConnectionIdentifier client)
{
	ServiceIdentifier sid(0, 0, m_clientId.getId());
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtClientDisconnected));
}


bool	YSMSServer::onAccept(ConnectionIdentifier client)
{
	m_clientId = client;
	ServiceIdentifier sid(0, 0, m_clientId.getId());
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtClientConnected));
	return true;
}


int  YSMSServer::onReceive(ConnectionIdentifier client)
{
	Stream stream;
	int  ret;

	//receive the package length at first!
	stream.resize(3);
	ret = client.m_socketid.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info(" RCV: header : %s\n", cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 3)
	{
		if(ret > 0)
		{
			m_logger.error("RCV: failed, just received(%d) bytes\n", ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u16 length;
	stream.reset(1);
	stream.get16(length);

	u16 left = (length << 8) | (u8)(length >> 8);

	if(left == 0 || left >= YSMS::Max_Packet_Size)
	{
		m_logger.error("RCV: invalid length(%d)\n", left);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(left+8);
	//receive the left package data
	ret = client.m_socketid.recv(stream.getWriteBuffer(3), left + 5, Timestamp(5, 0));

	if(ret != (left + 5))
	{
		m_logger.error("RCV: want(%d) but received(%d)\n",left, ret);

		return recvError(ret);
	}
	if(m_logger.isDebugEnabled())
	{
		binbuf = "";
		m_logger.debug("RCV: %s\n", cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));
	}
	else
	{
		m_logger.info("RCV: %d bytes\n", stream.size());
	}

	if(!enqueue(stream))
	{
		m_logger.info("RCV: enqueue failed\n");
		return RECV_USER_ERROR;
	}
	return RECV_OK;
}


int YSMSServer::recvError(int ret)
{			
	if(ret == 0)
		return RECV_CLOSE;
	else if(ret > 0)
		return RECV_USER_ERROR;
	else
	{
		std::string errmsg;
		m_logger.error("[%04d] RCV: %s\n", 
			m_clientId.getId(), 
			SysError::lastErrorMessage(errmsg));

		return RECV_SOCKET_ERROR;
	}
}


bool YSMSServer::enqueue(Stream& stream)
{
	m_owner->handleClientMsg(stream);
	return true;
}

