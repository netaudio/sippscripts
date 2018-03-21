#include "stdafx.h"

#include <cacti/util/IniFile.h>

#include "msgprocess.h"
#include "baseprotocol.h"

#include "msgsk.h"
#include "reqmsg.h"
#include "resmsg.h"
#include "tag.h"


#define new DEBUG_NEW

const char* g_dataFileName		= "./data.conf";

//------------------------------------------------------------------MsgProcess
MsgProcess::MsgProcess(MsgServiceSK& owner
					   , const ServiceIdentifier& myID
					   , int clientType
					   , const char* configFile
					   , const char* serviceName)
:m_owner(owner) 
,m_logger(Logger::getInstance("msggw")) 
,m_processId(myID)
,m_myQueue(2048)
,m_sleep(0, 1000 * 100)
,m_activeTestTime(0)
,m_serviceName(serviceName)
,m_timerID(ActiveTimer::INVALID_ID)
,m_configFile(configFile)
,m_clientType(clientType)
,m_routeTable(serviceName)
{
	started = false;
}

MsgProcess::~MsgProcess()
{

}

void MsgProcess::start()
{
	char acKey[256] = {0};
	cacti::IniFile iniFile;
	int actTimes = 0;
	if(iniFile.open(m_configFile.c_str()))
	{
		sprintf(acKey,"client.%d.activeTestTime", m_processId.m_appref);
		actTimes = iniFile.readInt("gateway", acKey, 0);
		if(actTimes < 3)//< 20, 1 for test
			actTimes = 3;
		m_activeTestTime = actTimes*1000;

		sprintf(acKey,"client.%d.sieID", m_processId.m_appref);
		u32 sieID = iniFile.readInt("gateway", acKey, 0);
		m_sieId.m_appid   = sieID;
		m_sieId.m_appport = AppPort::_apSlee;
		m_sieId.m_appref  = 0;

		sprintf(acKey,"client.%d.flowID", m_processId.m_appref);
		m_flowID = iniFile.readInt("gateway", acKey, 0);
	}

	m_logger.info("[%02d] Start Process(slee:%d)\n", m_processId.m_appref,m_sieId.m_appid);

	if(false == m_routeTable.loadConfig(g_dataFileName))
	{
		m_logger.info("[%02d] (%s) load route data (%s) failed\n", 
			m_processId.m_appref, m_serviceName.c_str(), g_dataFileName);
	}
	else
	{
		m_logger.info("[%02d] (%s) load route data (%s) ok\n", 
			m_processId.m_appref, m_serviceName.c_str(), g_dataFileName);
	}

	m_handler = MsgHandler::createHandler(m_clientType, 
		m_processId,
		m_configFile,
		this);

	assert(m_handler);
	if(!m_handler->open())
	{
		m_logger.error("[%02d] open failed\n", m_processId.m_appref);
		Service::printConsole("Warning !!!! Client %02d start failed  !!!!\n", m_processId.m_appref);
	}

	m_thread.start(this, &MsgProcess::process);
	started = true;	

	//m_handler->setActiveTestTime(m_activeTestTime);
	//setTimer(m_activeTestTime);
}

void MsgProcess::stop()
{
	m_logger.info("[%02d] Stop Process\n", m_processId.m_appref);

	clearTimer();

	UTMData data;
	ServiceIdentifier sid;
	data.utmPtr = UTM::create(sid, sid, _EvtStopProcess);
	m_stopEvent.reset();
	m_myQueue.putHead(data);
	m_stopEvent.wait();

	m_handler->close();
	m_logger.info("[%02d] Process Stopped\n", m_processId.m_appref);
}

bool MsgProcess::putReq(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	if(started == false)
		return false;

	UTMData utmData;
	utmData.sender = sender;
	utmData.utmPtr = UTM::create(utm);
	return m_myQueue.tryPut(utmData);
}
string MsgProcess::getFlowSequence(string sequence)
{
	return m_owner.getFlowSequence(sequence);
}
void MsgProcess::addFlowSequence(string sequence,string flowSequence)
{
	m_owner.addFlowSequence(sequence,flowSequence);
}
bool MsgProcess::postMessage(const ServiceIdentifier& recevier, UserTransferMessage& utm)
{
	return m_owner.postMessage(recevier,utm);
}

