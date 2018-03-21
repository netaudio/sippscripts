#ifndef __MSG_PROCESS_H_
#define __MSG_PROCESS_H_
#pragma once

#include <cacti/util/BoundedQueue.h>
#include <cacti/message/ServiceIdentifier.h>
#include <cacti/message/TransferMessage.h>
#include <cacti/kernel/Thread.h>
#include <cacti/kernel/Mutex.h>
#include <cacti/logging/LogManager.h>
#include <cacti/util/Timer.h>

#include "base/baseprotocol.h"
#include "base/routetable.h"

using namespace cacti;
using namespace std;

class MsgServiceSK;

struct UTMData
{
	ServiceIdentifier        sender;
	UserTransferMessagePtr   utmPtr;
};

typedef BoundedQueue<UTMData> UtmQueue;	

class MsgProcess : HandlerInterface
{
public:
	MsgProcess(MsgServiceSK& owner
		, const ServiceIdentifier& myID
		, int clientType
		, const char*  configFile
		, const char*  servieName);
	
	~MsgProcess();

public:
	bool          putReq(const ServiceIdentifier& sender, UserTransferMessage& utm);
	bool		  postMessage(const ServiceIdentifier& recevier, UserTransferMessage& utm);
	void          start();
	void          stop();
	ServiceIdentifier getMyID() { return m_processId; };
	ServiceIdentifier getSieID() { return m_sieId; };
	u32				  getFlowID() { return m_flowID; };
	string            getServiceName() { return m_serviceName; }
	void				addSieList(ServiceIdentifier sid);
	ServiceIdentifier getSie();

	string getFlowSequence(string sequence);
	void addFlowSequence(string sequence,string flowSequence) ;
	void          handleMessage(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	BaseProtocolPtr      m_handler;

private:
	virtual void          process();
//	void          handleMessage(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void          onSendSMS(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void          onDispatchSMS(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void          onDispatchSMSReport(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void		  onGatewayRequest(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void          clearTimer();
	void          setTimer(u32 expiredTime);
	void          onActiveTest(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	void          onMyTest(const ServiceIdentifier& sender, UserTransferMessagePtr utm);

private:
	MsgServiceSK&        m_owner;
	cacti::Thread        m_thread;
	UtmQueue             m_myQueue;
	cacti::Logger&       m_logger;
	ServiceIdentifier    m_processId;
	Event                m_stopEvent;
	bool                 started;
	Timestamp            m_sleep;
	string               m_configFile;
	RecursiveMutex       m_lock;
	TimerID              m_timerID;
	u32                  m_activeTestTime;
	string               m_serviceName;
	int                  m_clientType;
	ServiceIdentifier    m_sieId;
	u32                  m_flowID;
	RouteTable			 m_routeTable;
	list<ServiceIdentifier> m_sieList;
};

#endif