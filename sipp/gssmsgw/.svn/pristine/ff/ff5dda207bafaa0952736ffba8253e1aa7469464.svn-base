#ifndef __SCSMS_CLIENT_H
#define __SCSMS_CLIENT_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "scsmspacket.h"

using namespace cacti;
using namespace SCSMS;

class ServerInterface;

// use NetworkPackage as transfer package
class SCSMSClient : public ActiveClient
{
public:
	SCSMSClient(int clientID, ServerInterface* owner);

	bool sendPackage(SCSMSPacket& pkg);

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