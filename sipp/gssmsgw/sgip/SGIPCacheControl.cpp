#include "SGIPCacheControl.h"
#ifdef _TEST
#include "cacti/logging/BALog.h"
#include "cacti/util/FileSystem.h"
#endif
SGIPCacheControl::SGIPCacheControl(BaseSGIPProtocol* owner,int max_window_size,int recvtimeout)
:hightBegin(0),
hightSend(0),
hightEnd(0),
hightBufferHead(0),
hightCacheQueue(MAX_BUFFER_SIZE*0.1),
normalBegin(0),
normalSend(0),
normalEnd(0),
normalBufferHead(0),
normalCacheQueue(MAX_BUFFER_SIZE*0.4),
lowBegin(0),
lowSend(0),
lowEnd(0),
lowBufferHead(0),
lowCacheQueue(MAX_BUFFER_SIZE*0.5),
activetime(1000),//考虑时间精度问题，默认将1秒分为10个时间片
m_recvTimeout(recvtimeout),
m_max_sendwindow_size(max_window_size),
m_logger(Logger::getInstance("msggw")),
m_comQueue(2048),
sendSpeedStatus(0),
receivSpeedStatus(0),
updateSpeedStatus(0)
{
	m_owner = owner;
	m_sendSpeed = m_max_sendwindow_size*activetime/1000;
}
SGIPCacheControl::~SGIPCacheControl()
{
	hightCacheQueue.clear();
	normalCacheQueue.clear();
	lowCacheQueue.clear();
}
bool SGIPCacheControl::getSendPacket(SGIPPacketCachePtr& pkg)
{
	//RecursiveMutex::ScopedLock lock(m_mutex);
	int priority=-1;
	if(hightSend!=hightEnd)
		priority =SGIP::Priority_High;
	else if(normalSend!=normalEnd)
		priority =SGIP::Priority_Normal;
	else if(lowSend!=lowEnd)
		priority =SGIP::Priority_Low;
	if(priority==-1)
		return false;

	switch(priority)
	{
		case SGIP::Priority_VeryHigh:
		case SGIP::Priority_High:
			{
				hightCacheQueue[hightSend]->status=1;
				pkg = hightCacheQueue[hightSend];
				//std::swap(hightCacheQueue[hightSend],pkg);
				moveSendPoint(SGIP::Priority_High);
				return true;
			}
			break;
		case SGIP::Priority_Normal:
			{
				normalCacheQueue[normalSend]->status=1;
				pkg = normalCacheQueue[normalSend];
				//SGIPPacketCachePtr t;
				//std::swap(normalCacheQueue[normalSend],t);
				moveSendPoint(SGIP::Priority_Normal);
				return true;		
			}
		case SGIP::Priority_Low:
			{
				
				lowCacheQueue[lowSend]->status=1;
				pkg = lowCacheQueue[lowSend];
			//	std::swap(lowCacheQueue[lowSend],pkg);
				moveSendPoint(SGIP::Priority_Low);
				return true;
				
			}
		default:
			break;
	}
	return false;
}
bool SGIPCacheControl::put(SGIPSubmit& pkg)
{
	u8 priority=pkg.priority;
	if(priority!=SGIP::Priority_VeryHigh&&priority!=SGIP::Priority_High
		&&priority!=SGIP::Priority_Normal&&priority!=SGIP::Priority_Low)
	{
		m_logger.error("[%02d][%08x] unkown priority(%d)  packet \n",m_owner->getMyID(),pkg.getSequence(),priority);
		return false;
	}

	UserTransferMessagePtr utm=UTM::create(_CMD_PUTMSG);
	(*utm)[_TAG_SEQID]    = pkg.getSequence();
	(*utm)[_TAG_PRIORITY] = pkg.priority;
	u8array content;
	pkg.toArray(content);
	(*utm)[_TAG_CONTENT]  = content;

	m_comQueue.tryPut(utm);
	
	return true;

}
void SGIPCacheControl::handleResponse(UserTransferMessagePtr utm)
{
	u32 packetid;
	packetid = (*utm)[_TAG_SEQID];

	for(int i=hightBegin;i!=hightSend;i=(i+1)%hightCacheQueue.size())
	{
		if(hightCacheQueue[i]->packetID == packetid)
		{
			hightCacheQueue[i]->status = 2; //finished
			if(i==hightBegin)
			{
				SGIPPacketCachePtr t; 
				std::swap(hightCacheQueue[i],t);
				moveBeginPoint(SGIP::Priority_High);
			}
			return ;
		}
	}

	for(int i=normalBegin;i!=normalSend;i=(i+1)%normalCacheQueue.size())
	{
		if(normalCacheQueue[i]->packetID == packetid)
		{
			normalCacheQueue[i]->status = 2; //finished

			
			if(i==normalBegin)
			{
				SGIPPacketCachePtr t; 
				std::swap(normalCacheQueue[i],t);
				moveBeginPoint(SGIP::Priority_Normal);
			}
			return ;
		}
	}

	for(int i=lowBegin;i!=lowSend;i=(i+1)%lowCacheQueue.size())
	{
		if(lowCacheQueue[i]->packetID == packetid)
		{
			lowCacheQueue[i]->status = 2; //finished
			if(i==lowBegin)
			{
				SGIPPacketCachePtr t; 
				std::swap(lowCacheQueue[i],t);
				moveBeginPoint(SGIP::Priority_Low);
			}
			return ;
		}
	}
}
void SGIPCacheControl::handlePutPkg(UserTransferMessagePtr utm)
{

	SGIPPacketCachePtr packet = SGIPPacketCachePtr(new SGIPPacketCache());
	packet->priority=(*utm)[_TAG_PRIORITY];
	packet->packetID=(*utm)[_TAG_SEQID];
	u8array content;
	content = (*utm)[_TAG_CONTENT];
	packet->content.putStream(&content[0],content.size());



	switch(packet->priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			if(hightBegin != (hightBufferHead+1) % hightCacheQueue.size())//  not  full 
			{
				hightCacheQueue[hightBufferHead] =  packet;
				moveBufferHeader(SGIP::Priority_High);
			}
			receivSpeedStatus++;
			return;
		}
		break;
	case SGIP::Priority_Normal:
		{
			if(normalBegin != (normalBufferHead+1) % normalCacheQueue.size())
			{
				normalCacheQueue[normalBufferHead] = packet;
				moveBufferHeader(SGIP::Priority_Normal);
			}
			receivSpeedStatus++;
			return;
		}
		break;
	case SGIP::Priority_Low:
		{
			if(lowBegin != (lowBufferHead+1)%normalCacheQueue.size())
			{
				lowCacheQueue[lowBufferHead] = packet;
				moveBufferHeader(SGIP::Priority_Low);
			}
			receivSpeedStatus++;
			return;
		}
		break;
	default:
		
		return;
	}

}
void SGIPCacheControl::handleUpdateWindowEndPoint()
{
//	RecursiveMutex::ScopedLock lock(m_mutex);
	
	int hightWinSize,normalWinSize,lowWinSize,moveSize;
	hightWinSize = hightEnd>=hightBegin?(hightEnd-hightBegin):(hightEnd+hightCacheQueue.size()-hightBegin);
	normalWinSize = normalEnd>=normalBegin?(normalEnd-normalBegin):(normalEnd+normalCacheQueue.size()-normalBegin);
	lowWinSize = lowEnd>=lowBegin?(lowEnd-lowBegin):(lowEnd+lowCacheQueue.size()-lowBegin);
	
	if(hightWinSize+normalWinSize+lowWinSize>=m_max_sendwindow_size)
		return;
	moveSize = m_max_sendwindow_size -hightWinSize-normalWinSize-lowWinSize;
	moveSize =  moveSize<=m_sendSpeed?moveSize:m_sendSpeed;
	
	//int moveSize=100;
	
	for(int i=0;i<moveSize;i++)
	{
		if(moveEndPoint(SGIP::Priority_High))
		{
			updateSpeedStatus++;
			continue;
		}
		if(moveEndPoint(SGIP::Priority_Normal))
		{
			updateSpeedStatus++;
			continue;
		}
		if(moveEndPoint(SGIP::Priority_Low))
		{
			updateSpeedStatus++;
			continue;
		}
		break ;//如果都无法移动，则退出
	}
	if(receivSpeedStatus!=0||updateSpeedStatus!=0||sendSpeedStatus!=0)
		m_logger.debug("[%02d] Record run status:update point(%d),sendSpeed(%d),receivSpeed(%d) ,current point lb(%d),ls(%d),le(%d),lbh(%d),nb(%d),ns(%d),ne(%d),nbh(%d) hb(%d),hs(%d),he(%d),hbh(%d)\n", m_owner->getMyID(),updateSpeedStatus,sendSpeedStatus,receivSpeedStatus,lowBegin,lowSend,lowEnd,lowBufferHead,normalBegin,normalSend,normalEnd,normalBufferHead,hightBegin,hightSend,hightEnd,hightBufferHead);
	
	updateSpeedStatus = 0;
	sendSpeedStatus=0;
	receivSpeedStatus=0;

}
void SGIPCacheControl::handleDelTimeoutPkg()
{
	int count;
	count=0;
	cacti::Timestamp curTime(time(0));

	for(int i=hightBegin;i!=hightSend;i=(i+1)%hightCacheQueue.size())
	{
			if(hightCacheQueue[i]->addStamp > m_recvTimeout)
			{
				SGIPPacketCachePtr t; 
				std::swap(hightCacheQueue[i],t);
				moveBeginPoint(SGIP::Priority_High);
				count++;
			}else
			{
				break;
			}
	}
	for(int i=normalBegin;i!=normalSend;i=(i+1)%normalCacheQueue.size())
	{
		if(normalCacheQueue[i]->addStamp > m_recvTimeout)
		{
			SGIPPacketCachePtr t; 
			std::swap(normalCacheQueue[i],t);
			moveBeginPoint(SGIP::Priority_Normal);
			count++;
		}else
		{
			break;
		}
	}
	for(int i=lowBegin;i!=lowSend;i=(i+1)%lowCacheQueue.size())
	{
		if(lowCacheQueue[i]->addStamp > m_recvTimeout)
		{
			SGIPPacketCachePtr t; 
			std::swap(lowCacheQueue[i],t);
			moveBeginPoint(SGIP::Priority_Low);
			count++;
		}else
		{
			break;
		}
	}


/*
	if(hightSend!=hightBegin)
	{
		while(curTime-hightCacheQueue[hightBegin]->addStamp > m_recvTimeout)
		{

			
			if(!moveBeginPoint(SGIP::Priority_High))
				break;
			count++;
		}
	}

	if(normalEnd!=normalBegin)
	{
		while(curTime-normalCacheQueue[normalBegin]->addStamp > m_recvTimeout)
		{
			SGIPPacketCachePtr t; 
			std::swap(hightCacheQueue[hightBegin],t);
			if(!moveBeginPoint(SGIP::Priority_Normal))
				break;
			count++;
		}
	}

	if(lowEnd!=lowBegin)
	{
		while(curTime-lowCacheQueue[lowBegin]->addStamp > m_recvTimeout)
		{
			if(!moveBeginPoint(SGIP::Priority_Low))
				break;
			count++;
		}
	}
*/
	//m_logger.debug("[%02d] Record  del timetout pkg  count(%d)\n",m_owner->getMyID(),count); ;


}

