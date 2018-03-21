#ifndef __SCSMS_PROTOCOL_H
#define __SCSMS_PROTOCOL_H

#pragma once

#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"

#include "scsmsdef.h"
#include "scsmspacket.h"

using namespace std;
using namespace cacti;
using namespace SCSMS;

class SCSMSClient;

class SCSMSProtocol : public BaseProtocol, ServerInterface
{
	friend class SCSMSClient;

public:
	SCSMSProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~SCSMSProtocol();


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
	
	bool			handleDeliver(SCSMSDeliver* pkg);
	bool			handleGwReport(SCSMSGWReport* pkg);
	bool			handleTermReport(SCSMSTermReport* pkg);

private:
	SCSMSClient*		m_client;
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
	
	int					m_cycleTimes;	//循环次数
	PacketCacheControl	m_cacheList;	//缓存列表,反向消息使用
};

#endif
