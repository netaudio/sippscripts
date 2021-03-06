#include "stdafx.h"

#include "cacti/util/IniFile.h"
#include "startup/Service.h"


#include "msgsk.h"
#include "tag.h"
#include "evtmsg.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW

extern const char* g_cfgFileName;

MTL_BEGIN_MESSAGE_MAP(MsgServiceSK)

	MTL_ON_MESSAGE(_EvtActiveTest,		onActiveTest)
	MTL_ON_MESSAGE(_EvtMyTest,			onMyTest)
	MTL_ON_MESSAGE(_ReqSendSMS,			onDistributeRequest)
	MTL_ON_MESSAGE(_ReqSMSSendBatch,	onDistributeRequest)
	MTL_ON_MESSAGE(_ReqQuerySendWindowSize,onDistributeRequest)
	MTL_ON_MESSAGE(_ReqGWService,		onDistributeRequest)
	MTL_ON_MESSAGE(_ResSendSMS,			onDistributeRequest)
	MTL_ON_MESSAGE(_ReqDispatchSMS,		onDispatchSMS	)
	MTL_ON_MESSAGE(_ReqDispatchSMSReport, onDispatchSMSReport	)
	MTL_ON_MESSAGE(_ResGWService,		onDistributeRequest)
	MTL_ON_MESSAGE(_ResStartFlow,		onDistributeRequest)
	MTL_ON_MESSAGE(_ResSendWapPush,	    onDistributeRequest)
	MTL_ON_MESSAGE(_EvtReceiveMessage,	    onDistributeRequest)
	MTL_ON_MESSAGE(_EvtModuleActive,	onModuleActive)
	MTL_ON_MESSAGE(_EvtModuleDeactive,	onModuleDeactive)
	MTL_ON_MESSAGE(_ResRegisterService, onRegisterServiceResponse);
	MTL_ON_MESSAGE(_EvtSnapshot,		onSnapshot);
	MTL_ON_MESSAGE(_EvtCommand,			onDistributeAll);
	MTL_ON_MESSAGE(_EvtTimeOutException,	onEvtTimeOutException);

MTL_END_MESSAGE_MAP


MsgServiceSK::MsgServiceSK(MessageDispatcher* dispatcher)
:ServiceSkeleton(AppPort::_apGateway, dispatcher), m_logger(Logger::getInstance("msggw"))
{
}

MsgServiceSK::~MsgServiceSK()
{
	uninit();
}

bool MsgServiceSK::init()
{
	cacti::IniFile iniFile;
	if(!iniFile.open(g_cfgFileName))
		return false;

	int i = 0;
	char acKey[256] = {0};
	string serviceName = "";
	int serviceType = 0;
	hasProxy = false;
	while(true)
	{
		i++;
		sprintf(acKey,"client.%d.name", i );
		serviceName = iniFile.readString("gateway", acKey, "");
		if("" == serviceName)
			break;

		sprintf(acKey,"client.%d.type", i );
		serviceType = iniFile.readInt("gateway", acKey, 0);

		if(0 == serviceType)
			continue;

		//判断是不是已经读过的或者已有
		std::map<string,MsgProcess*>::iterator it;
		it = m_gwlist.find(serviceName);
		if(it != m_gwlist.end())
			continue;
		

		MsgProcess* msgProc = new MsgProcess(*this, 
			myIdentifier(i), 
			serviceType, 
			g_cfgFileName,
			serviceName.c_str());

		m_gwlist[serviceName] = msgProc;

		msgProc->start();
		
		m_logger.info("[%02d] start process [%s][%s]\n",
			i,
			serviceName.c_str(), 
			msgProc->getMyID().toString().c_str());
	}
	UserTransferMessagePtr utm = UTM::create(_EvtTimeOutException);
	setTimer(FlowSeqCheckTime,utm);
	return true;
}

void MsgServiceSK::uninit()
{
	std::map<string,MsgProcess*>::iterator it;
	MsgProcess* msgProc = NULL;
	for(it = m_gwlist.begin(); it != m_gwlist.end(); it++)
	{
		msgProc = it->second;
		msgProc->stop();
		delete msgProc;
	}
	m_gwlist.clear();
}



