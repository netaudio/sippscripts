#ifndef  __YLX_CACHE_CONTROL
#define  __YLX_CACHE_CONTROL

#include <cacti/util/BoundedQueue.h>
//#include "ylxclient.h"
#include "ylxprotocol.h"
using namespace std;
using namespace cacti;

//�������ȼ����еĴ�С�ܺͣ�Ⱥ������ low ռ50%  normalռ 40% hightռ 10%
const static int MAX_BUFFER_SIZE = 10000;

class YLXPacketCache
{
public:
	YLXPacketCache();
	//YLXPacketCache(YLXSubmit& pkg);
	~YLXPacketCache();
	u32		packetID;
	u8		priority;  //0 low   1  normal  2 hight   
	u32		status;//0 normal  1 waiting response 2 finished 
	Timestamp   addStamp;
	Stream content;
	//YLXPacketCache& operator=(YLXPacketCache& other);
};
//typedef boost::shared_ptr<YLXPacketCache> YLXPacketCachePtr; 

/*�����������ƹ��ܵ�cache����
2010/10/25 ken ��Ҫ�ǰ�TCPЭ��Ļ�������˼����˹�����������һ��ͨ�õģ�ʱ��ִ٣����ö���������
����֮�������������Ǳ�Ȼ��
2010/12/1 ken  �������ǻ�����ÿ����ָ���д�ĵط���������������������ܻ�Ӱ�����ʿ��Ƶľ��ȣ���������һ��ѹ��������˵��

2011/1/21��
1 ken  put������response����������Ϊ����ָ�������м����أ���ֹsmgpprotocol�ڵ��ô˺���ʱ����������
2 ken  ɾ������RecursiveMutex::ScopedLock lock(m_mutex); ��ʽ������
				
*/
enum CmdCacheControl
{
	_CMD_UPDATEPOINT  = 0x01,
	_CMD_TIMEOUT	  = 0x02,
	_CMD_RESPONSE	  = 0x03,
	_CMD_PUTMSG	      = 0x04,
};
enum YLXTag
{
	_TAG_SEQID    =0x10000001,
	_TAG_PRIORITY =0x10000002,
	_TAG_CONTENT  =0x10000003,

};

class YLXCacheControl
{
public:
	YLXCacheControl(BaseYLXProtocol* owner,int  max_sendwindow_size,int recvtimeout);
	~YLXCacheControl();
	bool open();
	int	 getCanPutMsgSize(u8 priority);
	void response(u32 packetid);
	bool put(YLXSubmit& pkg);
	void process();//main thread function
	
	
private:
	void sendProcess();
	void timeoutProcess();
	void updatePointProcess();//���߳����ڿ��ƴ��ڴ�С

	void handleMessage(UserTransferMessagePtr utm);
	void handleUpdateWindowEndPoint();
	void handleDelTimeoutPkg();
	void handlePutPkg(UserTransferMessagePtr utm);
	void handleResponse(UserTransferMessagePtr utm);

	bool moveBeginPoint(int priority);
	bool moveEndPoint(int priority);
	bool moveSendPoint(int priority);
	bool moveBufferHeader(int priority);

	bool getSendPacket(YLXPacketCachePtr& packet);
	
//	void updateSendPoint();
	Thread		m_mainThread;
	Thread		m_sendThread;
	Thread		m_timeoutThread;
	Thread		m_updatePointThread;
	//��������һ������㶨��Ū���Զ����صĶ����ȼ���ʱ��+�������أ�����ô�򻯰�
	vector<YLXPacketCachePtr> hightCacheQueue;
	vector<YLXPacketCachePtr> normalCacheQueue;
	vector<YLXPacketCachePtr> lowCacheQueue;

	int hightBegin;//�������ڿ�ʼָ��
	int hightSend;//��Ϣ����ָ��
	int hightEnd;//�������ڽ���ָ��
	int hightBufferHead;//���������ͷ��ָ��


	int normalBegin;//�������ڿ�ʼָ��
	int normalSend;//��Ϣ����ָ��
	int normalEnd;//�������ڽ���ָ��
	int normalBufferHead;//���������ͷ��ָ��


	int lowBegin;//�������ڿ�ʼָ��
	int lowSend;//��Ϣ����ָ��
	int lowEnd;//�������ڽ���ָ��
	int lowBufferHead;//���������ͷ��ָ��
	BoundedQueue<UserTransferMessagePtr> m_comQueue;//������У�Ϊ�˲�Ӱ�춨ʱ�̵߳ľ��ȣ���һ���������̴߳������񣬶�ʱ�߳�ֻ���𴥷�ָ�
	int m_max_sendwindow_size;
	BaseYLXProtocol*	m_owner;
//	RecursiveMutex				m_mutex;
	int activetime;//unit :mills
	int m_recvTimeout;  // unit: s
	int m_sendSpeed;
	cacti::Logger& m_logger;

	int sendSpeedStatus;
	int receivSpeedStatus;
	int updateSpeedStatus;

	
};

/*
class YLXCacheSendThread
{
public:
	YLXCacheSendThread();
	~YLXCacheSendThread();
	bool						init();
	void						start();
	void						process();
private:
	YLXCacheControl*			m_owner;
	YLXClient*					m_client;
	cacti::Thread				m_thread;
	int activetime;//unit :ms
};
class YLXCacheTimoutThread
{
public:
	YLXCacheTimoutThread();
	~YLXCacheTimoutThread();
	bool						init();
	void						start();
	void						process();
private:
	YLXCacheControl*			m_owner;
	cacti::Thread				m_thread;
	
};
*/
#endif 
