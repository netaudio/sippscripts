#include "stdafx.h"

#include "YSMSpacket.h"

#define new DEBUG_NEW


#define YSMS_PUT_INTEGER(content, val, size) \
	{ \
	if(size <= 0) return false; \
	u8array buf(size, ' '); \
	YSMS_ConvertInteger(&buf[0], val, size); \
	content.putStream(&buf[0], size); \
	} \

#define YSMS_GET_INTEGER(content, val, size) \
	{	\
	if(size <= 0) return false; \
	char buf[size+1];  \
	buf[size] = '\0';  \
	if(content.getStream((u8*)(&buf[0]), size)) \
	val = atoi((char*)&buf[0]); \
		else \
		return false; \
	} \


void YSMS_REPLACE_32(u8* outVal, const u32 val)
{
	assert(outVal);
	outVal[0] = u8(val);
	outVal[1] = u8(val >> 8);
	outVal[2] = u8(val >> 16);
	outVal[3] = u8(val >> 24);
}

void YSMS_REPLACE_16(u8* outVal, const u32 val)
{
	assert(outVal);
	outVal[0] = u8(val);
	outVal[1] = u8(val >> 8);
}

void YSMS_ConvertInteger(u8* outVal, const u32 val, const u32 size)
{
	assert(outVal);

	char buf[16] = {0};
	int realSize = _snprintf(buf, size, "%u", val);

	for(int i=0; i<realSize && i<size; i++)
	{
		outVal[i] = buf[i];
	}

	return;
}

bool YSMSHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(command));
	u16 tmp = 0;
	FAIL_RETURN(content.get16(tmp));
	totalLength = tmp << 8 | (u8)(tmp >> 8);

	return true;
}


bool YSMSHeader::toStream(cacti::Stream &content)
{
	content.put8(command);
	u16 tmp = (totalLength << 8) | (u8)(totalLength >> 8);
	content.put16(tmp);

	return true;
}


YSMSHeader& YSMSHeader::operator = (const YSMSHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
	}

	return *this;
}

bool YSMSPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool YSMSPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);

	//end tag
	for(int i = 0; i < 5; i++)
	{
		content.put8(1);
	}

	size_t length = content.size() - 8;
	if(length <= 0 && length > YSMS::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer(1);
	YSMS_REPLACE_16(tmp, length);

	return true;
}

bool YSMSPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool YSMSPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


void YSMSLogin::checkBuffer()
{
	clientID.resize(10, YSMS::FILL_CHAR);
	password.resize(50, YSMS::FILL_CHAR);
}

bool YSMSLogin::encodeBody(cacti::Stream &content)
{
	content.putStream(&clientID[0], clientID.size());
	content.putStream(&password[0], password.size());

	return true;
}

bool YSMSLoginResp::decodeBody(cacti::Stream &content)
{
	YSMS_GET_INTEGER(content, status, 1);

	return true;
}

void YSMSDeliver::checkBuffer()
{
	LineNo.resize(50);
	Msg_Id.resize(50);
	Dest_Id.resize(21);
	Src_Id.resize(32);
	Service_Code.resize(50);
	Msg_Content.resize(255);
	LinkId.resize(50);
}

bool YSMSDeliver::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&Msg_Id[0], Msg_Id.size()));
	FAIL_RETURN(content.getStream(&Dest_Id[0], Dest_Id.size()));
	YSMS_GET_INTEGER(content, Dest_Id_Type, 1);
	FAIL_RETURN(content.getStream(&Src_Id[0], Src_Id.size()));
	YSMS_GET_INTEGER(content, Src_Id_Type, 1);
	FAIL_RETURN(content.getStream(&Service_Code[0], Service_Code.size()));
	YSMS_GET_INTEGER(content, Msg_Fmt, 2);
	FAIL_RETURN(content.getStream(&Msg_Content[0], Msg_Content.size()));
	FAIL_RETURN(content.getStream(&LinkId[0], LinkId.size()));

	return true;
}

