#include "stdafx.h"
#include "boost/lexical_cast.hpp"
#include <cacti/util/MD5.h>
#include <cacti/util/StringUtil.h>
#include "ylxpacket.h"
#include <time.h>

#define new DEBUG_NEW


bool YLXHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(sequence));

	return true;
}


bool YLXHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(sequence);
	return true;
}


YLXHeader& YLXHeader::operator = (const YLXHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		sequence = other.sequence;
	}

	return *this;
}

bool YLXPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool YLXPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > YLX::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool YLXPacket::toArray(u8array &content)
{
	cacti::Stream buf;
	toStream(buf);
	buf.reset();
	content.resize(buf.size());
	buf.getStream(&content[0], buf.size());
	return true;
}
bool YLXPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool YLXPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


const char* YLXPacket::toString(const char* split)
{
	return split;	
}

const char* YLXPacket::getPacketSequence()
{
	sprintf(m_tmp,"%09lu",header.sequence);
	return m_tmp;
}

//
//
//void YLXLogin::checkBuffer()
//{
//	loginName.resize(16);
//	password.resize(16);
//	reserve.resize(8);
//}
//
//bool YLXLogin::encodeBody(cacti::Stream &content)
//{
//	//content.put8(loginType);
//	//content.putStream(&loginName[0], loginName.size());
//	//content.putStream(&password[0], password.size());
//	//content.putStream(&reserve[0], reserve.size());
//	return true;
//}
//
//bool YLXLogin::decodeBody(cacti::Stream &content)
//{
//	FAIL_RETURN(content.get8(loginType));
//	FAIL_RETURN(content.getStream(&loginName[0], loginName.size()));
//	FAIL_RETURN(content.getStream(&password[0], password.size()));
//	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
//
//	return true;
//}


//void YLXLoginResp::checkBuffer()
//{
//	//reserve.resize(8);
//}
//
//bool YLXLoginResp::encodeBody(cacti::Stream &content)
//{
//	//content.put8(status);
//	//content.putStream(&reserve[0], reserve.size());
//	return true;
//}
//
//bool YLXLoginResp::decodeBody(cacti::Stream &content)
//{
//	//FAIL_RETURN(content.get8(status));
//	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
//	return true;
//}
//



void YLXSubmit::checkBuffer()
{
	//spNumber.resize(21);
	//chargeNumber.resize(21);
	//userNumber.resize(21);
	//corpId.resize(5);
	//serviceType.resize(10);
	//feeValue.resize(6);
	//givenValue.resize(6);
	//expireTime.resize(16);
	//scheduleTime.resize(16);
	//reserve.resize(8);
}

bool YLXSubmit::encodeBody(cacti::Stream &content)
{
	//content.putStream(&spNumber[0],spNumber.size());
	//content.putStream(&chargeNumber[0],chargeNumber.size());
	//content.put8(userCount);
	//content.putStream(&userNumber[0],userNumber.size());
	//content.putStream(&corpId[0],corpId.size());
	//content.putStream(&serviceType[0],serviceType.size());
	//content.put8(feeType);
	//content.putStream(&feeValue[0],feeValue.size());
	//content.putStream(&givenValue[0],givenValue.size());
	//content.put8(agentFlag);
	//content.put8(morelatetoMTFlag);
	//content.put8(priority);
	//content.putStream(&expireTime[0],expireTime.size());
	//content.putStream(&scheduleTime[0],scheduleTime.size());
	//content.put8(reportFlag);
	//content.put8(tpPid);
	//content.put8(tpUdhi);
	//content.put8(messageCoding);
	//content.put8(messageType);
	//content.put32(messageLength);
	//if(messageLength>0){
	//	messageContent.resize(messageLength);
	//}
	//content.putStream(&messageContent[0],messageContent.size());
	//content.putStream(&reserve[0],reserve.size());
	return true;
}

