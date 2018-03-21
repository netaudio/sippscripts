#include "stdafx.h"

#include "cmppclient.h"
#include "cmpppacket.h"
#include "base/baseprotocol.h"

#include "evtmsg.h"
#include "tag.h"
#define new DEBUG_NEW

CMPPClient::CMPPClient(int clientID, ServerInterface* owner)
: ActiveClient(clientID)
, m_owner(owner)
{
	m_logger.info("[%04d] smgp client init\n", clientID);
}

bool CMPPClient::sendPackage(CMPPPacket& pkg)
{
	Stream stream;
	stream.reserve(CMPP::Max_Packet_Size);
	pkg.toStream(stream);
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

bool CMPPClient::sendStream(cacti::Stream& stream)
{
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

void CMPPClient::onConnected()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerConnected));
}

void CMPPClient::onBroken()
{
	ServiceIdentifier sid(0, 0, m_id);
	//assert(m_owner);
	m_owner->handleClientEvt(sid, UTM::create(sid, sid, _EvtServerDisconnected));
}

int  CMPPClient::onReceive()
{
	Stream stream;
	int  ret;

	//receive the package length at first!
	stream.resize(4);
	ret = m_socket.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] RCV: header : %s\n", 
		m_id,
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 4)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] RCV: failed, just received(%d) bytes\n", 
				m_id, ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u32 length;
	stream.get32(length);

	u32 left = length - 4;

	if(left == 0 || left >= CMPP::Max_Packet_Size)
	{
		m_logger.error("[%04d] RCV: invalid length(%d)\n", m_id, length);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(length);
	//receive the left package data
	ret = m_socket.recv(stream.getWriteBuffer(4), left, Timestamp(5, 0));

	if(ret != left)
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

int CMPPClient::recvError(int ret)
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

bool CMPPClient::enqueue(Stream& stream)
{
	//assert(m_owner);
	m_owner->handleClientMsg(stream);
	return true;
}

CMPPClientMo::CMPPClientMo(int clientID, ServerInterface* owner)
: CMPPClient(clientID,owner)
, m_owner(owner)
{
	m_moHandle = 0;
	m_moPort = 0;
	m_moActiveTest = false;
	m_moConnectNo = 0;
	m_logger.info("[%04d] cmpp client Mo port init\n", clientID);
}

bool CMPPClientMo::sendPackage(CMPPPacket& pkg)
{
	Stream stream;
	stream.reserve(CMPP::Max_Packet_Size);
	pkg.toStream(stream);
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

bool CMPPClientMo::sendStream(cacti::Stream& stream)
{
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

void CMPPClientMo::onConnected()
{
	ServiceIdentifier sid(0, 0, m_id);
	UserTransferMessagePtr utm = UTM::create(sid, sid, _EvtServerConnected);
	(*utm)[_TagData] = (u32)PortIsMo;
	//assert(m_owner);
	m_owner->handleClientEvt(sid,utm);
}

void CMPPClientMo::onBroken()
{
	ServiceIdentifier sid(0, 0, m_id);
	UserTransferMessagePtr utm = UTM::create(sid, sid, _EvtServerDisconnected);
	(*utm)[_TagData] = (u32)PortIsMo;
	//assert(m_owner);
	m_owner->handleClientEvt(sid,utm);
}

int  CMPPClientMo::onReceive()
{
	Stream stream;
	int  ret;
	m_moActiveTest = true;
	m_moConnectNo = PortIsMo;
	//receive the package length at first!
	stream.resize(4);
	ret = m_socket.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] RCV: header : %s\n", 
		m_id,
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 4)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] RCV: failed, just received(%d) bytes\n", 
				m_id, ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u32 length;
	stream.get32(length);

	u32 left = length - 4;

	if(left == 0 || left >= CMPP::Max_Packet_Size)
	{
		m_logger.error("[%04d] RCV: invalid length(%d)\n", m_id, length);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(length);
	//receive the left package data
	ret = m_socket.recv(stream.getWriteBuffer(4), left, Timestamp(5, 0));

	if(ret != left)
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

int CMPPClientMo::recvError(int ret)
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

bool CMPPClientMo::enqueue(Stream& stream)
{
	//assert(m_owner);
	m_owner->handleClientMsg(stream);
	return true;
}

CMPPClientMt::CMPPClientMt(int clientID, ServerInterface* owner)
: CMPPClient(clientID,owner)
, m_owner(owner)
{
	m_mtHandle = 0;
	m_mtPort = 0;
	m_mtActiveTest = false;
	m_mtConnectNo = 0;
	m_logger.info("[%04d] cmpp client Mt port init\n", clientID);
}

bool CMPPClientMt::sendPackage(CMPPPacket& pkg)
{
	Stream stream;
	stream.reserve(CMPP::Max_Packet_Size);
	pkg.toStream(stream);
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

bool CMPPClientMt::sendStream(cacti::Stream& stream)
{
	return send(stream.getWriteBuffer(), (int)stream.size()) == stream.size();
}

void CMPPClientMt::onConnected()
{
	ServiceIdentifier sid(0, 0, m_id);
	UserTransferMessagePtr utm = UTM::create(sid, sid, _EvtServerConnected);
	(*utm)[_TagData] = (u32)PortIsMt;
	//assert(m_owner);
	m_owner->handleClientEvt(sid,utm);
}

void CMPPClientMt::onBroken()
{
	ServiceIdentifier sid(0, 0, m_id);
	UserTransferMessagePtr utm = UTM::create(sid, sid, _EvtServerDisconnected);
	(*utm)[_TagData] = (u32)PortIsMt;
	m_owner->handleClientEvt(sid,utm);
}

int  CMPPClientMt::onReceive()
{
	Stream stream;
	int  ret;
	m_mtActiveTest = true;
	m_mtConnectNo = PortIsMt;
	//receive the package length at first!
	stream.resize(4);
	ret = m_socket.recv(stream.getWriteBuffer(), (int)stream.size(), Timestamp(5, 0));

	std::string binbuf;
	m_logger.info("[%04d] RCV: header : %s\n", 
		m_id,
		cacti::StringUtil::formatBinary((unsigned char*)stream.getWriteBuffer(), (int)stream.size(), binbuf));

	//check valid header
	if(ret != 4)
	{
		if(ret > 0)
		{
			m_logger.error("[%04d] RCV: failed, just received(%d) bytes\n", 
				m_id, ret);
		}
		return recvError(ret);
	}

	//convert to host byte order
	u32 length;
	stream.get32(length);

	u32 left = length - 4;

	if(left == 0 || left >= CMPP::Max_Packet_Size)
	{
		m_logger.error("[%04d] RCV: invalid length(%d)\n", m_id, length);
		return RECV_USER_ERROR;
	}
	// alloc the new buffer
	stream.resize(length);
	//receive the left package data
	ret = m_socket.recv(stream.getWriteBuffer(4), left, Timestamp(5, 0));

	if(ret != left)
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

int CMPPClientMt::recvError(int ret)
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

bool CMPPClientMt::enqueue(Stream& stream)
{
	//assert(m_owner);
	m_owner->handleClientMsg(stream);
	return true;
}
