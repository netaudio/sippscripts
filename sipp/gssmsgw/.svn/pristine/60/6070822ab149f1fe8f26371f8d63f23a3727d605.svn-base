#ifndef __YLX_SERVER_PROTOCOL_H
#define __YLX_SERVER_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"
#include "BaseYLXProtocol.h"
#include "YLXdef.h"
#include "YLXpacket.h"

using namespace std;
using namespace cacti;
using namespace YLX;


//class YLXCacheControl;
//class YLXPacketCache;

class YLXServer;
typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<YLXPacketCache> YLXPacketCachePtr; 

class YLXServerProtocol : public BaseYLXProtocol
{
	friend class YLXClient;
	friend class YLXServer;
public:
	YLXServerProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~YLXServerProtocol();


public:
	virtual bool	open();
	virtual bool	close();

	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm){return 1;};
	virtual bool	activeTest();
	virtual bool	sendPackage(YLXPacketCachePtr pkg);
	bool			sendPackage(char  *buf,int len);

protected:               
	virtual bool	sendPackage(YLXPacket& pkg);

private:
	YLXServer*		m_server;
};

#endif
