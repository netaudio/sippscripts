#ifndef __YLX_SERVER_H
#define __YLX_SERVER_H

#pragma once

#include "cacti/socket/ActiveServer.h"
#include "cacti/mtl/DispatcherInterface.h"
#include "cacti/message/Stream.h"

#include "ylxpacket.h"

using namespace cacti;
using namespace YLX;


class ServerInterface;

// use NetworkPackage as transfer package

class YLXServer : public ActiveServer
{
public:
	YLXServer(int clientID, ServerInterface* owner);

	bool sendPackage(YLXPacket& pkg);
	bool sendStream(cacti::Stream& stream);
	bool sendbuf(char *buf,int len);
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


