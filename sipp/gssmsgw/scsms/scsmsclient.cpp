#include "stdafx.h"

#include "scsmsclient.h"
#include "scsmspacket.h"
#include "base/baseprotocol.h"

#include "evtmsg.h"

#define new DEBUG_NEW

SCSMSClient::SCSMSClient(int clientID, ServerInterface* owner)
: ActiveClient(clientID)
, m_owner(owner)
{
	m_logger.info("[%04d] scsms client init\n", clientID);
}

bool SCSMSClient::sendPackage(SCSMSPacket& pkg)
{
	Stream stream;
	stream.reserve(SCSMS::Max_Packet_Size);
	pkg.toStream(stream);
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

void SCSMSClient::onConnected()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerConnected));
}

void SCSMSClient::onBroken()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerDisconnected));
}

int  SCSMSClient::onReceive()
{
	Stream stream;
	int  ret;

	//receive the package length at first!
	stream.resize(3);
	ret = m_socket.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] RCV: header : %s\n", 
		m_id,
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 3)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] RCV: failed, just received(%d) bytes\n", 
				m_id, ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u16 length;
	stream.reset(1);
	stream.get16(length);

	u16 left = (length << 8) | (u8)(length >> 8);

	if(left == 0 || left >= SCSMS::Max_Packet_Size)
	{
		m_logger.error("[%04d] RCV: invalid length(%d)\n", m_id, left);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(left+8);
	//receive the left package data
	ret = m_socket.recv(stream.getWriteBuffer(3), left + 5, Timestamp(5, 0));

	if(ret != (left + 5))
	{
		m_logger.error("[%04d] RCV: want(%d) but received(%d)\n",
			m_id, left, ret);

		return recvError(ret);
	}
	if(m_logger.isDebugEnabled())
	{
		binbuf = "";
		m_logger.debug("[%04d] RCV: %s\n", 
			m_id,
			cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));
	}
	else
	{
		m_logger.info("[%04d] RCV: %d bytes\n", 
			m_id, 
			stream.size());
	}

	if(!enqueue(stream))
	{
		m_logger.info("[%04d] RCV: enqueue failed\n", m_id);
		return RECV_USER_ERROR;
	}
	return RECV_OK;
}

int SCSMSClient::recvError(int ret)
{			
	if(ret == 0)
		return RECV_CLOSE;
	else if(ret > 0)
		return RECV_USER_ERROR;
	else
	{
		std::string errmsg;
		m_logger.error("[%04d] RCV: %s\n", 
			m_id, 
			SysError::lastErrorMessage(errmsg));

		return RECV_SOCKET_ERROR;
	}
}

bool SCSMSClient::enqueue(Stream& stream)
{
	//assert(m_owner);
	m_owner->handleClientMsg(stream);
	return true;
}
