#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"

#include "YLXprotocol.h"

#include "YLXclient.h"
#include "YLXserver.h"
#include "YLXCacheControl.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW

#define  _TEST

YLXProtocol::YLXProtocol(const ServiceIdentifier& id,
						   const string & configFile, 
						   HandlerInterface * handler)
:BaseYLXProtocol(id, configFile, handler)
{
	
	m_client = NULL;

	setStatus(ST_NONE);
}

YLXProtocol::~YLXProtocol()
{
	close();
	if(m_client)
	{
		delete m_client;
		m_client = NULL;
	}

}

bool	YLXProtocol::open()
{
	char month[10] = {0};
	int day;
	int year;
	sscanf(__DATE__, "%s %d %d", &month, &day, &year);
	int imonth = 0;
	if (strcmp(month, "Jan") == 0 )
		imonth = 1;
	else if (strcmp(month, "Feb") == 0 )
		imonth = 2;
	else if (strcmp(month, "Mar") == 0 )
		imonth = 3;
	else if (strcmp(month, "Apr") == 0 )
		imonth = 4;
	else if (strcmp(month, "May") == 0 )
		imonth = 5;
	else if (strcmp(month, "Jun") == 0 )
		imonth = 6;
	else if (strcmp(month, "Jul") == 0 )
		imonth = 7;
	else if (strcmp(month, "Aug") == 0 )
		imonth = 8;
	else if (strcmp(month, "Sep") == 0 )
		imonth = 9;
	else if (strcmp(month, "Oct") == 0 )
		imonth = 10;
	else if (strcmp(month, "Nov") == 0 )
		imonth = 11;
	else if (strcmp(month, "Dec") == 0 )
		imonth = 12;


	m_logger.info("[%02d] Start Process, build(%04d-%02d-%02d %s)\n",  
		m_myID.m_appref, year, imonth, day, __TIME__);

	m_startTime = time(0);
	loadConfig();

	if(!bootupSocket())
	{
		m_logger.info("[%02d] %s\n",m_myID.m_appref, "Socket stack failed");
		return false;
	}

	if(NULL == m_client)
	{
		m_client = new YLXClient( m_myID.m_appref,this );
		

		m_client->setReconnectInterval(120);
#pragma message("to do ...")
		
	}
	
		assert(m_client);
	
	
	setStatus(ST_INIT);
	m_logger.info("[%02d] YLX client (0x%x) (%s, %d) init, support long sms(%d)\n",
		m_myID.m_appref, 
		m_version,
		m_serverIP.c_str(), m_serverPort,
		m_longSms);

	//m_sendThread.start(this, &YLXProtocol::runSubmitSend);
//	m_stopSendThread = false;	
	


	 bool ret=m_client->start(m_serverPort, m_serverIP.c_str(), false);

	if(!ret)
	{
		setStatus(ST_ERROR);
	}
	m_logger.info("[%02d] YLX client connect to server %s\n",
		m_myID.m_appref, ret == true ? "ok" : "failed");
	

	return ret;
}

bool	YLXProtocol::close()
{
	if(getStatus() == ST_READY)
	{
		sendExit();
	}

	setStatus(ST_ERROR);
	return true;
}


bool YLXProtocol::sendPackage(YLXPacket& pkg)
{

	//m_logger.debug("send package :%d\n",pkg.getHeader().command);
	Stream stream;
	stream.reserve(YLX::Max_Packet_Size);
	if(!pkg.toStream(stream))
	{
		return false;
	}

	bool ret = m_client->sendStream(stream);
	if(ret)
	{
		m_statPackets.totalSendPackets ++;
	}
	else
	{
		m_statPackets.sendErrorPackets ++;
	}

	return ret;
}


bool YLXProtocol::sendPackage(YLXPacketCachePtr pkg)
{
	bool ret = m_client->sendStream(pkg->content);
	if(ret)
	{
		m_statPackets.totalSendPackets ++;
	}
	else
	{
		m_statPackets.sendErrorPackets ++;
	}
	return ret;
}


bool	YLXProtocol::activeTest()
{
// 	static bool hastTest = false;
#ifdef _TEST
	test(2);
#endif
// 	hastTest = true;




	switch(getStatus())
	{
	case ST_NONE:
		return open();

	case ST_INIT:
		return true;

	case ST_READY:
		break;

	case ST_BUSY:
		break;

	case ST_ERROR:
		return open();

	default:
		break;
	}

	
	/*
	if( (time(0) - m_lastRecvTime) > (m_cycleTimes * 60) )
	{
		if(m_cycleTimes >= m_checkTimes)
		{
			m_cycleTimes = 0;
		}

		m_logger.error("[%02d] error, recv nothing, %d seconds \n", m_myID.m_appref, m_cycleTimes * 60);
		m_client->stop();
		setStatus(ST_ERROR);
		return true;
	}
	*/

	m_logger.debug("[%02d] recv(%ld), send(%ld), recv error(%ld), send error(%ld)\n", 
		m_myID.m_appref, m_statPackets.totalRecvPackets, m_statPackets.totalSendPackets,
		m_statPackets.recvErrorPackets, m_statPackets.sendErrorPackets);

	return sendActiveTest();
}