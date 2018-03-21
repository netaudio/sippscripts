#include "stdafx.h"
#include "boost/lexical_cast.hpp"
#include <cacti/util/MD5.h>
#include <cacti/util/StringUtil.h>
#include "sgippacket.h"
#include <time.h>

#define new DEBUG_NEW


bool SGIPHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(sourceCode));
	FAIL_RETURN(content.get32(sendDate));
	FAIL_RETURN(content.get32(sequence));

	return true;
}


bool SGIPHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(sourceCode);
	content.put32(sendDate);
	content.put32(sequence);
	return true;
}


SGIPHeader& SGIPHeader::operator = (const SGIPHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		sourceCode=other.sourceCode;
		sendDate=other.sendDate;
		sequence = other.sequence;
	}

	return *this;
}

bool SGIPPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool SGIPPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > SGIP::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool SGIPPacket::toArray(u8array &content)
{
	cacti::Stream buf;
	toStream(buf);
	buf.reset();
	content.resize(buf.size());
	buf.getStream(&content[0], buf.size());
	return true;
}
bool SGIPPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


const char* SGIPPacket::toString(const char* split)
{
	return split;	
}

const char* SGIPPacket::getPacketSequence(){
	sprintf(m_tmp,"%lu%lu%09lu",header.sourceCode,header.sendDate,header.sequence);
	return m_tmp;
}


void SGIPLogin::checkBuffer()
{
	loginName.resize(16);
	password.resize(16);
	reserve.resize(8);
}

bool SGIPLogin::encodeBody(cacti::Stream &content)
{
	content.put8(loginType);
	content.putStream(&loginName[0], loginName.size());
	content.putStream(&password[0], password.size());
	content.putStream(&reserve[0], reserve.size());
	return true;
}

bool SGIPLogin::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(loginType));
	FAIL_RETURN(content.getStream(&loginName[0], loginName.size()));
	FAIL_RETURN(content.getStream(&password[0], password.size()));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));

	return true;
}


void SGIPLoginResp::checkBuffer()
{
	reserve.resize(8);
}

bool SGIPLoginResp::encodeBody(cacti::Stream &content)
{
	content.put8(status);
	content.putStream(&reserve[0], reserve.size());
	return true;
}

bool SGIPLoginResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(status));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	return true;
}

SGIPSubmit::SGIPSubmit()
:SGIPPacket(SGIP_Submit),
userCount(1),
messageType(0),
morelatetoMTFlag(MO_Order),
reportFlag(Report_Error),
messageCoding(Format_GB),
agentFlag(0),
tpPid(0),
tpUdhi(0)
//chargeNumber("000000000000000000000")

{

};

void SGIPSubmit::checkBuffer()
{
	spNumber.resize(21);
	chargeNumber.resize(21);
	userNumber.resize(21);
	corpId.resize(5);
	serviceType.resize(10);
	feeValue.resize(6);
	givenValue.resize(6);
	expireTime.resize(16);
	scheduleTime.resize(16);
	reserve.resize(8);
}

bool SGIPSubmit::encodeBody(cacti::Stream &content)
{
	content.putStream(&spNumber[0],spNumber.size());
	content.putStream(&chargeNumber[0],chargeNumber.size());
	content.put8(userCount);
	content.putStream(&userNumber[0],userNumber.size());
	content.putStream(&corpId[0],corpId.size());
	content.putStream(&serviceType[0],serviceType.size());
	content.put8(feeType);
	content.putStream(&feeValue[0],feeValue.size());
	content.putStream(&givenValue[0],givenValue.size());
	content.put8(agentFlag);
	content.put8(morelatetoMTFlag);
	content.put8(priority);
	content.putStream(&expireTime[0],expireTime.size());
	content.putStream(&scheduleTime[0],scheduleTime.size());
	content.put8(reportFlag);
	content.put8(tpPid);
	content.put8(tpUdhi);
	content.put8(messageCoding);
	content.put8(messageType);
	content.put32(messageLength);
	if(messageLength>0){
		messageContent.resize(messageLength);
	}else{
		return false;
	}
	content.putStream(&messageContent[0],messageContent.size());
	content.putStream(&reserve[0],reserve.size());
	return true;
}

