#ifndef __YLX_CLIENT_H
#define __YLX_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "ylxpacket.h"

using namespace cacti;
using namespace YLX;

class ServerInterface;

// use NetworkPackage as transfer package
class YLXClient : public ActiveClient
{
public:
	YLXClient(int clientID, ServerInterface* owner);

	bool sendPackage(YLXPacket& pkg);
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