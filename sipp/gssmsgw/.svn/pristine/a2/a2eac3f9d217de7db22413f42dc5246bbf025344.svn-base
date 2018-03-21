#include "stdafx.h"
#include <stdio.h>
#include "msggw.h"
#include "msgsk.h"
#include "reqmsg.h"
#include "cacti/logging/BALog.h"
#include "cacti/util/FileSystem.h"
#include <cacti/util/IniFile.h>

#include "srvsock/srvsock.h"
#include "ylx.h"

#define new DEBUG_NEW

const char* g_cfgFileName		= "./msggw.conf";
const char* g_aclientName		= "./log/act.log";
const char* g_logFileName		= "./log/msggw.log";
const char* g_smgpLogFileName	= "./data/smgp.log";
const char* g_sgipLogFileName	= "./data/sgip.log";
const char* g_cngpLogFileName	= "./data/cngp.log";
const char* g_cngpReportFileName = "./data/cngp_report.log";
const char* g_cbgpLogFileName	= "./data/cbgp.log";
const char* g_scsmsLogFileName	= "./data/scsms.log";
const char* g_cmppLogFileName	= "./data/cmpp.log";



boost::shared_ptr<Service> createService()
{
	return boost::shared_ptr<Service> (new MsgService());
}

bool MsgService::start()
{ 
	LogHandlerPtr msgHandler1 = LogHandlerPtr(new StarFileHandler(g_logFileName));
	StarFormatter* ttic1 = new StarFormatter;
	ttic1->logIndex(false);
	ttic1->autoNewLine(false);
	FormatterPtr ttic1Ptr(ttic1);
	msgHandler1->setFormatter(ttic1Ptr);
	Logger::getInstance("msggw").addHandler(msgHandler1);
	Logger::getInstance("msggw").setLevel(LogLevel::DBG);
	Logger::getInstance("msggw").fatal("Starting Msg gateway...\n");

	FileSystem::mkdir("./data");

	LogHandlerPtr msgHandler2 = LogHandlerPtr(new StarFileHandler(g_smgpLogFileName));
	MsgFormatter* ttic2 = new MsgFormatter;
	ttic2->logIndex(false);
	ttic2->autoNewLine(false);
	FormatterPtr ttic2Ptr(ttic2);
	msgHandler2->setFormatter(ttic2Ptr);
	Logger::getInstance("smgp").addHandler(msgHandler2);
	Logger::getInstance("smgp").setLevel(LogLevel::DBG);

	LogHandlerPtr msgHandler3 = LogHandlerPtr(new StarFileHandler(g_sgipLogFileName));
	MsgFormatter* ttic3 = new MsgFormatter;
	ttic3->logIndex(false);
	ttic3->autoNewLine(false);
	FormatterPtr ttic3Ptr(ttic3);
	msgHandler3->setFormatter(ttic3Ptr);
	Logger::getInstance("sgip").addHandler(msgHandler3);
	Logger::getInstance("sgip").setLevel(LogLevel::DBG);

	LogHandlerPtr msgHandler4 = LogHandlerPtr(new StarFileHandler(g_cngpLogFileName));
	MsgFormatter* ttic4 = new MsgFormatter;
	ttic4->logIndex(false);
	ttic4->autoNewLine(false);
	FormatterPtr ttic4Ptr(ttic4);
	msgHandler4->setFormatter(ttic4Ptr);
	Logger::getInstance("cngp").addHandler(msgHandler4);
	Logger::getInstance("cngp").setLevel(LogLevel::DBG);

	LogHandlerPtr msgHandler5 = LogHandlerPtr(new StarFileHandler(g_cngpLogFileName));
	MsgFormatter* ttic5 = new MsgFormatter;
	ttic5->logIndex(false);
	ttic5->autoNewLine(false);
	FormatterPtr ttic5Ptr(ttic5);
	msgHandler5->setFormatter(ttic5Ptr);
	Logger::getInstance("cbgp").addHandler(msgHandler5);
	Logger::getInstance("cbgp").setLevel(LogLevel::DBG);

	LogHandlerPtr msgHandler6 = LogHandlerPtr(new StarFileHandler(g_scsmsLogFileName));
	MsgFormatter* ttic6 = new MsgFormatter;
	ttic6->logIndex(false);
	ttic6->autoNewLine(false);
	FormatterPtr ttic6Ptr(ttic6);
	msgHandler6->setFormatter(ttic6Ptr);
	Logger::getInstance("scsms").addHandler(msgHandler6);
	Logger::getInstance("scsms").setLevel(LogLevel::DBG);

	u32 rolloverTime=24;
	cacti::IniFile iniFile;
	if(iniFile.open(g_cfgFileName)){
		rolloverTime=iniFile.readInt("log","log.data.mgp",24);
	}
		

	LogHandlerPtr msgHandler7 = LogHandlerPtr(new StarFileHandler(g_cmppLogFileName,100*1024*1024,30,rolloverTime*60*60));
	MsgFormatter* ttic7 = new MsgFormatter;
	ttic7->logIndex(false);
	ttic7->autoNewLine(false);
	FormatterPtr ttic7Ptr(ttic7);
	msgHandler7->setFormatter(ttic7Ptr);
	Logger::getInstance("cmpp").addHandler(msgHandler7);
	Logger::getInstance("cmpp").setLevel(LogLevel::DBG);


	LogHandlerPtr msgHandler8 = LogHandlerPtr(new StarFileHandler(g_aclientName));
	MsgFormatter* ttic8 = new MsgFormatter;
	ttic8->logIndex(false);
	ttic8->autoNewLine(false);
	FormatterPtr ttic8Ptr(ttic8);
	msgHandler8->setFormatter(ttic8Ptr);
	Logger::getInstance("socket.aclt").addHandler(msgHandler8);
	Logger::getInstance("socket.aclt").setLevel(LogLevel::DBG);

	if(!m_dispatcher.start(g_cfgFileName))
	{
		printf("Start Message Dispatcher failed\n");
		return false;
	}
	m_msgSK = new MsgServiceSK(&m_dispatcher);
	if(!m_msgSK->start())
	{
		printf("start MsgGW failed\n");
		m_dispatcher.stop();
		return false;
	}

	//UINT nPort = GetPrivateProfileInt("SMSGW", "Port", 0, "./smsgw.ini");

	//if(StartSrv(nPort,100,(void *)&OnMyRecv) != 0 )
	//	return false;

	printf("Start MsgGW Success\n");

	return true;
}



