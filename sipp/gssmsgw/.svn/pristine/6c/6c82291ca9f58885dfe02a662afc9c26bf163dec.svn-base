#ifndef _MSG_GW_H_
#define _MSG_GW_H_
#pragma once

#include "startup/Service.h"
#include "cacti/mtl/MessageDispatcher.h"

using namespace cacti;

class MsgServiceSK;

class MsgService : public Service
{
public:
	MsgService():Service("smsgw", "CTI_SmsGw_1.1.2_20151014", "sms gateway"){};
	virtual bool start();
	virtual void stop();
	virtual void snapshot();

	virtual void handleUICommand(std::vector<std::string> & vec);

private:
	MessageDispatcher m_dispatcher;
	MsgServiceSK* m_msgSK;
};

class MsgFormatter : public TTICFormatter
{
public:
	virtual void format(std::ostream& stream, const LogRecord& rec);
private:
	char m_buffer[4096];
};

#endif