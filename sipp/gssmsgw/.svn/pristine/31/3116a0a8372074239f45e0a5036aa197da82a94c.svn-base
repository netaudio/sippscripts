#ifndef __CNGP_PROTOCOL_H
#define __CNGP_PROTOCOL_H

#pragma once

#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"
#include "cacti/kernel/Mutex.h"

#include "cngpdef.h"
#include "cngppacket.h"

using namespace std;
using namespace cacti;
using namespace CNGP;

class CNGPClient;


struct Requester 
{
	ServiceIdentifier m_sender;
	ServiceIdentifier m_session;
	Requester() {};
	Requester(const ServiceIdentifier &sender, const ServiceIdentifier &session )
	{
		m_sender = sender;
		m_session = session;
	}
};

class RequesterList
{
public:
	bool Get( u32 seqno, Requester & requester ) 
	{
		Mutex::ScopedLock m_scopelock(m_lock);
		map<u32,Requester>::iterator it = m_requestMap.find(seqno);
		if( it == m_requestMap.end() )
			return false;
		requester = it->second;
		m_requestMap.erase(it);
		return true;
		
	}
	void Put(u32 seqno, Requester & requester)
	{
		Mutex::ScopedLock m_scopelock(m_lock);
		m_requestMap[seqno]  = requester;
	}
private:
	Mutex m_lock;
	map<u32,Requester>  m_requestMap;
};

class CNGPProtocol : public BaseProtocol
{
	friend class CNGPClient;

public:
	CNGPProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~CNGPProtocol();


public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

protected:
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);
	void			process();

	bool			sendLogin();
	bool			sendExit();
	bool			sendActiveTest();
	bool			sendActiveTestResp(u32 seq);
	bool			sendQueryUserStatus(const ServiceIdentifier& sender,UserTransferMessagePtr utm);
	bool			sendPaymentRequest(const ServiceIdentifier& sender,UserTransferMessagePtr utm);
	bool			sendPaymentAffirm(const ServiceIdentifier& sender,UserTransferMessagePtr utm);
	int				sendSubmit(UserTransferMessagePtr utm);
	bool			sendDeliverResp(u32 seq, u8array msgID, u32 status);
	
	bool			handleSubmitResp(CNGPSubmitResp* pkg);
	bool			handleDeliver(CNGPDeliver* pkg);
	bool			handleQueryUserStatusResp(CNGPQueryUserStateResp *pkg);
	bool			handlePaymentRequestResp(CNGPPaymentRequestResp* pkg);
	bool			handlePaymentAffirmResp(CNGPPaymentAffirmResp* pkg);

private:
	CNGPClient*			m_client;
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
	string				m_SmgwNo;
	string				m_FwdSmgwNo;
	string				m_SmscNo;
	RequesterList		m_requestList;
	static unsigned int	m_msgIdSeq ;
};

#endif
