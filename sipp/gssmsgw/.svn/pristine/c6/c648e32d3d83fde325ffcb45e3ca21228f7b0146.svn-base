#ifndef __BASE_PROTOCOL_H
#define __BASE_PROTOCOL_H
#pragma once

#include <cacti/message/Stream.h>
#include <cacti/message/ServiceIdentifier.h>
#include <cacti/message/TransferMessage.h>
#include <cacti/util/MD5.h>
#include <cacti/util/Timestamp.h>
#include <cacti/kernel/SysObj.h>
#include <alarm/AlarmSender.h>

#include "startup/service.h"

#include <string>


using namespace std;
using namespace cacti;

int DecodeUcs2(const unsigned char   *pSrc,   char   *pDst,   int   nSrcLength,  int   nDstMaxLength);
int EncodeUcs2(const   char   *pSrc,   unsigned   char   *pDst,   int   nSrcLength,  int   nDstMaxLength);
int DecodeUcs2(const string&  src, string& dst);
int EncodeUcs2(const string&  src, string& dst);
int split_gbk_string(string& src, string& dst, int begin, int len);

class HandlerInterface
{
public:
	virtual bool  putReq(const ServiceIdentifier& sender, UserTransferMessage& utm) = 0;
	virtual bool  postMessage(const ServiceIdentifier& receiver, UserTransferMessage& utm) = 0;
	virtual 	string getFlowSequence(string sequence) = 0;
	virtual void addFlowSequence(string sequence,string flowSequence) = 0;
};

class ServerInterface
{
public:
	virtual void	handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm) = 0;
	virtual void	handleClientMsg(cacti::Stream& content) = 0;
};


struct  StatPackets
{
	time_t		startTime;            //开始统计时间
	u32			cycleTime;            //流量统计周期,单位秒
	u32			sendErrorPackets;     //失败发送的包
	u32			recvErrorPackets;     //收到的错误包
	u32			otherErrorPackets;    //收到的转发错误包
	u32			lostPackets;          //发送没有回的包
	u32         recvTimeoutPackets;   //收到的超时的包
	u32			totalSendPackets;     //总共发送包数
	u32			totalRecvPackets;     //总共接收包数
	u32			avgSendPackets;       //发送流量
	u32			avgRecvPackets;       //接收流量
	u32			totalSendLinkPackets; //总共发送链路请求包
	u32			totalReckLinkPackets; //总共接收链路请求包
	u32			requestPackets[1000]; //请求消息记录
	u32			responsePackets[1000];//应答消息记录

	StatPackets()
	{
		startTime				= 0;
		cycleTime				= 0;
		sendErrorPackets		= 0;
		recvErrorPackets		= 0;
		otherErrorPackets		= 0;
		lostPackets				= 0;
		recvTimeoutPackets		= 0;
		totalSendPackets		= 0;
		totalRecvPackets		= 0;
		avgSendPackets			= 0;
		avgRecvPackets			= 0;
		totalSendLinkPackets	= 0;
		totalReckLinkPackets	= 0;
		memset(requestPackets, 0, sizeof(u32)*1000);
		memset(responsePackets, 0, sizeof(u32)*1000);
	}
};

class CachedPacket
{
public:
	CachedPacket();
	virtual ~CachedPacket();

public:
	u32 packetID;
	int	packetType;
	ServiceIdentifier source;
	UserTransferMessagePtr utm;

	cacti::Timestamp   addStamp;

	string exchangeID;
	void*  userData;
};

typedef boost::shared_ptr<CachedPacket> CachedPacketPtr;
typedef std::map<string, CachedPacketPtr> CachedPacketList;

class PacketCacheControl
{
public:
	PacketCacheControl(int maxPacket = 2048, int liveTime=3600);
	virtual ~PacketCacheControl();

	bool addPacket(string packetID, CachedPacketPtr pkg);
	CachedPacket* getPacket(string packetID);
	void deletePacket(string packetID);
	
	bool addPacket(u32 packetID, CachedPacketPtr pkg);
	CachedPacket* getPacket(u32 packetID);
	void deletePacket(u32 packetID);
	
	bool changePacketID(u32 oldID, string newID);
	bool changePacketID(string oldID, u32 newID);

	int deleteExpirePacket();
	int getSize();
	void deleteAll();
	void setLiveTime(int timeout) { m_liveTime = timeout ;};

private:
	CachedPacketList m_cacheList;
	int m_maxPacket;
	int m_liveTime;
	cacti::Timestamp m_lastDeleteTime;
	RecursiveMutex  m_pkgMutex;
};

