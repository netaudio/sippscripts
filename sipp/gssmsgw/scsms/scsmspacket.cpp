#include "stdafx.h"

#include "scsmspacket.h"

#define new DEBUG_NEW


#define SCSMS_PUT_INTEGER(content, val, size) \
	{ \
	if(size <= 0) return false; \
	u8array buf(size, ' '); \
	SCSMS_ConvertInteger(&buf[0], val, size); \
	content.putStream(&buf[0], size); \
	} \

#define SCSMS_GET_INTEGER(content, val, size) \
	{	\
	if(size <= 0) return false; \
	char buf[size+1];  \
	buf[size] = '\0';  \
	if(content.getStream((u8*)(&buf[0]), size)) \
	val = atoi((char*)&buf[0]); \
		else \
		return false; \
	} \


void SCSMS_REPLACE_32(u8* outVal, const u32 val)
{
	assert(outVal);
	outVal[0] = u8(val);
	outVal[1] = u8(val >> 8);
	outVal[2] = u8(val >> 16);
	outVal[3] = u8(val >> 24);
}

void SCSMS_REPLACE_16(u8* outVal, const u32 val)
{
	assert(outVal);
	outVal[0] = u8(val);
	outVal[1] = u8(val >> 8);
}

void SCSMS_ConvertInteger(u8* outVal, const u32 val, const u32 size)
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

bool SCSMSHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(command));
	u16 tmp = 0;
	FAIL_RETURN(content.get16(tmp));
	totalLength = tmp << 8 | (u8)(tmp >> 8);

	return true;
}


bool SCSMSHeader::toStream(cacti::Stream &content)
{
	content.put8(command);
	u16 tmp = (totalLength << 8) | (u8)(totalLength >> 8);
	content.put16(tmp);

	return true;
}


SCSMSHeader& SCSMSHeader::operator = (const SCSMSHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
	}

	return *this;
}

bool SCSMSPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool SCSMSPacket::toStream(cacti::Stream &content)
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
	if(length <= 0 && length > SCSMS::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer(1);
	SCSMS_REPLACE_16(tmp, length);

	return true;
}

bool SCSMSPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SCSMSPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


void SCSMSLogin::checkBuffer()
{
	clientID.resize(10, SCSMS::FILL_CHAR);
	password.resize(50, SCSMS::FILL_CHAR);
}

bool SCSMSLogin::encodeBody(cacti::Stream &content)
{
	content.putStream(&clientID[0], clientID.size());
	content.putStream(&password[0], password.size());

	return true;
}

bool SCSMSLoginResp::decodeBody(cacti::Stream &content)
{
	SCSMS_GET_INTEGER(content, status, 1);

	return true;
}

void SCSMSDeliver::checkBuffer()
{
	LineNo.resize(50);
	Msg_Id.resize(50);
	Dest_Id.resize(21);
	Src_Id.resize(32);
	Service_Code.resize(50);
	Msg_Content.resize(255);
	LinkId.resize(50);
}

bool SCSMSDeliver::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&Msg_Id[0], Msg_Id.size()));
	FAIL_RETURN(content.getStream(&Dest_Id[0], Dest_Id.size()));
	SCSMS_GET_INTEGER(content, Dest_Id_Type, 1);
	FAIL_RETURN(content.getStream(&Src_Id[0], Src_Id.size()));
	SCSMS_GET_INTEGER(content, Src_Id_Type, 1);
	FAIL_RETURN(content.getStream(&Service_Code[0], Service_Code.size()));
	SCSMS_GET_INTEGER(content, Msg_Fmt, 2);
	FAIL_RETURN(content.getStream(&Msg_Content[0], Msg_Content.size()));
	FAIL_RETURN(content.getStream(&LinkId[0], LinkId.size()));

	return true;
}

