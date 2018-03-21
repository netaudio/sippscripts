#ifndef __ROUTE_TABLE_H
#define __ROUTE_TABLE_H

#pragma once

#include "boost/shared_ptr.hpp"
#include <string>
#include <map>
#include <vector>

#include "cacti/kernel/Mutex.h"

using namespace cacti;
using namespace std;

struct PrefixInfo
{
public:
	PrefixInfo(std::string myID="")
		:prefix(myID)
		,minLen(0)
		,maxLen(0)
		,serviceCode("")
	{};

protected:

private:

public:
	std::string prefix;				//字冠
	int			minLen;				//最小长度
	int			maxLen;				//最大长度
	std::string serviceCode;		//服务代码
};

struct ServiceInfo
{
public:
	ServiceInfo(std::string myID="")
		:serviceCode(myID)
		,ownerName("")
		,serviceID(120)	
		,serviceType(50)
		,flowID(0)	
		,accounting(0)	
		,billItem("0")	
		,billType(0)	
		,fluxControl(0)
		,fluxType(0)
		,fluxUnit(0)	
		,fluxSend(0)	
		,fluxRecv(0)	
		,priority(0)	
	{};

protected:

private:

public:
	std::string	serviceCode;		//服务代码
	std::string ownerName;			//服务属主(SP代码)
	int			serviceID;			//处理本服务的模块ID
	int			serviceType;		//处理本服务的模块类型
	int			flowID;				//流程号
	int			accounting;			//是否计费
	std::string billItem;			//计费编码
	int			billType;			//计费类型
	int			fluxControl;		//是否进行流量控制
	int			fluxType;			//流量类型
	int			fluxUnit;			//流量单位
	int			fluxSend;			//发送流量
	int			fluxRecv;			//接收流量
	int			priority;			//优先级
};

typedef boost::shared_ptr<PrefixInfo> PrefixInfoPtr;
typedef std::map<std::string, PrefixInfoPtr> PrefixList;

typedef boost::shared_ptr<ServiceInfo> ServiceInfoPtr;
typedef std::map<std::string, ServiceInfoPtr> ServiceList;

typedef std::map<std::string, ServiceInfoPtr> RouteList;


class RouteTable
{
public:
	RouteTable(std::string serviceName="")
		:m_myID(serviceName)
		,m_minLen(1)
		,m_maxLen(1)
	{};

	~RouteTable();

	ServiceInfo* getRouteInfoByNumber(const char* number);
	ServiceInfo* getRouteInfoByPrefix(const char* prefix);
	ServiceInfo* getRouteInfoByCode(const char* serviceCode);

	bool			loadConfig(const char* configFileName);

protected:

private:
	bool			buildPrefixList();
	std::vector<std::string>  buildServiceCodeList();
	bool			buildRouteList();

private:
	std::string		m_myID;
	PrefixList		m_PrefixList;		//<字冠, 字冠信息>
	ServiceList		m_ServiceList;		//<服务, 服务信息>
	RouteList		m_RouteList;		//<字冠, 服务信息>

	RouteList		m_PrefixCache;		//<字冠, 服务信息>
	RouteList		m_NumberCache;		//<号码, 服务信息>

	int				m_minLen;
	int				m_maxLen;

	std::string		m_ConfigFileName;
	Mutex			m_lock;
};

#endif