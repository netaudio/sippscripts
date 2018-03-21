#ifndef __CMPP_CLIENT_H
#define __CMPP_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "cmpppacket.h"

using namespace cacti;
using namespace CMPP;

class ServerInterface;

// use NetworkPackage as transfer package
class CMPPClient : public ActiveClient
{
public:
	CMPPClient(int clientID, ServerInterface* owner);

	bool sendPackage(CMPPPacket& pkg);
	bool sendStream(cacti::Stream& stream);

private:
	virtual void	onConnected();
	virtual void	onBroken();
	virtual int		onReceive();
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);

private:
	ServerInterface* m_owner;
};

class CMPPClientMo : public CMPPClient
{
public:
	CMPPClientMo(int clientID, ServerInterface* owner);

	bool sendPackage(CMPPPacket& pkg);
	bool sendStream(cacti::Stream& stream);

private:
	virtual void	onConnected();
	virtual void	onBroken();
	virtual int		onReceive();
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);

public:
	u32 m_moHandle;
	u16 m_moPort;
	bool m_moActiveTest;
	u32 m_moConnectNo;
private:
	ServerInterface* m_owner;
};

class CMPPClientMt : public CMPPClient
{
public:
	CMPPClientMt(int clientID, ServerInterface* owner);

	bool sendPackage(CMPPPacket& pkg);
	bool sendStream(cacti::Stream& stream);

private:
	virtual void	onConnected();
	virtual void	onBroken();
	virtual int		onReceive();
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);
public:
	u32 m_mtHandle;
	u16 m_mtPort;
	bool m_mtActiveTest;
	u32 m_mtConnectNo;
private:
	ServerInterface* m_owner;
};

#endif