bool MsgServiceSK::dispatchMessage(const ServiceIdentifier& receiver, UserTransferMessagePtr utm)
{
	std::string procname = (*utm)[_TagGatewayName];
	MsgProcess * proc = findProcess(procname);
	if(proc != NULL)
	{
		proc->m_handler->postMessage(receiver, *utm);
//		proc->m_handler->postToOwner(receiver, utm);
		return true;
	}
	return false;
}


void MsgServiceSK::onModuleActive(const ServiceIdentifier& sender, UserTransferMessage& utm)
{

	switch(sender.m_appport)
	{
	case AppPort::_apSlee:
		{
			m_logger.info("slee(%s) active\n", sender.toString().c_str());
			std::map<string,MsgProcess*>::iterator it;
			MsgProcess* msgProc = NULL;
			for(it = m_gwlist.begin(); it != m_gwlist.end(); it++)
			{
				msgProc = it->second;

				UserTransferMessagePtr requtm = UTM::create(_ReqRegisterService);
				(*requtm)[_TagGatewayName] = msgProc->getServiceName();
				m_logger.info("process (%s) register my gateway name(%s)\n", 
					msgProc->getMyID().toString().c_str(),  msgProc->getServiceName().c_str());

				Service::printConsole("process (%s) register my gateway name(%s)\n", 
					msgProc->getMyID().toString().c_str(),  msgProc->getServiceName().c_str());

				if(!postMessage(sender, requtm))
				{
					m_logger.error("[00] post message[_ReqRegisterService] FAILED\n");
				}
			}
		}
		break;
	case AppPort::_apGateway:
		{
			string command		= utm[_TagCommand];
			string serviceName	= utm[_TagGatewayName];
			u32	 status			= (u32)(utm[_TagStatus]);
			if(0 == command.compare("_ReqRegMGPClientStatus"))
			{
				
				
					UserTransferMessagePtr resutm = UTM::create(this->myIdentifier(0),sender,_ResGWService);
					(*resutm)[_TagCommand] = "_ResRegMGPClientStatus";
					(*resutm)[_TagGatewayName] = serviceName;
				
					(*resutm)[_TagResult] = (u32)0;
									
					if(!postMessage(sender, resutm))
					{
						m_logger.error("[00] post message[_ResRegMGPClientStatus] FAILED\n");
					}
				

			}
		}
		break;
	default:
		m_logger.info("[00] (%s) active\n", sender.toString().c_str());
	}
}


void MsgServiceSK::onModuleDeactive(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	if(sender.m_appport == AppPort::_apSlee)
	{
		m_logger.info("[00] slee(%s) deactive\n", sender.toString().c_str());
	}
	else
	{
		m_logger.info("[00] (%s) deactive\n", sender.toString().c_str());
	}

	if(sender.m_appport == AppPort::_apGateway)
	{
		

		UserTransferMessagePtr resutm = UTM::create(sender,this->myIdentifier(0),_ResGWService);
		(*resutm)[_TagCommand] = "_ResRegMGPClientStatus";
		//(*resutm)[_TagGatewayName] = serviceName;
		(*resutm)[_TagResult] = (u32)1; // regfailed
		
		// broadcast myself  clients
		
		onDistributeAll(sender, *resutm);


		//try notify sender   
		if(!postMessage(sender, *resutm))
		{
			m_logger.info("[00] post message[_ResRegMGPClientStatus] to (%d) FAILED\n",sender.m_appid);
		}else 
		{
			m_logger.info("[00] post message[_ResRegMGPClientStatus] to (%d) success\n",sender.m_appid);
		}
		
	}
	
}

void MsgServiceSK::onRegisterServiceResponse(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	if(utm.getReturn() == 0)
	{
		m_logger.info("register ok\n");
		string serviceName = utm[_TagGatewayName];
		MsgProcess* proc = findProcess(serviceName);
		if(proc != NULL)
		{
			proc->addSieList(sender);
		}
	}
	else
		m_logger.info("register failed, %d\n", utm.getReturn());
}

MsgProcess* MsgServiceSK::findProcess(const string serviceName)
{
	if(serviceName.length() <= 0)
		return NULL;

	std::map<string,MsgProcess*>::iterator it;
	it=	m_gwlist.find(serviceName);

	if(it != m_gwlist.end())
	{
		return it->second;
	}

	return NULL;
}

