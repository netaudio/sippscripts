#include "stdafx.h"

#include "YLXclient.h"
#include "YLXpacket.h"
#include "base/baseprotocol.h"

#include "evtmsg.h"

#define new DEBUG_NEW

YLXClient::YLXClient(int clientID, ServerInterface* owner)
: ActiveClient(clientID)
, m_owner(owner)
{
	m_logger.info("[%04d] sigp client init\n", clientID);
}

bool YLXClient::sendPackage(YLXPacket& pkg)
{
	Stream stream;
	stream.reserve(YLX::Max_Packet_Size);
	pkg.toStream(stream);
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

bool YLXClient::sendStream(cacti::Stream& stream)
{
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

void YLXClient::onConnected()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerConnected));
}

void YLXClient::onBroken()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerDisconnected));
}

int  YLXClient::onReceive()
{
//////

	Stream stream;
	int  ret;

	//receive the package length at first!
	stream.resize(4);
	ret = m_socket.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] SIGPRCV: header : %s\n", 
		m_id,
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 4)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] SIGPRCV: failed, just received(%d) bytes\n", 
				m_id, ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u32 length;
	stream.get32(length);

	u32 left = length - 4;

	if(left == 0 || left >= YLX::Max_Packet_Size)
	{
		m_logger.error("[%04d] SIGPRCV: invalid length(%d)\n", m_id, length);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(length);
	//receive the left package data
	ret = m_socket.recv(stream.getWriteBuffer(4), left, Timestamp(5, 0));

	if(ret != left)
	{
		m_logger.error("[%04d] SIGPRCV: want(%d) but received(%d)\n",
			m_id, left, ret);

		return recvError(ret);
	}
	if(m_logger.isDebugEnabled())
	{
		binbuf = "";
		m_logger.debug("[%04d] SIGPRCV: %s\n", 
			m_id,
			cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));
	}
	else
	{
		m_logger.info("[%04d] SIGPRCV: %d bytes\n", m_id, stream.size());
	}

	if(!enqueue(stream))
	{
		m_logger.info("[%04d] SIGPRCV: enqueue failed\n", m_id);
		return RECV_USER_ERROR;
	}
	return RECV_OK;
}

int YLXClient::recvError(int ret)
{			
	if(ret == 0)
		return RECV_CLOSE;
	else if(ret > 0)
		return RECV_USER_ERROR;
	else
	{
		std::string errmsg;
		m_logger.error("[%04d] SIGPRCV: %s\n", 
			m_id, 
			SysError::lastErrorMessage(errmsg));

		return RECV_SOCKET_ERROR;
	}
}

bool YLXClient::enqueue(Stream& stream)
{
	//assert(m_owner);
	m_owner->handleClientMsg(stream);
	return true;
}
