#ifndef __SMGP_PROTOCOL_H
#define __SMGP_PROTOCOL_H

#pragma once

#include <cacti/util/BoundedQueue.h>
#include "cacti/logging/LogManager.h"
#include "cacti/util/SeqGen.h"
#include "base/baseprotocol.h"

#include "smgpdef.h"
#include "smgppacket.h"

using namespace std;
using namespace cacti;
using namespace SMGP;

/*
ken:�����е�packettype��Ҫ��Ӧ���ڡ���������Э��SMGP3.0���й涨�ķ��������� ��ĿǰC��ҵ��Ӧ�����޴���ҵ�񣬵�Ϊ��֤���ݣ�����ԭ�д��룬��������3
if(cachePkg->packetType == 0)//���Ͷ�����ϵ����ɹ�  
{
cachePkg->packetType = 1;//�ȴ�ͬ��������ϵLinkID

else if(cachePkg->packetType == 2)//��ʽ�·����ųɹ�

3 ��ͨ����Ϣ�ȴ�submitresp�ظ�
*/


class SMGPClient;
class SMGPCacheControl;
class SMGPPacketCache;

typedef BoundedQueue<cacti::Stream> SmgpStreamQueue;	
typedef boost::shared_ptr<SMGPPacketCache> SMGPPacketCachePtr; 

class SMGPProtocol : public BaseProtocol, ServerInterface
{
	friend class SMGPClient;

public:
	SMGPProtocol(const ServiceIdentifier& id,const string & configFile,HandlerInterface * handler);
	~SMGPProtocol();


public:
	virtual bool	open();
	virtual bool	close();
	virtual bool	activeTest();
	virtual int		sendMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm);

	void			test(int unit);

	virtual void	snapshot();
	virtual void	handleCommand(const ServiceIdentifier& sender, UserTransferMessagePtr utm);
	bool			sendPackage(SMGPPacketCachePtr pkg);
protected:               
	virtual void	loadConfig();
	void			handleClientEvt(const ServiceIdentifier& sender, const UserTransferMessagePtr& utm);
	void			handleClientMsg(cacti::Stream& content);
	void			process();
	int				respMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret);
	int				respSubmitMessage(const ServiceIdentifier& source, UserTransferMessagePtr utm, u32 ret,SMGPSubmitResp& pkg);

	bool			sendLogin();
	bool			sendExit();
	bool			sendActiveTest();
	bool			sendActiveTestResp(u32 seq);
	int				sendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);
	int				handleSendSubmit(const ServiceIdentifier& source, UserTransferMessagePtr utm);//smshttpgw ֱ�Ӵ������Ķ��ŷ���ָ��
	bool			sendDeliverResp(u32 seq, u8array msgID, u32 status);
	
	bool			handleSubmitResp(SMGPSubmitResp* pkg);
	bool			handleDeliver(SMGPDeliver* pkg);

	bool			sendPackage(SMGPPacket& pkg);
	
	bool			sendSubmitPackage(SMGPSubmit& pkg);
	bool			transferReport(string caller,string called,string reportcontent);//ת��״̬����
	bool			transferDeliver(string caller,string called,string msgcontent);//ת��������Ϣ

	void			parseReportMsgContent(const u8array& msgConent,int msglength,string& newContent);
private:
//	void			runSubmitSend();
	bool			handleQuerySendWindowSize(const ServiceIdentifier& source, UserTransferMessagePtr utm);

private:
	SMGPClient*			m_client;
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
	SMGPCacheControl*    m_cacheControl;
};

#endif
