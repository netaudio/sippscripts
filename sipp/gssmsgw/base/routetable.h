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
	std::string prefix;				//�ֹ�
	int			minLen;				//��С����
	int			maxLen;				//��󳤶�
	std::string serviceCode;		//�������
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
	std::string	serviceCode;		//�������
	std::string ownerName;			//��������(SP����)
	int			serviceID;			//���������ģ��ID
	int			serviceType;		//���������ģ������
	int			flowID;				//���̺�
	int			accounting;			//�Ƿ�Ʒ�
	std::string billItem;			//�Ʒѱ���
	int			billType;			//�Ʒ�����
	int			fluxControl;		//�Ƿ������������
	int			fluxType;			//��������
	int			fluxUnit;			//������λ
	int			fluxSend;			//��������
	int			fluxRecv;			//��������
	int			priority;			//���ȼ�
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
	PrefixList		m_PrefixList;		//<�ֹ�, �ֹ���Ϣ>
	ServiceList		m_ServiceList;		//<����, ������Ϣ>
	RouteList		m_RouteList;		//<�ֹ�, ������Ϣ>

	RouteList		m_PrefixCache;		//<�ֹ�, ������Ϣ>
	RouteList		m_NumberCache;		//<����, ������Ϣ>

	int				m_minLen;
	int				m_maxLen;

	std::string		m_ConfigFileName;
	Mutex			m_lock;
};

#endif