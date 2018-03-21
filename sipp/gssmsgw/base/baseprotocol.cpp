#include "stdafx.h"

#include <cacti/util/IniFile.h>
#include <sys/timeb.h>
#ifndef WIN32
#include <sys/time.h>
#endif

#include "baseprotocol.h"
#include "smgp/smgpprotocol.h"

#include "cngp/cngpprotocol.h"
#include "smpp/smppprotocol.h"
#include "cmpp/cmppprotocol.h"
#include "sgip/sgipprotocol.h"
#include "scsms/scsmsprotocol.h"
#include "sgip/sgipserverprotocol.h"
#include "ylxsms/ylxserverprotocol.h"
#include "ysms/ysmsprotocol.h"

#include "tag.h"


#define new DEBUG_NEW

//------------------------------------------------------------------CachedPacket
CachedPacket::CachedPacket()
:addStamp(time(0))
,userData(NULL)
{

}

CachedPacket::~CachedPacket()
{
	if(userData != NULL)
	{
		delete userData;
		userData = NULL;
	}
}

//------------------------------------------------------------PacketCacheControl
PacketCacheControl::PacketCacheControl(int maxPacket, int liveTime)
:m_maxPacket(maxPacket),
m_liveTime(liveTime)
{

}

PacketCacheControl::~PacketCacheControl()
{
	deleteAll();
}

bool PacketCacheControl::addPacket(string packetID, CachedPacketPtr pkg)
{
	if(m_cacheList.size() > m_maxPacket)
	{
		deleteExpirePacket();
	}

	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	CachedPacketList::iterator it = m_cacheList.find(packetID);
	if(it == m_cacheList.end())
	{
		m_cacheList.insert(std::make_pair(packetID, pkg));
		return true;
	}

	return false;
}

CachedPacket* PacketCacheControl::getPacket(string packetID)
{
	CachedPacketList::iterator it = m_cacheList.find(packetID);
	if(it != m_cacheList.end())
	{
		return (it->second).get();
	}
	
	return NULL;
}

void PacketCacheControl::deletePacket(string packetID)
{
	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	m_cacheList.erase(packetID);
	m_lastDeleteTime = cacti::Timestamp(time(0));
}

bool PacketCacheControl::addPacket(u32 packetID, CachedPacketPtr pkg)
{
	char acTemp[21] = {0};
	sprintf_s(acTemp, 20, "%u", packetID);
	
	return addPacket(acTemp, pkg);
}

CachedPacket* PacketCacheControl::getPacket(u32 packetID)
{
	char acTemp[21] = {0};
	sprintf_s(acTemp, 20, "%u", packetID);
	
	return getPacket(acTemp);
}

void PacketCacheControl::deletePacket(u32 packetID)
{
	char acTemp[21] = {0};
	sprintf_s(acTemp, 20, "%u", packetID);

	deletePacket(acTemp);
}

bool PacketCacheControl::changePacketID(u32 oldID, string newID)
{
	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	char acTemp[21] = {0};
	sprintf_s(acTemp, 20, "%u", oldID);

	CachedPacketList::iterator it = m_cacheList.find(acTemp);
	if(it == m_cacheList.end())
	{
		return false;
	}
	
	CachedPacketPtr pkg = it->second;
	m_cacheList.erase(acTemp);	
	m_cacheList.insert(std::make_pair(newID, pkg));

	return true;
}

bool PacketCacheControl::changePacketID(string oldID, u32 newID)
{
	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	CachedPacketList::iterator it = m_cacheList.find(oldID);
	if(it == m_cacheList.end())
	{
		return false;
	}

	char acTemp[21] = {0};
	sprintf_s(acTemp, 20, "%u", newID);

	CachedPacketPtr pkg = it->second;
	m_cacheList.erase(oldID);	
	m_cacheList.insert(std::make_pair(acTemp, pkg));

	return true;
}

int PacketCacheControl::getSize()
{
	return m_cacheList.size();
}

int PacketCacheControl::deleteExpirePacket()
{
	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	int ret = 0;
	cacti::Timestamp curTime(time(0));

	CachedPacketList::iterator it = m_cacheList.begin();
	while(it != m_cacheList.end())
	{
		if( (curTime - (*(it->second)).addStamp) > m_liveTime )
		{
			m_cacheList.erase(it++);
			ret ++;
		}
		else
		{
			++it;
		}
	}

	return ret;
}

void PacketCacheControl::deleteAll()
{
	RecursiveMutex::ScopedLock lock(m_pkgMutex);

	m_cacheList.clear();
	m_lastDeleteTime = cacti::Timestamp(time(0));
}