void YSMSSubmit::checkBuffer()
{
	Msg_Kind.resize(10, YSMS::FILL_CHAR);
	LineNo.resize(50, YSMS::FILL_CHAR);
	Service_Code.resize(10, YSMS::FILL_CHAR);
	FeeType.resize(6, YSMS::FILL_CHAR);
	FeeCode.resize(6, YSMS::FILL_CHAR);
	FixedFee.resize(6, YSMS::FILL_CHAR);
	GivenValue.resize(6, YSMS::FILL_CHAR);
	Fee_terminal_Id.resize(32, YSMS::FILL_CHAR);
	Src_Id.resize(21, YSMS::FILL_CHAR);
	Dest_Id.resize(32, YSMS::FILL_CHAR);
	Msg_Content.resize(255, YSMS::FILL_CHAR);
	LinkID.resize(50, YSMS::FILL_CHAR);
	SpCode.resize(10, YSMS::FILL_CHAR);
	ServiceID.resize(10, YSMS::FILL_CHAR);
}

bool YSMSSubmit::encodeBody(cacti::Stream &content)
{
	checkBuffer();

	content.putStream(&Msg_Kind[0], Msg_Kind.size());		
	content.putStream(&LineNo[0], LineNo.size());
	YSMS_PUT_INTEGER(content, Pk_total, 1);
	YSMS_PUT_INTEGER(content, Pk_number, 1);
	content.put8(Fee_UserType);
	content.putStream(&Service_Code[0], Service_Code.size());
	content.putStream(&FeeType[0], FeeType.size());
	content.putStream(&FeeCode[0], FeeCode.size());
	content.putStream(&FixedFee[0], FixedFee.size());
	content.putStream(&GivenValue[0], GivenValue.size());
	content.putStream(&Fee_terminal_Id[0], Fee_terminal_Id.size());
	YSMS_PUT_INTEGER(content, Fee_teminal_type, 1);
	YSMS_PUT_INTEGER(content, Msg_Fmt, 2);
	content.putStream(&Src_Id[0], Src_Id.size());
	content.putStream(&Dest_Id[0], Dest_Id.size());
	content.put8(Dest_Id_type);
	content.putStream(&Msg_Content[0], Msg_Content.size());
	content.putStream(&LinkID[0], LinkID.size());
	//content.put8(MsgType);
	YSMS_PUT_INTEGER(content,MsgType,1);
	content.putStream(&SpCode[0], SpCode.size());
	content.putStream(&ServiceID[0], ServiceID.size());

	return true;
}

void YSMSSubmitGroup::checkBuffer()
{
	Msg_Kind.resize(10, YSMS::FILL_CHAR);
	LineNo.resize(50, YSMS::FILL_CHAR);
	Service_Code.resize(10, YSMS::FILL_CHAR);
	FeeType.resize(6, YSMS::FILL_CHAR);
	FeeCode.resize(6, YSMS::FILL_CHAR);
	FixedFee.resize(6, YSMS::FILL_CHAR);
	GivenValue.resize(6, YSMS::FILL_CHAR);
	Fee_terminal_Id.resize(32, YSMS::FILL_CHAR);
	ValidTime.resize(19, YSMS::FILL_CHAR);
	AtTime.resize(19, YSMS::FILL_CHAR);
	Src_Id.resize(21, YSMS::FILL_CHAR);
	Dest_Id.resize(32, YSMS::FILL_CHAR);
	Msg_Content.resize(255, YSMS::FILL_CHAR);
	LinkID.resize(50, YSMS::FILL_CHAR);
	SpCode.resize(10, YSMS::FILL_CHAR);
	ServiceID.resize(10, YSMS::FILL_CHAR);
}