bool YLXSubmit::decodeBody(cacti::Stream &content)
{
	//FAIL_RETURN(content.getStream(&spNumber[0],spNumber.size()));
	//FAIL_RETURN(content.getStream(&chargeNumber[0],chargeNumber.size()));
	//FAIL_RETURN(content.get8(userCount));
	//FAIL_RETURN(content.getStream(&userNumber[0],userNumber.size()));
	//FAIL_RETURN(content.getStream(&corpId[0],corpId.size()));
	//FAIL_RETURN(content.getStream(&serviceType[0],serviceType.size()));
	//FAIL_RETURN(content.get8(feeType));
	//FAIL_RETURN(content.getStream(&feeValue[0],feeValue.size()));
	//FAIL_RETURN(content.getStream(&givenValue[0],givenValue.size()));
	//FAIL_RETURN(content.get8(agentFlag));
	//FAIL_RETURN(content.get8(morelatetoMTFlag));
	//FAIL_RETURN(content.get8(priority));
	//FAIL_RETURN(content.getStream(&expireTime[0],expireTime.size()));
	//FAIL_RETURN(content.getStream(&scheduleTime[0],scheduleTime.size()));
	//FAIL_RETURN(content.get8(reportFlag));
	//FAIL_RETURN(content.get8(tpPid));
	//FAIL_RETURN(content.get8(tpUdhi));
	//FAIL_RETURN(content.get8(messageCoding));
	//FAIL_RETURN(content.get8(messageType));
	//FAIL_RETURN(content.get32(messageLength));
	//if(messageLength>0){
	//	messageContent.resize(messageLength);
	//}
	//FAIL_RETURN(content.getStream(&reserve[0],reserve.size()));
	//return true;

	return true;
}

const char* YLXSubmit::toString(const char* split)
{
	//if(!m_str.empty())
	//{
	//	return m_str.c_str();
	//}
	//string tmp;

	//
	//tmp.assign(spNumber.begin(),spNumber.end());
	//m_str+=tmp.c_str();
	//m_str.append(split);
	//tmp.assign(chargeNumber.begin(),chargeNumber.end());
	//m_str+=tmp.c_str();
	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",userCount));
	//
	//m_str.append(split);
	//tmp.assign(userNumber.begin(),userNumber.end());
	//m_str+=tmp.c_str();

	//m_str.append(split);
	//tmp.assign(corpId.begin(),corpId.end());
	//m_str+=tmp.c_str();

	//m_str.append(split);
	//tmp.assign(serviceType.begin(),serviceType.end());
	//m_str+=tmp.c_str();


	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",feeType));


	//m_str.append(split);
	//tmp.assign(feeValue.begin(),feeValue.end());
	//m_str+=tmp.c_str();


	//m_str.append(split);
	//tmp.assign(givenValue.begin(),givenValue.end());
	//m_str+=tmp.c_str();

	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",agentFlag));

	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",morelatetoMTFlag));



	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",priority));



	//m_str.append(split);
	//tmp.assign(expireTime.begin(),expireTime.end());
	//m_str+=tmp.c_str();


	//m_str.append(split);
	//tmp.assign(scheduleTime.begin(),scheduleTime.end());
	//m_str+=tmp.c_str();


	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",reportFlag));



	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",tpPid));



	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",tpUdhi));

	//m_str.append(split);

	//m_str.append(StringUtil::format("%d",messageCoding));

	//m_str.append(split);

	//m_str.append(StringUtil::format("%d",messageType));


	//m_str.append(split);
	//m_str.append(StringUtil::format("%d",messageLength));

	//m_str.append(split);
	//tmp.assign(messageContent.begin(),messageContent.end());
	//m_str+=tmp.c_str();


	//m_str.append(split);
	//tmp.assign(reserve.begin(),reserve.end());
	//m_str+=tmp.c_str();


	//return m_str.c_str();
	return NULL;
}

void YLXSubmitResp::checkBuffer()
{
	//reserve.resize(8);
}

bool YLXSubmitResp::encodeBody(cacti::Stream &content)
{

	//content.put8(status);
	//content.putStream(&reserve[0], reserve.size());

	return true;
}

bool YLXSubmitResp::decodeBody(cacti::Stream &content)
{
	//FAIL_RETURN(content.get8(status));
	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));


	return true;
}


YLXDeliver::YLXDeliver()
:YLXPacket(YLX_Deliver)
{

};

void YLXDeliver::checkBuffer()
{
	//userNumber.resize(21);
	//spNumber.resize(21);
	//reserve.resize(8);
}