//------------------------------------------------------------------MsgHandler
BaseProtocolPtr MsgHandler::createHandler(int type, 
										  const ServiceIdentifier& id, 
										  const string & configFile,
										  HandlerInterface * handler)
{
	assert(handler);
	switch(type)
	{
	case SMS_SMGP:
		return BaseProtocolPtr(new SMGPProtocol(id, configFile, handler));

	case SMS_CNGP:
		return BaseProtocolPtr(new CNGPProtocol(id, configFile, handler));
	//case SMPP_SERVER:
	//	return BaseProtocolPtr(new SMPPProtocol(id, configFile, handler));
	case SMS_CMPP:
		return BaseProtocolPtr(new CMPPProtocol(id, configFile, handler));

	case SMS_SGIP:
		{
			return BaseProtocolPtr(new SGIPProtocol(id, configFile, handler));
		}
	case SMS_SGIP_SERVER:
		return BaseProtocolPtr(new SGIPServerProtocol(id, configFile, handler));

	case SMS_SC:
		return BaseProtocolPtr(new SCSMSProtocol(id, configFile, handler));

	case SMS_YLX:
		return BaseProtocolPtr(new YLXServerProtocol(id, configFile, handler));

	case SMS_YS:
		return BaseProtocolPtr(new YSMSProtocol(id, configFile, handler));

	default:
		return BaseProtocolPtr(new BaseProtocol(id, configFile, handler));
	}
}

//----------------------------------------------------------------BaseProtocol
BaseProtocol::BaseProtocol(const ServiceIdentifier& myId
						   , const string & configFile
						   , HandlerInterface * handler)
:m_myID(myId)
,m_configFile(configFile)
,m_handler(handler)
,m_status(ST_NONE)
,m_checkTimes(0)
,m_checkTransfer(1)
,m_recvTimeout(60)
,m_multiPacket(0)
,m_startTime(time(0))
{

}

void BaseProtocol::loadConfig()
{
	cacti::IniFile iniFile;
	if(!iniFile.open(m_configFile.c_str()))
		return;

	char key[256] = {0};
	sprintf(key,"client.%d.name", m_myID.m_appref);
	m_serviceName = iniFile.readString("gateway", key, "");

	sprintf(key,"client.%d.exName", m_myID.m_appref);
	m_exName = iniFile.readString("gateway", key, "");
	
	sprintf(key, "client.%d.serverIP", m_myID.m_appref);
	m_serverIP     = iniFile.readString("gateway", key, "127.0.0.1");
	
	sprintf(key, "client.%d.serverPort", m_myID.m_appref);
	m_serverPort   = iniFile.readInt("gateway", key, 9999);

	sprintf(key, "client.%d.serverPortMt", m_myID.m_appref);
	m_serverPortMt   = iniFile.readInt("gateway", key, 9999);

	sprintf(key, "client.%d.clientIP", m_myID.m_appref);
	m_clientIP     = iniFile.readString("gateway", key, "127.0.0.1");

	sprintf(key, "client.%d.clientPort", m_myID.m_appref);
	m_clientPort   = iniFile.readInt("gateway", key, 0);

	sprintf(key, "client.%d.changeLinkId", m_myID.m_appref);
	m_changLinkId   = iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.loginType", m_myID.m_appref);
	m_loginType     = iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.userName", m_myID.m_appref);
	m_userName     = iniFile.readString("gateway", key, "admin");

	sprintf(key, "client.%d.protocol", m_myID.m_appref);
	m_protocol     = iniFile.readString("gateway", key, "SMGP");

	sprintf(key, "client.%d.userPassword", m_myID.m_appref);
	m_userPassword = iniFile.readString("gateway", key, "123456");

	sprintf(key, "client.%d.serviceCode", m_myID.m_appref);
	m_serviceCode  = iniFile.readString("gateway", key, "");

	sprintf(key, "client.%d.testNumber", m_myID.m_appref);
	m_testNumber  = iniFile.readString("gateway", key, "");

	sprintf(key, "client.%d.connection", m_myID.m_appref);
	m_connectionNum= iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.version", m_myID.m_appref);
	m_version	= (u8) iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.spID", m_myID.m_appref);
	m_spId	= iniFile.readString("gateway",key,"168168");

	sprintf(key, "client.%d.sourceID", m_myID.m_appref);
	m_sourceId	= iniFile.readInt("gateway",key,0);

	sprintf(key, "client.%d.checkTimes", m_myID.m_appref);
	m_checkTimes = iniFile.readInt("gateway", key, 3);
	if(m_checkTimes >= 60 || m_checkTimes <= 0)
	{
		m_checkTimes = 2;
	}

	sprintf(key, "client.%d.checkTransfer", m_myID.m_appref);
	m_checkTransfer = iniFile.readInt("gateway", key, 1);

	sprintf(key, "client.%d.recvTimeout", m_myID.m_appref);
	m_recvTimeout = iniFile.readInt("gateway", key, 30);
	if(m_recvTimeout > 180 || m_recvTimeout <= 0)
	{
		m_recvTimeout = 30;
	}

	sprintf(key, "client.%d.multiPacket", m_myID.m_appref);
	m_multiPacket = iniFile.readInt("gateway", key, 0);

}

