#include "stdafx.h"

#include "SMPPpacket.h"
#include <time.h>

#define new DEBUG_NEW


bool SMPPHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(command_status));
	FAIL_RETURN(content.get32(sequence));
	return true;
}


bool SMPPHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(command_status);
	content.put32(sequence);

	return true;
}


SMPPHeader& SMPPHeader::operator = (const SMPPHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		command_status = other.command_status;
		sequence = other.sequence;

	}

	return *this;
}

bool SMPPPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool SMPPPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > SMPP::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool SMPPPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SMPPPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


void SMPPLogin::checkBuffer()
{
}

bool SMPPLogin::encodeBody(cacti::Stream &content)
{
	return true;
}
bool SMPPLogin::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getString(systemId));
	FAIL_RETURN(content.getString(password));
	FAIL_RETURN(content.getString(systemType));
	FAIL_RETURN(content.get8(version));
	FAIL_RETURN(content.get8(Ton));
	FAIL_RETURN(content.get8(Npi));
	FAIL_RETURN(content.getString(addressRange));
	return true;
}

void SMPPLoginResp::checkBuffer()
{
}

bool SMPPLoginResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getString(systemId));
	return true;
}

bool SMPPLoginResp::encodeBody(cacti::Stream &content)
{
	content.putString(systemId);
	return true;

}

void SMPPSubmit::checkBuffer()
{
}

bool  SMPPSubmit::decodeBody(cacti::Stream &content)
{	
	FAIL_RETURN(content.getString(service_type));
	FAIL_RETURN(content.get8(source_address_ton));
	FAIL_RETURN(content.get8(source_address_npi));
	FAIL_RETURN(content.getString(source_address));
	FAIL_RETURN(content.get8(dest_address_ton));
	FAIL_RETURN(content.get8(dest_address_np));
	FAIL_RETURN(content.getString(destination_address));
	FAIL_RETURN(content.get8(esm_class));
	FAIL_RETURN(content.get8(protocol_ID));
	FAIL_RETURN(content.get8(priority_flag));
	FAIL_RETURN(content.getString(schedule_delivery_time));
	FAIL_RETURN(content.getString(validity_peroid));
	FAIL_RETURN(content.get8(registered_delivery_flag));
	FAIL_RETURN(content.get8(replace_if_present_flag));
	FAIL_RETURN(content.get8(data_coding));
	FAIL_RETURN(content.get8(sm_default_msg_id));
	FAIL_RETURN(content.get8(sm_length));
	u8 buf[256];
	memset( buf,0,sizeof(buf));
	if ( sm_length > 0 && sm_length < 256 )
	{
		content.getStream( (u8 *)&buf,sm_length);
		buf[sm_length] = 0;
	}
	short_message_text = (char*)buf;
	return true;
}

bool SMPPSubmit::encodeBody(cacti::Stream &content)
{
	return true;
}

void SMPPSubmitResp::checkBuffer()
{
}

bool SMPPSubmitResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getString(Message_id));
	return true;
}
bool SMPPSubmitResp::encodeBody(cacti::Stream &content)
{
	content.putString(Message_id);
	return true;
}

void SMPPDeliver::checkBuffer()
{
	msgID.resize(10);
	recvTime.resize(14);
	srcTermID.resize(21);
	destTermID.resize(21);
//	msgContent.resize();
	reserve.resize(8);
}

bool SMPPDeliver::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&msgID[0], msgID.size()));
	FAIL_RETURN(content.get8(isReport));
	FAIL_RETURN(content.get8(msgFormat));
	FAIL_RETURN(content.getStream(&recvTime[0], recvTime.size()));
	FAIL_RETURN(content.getStream(&srcTermID[0], srcTermID.size()));
	FAIL_RETURN(content.getStream(&destTermID[0], destTermID.size()));
	FAIL_RETURN(content.get8(msgLength));
	if(msgLength > 0)
	{
		msgContent.resize(msgLength);
		FAIL_RETURN(content.getStream(&msgContent[0], msgLength));
	}
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));

	return true;
}

void SMPPDeliverResp::checkBuffer()
{
	msgID.resize(10);
}

bool SMPPDeliverResp::encodeBody(cacti::Stream &content)
{
	content.putStream(&msgID[0], msgID.size());
	content.put32(status);

	return true;
}


SMPPPacketPtr SMPPPacketFactory::createPacket(u32 cmd)
{
	switch(cmd)
	{
	case SMPP_BindRecv:
	case SMPP_BindTrn:
		return SMPPPacketPtr(new SMPPLogin());
	case SMPP_BindRecv_Resp:
	case SMPP_BindTrn_Resp:
		return SMPPPacketPtr(new SMPPLoginResp());
	case SMPP_Submit:
		return SMPPPacketPtr(new SMPPSubmit());
	case SMPP_Submit_Resp:
		return SMPPPacketPtr(new SMPPSubmitResp());
	case SMPP_Deliver:
		return SMPPPacketPtr(new SMPPDeliver());	
	case SMPP_Deliver_Resp:
		return SMPPPacketPtr(new SMPPDeliverResp());
	default:
		return SMPPPacketPtr(new SMPPPacket(cmd));
	}
}