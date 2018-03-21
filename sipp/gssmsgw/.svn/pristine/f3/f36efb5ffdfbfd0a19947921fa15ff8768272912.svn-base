#include "stdafx.h"

#include <cacti/util/IniFile.h>
#include "cacti/util/StringUtil.h"
#include "routetable.h"

#define new DEBUG_NEW


RouteTable::~RouteTable()
{
// 	m_PrefixList.clear();
// 	m_ServiceList.clear();
// 	m_RouteList.clear();
}

bool RouteTable::loadConfig(const char* configFileName)
{
	Mutex::ScopedLock m_scopelock(m_lock);

	m_ConfigFileName = configFileName;
	
	cacti::IniFile iniFile;
	if(!iniFile.open(m_ConfigFileName.c_str())
		|| m_myID.length() <= 0)
		return false;

	m_PrefixList.clear();
	m_ServiceList.clear();
	m_RouteList.clear();

	int i				= 0;
	int iNext			= 0;
	char acKey[256]		= {0};
	std::string prefix	= "";
	int minLen			= 0;
	int maxLen			= 0;

	while(true)
	{
		sprintf(acKey,"%s.%d.next", m_myID.c_str(), i);
		iNext = iniFile.readInt("route", acKey, 0);
		if(iNext == 0 || iNext <= i)
			break;

		sprintf(acKey,"%s.%d.prefix", m_myID.c_str(), iNext);
		prefix = iniFile.readString("route", acKey, "");
		if("" == prefix)
			break;

		sprintf(acKey,"%s.%d.minLen", m_myID.c_str(), iNext);
		minLen = iniFile.readInt("route", acKey, 0);

		sprintf(acKey,"%s.%d.maxLen", m_myID.c_str(), iNext);
		maxLen = iniFile.readInt("route", acKey, 0);

		if(minLen <= 0 || maxLen<= 0 || maxLen > 32)
		{
			i++;
			continue;
		}

		if(m_RouteList.find(prefix) != m_RouteList.end())
		{
			i++;
			continue;
		}

		PrefixInfoPtr info = PrefixInfoPtr(new PrefixInfo());
		m_PrefixList[prefix] = info;
		info->prefix = prefix;
		info->minLen = minLen;
		info->maxLen = maxLen;
		
		sprintf(acKey,"%s.%d.serviceCode", m_myID.c_str(), iNext);
		info->serviceCode = iniFile.readString("route", acKey, "");

		if(minLen < m_minLen)
			m_minLen = minLen;

		if(maxLen > m_maxLen)
			m_maxLen = maxLen;

		i++;
	}

	std::vector<std::string> scList = buildServiceCodeList();
	if(scList.size() <= 0)
		return false;

	unsigned int j = 0;
	const char* scCode = NULL;
	std::string ownerName = "";
	u32	serviceID = 0;
	u32 serviceType = 0;

	for(j = 0; j < scList.size(); j++)
	{
		if(scList[j].length() > 200)
			continue;

		scCode = scList[j].c_str();

		sprintf(acKey,"%s.%s.ownerName", m_myID.c_str(), scCode);
		ownerName = iniFile.readString("service", acKey, "");
		if("" == ownerName)
			continue;
		
		sprintf(acKey,"%s.%s.serviceID", m_myID.c_str(), scCode);
		serviceID = iniFile.readInt("service", acKey, 0);

		if(0 == serviceID)
			continue;

		sprintf(acKey,"%s.%s.serviceType", m_myID.c_str(), scCode);
		serviceType = iniFile.readInt("service", acKey, 0);

		if(0 == serviceType)
			continue;

		ServiceInfoPtr service = ServiceInfoPtr(new ServiceInfo(scList[j]));
		m_ServiceList[(scList[j])] = service;
		service->ownerName = ownerName;
		service->serviceID = serviceID;
		service->serviceType = serviceType;

		sprintf(acKey,"%s.%s.flowID", m_myID.c_str(), scCode);
		service->flowID = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.accounting", m_myID.c_str(), scCode);
		service->accounting = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.billItem", m_myID.c_str(), scCode);
		service->billItem = iniFile.readString("service", acKey, "");

		sprintf(acKey,"%s.%s.billType", m_myID.c_str(), scCode);
		service->billType = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.fluxControl", m_myID.c_str(), scCode);
		service->fluxControl = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.fluxType", m_myID.c_str(), scCode);
		service->fluxType = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.fluxUnit", m_myID.c_str(), scCode);
		service->fluxUnit = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.fluxSend", m_myID.c_str(), scCode);
		service->fluxSend = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.fluxRecv", m_myID.c_str(), scCode);
		service->fluxRecv = iniFile.readInt("service", acKey, 0);

		sprintf(acKey,"%s.%s.priority", m_myID.c_str(), scCode);
		service->priority = iniFile.readInt("service", acKey, 0);
	}

	buildRouteList();

	return true;
}