bool YSMSSubmitGroup::encodeBody(cacti::Stream &content)
{
	checkBuffer();

	content.putStream(&Msg_Kind[0], Msg_Kind.size());		
	content.putStream(&LineNo[0], LineNo.size());
	YSMS_PUT_INTEGER(content, Pk_total, 1);
	YSMS_PUT_INTEGER(content, Pk_number, 1);
	content.put8(Fee_UserType);
	content.putStream(&Service_Code[0], Service_Code.size());
	content.putStream(&FeeType[0], FeeType.size());
	content.putStream(&FeeCode[0], FeeCode.size());
	content.putStream(&FixedFee[0], FixedFee.size());
	content.putStream(&GivenValue[0], GivenValue.size());
	content.putStream(&Fee_terminal_Id[0], Fee_terminal_Id.size());
	YSMS_PUT_INTEGER(content, Fee_teminal_type, 1);
	YSMS_PUT_INTEGER(content, Msg_Fmt, 2);
	content.putStream(&ValidTime[0], ValidTime.size());
	content.putStream(&AtTime[0], AtTime.size());
	content.putStream(&Src_Id[0], Src_Id.size());
	content.putStream(&Dest_Id[0], Dest_Id.size());
	content.put8(Dest_Id_type);
	content.putStream(&Msg_Content[0], Msg_Content.size());
	content.putStream(&LinkID[0], LinkID.size());
	//content.put8(MsgType);
	YSMS_PUT_INTEGER(content,MsgType,1);
	content.putStream(&SpCode[0], SpCode.size());
	content.putStream(&ServiceID[0], ServiceID.size());

	return true;
}


bool YSMSSubmitGroupResp::decodeBody(cacti::Stream &content)
{
	YSMS_GET_INTEGER(content, status, 1);

	return true;
}

bool YSMSActiveTest::encodeBody(cacti::Stream &content)
{
	YSMS_PUT_INTEGER(content, status, 1);
	return true;
}


bool YSMSActiveTestResp::decodeBody(cacti::Stream &content)
{
	YSMS_GET_INTEGER(content, status, 1);
	return true;
}

void YSMSGWReport::checkBuffer()
{
	LineNo.resize(50);
	ReportFlag.resize(50);
	ReportFlagTime.resize(19);
}

bool YSMSGWReport::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&ReportFlag[0], ReportFlag.size()));
	FAIL_RETURN(content.getStream(&ReportFlagTime[0], ReportFlagTime.size()));

	return true;
}

void YSMSTermReport::checkBuffer()
{
	LineNo.resize(50);
	ReportFlag.resize(50);
	ReportFlagTime.resize(19);
}

bool YSMSTermReport::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&ReportFlag[0], ReportFlag.size()));
	FAIL_RETURN(content.getStream(&ReportFlagTime[0], ReportFlagTime.size()));

	return true;
}

YSMSPacketPtr YSMSPacketFactory::createPacket(u8 cmd)
{
	switch(cmd)
	{
	case YSMS_Login:
		return YSMSPacketPtr(new YSMSLogin());
	case YSMS_Login_Resp:
		return YSMSPacketPtr(new YSMSLoginResp());
	case YSMS_Submit:
		return YSMSPacketPtr(new YSMSSubmit());
	case YSMS_Deliver:
		return YSMSPacketPtr(new YSMSDeliver());

	case YSMS_Submit_Group:
		return YSMSPacketPtr(new YSMSSubmitGroup());	
	case YSMS_Submit_Group_Resp:
		return YSMSPacketPtr(new YSMSSubmitGroupResp());

	case YSMS_Active_Test:
		return YSMSPacketPtr(new YSMSActiveTest());
	case YSMS_Active_Test_Resp:
		return YSMSPacketPtr(new YSMSActiveTestResp());

	case YSMS_GW_REPORT:
		return YSMSPacketPtr(new YSMSGWReport());
	case YSMS_TERM_REPORT:
		return YSMSPacketPtr(new YSMSTermReport());

	default:
		return YSMSPacketPtr(new YSMSPacket(cmd));
	}
}