bool SGIPSubmit::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&spNumber[0],spNumber.size()));
	FAIL_RETURN(content.getStream(&chargeNumber[0],chargeNumber.size()));
	FAIL_RETURN(content.get8(userCount));
	FAIL_RETURN(content.getStream(&userNumber[0],userNumber.size()));
	FAIL_RETURN(content.getStream(&corpId[0],corpId.size()));
	FAIL_RETURN(content.getStream(&serviceType[0],serviceType.size()));
	FAIL_RETURN(content.get8(feeType));
	FAIL_RETURN(content.getStream(&feeValue[0],feeValue.size()));
	FAIL_RETURN(content.getStream(&givenValue[0],givenValue.size()));
	FAIL_RETURN(content.get8(agentFlag));
	FAIL_RETURN(content.get8(morelatetoMTFlag));
	FAIL_RETURN(content.get8(priority));
	FAIL_RETURN(content.getStream(&expireTime[0],expireTime.size()));
	FAIL_RETURN(content.getStream(&scheduleTime[0],scheduleTime.size()));
	FAIL_RETURN(content.get8(reportFlag));
	FAIL_RETURN(content.get8(tpPid));
	FAIL_RETURN(content.get8(tpUdhi));
	FAIL_RETURN(content.get8(messageCoding));
	FAIL_RETURN(content.get8(messageType));
	FAIL_RETURN(content.get32(messageLength));
	if(messageLength>0){
		messageContent.resize(messageLength);
	}
	FAIL_RETURN(content.getStream(&reserve[0],reserve.size()));
	return true;

	return true;
}

const char* SGIPSubmit::toString(const char* split){
	if(!m_str.empty())
	{
		return m_str.c_str();
	}
	string tmp;

	
	tmp.assign(spNumber.begin(),spNumber.end());
	m_str+=tmp.c_str();
	m_str.append(split);
	tmp.assign(chargeNumber.begin(),chargeNumber.end());
	m_str+=tmp.c_str();
	m_str.append(split);
	m_str.append(StringUtil::format("%d",userCount));
	
	m_str.append(split);
	tmp.assign(userNumber.begin(),userNumber.end());
	m_str+=tmp.c_str();

	m_str.append(split);
	tmp.assign(corpId.begin(),corpId.end());
	m_str+=tmp.c_str();

	m_str.append(split);
	tmp.assign(serviceType.begin(),serviceType.end());
	m_str+=tmp.c_str();


	m_str.append(split);
	m_str.append(StringUtil::format("%d",feeType));


	m_str.append(split);
	tmp.assign(feeValue.begin(),feeValue.end());
	m_str+=tmp.c_str();


	m_str.append(split);
	tmp.assign(givenValue.begin(),givenValue.end());
	m_str+=tmp.c_str();

	m_str.append(split);
	m_str.append(StringUtil::format("%d",agentFlag));

	m_str.append(split);
	m_str.append(StringUtil::format("%d",morelatetoMTFlag));



	m_str.append(split);
	m_str.append(StringUtil::format("%d",priority));



	m_str.append(split);
	tmp.assign(expireTime.begin(),expireTime.end());
	m_str+=tmp.c_str();


	m_str.append(split);
	tmp.assign(scheduleTime.begin(),scheduleTime.end());
	m_str+=tmp.c_str();


	m_str.append(split);
	m_str.append(StringUtil::format("%d",reportFlag));



	m_str.append(split);
	m_str.append(StringUtil::format("%d",tpPid));



	m_str.append(split);
	m_str.append(StringUtil::format("%d",tpUdhi));

	m_str.append(split);

	m_str.append(StringUtil::format("%d",messageCoding));

	m_str.append(split);

	m_str.append(StringUtil::format("%d",messageType));


	m_str.append(split);
	m_str.append(StringUtil::format("%d",messageLength));

	m_str.append(split);
	tmp.assign(messageContent.begin(),messageContent.end());
	m_str+=tmp.c_str();


	m_str.append(split);
	tmp.assign(reserve.begin(),reserve.end());
	m_str+=tmp.c_str();


	return m_str.c_str();
}

void SGIPSubmitResp::checkBuffer()
{
	reserve.resize(8);
}

bool SGIPSubmitResp::encodeBody(cacti::Stream &content)
{

	content.put8(status);
	content.putStream(&reserve[0], reserve.size());

	return true;
}

bool SGIPSubmitResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(status));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));


	return true;
}


SGIPDeliver::SGIPDeliver()
:SGIPPacket(SGIP_Deliver)
{

};

void SGIPDeliver::checkBuffer()
{
	userNumber.resize(21);
	spNumber.resize(21);
	reserve.resize(8);
}

