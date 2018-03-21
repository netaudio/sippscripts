#ifndef __CNGP_CLIENT_H
#define __CNGP_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "cngppacket.h"

using namespace cacti;
using namespace CNGP;

class CNGPProtocol;

// use NetworkPackage as transfer package
class CNGPClient : public ActiveClient
{
public:
	CNGPClient(int clientID, CNGPProtocol* owner);

	bool sendPackage(CNGPPacket& pkg);

private:
	virtual void	onConnected();
	virtual void	onBroken();
	virtual int		onReceive();
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);

private:
	CNGPProtocol* m_owner;
};

#endif