class BaseProtocol
{
public:
	BaseProtocol(
		const ServiceIdentifier& myId, 
		const string & configFile, 
		HandlerInterface* handler);
	virtual ~BaseProtocol() { };

public:
	virtual bool	open()       { return false; };
	virtual bool	close()      { return false; };
	virtual bool	activeTest() { return false; };
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm) { return -1; }
	virtual void	snapshot() {};
	virtual void	handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm) {};

	void			postToOwner(const ServiceIdentifier& sender, UserTransferMessagePtr utm);

	u32				getStatus()  { return m_status; };
	void			setStatus(u32 status)  { m_status = status; }
	bool			isReady()    { return (m_status == ST_READY ? true: false); };
	void			string2Vector( u8array& v, const string & s) {	memcpy(&v[0],&s[0],v.size() <= s.length() ? v.size() : s.length());}
	string			getServiceName() { return m_serviceName; };
	u32				getMyID() { return m_myID.m_appref; };
	ServiceIdentifier getServiceID() { return m_myID; };
	string getExNameID() { return m_exName; };
	void			setActiveTestTime(u32 actime) { m_activeTestTime = actime; }
	string			getProtocol(){return m_protocol;}

	u32		getSourceId(){return m_sourceId;};
	string getTestNum(){return m_testNumber;};
	enum 
	{
		ST_NONE         = 0,
		ST_INIT         = 1,
		ST_READY        = 2,
		ST_BUSY         = 3,
		ST_ERROR        = 4,
	};

	void			postMessage(const ServiceIdentifier& recevier, UserTransferMessage& utm);

protected:
	virtual void	loadConfig();
//	void			postMessage(const ServiceIdentifier& recevier, UserTransferMessage& utm);

	bool			recordPacketsNum(u32 cmd);

	string getFlowSequence(string sequence);
	void addFlowSequence(string sequence,string flowSequence);

	ServiceIdentifier     m_myID;
	string                m_configFile;
	string				  m_serviceName;

	string                m_exName;
	string                m_serverIP;
	short                 m_serverPort;
	short                 m_serverPortMt;
	string				 m_clientIP;
	short			m_clientPort;	
	u8			m_loginType;//登陆类型
	string                m_userName;
	string                m_userPassword;
	string                m_serviceCode;	//默认服务代码
	int                   m_connectionNum;	//连接数		 
	u8					  m_version;		//协议版本
	string				  m_spId;			//SP代码	
	u32					m_sourceId;
	int					  m_checkTimes;		//超时检查次数
	int					  m_checkTransfer;	//是否检查转发包格式
	int					  m_recvTimeout;	//收包超时
	int					  m_multiPacket;	//是否支持单请求多个回应包
	u32                   m_activeTestTime; //连接检查周期，秒
	
	time_t                m_lastRecvTime; //最后收到消息时间
	time_t                m_startTime;    //启动时间

	StatPackets           m_statPackets;  //收发包统计

	u32					m_changLinkId; //MGPCLIENT特有
	string				m_protocol;
	string				m_testNumber;
private:
	u32                   m_status;
	HandlerInterface*     m_handler;
};

typedef boost::shared_ptr<BaseProtocol> BaseProtocolPtr;

	enum 
	{
		NONE			= 0,
		SMS_SMGP		= 1,
		//CRBT_ZTE		= 2,
		SMS_CNGP		= 2,
		SMS_CMPP		= 3,
		SMS_SGIP		= 4,
		SMS_SGIP_SERVER		= 5,
		//SMPP_SERVER		= 4,
		//CRBT_CBGP		= 5,
		//MGP_SERVER		= 6,
		//FTP_CLIENT		= 7,
		//MUSIC_PROXY		= 8,
		SMS_SC			= 6,
		//MUSIC_CBGP		= 10,//mgp client
		SMS_YLX			= 11,
		SMS_YS			= 12,
	};


class MsgHandler
{
public:
	static BaseProtocolPtr createHandler(
		int type, 
		const ServiceIdentifier& id, 
		const string & configFile,
		HandlerInterface * handler);
};

u32 makeTimestamp();
string getStrTime(time_t tagTime, const char* format);
void makeAuthCode(std::string &code, const char* clientID, const char *secret, u32 stamp,int protocol);


bool	getParameter(string& outValue, const char* paramName,const UserTransferMessage& utm);
bool	getParameter(u8array& outValue, const char* paramName,const UserTransferMessage& utm);
bool	getParameter(u32& outValue, const char* paramName,const UserTransferMessage& utm);
bool	getParameter(u16& outValue, const char* paramName,const UserTransferMessage& utm);
bool	getParameter(u8& outValue, const char* paramName,const UserTransferMessage& utm);

#endif