void MsgService::stop()
{
	if(m_msgSK)
	{
		m_msgSK->stop();
		delete m_msgSK;
	}

	m_dispatcher.stop();

}



void MsgService::snapshot()
{
	if(m_msgSK)
	{
		m_msgSK->postSelfMessage(UTM::create(_EvtSnapshot));
	}
}



void MsgService::handleUICommand(std::vector<std::string> & vec)
{
	if (vec[0]=="1")
	{
		UserTransferMessagePtr utm = UTM::create(m_msgSK->myIdentifier(), m_msgSK->myIdentifier(), _ReqSendSMS);
		string gname="10658605";
		string userPhone="13011111111";
		string linkId="12345678";
		(*utm)[_TagGatewayName]=gname;

		(*utm)[_TagCalling] = gname;
		(*utm)[_TagCallee] = userPhone;

		(*utm)[_TagMessage] = "616";

		std::vector<std::string> m_paramName;
		std::vector<std::string> m_paramValues;

		m_paramName.push_back("FreeUserType");
		m_paramValues.push_back("2");
		m_paramName.push_back("MsgFormat");
		m_paramValues.push_back("15");
		m_paramName.push_back("FeeType");
		m_paramValues.push_back("01");
		m_paramName.push_back("FeeCode");
		m_paramValues.push_back("0");
		m_paramName.push_back("LinkID");
		m_paramValues.push_back(linkId);

		(*utm)[_TagParameterName] = m_paramName;
		(*utm)[_TagParameterValue] = m_paramValues;

		m_msgSK->postSelfMessage(utm);
		//	ServiceIdentifier si;
		//	si.m_appid=1;
		//	si.m_appport=70;
		//m_msgSK->postMessage(si,utm);
	} 
	if (vec[0]=="2")
	{
		UserTransferMessagePtr utm = UTM::create(m_msgSK->myIdentifier(), m_msgSK->myIdentifier(), _ReqSendSMS);
		string gname="10658605";
		string userPhone="13011111111";
		string linkId="12345678";
		(*utm)[_TagGatewayName]=gname;

		(*utm)[_TagCalling] = gname;
		(*utm)[_TagCallee] = userPhone;

		(*utm)[_TagMessage] = "616";

		std::vector<std::string> m_paramName;
		std::vector<std::string> m_paramValues;

		m_paramName.push_back("FreeUserType");
		m_paramValues.push_back("2");
		m_paramName.push_back("MsgFormat");
		m_paramValues.push_back("15");
		m_paramName.push_back("FeeType");
		m_paramValues.push_back("01");
		m_paramName.push_back("FeeCode");
		m_paramValues.push_back("0");
		m_paramName.push_back("NeeReport");
		m_paramValues.push_back("1");
		m_paramName.push_back("LinkID");
		m_paramValues.push_back(linkId);

		(*utm)[_TagParameterName] = m_paramName;
		(*utm)[_TagParameterValue] = m_paramValues;

		m_msgSK->postSelfMessage(utm);
		//	ServiceIdentifier si;
		//	si.m_appid=1;
		//	si.m_appport=70;
		//m_msgSK->postMessage(si,utm);
	} 
	else
	{
		UserTransferMessagePtr utm =  UTM::create(_EvtCommand);

		(*utm)[_TagCommand] = vec;
		m_msgSK->postSelfMessage(utm);

#ifdef _DEBUG
		AlarmSender* alarm = AlarmSender::getInstance();
		std::string key;
		if(vec.size() > 0)
		{
			key = vec[0];
			if(key == "console")
			{
				alarm->console("console test");
			}
			else
			{
				alarm->send(ServiceIdentifier(), "recv cmd", 0, vec);
			}
		}
#endif

	}
}

