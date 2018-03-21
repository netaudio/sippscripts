#ifndef __SGIP_SERVER_PROTOCOL_H
#define __SGIP_SERVER_PROTOCOL_H

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


class SGIPCacheControl;
class SGIPPacketCache;
class SGIPServer;
typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<SGIPPacketCache> SGIPPacketCachePtr; 

class SGIPServerProtocol : public BaseSGIPProtocol
{
	friend class SGIPClient;
	friend class SGIPServer;
public:
	SGIPServerProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~SGIPServerProtocol();


public:
	virtual bool	open();
	virtual bool	close();

	//virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);
virtual bool	activeTest();


	virtual bool			sendPackage(SGIPPacketCachePtr pkg);
protected:               

	virtual bool			sendPackage(SGIPPacket& pkg);


private:
	SGIPServer*			m_server;

};

#endif
