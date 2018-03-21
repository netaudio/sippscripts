#ifndef __SGIP_SERVER_H
#define __SGIP_SERVER_H

#pragma once

#include "cacti/socket/ActiveServer.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "sgippacket.h"

using namespace cacti;
using namespace SGIP;


class ServerInterface;
// use NetworkPackage as transfer package
class SGIPServer : public ActiveServer
{
public:
	SGIPServer(int clientID, ServerInterface* owner);

	bool sendPackage(SGIPPacket& pkg);
	bool sendStream(cacti::Stream& stream);
private:
	virtual bool	onAccept(ConnectionIdentifier client) ;
	virtual void	onBroken(ConnectionIdentifier client) ;
	virtual int		onReceive(ConnectionIdentifier client);
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);
	ConnectionIdentifier m_clientId;

private:
	ServerInterface* m_owner;
};

#endif