void SGIPCacheControl::updatePointProcess()
{

	UserTransferMessagePtr cmd=UTM::create(_CMD_UPDATEPOINT);
	for(;;)
	{
		//m_comQueue.tryPut(cmd);
		handleUpdateWindowEndPoint(); //如果短信平台处理窗口为100条/秒，则可以由短信平台控制，如果必须控制每秒的总提交数，则需要改为一次只移动一个位置
		m_updatePointThread.sleep(Timestamp(1,0));
	}
}
void SGIPCacheControl::response(u32 packetid)
{
	UserTransferMessagePtr utm =UTM::create(_CMD_RESPONSE);
	(*utm)[_TAG_SEQID] = packetid;
	m_comQueue.tryPut(utm);
}

bool SGIPCacheControl::moveBeginPoint(int priority)
{
	//RecursiveMutex::ScopedLock lock(m_mutex);
	switch(priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			if(hightBegin!=hightSend)
			{
				hightBegin = (hightBegin+1)%hightCacheQueue.size();
				return true;
			}
		}
	case  SGIP::Priority_Normal:
		{
			if(normalBegin!=normalSend)
			{
				normalBegin = (normalBegin+1)%normalCacheQueue.size();
				return true;
			}
		}
	case SGIP::Priority_Low:
		{
			if(lowBegin!=lowSend)
			{
				lowBegin = (lowBegin+1)%lowCacheQueue.size();
				return true;
			}
		}
	default:
		break;
	}
	return false;
}
bool SGIPCacheControl::moveEndPoint(int priority)
{
	//RecursiveMutex::ScopedLock lock(m_mutex);
	switch(priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			if(hightEnd!=hightBufferHead)
			{
				hightEnd = (hightEnd+1)%hightCacheQueue.size();
				return true;
			}
		}
	case  SGIP::Priority_Normal:
		{
			if(normalEnd!=normalBufferHead)
			{
				normalEnd = (normalEnd+1)%normalCacheQueue.size();
				return true;
			}
		}
	case SGIP::Priority_Low:
		{
			if(lowEnd!=lowBufferHead)
			{
				lowEnd = (lowEnd+1)%lowCacheQueue.size();
				return true;
			}
		}
	default:
		break;
	}
	return false;
}
bool SGIPCacheControl::moveSendPoint(int priority){
	//RecursiveMutex::ScopedLock lock(m_mutex);
	switch(priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			if(hightSend!=hightEnd)
			{
			//	m_logger.debug("record  hightbegin(%d),hightSend(%d),hightEnd(%d),hightBufferhead(%d)\n",hightBegin,hightSend,hightEnd,hightBufferHead);
				hightSend = (hightSend+1)%hightCacheQueue.size();
				return true;
			}
		}
	case  SGIP::Priority_Normal:
		{
			if(normalSend!=normalEnd)
			{
				normalSend = (normalSend+1)%normalCacheQueue.size();
				return true;
			}
		}
	case SGIP::Priority_Low:
		{
			if(lowSend!=lowEnd)
			{
				//m_logger.debug("record  lowbegin(%d),lowSend(%d),lowEnd(%d),lowBufferhead(%d)\n",lowBegin,lowSend,lowEnd,lowBufferHead);
				lowSend = (lowSend+1)%lowCacheQueue.size();
				return true;
			}
		}
	default:
		break;
	}
	return false;
}





