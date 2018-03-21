#ifndef  __YLX_CACHE_CONTROL
#define  __YLX_CACHE_CONTROL

#include <cacti/util/BoundedQueue.h>
//#include "ylxclient.h"
#include "ylxprotocol.h"
using namespace std;
using namespace cacti;

//三个优先级队列的大小总和，群发队列 low 占50%  normal占 40% hight占 10%
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

/*具有流量控制功能的cache管理
2010/10/25 ken 主要是把TCP协议的滑动窗口思想搬了过来，本想做一个通用的，时间仓促，先拿短信练练手
不足之处…………那是必然滴
2010/12/1 ken  经过考虑还是在每个对指针读写的地方添加了锁，但是这样可能会影响速率控制的精度，明天再做一次压力测试再说。

2011/1/21。
1 ken  put（）、response（）函数改为发送指令到任务队列即返回，防止smgpprotocol在调用此函数时发生阻塞。
2 ken  删除所有RecursiveMutex::ScopedLock lock(m_mutex); 形式的锁。
				
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
	void updatePointProcess();//此线程用于控制窗口大小

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
	//本来想用一个数组搞定，弄成自动调控的多优先级，时间+体力因素，先这么简化吧
	vector<YLXPacketCachePtr> hightCacheQueue;
	vector<YLXPacketCachePtr> normalCacheQueue;
	vector<YLXPacketCachePtr> lowCacheQueue;

	int hightBegin;//滑动窗口开始指针
	int hightSend;//消息处理指针
	int hightEnd;//滑动窗口结束指针
	int hightBufferHead;//整个缓存的头部指针


	int normalBegin;//滑动窗口开始指针
	int normalSend;//消息处理指针
	int normalEnd;//滑动窗口结束指针
	int normalBufferHead;//整个缓存的头部指针


	int lowBegin;//滑动窗口开始指针
	int lowSend;//消息处理指针
	int lowEnd;//滑动窗口结束指针
	int lowBufferHead;//整个缓存的头部指针
	BoundedQueue<UserTransferMessagePtr> m_comQueue;//命令队列，为了不影响定时线程的精度，由一个单独的线程处理事务，定时线程只负责触发指令。
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
