#ifndef __SMPP_CLIENT_H
#define __SMPP_CLIENT_H

#pragma once

#include "cacti/socket/ActiveServer.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "SMPPpacket.h"

using namespace cacti;
using namespace SMPP;

class SMPPProtocol;

// use NetworkPackage as transfer package
class SMPPServer : public ActiveServer
{
public:
	SMPPServer(int clientID, SMPPProtocol* owner);

	bool sendPackage(SMPPPacket& pkg);

private:
	virtual bool	onAccept(ConnectionIdentifier client) ;
	virtual void	onBroken(ConnectionIdentifier client) ;
	virtual int		onReceive(ConnectionIdentifier client);
	int recvError(int ret);
	bool enqueue(cacti::Stream& stream);
	ConnectionIdentifier m_clientId;

private:
	SMPPProtocol* m_owner;
};

#endif