void  BaseProtocol::postToOwner(const ServiceIdentifier& sender, UserTransferMessagePtr utm)
{
	m_handler->putReq(sender, *utm.get());
};


string BaseProtocol::getFlowSequence(string sequence)
{
	return m_handler->getFlowSequence(sequence);
}

void BaseProtocol::addFlowSequence(string sequence,string flowSequence)
{
	m_handler->addFlowSequence(sequence,flowSequence);
}

void  BaseProtocol::postMessage(const ServiceIdentifier& recevier, UserTransferMessage& utm)
{
	m_handler->postMessage(recevier, utm);
};

bool BaseProtocol::recordPacketsNum(u32 cmd)
{
	if( (cmd & 0xFF000000) == 0x03000000 && (cmd & 0x0000FFFF) < 0x0999 )
	{
		u32 index =   cmd & 0x0000000F;
		index    += ((cmd & 0x000000F0) >> 4) * 10;
		index    += ((cmd & 0x00000F00) >> 8) * 100;

		if((cmd & 0x000F0000) == 0x00010000)
		{
			m_statPackets.requestPackets[index] ++;
		}
		else if((cmd & 0x000F0000) == 0x00020000)
		{
			m_statPackets.responsePackets[index] ++;
		}

		return true;
	}

	return false;
}

bool getParameter(std::string& outValue, const char* paramName,const UserTransferMessage& utm)
{
	if(!utm.hasKey(_TagParameterName, TLV::TYPE_ARRAY_STRING)
		|| !utm.hasKey(_TagParameterValue, TLV::TYPE_ARRAY_STRING))
		return false;

	std::vector<std::string> m_paramName = utm[_TagParameterName];
	std::vector<std::string> m_paramValues = utm[_TagParameterValue];
	if(m_paramName.size() != m_paramValues.size())
		return false;

	std::vector<std::string>::iterator iter = std::find(m_paramName.begin(), m_paramName.end(), paramName);
	if(iter == m_paramName.end())
		return false;

	outValue = m_paramValues[iter - m_paramName.begin()];
	
	return true;
}

bool getParameter(u8array& outValue, const char* paramName,const UserTransferMessage& utm)
{
	std::string tmp;
	if(!getParameter(tmp, paramName, utm))
	{
		return false;
	}

	outValue.assign(tmp.begin(), tmp.end());
	return true;
}

bool getParameter(u32& outValue, const char* paramName,const UserTransferMessage& utm)
{
	std::string tmp;
	if(!getParameter(tmp, paramName, utm))
	{
		return false;
	}

	outValue = atoi(tmp.c_str());
	return true;
}

bool getParameter(u16& outValue, const char* paramName,const UserTransferMessage& utm)
{
	std::string tmp;
	if(!getParameter(tmp, paramName, utm))
	{
		return false;
	}

	outValue = atoi(tmp.c_str());
	return true;
}

bool getParameter(u8& outValue, const char* paramName,const UserTransferMessage& utm)
{
	std::string tmp;
	if(!getParameter(tmp, paramName, utm))
	{
		return false;
	}

	outValue = atoi(tmp.c_str());
	return true;
}

u32 makeTimestamp()
{
	time_t now = time(0);
	tm     tmnow;

#if defined(WIN32)
	tmnow = *localtime(&now);
#else
	localtime_r(&now, &tmnow);
#endif

	char tmp[32];
	sprintf(tmp, "%02d%02d%02d%02d%02d",
		tmnow.tm_mon+1,
		tmnow.tm_mday,
		tmnow.tm_hour,
		tmnow.tm_min,
		tmnow.tm_sec);

	return atoi(tmp);
}

void makeAuthCode(std::string &code, const char* clientID, const char *secret, 
							u32 stamp, int protocol)
{
	char md5[16+1] = {0};
	char msg[256]  = {0};
	char *ptr = msg;
	int maxClientIdLen;

	if(SMS_CNGP == protocol )
		maxClientIdLen = 10;
	else if(SMS_CMPP==protocol){
		maxClientIdLen=15;
	}
	else
		maxClientIdLen = 8;

	strncpy(ptr, clientID,maxClientIdLen);
	ptr += strlen(clientID);

	ptr += 7;

	strncpy(ptr, secret, 15);
	ptr += strlen(secret);

	sprintf(ptr, "%010d", stamp);
	ptr += 10;

	md5digest(md5, msg, ptr-msg);
	code = md5;
}

string getStrTime(time_t tagTime, const char* format)
{
	assert(format);

	tm     tmTmp;

#if defined(WIN32)
	tmTmp = *localtime(&tagTime);
#else
	localtime_r(&tagTime, &tmTmp);
#endif

	char buf[32] = {0};
	size_t len = ::strftime(buf, 31, format, &tmTmp);

	buf[len] = '\0';
	return buf;
}