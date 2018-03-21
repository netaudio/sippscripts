#ifndef __YSMS_SERVER_H
#define __YSMS_SERVER_H

#pragma once

#include "cacti/socket/ActiveClient.h"
#include "cacti/socket/ActiveServer.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "YSMSpacket.h"

using namespace cacti;
using namespace YSMS;

class ServerInterface;
class ServerInterface;

// use NetworkPackage as transfer package
//class YSMSServer : public ActiveClient
class YSMSServer : public ActiveServer
{
public:
	YSMSServer(int clientID, ServerInterface* owner);

	bool sendPackage(YSMSPacket& pkg);

private:
	virtual bool	onAccept(ConnectionIdentifier client) ;
	virtual int		onReceive(ConnectionIdentifier client);
	virtual void	onBroken(ConnectionIdentifier client) ;

	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);
	ConnectionIdentifier m_clientId;

private:
	ServerInterface* m_owner;
};

#endif