bool SGIPCacheControl::moveBufferHeader(int priority){
//	RecursiveMutex::ScopedLock lock(m_mutex);
	switch(priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			if(hightBegin != (hightBufferHead+1) % hightCacheQueue.size())
			{
				hightBufferHead = (hightBufferHead+1)%hightCacheQueue.size();
				return true;
			}
		}
	case  SGIP::Priority_Normal:
		{
			if(normalBegin != (normalBufferHead+1) % normalCacheQueue.size())
			{
				normalBufferHead = (normalBufferHead+1)%normalCacheQueue.size();
				return true;
			}
		}
	case SGIP::Priority_Low:
		{
			if(lowBegin != (lowBufferHead+1) % lowCacheQueue.size())
			{
				lowBufferHead = (lowBufferHead+1)%lowCacheQueue.size();
				return true;
			}
		}
	default:
		break;
	}
	return false;
}


bool SGIPCacheControl::open()
{
	m_mainThread.start(this,&SGIPCacheControl::process);
	m_sendThread.start(this,&SGIPCacheControl::sendProcess);
	m_timeoutThread.start(this,&SGIPCacheControl::timeoutProcess);
	m_updatePointThread.start(this,&SGIPCacheControl::updatePointProcess);

	return true;
}


void SGIPCacheControl::sendProcess()
{
	
	SGIPPacketCachePtr packet ;
	for(;;)
	{
		if(!getSendPacket(packet))
		{
			continue;
		}
		bool ret = m_owner->sendPackage(packet);
		sendSpeedStatus++;


		//shit ,the second parameter is   mirosecond unit
		//m_sendThread.sleep(Timestamp(0, 10000));

		//Sleep(10);
	}
}