void MsgServiceSK::onDispatchSMS(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	string serviceName = utm[_TagGatewayName];
	MsgProcess* proc = findProcess(serviceName);
	if(proc != NULL)
	{
		proc->putReq(sender, utm);
		return;
	}
}


void MsgServiceSK::onDispatchSMSReport(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	string serviceName = utm[_TagGatewayName];
	MsgProcess* proc = findProcess(serviceName);
	if(proc != NULL)
	{
		proc->putReq(sender, utm);
		return;
	}
}



void MsgServiceSK::onDistributeRequest(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	string serviceName = utm[_TagGatewayName];
	MsgProcess* proc = findProcess(serviceName);
	if(proc != NULL)
	{
		proc->putReq(sender, utm);
		return;
	}
	utm.setRes(utm.getReq());
	utm.setReq(myIdentifier());
	utm.setReturn(-1);
	switch(utm.getMessageId())
	{
	case _ReqSendSMS:
		utm.setMessageId(_ResSendSMS);
		m_logger.info("[00] failed to SendSMS by ServiceName(%s)  from (%s)\n",
			serviceName.c_str(), 
			sender.toString().c_str());
		break;

	case _ReqGWService:
		utm.setMessageId(_ResGWService);
		m_logger.info("[00] failed to ReqGWService by ServiceName(%s) from (%s)\n",
			serviceName.c_str(), 
			sender.toString().c_str());
		break;

	case _ResStartFlow:
		break;
	case _ResGWService:
		onDistributeAll(sender,utm);
		return;
	case _ReqQuerySendWindowSize:
		m_logger.info("[00] failed to ReqQuerySendWindowSize by ServiceName(%s) from (%s)\n",
			serviceName.c_str(), 
			sender.toString().c_str());
		return ;
	default:
		m_logger.info("[00] failed to distribute (%s, 0x%x) from (%s)\n",
			serviceName.c_str(),
			utm.getMessageId(), 
			sender.toString().c_str());
		return;
	}

	postMessage(sender, utm);
}

void MsgServiceSK::onActiveTest(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	string serviceName = utm[_TagGatewayName];
	MsgProcess* proc = findProcess(serviceName);
	if(proc != NULL)
	{
		proc->putReq(sender, utm);
	}
}

void MsgServiceSK::onMyTest(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	string serviceName = utm[_TagGatewayName];
	MsgProcess* proc = findProcess(serviceName);
	if(proc != NULL)
	{
		proc->putReq(sender, utm);
	}
}


void MsgServiceSK::onSnapshot(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	onDistributeAll(sender, utm);
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	map<string,FlowSequence>::iterator it =  m_flowSequenceMap.begin();
	while ( it != m_flowSequenceMap.end())
	{
		printf("lowSequence:(%s)------(%s)\n",((*it).first).c_str(),((*it).second).m_seq.c_str());
		it ++;
	}
}

void MsgServiceSK::onDistributeAll(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	std::map<string,MsgProcess*>::iterator it;
	MsgProcess* msgProc = NULL;
	for(it = m_gwlist.begin(); it != m_gwlist.end(); it++)
	{
		msgProc = it->second;
		msgProc->putReq(sender, utm);
	}
}
void MsgServiceSK::onEvtTimeOutException(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	map<string,FlowSequence>::iterator it =  m_flowSequenceMap.begin();
	while(it != m_flowSequenceMap.end())
	{
		FlowSequence temp = (*it).second;
		temp.m_state++;
		if (temp.m_state == FlowSeqRemoveTime)
		{
			it = m_flowSequenceMap.erase(it);
		}else
		{
			it ++;
		}
	}
	UserTransferMessagePtr checkUtm = UTM::create(_EvtTimeOutException);
	setTimer(FlowSeqCheckTime,checkUtm);
}
void MsgServiceSK::addFlowSequence(string sequence,string flowSequence)
{
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	FlowSequence fseq(flowSequence,0);
	m_flowSequenceMap.insert(make_pair(sequence,fseq));
}
string MsgServiceSK::getFlowSequence(string sequence)
{
	cacti::RecursiveMutex::ScopedLock  scopedlock(m_lock);
	map<string,FlowSequence>::iterator it = m_flowSequenceMap.find(sequence);
	if (it != m_flowSequenceMap.end())
	{
		FlowSequence temp = (*it).second;
		m_flowSequenceMap.erase(it);
		return temp.m_seq;
	}
	return "";
}
