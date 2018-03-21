#ifndef __YLX_PROTOCOL_H
#define __YLX_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"
#include "BaseYLXProtocol.h"
#include "ylxdef.h"
#include "ylxpacket.h"

using namespace std;
using namespace cacti;
using namespace YLX;

class YLXClient;
class YLXCacheControl;
class YLXPacketCache;
class YLXServer;

typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<YLXPacketCache> YLXPacketCachePtr; 

class YLXProtocol : public BaseYLXProtocol
{
	friend class YLXClient;
	friend class YLXServer;
public:
	YLXProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~YLXProtocol();


public:
	virtual bool	open();
	virtual bool	close();

	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm)=0;

	bool	activeTest();

	virtual bool			sendPackage(YLXPacketCachePtr pkg);

protected:               
	virtual bool			sendPackage(YLXPacket& pkg);
	
private:
	YLXClient*			m_client;

};

#endif