void SGIPCacheControl::timeoutProcess()
{
	UserTransferMessagePtr cmd =UTM::create(_CMD_TIMEOUT);
	while(true)
	{
		m_comQueue.tryPut(cmd);
		//delTimeoutPkg();
		m_timeoutThread.sleep(Timestamp(0, activetime*1000));
	}
}

/*
执行此函数后，队列指针会发生变化，业务端只应以此作为参考值
*/
int SGIPCacheControl::getCanPutMsgSize(u8 priority)
{

	//RecursiveMutex::ScopedLock lock(m_mutex);
	switch(priority)
	{
	case SGIP::Priority_VeryHigh:
	case SGIP::Priority_High:
		{
			return hightCacheQueue.size() -  (hightEnd>=hightBegin?(hightEnd-hightBegin):(hightEnd+hightCacheQueue.size()-hightBegin));
		}
	case SGIP::Priority_Normal:
		{
			return normalCacheQueue.size() -  (normalEnd>=normalBegin?(normalEnd-normalBegin):(normalEnd+normalCacheQueue.size()-normalBegin));
		}
	case SGIP::Priority_Low:
		{
			return lowCacheQueue.size() - (lowEnd>=lowBegin?(lowEnd-lowBegin):(lowEnd+lowCacheQueue.size()-lowBegin));
		}
	default:
		return -1;
	}
}
void SGIPCacheControl::process()
{
	UserTransferMessagePtr cmd;
	for(;;)
	{
		m_comQueue.get(cmd);
		handleMessage(cmd);
	}
}
void SGIPCacheControl::handleMessage(UserTransferMessagePtr utm)
{
	switch(utm->getMessageId())
	{
		case _CMD_PUTMSG:
			{
				handlePutPkg(utm);
			}
			break;
		case _CMD_RESPONSE:
			{
				handleResponse(utm);
			}
			break;
		/*
			case _CMD_UPDATEPOINT:
			{
				handleUpdateWindowEndPoint();
			}
			break;
		*/
		case _CMD_TIMEOUT:
			{
				handleDelTimeoutPkg();
			}
			break;
		default:
			break;
	}
}
/*
SGIPPacketCache::SGIPPacketCache(SGIPSubmit& pkg)
:packetID(pkg.getSequence()),
priority(pkg.priority),
status(0),
addStamp(time(0)),
content(*(new Stream()))
{
	content.reserve(SGIP::Max_Packet_Size);
	pkg.toStream(content);
}
*/
SGIPPacketCache::SGIPPacketCache()
: status(0),
addStamp(time(0))//,
//content(*(new Stream()))
{
}
SGIPPacketCache::~SGIPPacketCache()
{
	content.clear();
	content.free();
}
/*
SGIPPacketCache& SGIPPacketCache::operator =(SGIPPacketCache &other)
{
	packetID = other.packetID;
	priority = other.priority;
	status = other.status;
	addStamp = other.addStamp;
	content.clear();
	u8 *tmp = (u8 *)(other.content.getWriteBuffer());
	content.putStream(tmp,(u32)other.content.size());
	return *this;
}
*/
 