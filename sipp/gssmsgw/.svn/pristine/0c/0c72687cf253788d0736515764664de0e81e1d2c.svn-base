#ifndef __CMPP_PROTOCOL_H
#define __CMPP_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"

#include "cmppdef.h"
#include "cmpppacket.h"

using namespace std;
using namespace cacti;
using namespace CMPP;

/*
ken:缓存中的packettype主要是应用于《电信网关协议CMPP3.0》中规定的反向定制流程 ，目前C网业务应该已无此类业务，但为保证兼容，保留原有代码，增加类型3
if(cachePkg->packetType == 0)//发送订购关系请求成功  
{
cachePkg->packetType = 1;//等待同步订购关系LinkID

else if(cachePkg->packetType == 2)//正式下发短信成功

3 普通短消息等待submitresp回复
*/


class CMPPClientMo;
class CMPPClientMt;
class CMPPCacheControl;
class CMPPPacketCache;

typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<CMPPPacketCache> CMPPPacketCachePtr; 


class CMPPProtocol : public BaseProtocol, ServerInterface
{
	friend class CMPPClient;

public:
	CMPPProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~CMPPProtocol();


public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

	void			test(int unit);

	virtual void	snapshot();
	virtual void	handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	bool			sendPackage(CMPPPacketCachePtr pkg);
protected:               
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);
	void			process();
	int				respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret);
	int				respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,CMPPSubmitResp& pkg);

	bool			sendLogin();
	bool			sendExit();
	bool			sendActiveTest();
	bool			sendActiveTestResp(u32 seq);
	int				sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);
	int				handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);//smshttpgw 直接传过来的短信发送指令
	bool			sendDeliverResp(u64 msgId,u32 status,u32 seq);
	
	bool			handleSubmitResp(CMPPSubmitResp* pkg);
	bool			handleDeliver(CMPPDeliver* pkg);

	bool			sendPackage(CMPPPacket& pkg);
	bool			sendMtPackage(CMPPPacket& pkg);
	
	bool			sendSubmitPackage(CMPPSubmit& pkg);
	bool			transferReport(string caller,string called,string reportcontent);//转发状态报告
	bool			transferDeliver(string caller,string called,string msgcontent);//转发上行消息

	void			parseReportMsgContent(const u8array& msgConent,int msglength,string& newContent);
private:
//	void			runSubmitSend();
	bool			handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm);

private:
	CMPPClientMo*			m_clientMo;
	CMPPClientMt*			m_clientMt;
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
	
	ServiceIdentifier	m_smshttpgw;
	string m_flowSequence;//流程送来的流水号
	int					m_cycleTimes;	//循环次数
	PacketCacheControl	m_cacheList;	//缓存列表,反向消息使用
	int                 m_longSms;      //支持长短信
	int                 m_splitSize;    //短信拆分大小
	u32					m_moSieId;
	u32					m_reportSieId;
/*
	cacti::Thread       m_sendThread;       //发送线程
	bool                m_stopSendThread;   //终止发送线程
	Event               m_stopSendEvent;    //停止信号
	
	SmgpStreamQueue     m_batchSmsQueue;    //群发短信缓冲队列
	SmgpStreamQueue     m_userSmsQueue;     //用户短信缓冲队列
*/
	int					m_maxSendWindowSize;    //最大的发送窗大小
	//CMPPCacheControl*    m_cacheControl;
	u32 m_connectNum;
};

#endif