void MsgProcess::process()
{
	m_logger.info("[%02d] thread Started\n",
		m_processId.m_appref);
	UTMData data;
	ServiceIdentifier tempSid;
	UserTransferMessage tempUtm;
	data.sender = tempSid;
	data.utmPtr = UTM::create(tempUtm);
	for(;;)
	{
		if(m_handler->isReady())
		{
			m_myQueue.get(data);
			//m_myQueue.tryGet(data);
			if (data.utmPtr->getMessageId() == 0)
			{
				Sleep(20);
				continue;
			}
			if(data.utmPtr->getMessageId() == _EvtStopProcess)
			{
				m_logger.info("[%02d] Stop Message getted\n", m_processId.m_appref);
				m_stopEvent.signal();
				break;
			}
			handleMessage(data.sender, data.utmPtr);
		}
		Sleep(20);
	}
}

void MsgProcess::handleMessage(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	switch(utm->getMessageId())
	{
	case _EvtSnapshot:
		{
			m_logger.info("[%02d] snapshot: name=%s, type=%d, activetest=%d, slee=%s\n",
				m_processId.m_appref, m_serviceName.c_str(), m_clientType, m_activeTestTime/1000, m_sieId.toString().c_str());

			Service::printConsole("[%02d] snapshot: name=%s, type=%d, activetest=%d, slee=%s\n",
				m_processId.m_appref, m_serviceName.c_str(), m_clientType, m_activeTestTime/1000, m_sieId.toString().c_str());
			
			m_handler->snapshot();

		}
		break;
	case _EvtCommand:
		{
			m_handler->handleCommand(sender, utm);
		}
		break;

	case _ReqSendSMS:
		onSendSMS(sender, utm);
		break;

	case _ReqDispatchSMS:
		onDispatchSMS(sender, utm);
		break;

	case _ReqDispatchSMSReport:

		onDispatchSMSReport(sender, utm);
		break;

	case _ReqGWService:
	case _ResGWService:
	case _ResSendSMS:
		onDispatchSMS(sender, utm);
		break;
	case _ResSendWapPush:
	case _ReqQuerySendWindowSize:
	case _ReqSMSSendBatch:
		onGatewayRequest(sender,utm);
		break;
	
	case _EvtActiveTest:
		{
			clearTimer();
			onActiveTest(sender, utm);
			break;
		}

	case _EvtMyTest:
		{
			clearTimer();
			onMyTest(sender, utm);
			break;
		}

	case _EvtReceiveMessage: //start flow to handle it
		{
			m_logger.debug("[%02d] _EvtReceiveMessage %s\n",m_processId.m_appref,m_serviceName.c_str());
			ServiceIdentifier sie = getSie();
			if (sie == NullSid)
			{
				m_logger.debug("[%02d] sie is invalid %s\n",m_processId.m_appref,m_serviceName.c_str());
				break;
			}
			UserTransferMessagePtr reqUtm = UTM::create(m_processId, sie, _ReqStartFlow);

			ostringstream errCode;
			errCode << ((*utm).getReturn());	
			string calling = (*utm)[_TagCalling];
			string callee = (*utm)[_TagCallee];
			string message = (*utm)[_TagMessage];
			string linkId = (*utm)[_TagLinkId];
			string serviceId = (*utm)[_TagServiceId];	
			string sequenceNumber = (*utm)[_TagSequenceNumber];
			
			vector<string> par;
			par.push_back(serviceId);
			par.push_back(calling);
			par.push_back(callee);
			par.push_back(message);
			par.push_back(linkId);
			par.push_back(errCode.str());
			par.push_back(sequenceNumber);
			(*reqUtm)[_TagParameter] = par;
			(*reqUtm)[_TagParameterNumber] = (u32)par.size();						
			(*reqUtm)[_TagGatewayName]= m_serviceName;
			m_owner.postMessage(sie, *reqUtm);
			m_logger.debug("[%02d] post StartFlow to(%s)\n",m_processId.m_appref,sie.toString().c_str());
		}
		break;

	case _EvtTransferMessage://response
		break;

	case _ResStartFlow:
		m_logger.debug("[%02d] responses start flow from (%s)\n", 
			m_processId.m_appref,
			sender.toString().c_str());
		break;

	case _ReqStartTraceFlow:
		{
			if(m_sieId.m_appid != 0)
			{
				m_owner.postMessage(m_sieId, *utm);
				m_logger.info("[%02d] _ReqStartTraceFlow to (%s)\n", 
					m_processId.m_appref, m_sieId.toString().c_str());
			}
		}
		break;
	
	default:
		m_logger.info("[%02d] unknown msg id(%08x) from (%s)\n", 
			m_processId.m_appref,
			utm->getMessageId(),
			sender.toString().c_str());
		break;
	}
}

