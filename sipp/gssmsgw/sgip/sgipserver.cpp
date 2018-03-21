#include "stdafx.h"

#include "sgipserver.h"
#include "sgippacket.h"
#include "base/baseprotocol.h"

#include "evtmsg.h"

#define new DEBUG_NEW
#define _TEST
SGIPServer::SGIPServer(int clientID, ServerInterface* owner)
: m_owner(owner)
{
	m_logger.info("[%04d] SGIP Server init\n", clientID);
}

bool SGIPServer::sendPackage(SGIPPacket& pkg)
{
	Stream stream;
	stream.reserve(SGIP::Max_Packet_Size);
	pkg.toStream(stream);
	return send(m_clientId.getId(),stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}


bool SGIPServer::sendStream(cacti::Stream& stream)
{
	int size = stream.size();
	int sentsize = send(m_clientId.getId(),stream.getWriteBuffer(), (int)stream.size()) ;
	return size == sentsize;
}


bool SGIPServer::onAccept(ConnectionIdentifier client)
{
	m_clientId = client;
	ServiceIdentifier sid(0, 0, m_clientId.getId());
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtClientConnected));
	return true;
}


void SGIPServer::onBroken(ConnectionIdentifier client)
{
	ServiceIdentifier sid(0, 0, m_clientId.getId());
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtClientDisconnected));
}

int  SGIPServer::onReceive(ConnectionIdentifier client)
{
	Stream stream;
	int  ret;

	m_clientId = client;

	//receive the package length at first!
	stream.resize(4);
	ret = client.m_socketid.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] RCV: header : %s\n", 
		client.getId(),
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 4)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] RCV: failed, just received(%d) bytes\n", 
				client.getId(), ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u32 length;
	stream.get32(length);

	u32 left = length - 4;

	if(left == 0 || left >= SGIP::Max_Packet_Size)
	{
		m_logger.error("[%04d] RCV: invalid length(%d)\n", client.getId(), length);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(length);
	//receive the left package data
	ret = client.m_socketid.recv(stream.getWriteBuffer(4), left, Timestamp(5, 0));

	if(ret != left)
	{
		m_logger.error("[%04d] RCV: want(%d) but received(%d)\n",
			client.getId(), left, ret);

		return recvError(ret);
	}
	if(m_logger.isDebugEnabled())
	{
		binbuf = "";
		m_logger.debug("[%04d] RCV: %s\n", 
			client.getId(),
			cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));
	}
	else
	{
		m_logger.info("[%04d] RCV: %d bytes\n", 
			client.getId(), 
			stream.size());
	}

	if(!enqueue(stream))
	{
		m_logger.info("[%04d] RCV: enqueue failed\n", client.getId());
		return RECV_USER_ERROR;
	}
	return RECV_OK;
}

int SGIPServer::recvError(int ret)
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

bool SGIPServer::enqueue(Stream& stream)
{
	m_owner->handleClientMsg(stream);
	return true;
}