bool YLXDeliver::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	//content.putStream(&userNumber[0], userNumber.size());
	//content.putStream(&spNumber[0], spNumber.size());
	//content.put8(tpPid);
	//content.put8(tpUdhi);
	//content.put8(messageCoding);
	//content.put32(messageLength);
	//if(messageLength>0){
	//	messageContent.resize(messageLength);
	//	content.putStream(&messageContent[0],messageLength);
	//}
	//content.putStream(&reserve[0], reserve.size());

	return true;
}

bool YLXDeliver::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	//FAIL_RETURN(content.getStream(&userNumber[0], userNumber.size()));
	//FAIL_RETURN(content.getStream(&spNumber[0], spNumber.size()));
	//FAIL_RETURN(content.get8(tpPid));
	//FAIL_RETURN(content.get8(tpUdhi));
	//FAIL_RETURN(content.get8(messageCoding));
	//FAIL_RETURN(content.get32(messageLength));
	//if(messageLength > 0)
	//{
	//	messageContent.resize(messageLength);
	//	FAIL_RETURN(content.getStream(&messageContent[0], messageLength));
	//}
	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));


	return true;
}

void YLXDeliverResp::checkBuffer()
{
	reserve.resize(8);
}

bool YLXDeliverResp::encodeBody(cacti::Stream &content)
{
	//content.put8(status);
	//content.putStream(&reserve[0], reserve.size());
	//

	return true;
}

bool YLXDeliverResp::decodeBody(cacti::Stream &content)
{
	//FAIL_RETURN(content.get8(status));
	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	//

	return true;
}



void YLXReport::checkBuffer()
{
	//userNumber.resize(21);
	//reserve.resize(8);
}

bool YLXReport::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	//content.put32(sourceCode);
	//content.put32(sequence);
	//content.put8(reportType);
	//content.putStream(&userNumber[0], userNumber.size());
	//content.put8(state);
	//content.put8(errorCode);
	//content.putStream(&reserve[0], reserve.size());

	return true;
}

bool YLXReport::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();
	//FAIL_RETURN(content.get32(sequence));
	//FAIL_RETURN(content.get8(reportType));
	//FAIL_RETURN(content.getStream(&userNumber[0], userNumber.size()));
	//FAIL_RETURN(content.get8(state));
	//FAIL_RETURN(content.get8(errorCode));
	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	return true;
}

void YLXReportResp::checkBuffer()
{
	//reserve.resize(8);
}

bool YLXReportResp::encodeBody(cacti::Stream &content)
{
	//content.put8(status);
	//content.putStream(&reserve[0], reserve.size());


	return true;
}

const char* YLXReport::getSubmitSequenceNumber()
{
	sprintf(m_sequence,"%lu%lu%09lu",sourceCode,sendDate,sequence);
	return m_sequence;
}

bool YLXReportResp::decodeBody(cacti::Stream &content)
{
	//FAIL_RETURN(content.get8(status));
	//FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));
	return true;
}


YLXPacketPtr YLXPacketFactory::createPacket(u32 cmd)
{
	switch(cmd)
	{
	//case YLX_Login:
	//	return YLXPacketPtr(new YLXLogin());
	//case YLX_Login_Resp:
	//	return YLXPacketPtr(new YLXLoginResp());
	//case YLX_Exit:
	//	return YLXPacketPtr(new YLXExit());
	//case YLX_Exit_Resp:
	//	return YLXPacketPtr(new YLXExitResp());

	case YLX_Stat:
		return YLXPacketPtr(new YLXStat());
	case YLX_Submit:
		return YLXPacketPtr(new YLXSubmit());
	case YLX_Submit_Resp:
		return YLXPacketPtr(new YLXSubmitResp());
	case YLX_Deliver:
		return YLXPacketPtr(new YLXDeliver());	
	case YLX_Deliver_Resp:
		return YLXPacketPtr(new YLXDeliverResp());
	case  YLX_Report:
		return  YLXPacketPtr(new YLXReport());
	//case YLX_Active_Test:
	//	return YLXPacketPtr(new YLXActiveTest());
	//case YLX_Active_Test_Resp:
	//	return YLXPacketPtr(new YLXActiveTestResp());
	default:
		return YLXPacketPtr(new YLXPacket(cmd));
	}
}