void SCSMSSubmit::checkBuffer()
{
	Msg_Kind.resize(10, SCSMS::FILL_CHAR);
	LineNo.resize(50, SCSMS::FILL_CHAR);
	Service_Code.resize(10, SCSMS::FILL_CHAR);
	FeeType.resize(6, SCSMS::FILL_CHAR);
	FeeCode.resize(6, SCSMS::FILL_CHAR);
	FixedFee.resize(6, SCSMS::FILL_CHAR);
	GivenValue.resize(6, SCSMS::FILL_CHAR);
	Fee_terminal_Id.resize(32, SCSMS::FILL_CHAR);
	Src_Id.resize(21, SCSMS::FILL_CHAR);
	Dest_Id.resize(32, SCSMS::FILL_CHAR);
	Msg_Content.resize(255, SCSMS::FILL_CHAR);
	LinkID.resize(50, SCSMS::FILL_CHAR);
	SpCode.resize(10, SCSMS::FILL_CHAR);
	ServiceID.resize(10, SCSMS::FILL_CHAR);
}

bool SCSMSSubmit::encodeBody(cacti::Stream &content)
{
	checkBuffer();

	content.putStream(&Msg_Kind[0], Msg_Kind.size());		
	content.putStream(&LineNo[0], LineNo.size());
	SCSMS_PUT_INTEGER(content, Pk_total, 1);
	SCSMS_PUT_INTEGER(content, Pk_number, 1);
	content.put8(Fee_UserType);
	content.putStream(&Service_Code[0], Service_Code.size());
	content.putStream(&FeeType[0], FeeType.size());
	content.putStream(&FeeCode[0], FeeCode.size());
	content.putStream(&FixedFee[0], FixedFee.size());
	content.putStream(&GivenValue[0], GivenValue.size());
	content.putStream(&Fee_terminal_Id[0], Fee_terminal_Id.size());
	SCSMS_PUT_INTEGER(content, Fee_teminal_type, 1);
	SCSMS_PUT_INTEGER(content, Msg_Fmt, 2);
	content.putStream(&Src_Id[0], Src_Id.size());
	content.putStream(&Dest_Id[0], Dest_Id.size());
	content.put8(Dest_Id_type);
	content.putStream(&Msg_Content[0], Msg_Content.size());
	content.putStream(&LinkID[0], LinkID.size());
	//content.put8(MsgType);
	SCSMS_PUT_INTEGER(content,MsgType,1);
	content.putStream(&SpCode[0], SpCode.size());
	content.putStream(&ServiceID[0], ServiceID.size());

	return true;
}

void SCSMSSubmitGroup::checkBuffer()
{
	Msg_Kind.resize(10, SCSMS::FILL_CHAR);
	LineNo.resize(50, SCSMS::FILL_CHAR);
	Service_Code.resize(10, SCSMS::FILL_CHAR);
	FeeType.resize(6, SCSMS::FILL_CHAR);
	FeeCode.resize(6, SCSMS::FILL_CHAR);
	FixedFee.resize(6, SCSMS::FILL_CHAR);
	GivenValue.resize(6, SCSMS::FILL_CHAR);
	Fee_terminal_Id.resize(32, SCSMS::FILL_CHAR);
	ValidTime.resize(19, SCSMS::FILL_CHAR);
	AtTime.resize(19, SCSMS::FILL_CHAR);
	Src_Id.resize(21, SCSMS::FILL_CHAR);
	Dest_Id.resize(32, SCSMS::FILL_CHAR);
	Msg_Content.resize(255, SCSMS::FILL_CHAR);
	LinkID.resize(50, SCSMS::FILL_CHAR);
	SpCode.resize(10, SCSMS::FILL_CHAR);
	ServiceID.resize(10, SCSMS::FILL_CHAR);
}

