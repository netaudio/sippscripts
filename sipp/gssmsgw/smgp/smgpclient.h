#ifndef __SMGP_CLIENT_H
#define __SMGP_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "smgppacket.h"

using namespace cacti;
using namespace SMGP;

class ServerInterface;

// use NetworkPackage as transfer package
class SMGPClient : public ActiveClient
{
public:
	SMGPClient(int clientID, ServerInterface* owner);

	bool sendPackage(SMGPPacket& pkg);
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