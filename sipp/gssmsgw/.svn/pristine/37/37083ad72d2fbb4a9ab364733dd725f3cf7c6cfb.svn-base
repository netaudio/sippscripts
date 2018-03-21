#ifndef __SMPP_PROTOCOL_H
#define __SMPP_PROTOCOL_H

#pragma once

#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"

#include "SMPPdef.h"
#include "SMPPpacket.h"

using namespace std;
using namespace cacti;
using namespace SMPP;

class SMPPServer;

class SMPPProtocol : public BaseProtocol
{
	friend class SMPPServer;

public:
	SMPPProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~SMPPProtocol();


public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

protected:
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);


	bool			sendLogin();
	bool			sendExit();
	
	int				sendSubmit(UserTransferMessagePtr utm);
	bool			sendDeliverResp(u32 seq, u8array msgID, u32 status);

	void			handleLogin(SMPPLogin *pkg);
	void			handleSubmit(SMPPSubmit *pkg);
	void			handleEnquire(SMPPPacket *pkg);
	void			handleUnbind(SMPPPacket *pkg);
	bool			handleSubmitResp(SMPPSubmitResp* pkg);
	bool			handleDeliver(SMPPDeliver* pkg);


private:
	SMPPServer*			m_client;
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
};

#endif