bool SGIPDeliver::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	content.putStream(&userNumber[0], userNumber.size());
	content.putStream(&spNumber[0], spNumber.size());
	content.put8(tpPid);
	content.put8(tpUdhi);
	content.put8(messageCoding);
	content.put32(messageLength);
	if(messageLength>0){
		messageContent.resize(messageLength);
		content.putStream(&messageContent[0],messageLength);
	}
	content.putStream(&reserve[0], reserve.size());

	return true;
}

bool SGIPDeliver::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	FAIL_RETURN(content.getStream(&userNumber[0], userNumber.size()));
	FAIL_RETURN(content.getStream(&spNumber[0], spNumber.size()));
	FAIL_RETURN(content.get8(tpPid));
	FAIL_RETURN(content.get8(tpUdhi));
	FAIL_RETURN(content.get8(messageCoding));
	FAIL_RETURN(content.get32(messageLength));
	if(messageLength > 0)
	{
		messageContent.resize(messageLength);
		FAIL_RETURN(content.getStream(&messageContent[0], messageLength));
	}
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));


	return true;
}

void SGIPDeliverResp::checkBuffer()
{
	reserve.resize(8);
}

bool SGIPDeliverResp::encodeBody(cacti::Stream &content)
{
	content.put8(status);
	content.putStream(&reserve[0], reserve.size());
	

	return true;
}

bool SGIPDeliverResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(status));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	

	return true;
}


SGIPReport::SGIPReport()
:SGIPPacket(SGIP_Report)
{

};

void SGIPReport::checkBuffer()
{
	userNumber.resize(21);
	reserve.resize(8);
}

bool SGIPReport::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	content.put32(sourceCode);
	content.put32(sendDate);
	content.put32(sequence);
	content.put8(reportType);
	content.putStream(&userNumber[0], userNumber.size());
	content.put8(state);
	content.put8(errorCode);
	content.putStream(&reserve[0], reserve.size());

	return true;
}

bool SGIPReport::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();
	FAIL_RETURN(content.get32(sourceCode));
	FAIL_RETURN(content.get32(sendDate));
	FAIL_RETURN(content.get32(sequence));
	FAIL_RETURN(content.get8(reportType));
	FAIL_RETURN(content.getStream(&userNumber[0], userNumber.size()));
	FAIL_RETURN(content.get8(state));
	FAIL_RETURN(content.get8(errorCode));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	return true;
}

void SGIPReportResp::checkBuffer()
{
	reserve.resize(8);
}

bool SGIPReportResp::encodeBody(cacti::Stream &content)
{
	content.put8(status);
	content.putStream(&reserve[0], reserve.size());

	return true;
}

const char* SGIPReport::getSubmitSequenceNumber(){
	sprintf(m_sequence,"%lu%lu%09lu",sourceCode,sendDate,sequence);
	return m_sequence;
}

bool SGIPReportResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(status));
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	return true;
}




bool SGIPExit::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPExit::decodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPExitResp::encodeBody(cacti::Stream &content)
{
	return true;
}


bool SGIPExitResp::decodeBody(cacti::Stream &content)
{
	return true;
}


bool SGIPActiveTest::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPActiveTest::decodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPActiveTestResp::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SGIPActiveTestResp::decodeBody(cacti::Stream &content)
{
	return true;
}

SGIPPacketPtr SGIPPacketFactory::createPacket(u32 cmd)
{
	switch(cmd)
	{
	case SGIP_Login:
		return SGIPPacketPtr(new SGIPLogin());
	case SGIP_Login_Resp:
		return SGIPPacketPtr(new SGIPLoginResp());
	case SGIP_Exit:
		return SGIPPacketPtr(new SGIPExit());
	case SGIP_Exit_Resp:
		return SGIPPacketPtr(new SGIPExitResp());
	case SGIP_Submit:
		return SGIPPacketPtr(new SGIPSubmit());
	case SGIP_Submit_Resp:
		return SGIPPacketPtr(new SGIPSubmitResp());
	case SGIP_Deliver:
		return SGIPPacketPtr(new SGIPDeliver());	
	case SGIP_Deliver_Resp:
		return SGIPPacketPtr(new SGIPDeliverResp());
	case  SGIP_Report:
		return  SGIPPacketPtr(new SGIPReport());
	case SGIP_Active_Test:
		return SGIPPacketPtr(new SGIPActiveTest());
	case SGIP_Active_Test_Resp:
		return SGIPPacketPtr(new SGIPActiveTestResp());
	default:
		return SGIPPacketPtr(new SGIPPacket(cmd));
	}
}

