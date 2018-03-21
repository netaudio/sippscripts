#ifndef __SGIP_CLIENT_H
#define __SGIP_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "sgippacket.h"

using namespace cacti;
using namespace SGIP;

class ServerInterface;

// use NetworkPackage as transfer package
class SGIPClient : public ActiveClient
{
public:
	SGIPClient(int clientID, ServerInterface* owner);

	bool sendPackage(SGIPPacket& pkg);
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

#endif