void MsgFormatter::format(std::ostream& stream, const cacti::LogRecord& rec)
{
	std::string timestr = "";
	if(rec.timestamp.getAsMilliseconds() != 0)
		timestr = rec.timestamp.getFormattedTime("%Y%m%d%H%M%S");

	int index = 0;
	m_buffer[index] = '\0';
	if(!timestr.empty())
	{
		if(rec.hasIndex)
		{
			index = snprintf(m_buffer, sizeof(m_buffer)-1, "%s[%X] ", timestr.c_str(), rec.index);
		}
		else
		{
			index = snprintf(m_buffer, sizeof(m_buffer)-1, "%s", timestr.c_str());
		}

		if(index >= 0)
		{
			if(rec.format)
				index = vsnprintf(m_buffer+index, sizeof(m_buffer)-index-1, rec.format, rec.args);
			else
				index = snprintf(m_buffer+index, sizeof(m_buffer)-index-1, "%s", rec.message.c_str());
		}
	}
	else
	{
		if(rec.hasIndex)
		{
			index = snprintf(m_buffer, sizeof(m_buffer)-1, "[%X] ", rec.index);
		}

		if(index >= 0)
		{
			if(rec.format)
				index = vsnprintf(m_buffer+index, sizeof(m_buffer)-index-1, rec.format, rec.args);
			else
				index = snprintf(m_buffer+index, sizeof(m_buffer)-index-1, "%s", rec.message.c_str());
		}
	}

	stream << m_buffer;
}
