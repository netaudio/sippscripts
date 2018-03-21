#ifndef __SGIP_PROTOCOL_H
#define __SGIP_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"
#include "BaseSGIPProtocol.h"
#include "sgipdef.h"
#include "sgippacket.h"

using namespace std;
using namespace cacti;
using namespace SGIP;

class SGIPClient;
class SGIPCacheControl;
class SGIPPacketCache;
class SGIPServer;
typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<SGIPPacketCache> SGIPPacketCachePtr; 

class SGIPProtocol : public BaseSGIPProtocol
{
	friend class SGIPClient;
	friend class SGIPServer;
public:
	SGIPProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~SGIPProtocol();


public:
	virtual bool	open();
	virtual bool	close();

	//virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

	bool	activeTest();

	virtual bool			sendPackage(SGIPPacketCachePtr pkg);
protected:               

	virtual bool			sendPackage(SGIPPacket& pkg);
	

private:
	SGIPClient*			m_client;

	
};

#endif