std::vector<std::string>  RouteTable::buildServiceCodeList()
{
	std::vector<std::string> scList;//std::set ?
	if(m_PrefixList.size() <= 0)
	{
		return scList;
	}

	PrefixList::iterator it;
	PrefixInfo* info = NULL;
	for(it = m_PrefixList.begin(); it != m_PrefixList.end(); it++)
	{
		info = it->second.get();
		if(info->prefix.length() > 0
			&& info->serviceCode.length() > 0)
		{
			std::vector<std::string> scBuffer;
			StringUtil::split(scBuffer, info->serviceCode, ".,\t ");
			for(unsigned int i = 0; i < scBuffer.size(); i++)
			{
				scList.push_back(scBuffer[i]);
			}
		}
	}
	
	if(scList.size() > 0)
	{
		std::sort(scList.begin(), scList.end());
		scList.erase(std::unique(scList.begin(), scList.end()), scList.end());
	}

	return scList;
}

bool RouteTable::buildRouteList()
{
	m_RouteList.clear();
	PrefixList::iterator it;
	PrefixInfo* info = NULL;
	for(it = m_PrefixList.begin(); it != m_PrefixList.end(); it++)
	{
		info = it->second.get();
		if(info->prefix.length() > 0
			&& info->serviceCode.length() > 0)
		{
			std::vector<std::string> scBuffer;
			StringUtil::split(scBuffer, info->serviceCode, ".,\t ");
			for(unsigned int i = 0; i < scBuffer.size(); i++)
			{
				if(m_ServiceList.find(scBuffer[i]) != m_ServiceList.end())
				{
					m_RouteList[info->prefix] = m_ServiceList[(scBuffer[i])]; //to do: change to insert
				}
			}
		}
	}

	return true;
}

ServiceInfo* RouteTable::getRouteInfoByNumber(const char* number)
{
	int len = strlen(number);
	if(len < m_minLen)
		return NULL;

	RouteList::iterator it;
	if(m_NumberCache.size() > 0)
	{
		it = m_NumberCache.find(number);
		if(it != m_NumberCache.end())
			return (it->second).get();
	}

	//应急写法，待有时间再改，没有判断字冠最小和最大合法性
	it = m_RouteList.find(number);
	if(it != m_RouteList.end())
		return (it->second).get();

	int i = m_maxLen;
	std::string prefix = "";
	ServiceInfo* info = NULL;

	for(; i >= m_minLen; i--)
	{
		prefix = std::string(number, 0, i);
		info = getRouteInfoByPrefix(prefix.c_str());
		if(info != NULL)
		{
			if(m_NumberCache.size() > 100)
			{
				m_NumberCache.erase(m_NumberCache.end());
			}
			m_NumberCache[number] = m_ServiceList[info->serviceCode];//to do: judge exist
			return info;
		}
	}

	return NULL;
}

ServiceInfo* RouteTable::getRouteInfoByPrefix(const char* prefix)
{
//	Mutex::ScopedLock m_scopelock(m_lock);
	RouteList::iterator it;
	if(m_PrefixCache.size() > 0)
	{
		it = m_PrefixCache.find(prefix);
		if(it != m_PrefixCache.end())
			return (it->second).get();
	}

	it = m_RouteList.find(prefix);
	if(it != m_RouteList.end())
	{
		if(m_PrefixCache.size() > 10)
		{
			m_PrefixCache.erase(m_PrefixCache.end());
		}
		
		m_PrefixCache[prefix] = it->second;
		
		return (it->second).get();
	}

	return NULL;
}

ServiceInfo* RouteTable::getRouteInfoByCode(const char* serviceCode)
{
//	Mutex::ScopedLock m_scopelock(m_lock);
	if(strlen(serviceCode) <= 0)
		return NULL;

	string scCode = serviceCode;
	if(m_ServiceList.find(scCode) != m_ServiceList.end())
	{
		return m_RouteList[scCode].get();
	}

	return NULL;
}