bool SCSMSSubmitGroup::encodeBody(cacti::Stream &content)
{
	checkBuffer();

	content.putStream(&Msg_Kind[0], Msg_Kind.size());		
	content.putStream(&LineNo[0], LineNo.size());
	SCSMS_PUT_INTEGER(content, Pk_total, 1);
	SCSMS_PUT_INTEGER(content, Pk_number, 1);
	content.put8(Fee_UserType);
	content.putStream(&Service_Code[0], Service_Code.size());
	content.putStream(&FeeType[0], FeeType.size());
	content.putStream(&FeeCode[0], FeeCode.size());
	content.putStream(&FixedFee[0], FixedFee.size());
	content.putStream(&GivenValue[0], GivenValue.size());
	content.putStream(&Fee_terminal_Id[0], Fee_terminal_Id.size());
	SCSMS_PUT_INTEGER(content, Fee_teminal_type, 1);
	SCSMS_PUT_INTEGER(content, Msg_Fmt, 2);
	content.putStream(&ValidTime[0], ValidTime.size());
	content.putStream(&AtTime[0], AtTime.size());
	content.putStream(&Src_Id[0], Src_Id.size());
	content.putStream(&Dest_Id[0], Dest_Id.size());
	content.put8(Dest_Id_type);
	content.putStream(&Msg_Content[0], Msg_Content.size());
	content.putStream(&LinkID[0], LinkID.size());
	//content.put8(MsgType);
	SCSMS_PUT_INTEGER(content,MsgType,1);
	content.putStream(&SpCode[0], SpCode.size());
	content.putStream(&ServiceID[0], ServiceID.size());

	return true;
}


bool SCSMSSubmitGroupResp::decodeBody(cacti::Stream &content)
{
	SCSMS_GET_INTEGER(content, status, 1);

	return true;
}

bool SCSMSActiveTest::encodeBody(cacti::Stream &content)
{
	SCSMS_PUT_INTEGER(content, status, 1);
	return true;
}


bool SCSMSActiveTestResp::decodeBody(cacti::Stream &content)
{
	SCSMS_GET_INTEGER(content, status, 1);
	return true;
}

void SCSMSGWReport::checkBuffer()
{
	LineNo.resize(50);
	ReportFlag.resize(50);
	ReportFlagTime.resize(19);
}

bool SCSMSGWReport::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&ReportFlag[0], ReportFlag.size()));
	FAIL_RETURN(content.getStream(&ReportFlagTime[0], ReportFlagTime.size()));

	return true;
}

void SCSMSTermReport::checkBuffer()
{
	LineNo.resize(50);
	ReportFlag.resize(50);
	ReportFlagTime.resize(19);
}

bool SCSMSTermReport::decodeBody(cacti::Stream &content)
{
	checkBuffer();

	FAIL_RETURN(content.getStream(&LineNo[0], LineNo.size()));
	FAIL_RETURN(content.getStream(&ReportFlag[0], ReportFlag.size()));
	FAIL_RETURN(content.getStream(&ReportFlagTime[0], ReportFlagTime.size()));

	return true;
}

SCSMSPacketPtr SCSMSPacketFactory::createPacket(u8 cmd)
{
	switch(cmd)
	{
	case SCSMS_Login:
		return SCSMSPacketPtr(new SCSMSLogin());
	case SCSMS_Login_Resp:
		return SCSMSPacketPtr(new SCSMSLoginResp());
	case SCSMS_Submit:
		return SCSMSPacketPtr(new SCSMSSubmit());
	case SCSMS_Deliver:
		return SCSMSPacketPtr(new SCSMSDeliver());

	case SCSMS_Submit_Group:
		return SCSMSPacketPtr(new SCSMSSubmitGroup());	
	case SCSMS_Submit_Group_Resp:
		return SCSMSPacketPtr(new SCSMSSubmitGroupResp());

	case SCSMS_Active_Test:
		return SCSMSPacketPtr(new SCSMSActiveTest());
	case SCSMS_Active_Test_Resp:
		return SCSMSPacketPtr(new SCSMSActiveTestResp());

	case SCSMS_GW_REPORT:
		return SCSMSPacketPtr(new SCSMSGWReport());
	case SCSMS_TERM_REPORT:
		return SCSMSPacketPtr(new SCSMSTermReport());

	default:
		return SCSMSPacketPtr(new SCSMSPacket(cmd));
	}
}