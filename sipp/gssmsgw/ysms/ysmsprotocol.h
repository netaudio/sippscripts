#ifndef __YSMS_PROTOCOL_H
#define __YSMS_PROTOCOL_H

#pragma once

#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"

#include "YSMSdef.h"
#include "YSMSpacket.h"

using namespace std;
using namespace cacti;
using namespace YSMS;

class YSMSServer;

class YSMSProtocol : public BaseProtocol, ServerInterface
{
	friend class YSMSServer;

public:
	YSMSProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~YSMSProtocol();


public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

	void			test(int unit);

protected:
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);
	void			process();
	int				respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret);

	bool			sendLogin();
	bool			sendActiveTest();
	bool			sendActiveTestResp();
	int				sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);
	
	bool			handleDeliver(YSMSDeliver* pkg);
	bool			handleGwReport(YSMSGWReport* pkg);
	bool			handleTermReport(YSMSTermReport* pkg);

private:
	YSMSServer*			m_server;
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
	
	int					m_cycleTimes;	//循环次数
	PacketCacheControl	m_cacheList;	//缓存列表,反向消息使用
};

#endif
