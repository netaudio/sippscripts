#ifndef __YLX_BASE_PROTOCOL_H
#define __YLX_BASE_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"
#include "srvsock/Packet.h"
#include "YLXdef.h"
#include "YLXpacket.h"

#include "../srvsock/packet.h"
#include "../srvsock/srvsock.h"

using namespace std;
using namespace cacti;
using namespace YLX;

class YLXCacheControl;
class YLXPacketCache;

typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<YLXPacketCache> YLXPacketCachePtr; 

class BaseYLXProtocol : public BaseProtocol,public ServerInterface
{
public:
	BaseYLXProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~BaseYLXProtocol();

public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

	void			test(int unit){};
	void			test(string userPhone,string linkId,const char* gname);
	virtual void	snapshot();
	virtual void	handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	virtual bool			sendPackage(YLXPacketCachePtr pkg);
	int		procResport(const ServiceIdentifier& source, UserTransferMessagePtr utm);
	virtual bool sendPackage(char *buf,int len )=0;

protected:               
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);
	void			process();
	int				respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret);
	int				respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,YLXSubmitResp& pkg);

	bool			sendLogin();
	bool			sendLoginResp(u32 sourceCode,u32 sendDate,u32 sequence);
	bool			sendExit();
	bool			sendExitResp(u32 sourceCode,u32 sendDate,u32 sequence);
	bool			sendActiveTest();
	bool			sendActiveTestResp(u32 seq);
	int				sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);
	int				handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);//smshttpgw ֱ�Ӵ������Ķ��ŷ���ָ��
	bool			sendDeliverResp(u32 souceCode,u32 sendDate,u32 seq,   u32 status);
	bool			sendReportResp(u32 souceCode,u32 sendDate,u32 seq,   u32 status);
	bool			handleStat(YLXPacket pkg);
	bool			handleSubmit(cacti::Stream& content);
	bool			handleSubmitResp(YLXSubmitResp* pkg);
	bool			handleDeliver(YLXDeliver* pkg);
	bool			handleReport(YLXReport* pkg);
	virtual bool			sendPackage(YLXPacket& pkg);
	
	bool			sendSubmitPackage(YLXSubmit& pkg);
	bool			transferReport(string caller,string called,string reportcontent);//ת��״̬����
	bool			transferDeliver(string caller,string called,string msgcontent);//ת��������Ϣ

	void			parseReportMsgContent(const u8array& msgConent,int msglength,string& newContent);
	void			dispatch_sms(ServiceIdentifier id,const char * userPhone,const char* caller,u8array message);
	void			dispatch_sms(ServiceIdentifier id,S_Msg msg);
	int				MakeSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);

protected:
//	void			runSubmitSend();
	bool			handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm);

protected:
	
	cacti::Logger&		m_logger;
	cacti::Logger&		m_msgLogger;
	SequenceGenerator	m_seqGen;
	
	ServiceIdentifier	m_smshttpgw;

	int					m_cycleTimes;	//ѭ������
	PacketCacheControl	m_cacheList;	//�����б�,������Ϣʹ��
	int                 m_longSms;      //֧�ֳ�����
	int                 m_splitSize;    //���Ų�ִ�С
/*
	cacti::Thread       m_sendThread;       //�����߳�
	bool                m_stopSendThread;   //��ֹ�����߳�
	Event               m_stopSendEvent;    //ֹͣ�ź�
	
	SmgpStreamQueue     m_batchSmsQueue;    //Ⱥ�����Ż������
	SmgpStreamQueue     m_userSmsQueue;     //�û����Ż������
*/
	int					m_maxSendWindowSize;    //���ķ��ʹ���С
	YLXCacheControl*    m_cacheControl;
	int					m_protocolType;
	bool m_tested;

};



#endif