void MsgProcess::onSendSMS(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	int ret = -1;
	if(m_handler->isReady())
	{
		ret = m_handler->sendMessage(sender, utm);
	}
	if (ret == 1)
	{
		utm->setReturn(0);//transfer to slee ret
	}else
	{
		utm->setReturn(-1);
	}	
	utm->setRes((*utm).getReq());
	utm->setReq(m_processId);
	utm->setMessageId(_ResSendSMS);

	//transfer message need return
	(*utm)[_TagGatewayName] = (*utm)[_TagController];
	
	char buf[20] = {0};
	sprintf(buf, "%d", ret);
	string message = buf;
	(*utm)[_TagMessage] = message;
	m_owner.postMessage(sender, *utm);

}


void MsgProcess::onDispatchSMS(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	int ret = -1;
	if(m_handler->isReady())
	{
		//m_owner.dispatchMessage(sender, utm);
		m_handler->sendMessage(sender, utm);
		ret = 0;
	}
	//if (ret == 1)
	//{
	//	utm->setReturn(0);//transfer to slee ret
	//}else
	//{
	//	utm->setReturn(-1);
	//}	
	//utm->setRes((*utm).getReq());
	//utm->setReq(m_processId);
	//utm->setMessageId(_ResSendSMS);

	////transfer message need return
	//(*utm)[_TagGatewayName] = (*utm)[_TagController];
	//
	//char buf[20] = {0};
	//sprintf(buf, "%d", ret);
	//string message = buf;
	//(*utm)[_TagMessage] = message;
	//m_owner.postMessage(sender, *utm);

}


void MsgProcess::onDispatchSMSReport(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	int ret = -1;
	if(m_handler->isReady())
	{
		//m_owner.dispatchMessage(sender, utm);
		m_handler->sendMessage(sender, utm);
		ret = 0;
	}
	//if (ret == 1)
	//{
	//	utm->setReturn(0);//transfer to slee ret
	//}else
	//{
	//	utm->setReturn(-1);
	//}	
	//utm->setRes((*utm).getReq());
	//utm->setReq(m_processId);
	//utm->setMessageId(_ResSendSMS);

	////transfer message need return
	//(*utm)[_TagGatewayName] = (*utm)[_TagController];
	//
	//char buf[20] = {0};
	//sprintf(buf, "%d", ret);
	//string message = buf;
	//(*utm)[_TagMessage] = message;
	//m_owner.postMessage(sender, *utm);

}




void MsgProcess::onGatewayRequest(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	m_handler->sendMessage(sender, utm);
	return;
}

void  MsgProcess::clearTimer()
{
	m_owner.killTimer(m_timerID);
	m_timerID = cacti::ActiveTimer::INVALID_ID;
}

void  MsgProcess::setTimer(u32 expiredTime)
{
	if(expiredTime <= 0)
		return;
	
	UserTransferMessagePtr utm(new UserTransferMessage);
	utm->setMessageId(_EvtActiveTest);
	utm->setRes(getMyID());
	utm->setReturn(0);

	(*utm)[_TagGatewayName] = m_serviceName;
	m_timerID  = m_owner.setTimer(expiredTime, utm);

	return;
}

void  MsgProcess::onActiveTest(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	m_logger.debug("[%02d] Active Test\n", m_processId.m_appref);
	m_handler->activeTest();
	clearTimer();
	setTimer(m_activeTestTime);
}


void  MsgProcess::onMyTest(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	m_logger.debug("[%02d] Active Test\n", m_processId.m_appref);
	m_handler->activeTest();
	clearTimer();
	setTimer(m_activeTestTime);
}


void	MsgProcess::addSieList(ServiceIdentifier sid)
{
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	list<ServiceIdentifier>::iterator it = m_sieList.begin();
	while (it != m_sieList.end())
	{
		if (sid.m_appid == (*it).m_appid)
		{
			m_logger.debug("[%02d] add sie(%s),has exsit\n", m_processId.m_appref,sid.toString().c_str());
			return;
		}
		it++;
	}
	m_sieList.push_back(sid);
	m_logger.debug("[%02d] add sie(%s)\n", m_processId.m_appref,sid.toString().c_str());
}
ServiceIdentifier MsgProcess::getSie()
{
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	ServiceIdentifier sid;
 	if (m_sieList.empty())
		return NullSid;
	sid = m_sieList.front();
	m_sieList.pop_front();
	if (sid.isValid())
	{		
		m_sieList.push_back(sid);
	}	
	return sid;
}