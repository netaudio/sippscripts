#include "stdafx.h"

#include <cacti/util/MD5.h>
#include <cacti/util/StringUtil.h>
#include "cmpppacket.h"
#include <time.h>

#define new DEBUG_NEW


bool CMPPHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(sequence));

	return true;
}


bool CMPPHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(sequence);

	return true;
}


CMPPHeader& CMPPHeader::operator = (const CMPPHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		sequence = other.sequence;
	}

	return *this;
}

bool CMPPPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool CMPPPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > CMPP::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool CMPPPacket::toArray(u8array &content)
{
	cacti::Stream buf;
	toStream(buf);
	buf.reset();
	content.resize(buf.size());
	buf.getStream(&content[0], buf.size());
	return true;
}
bool CMPPPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPPacket::decodeBody(cacti::Stream &content)
{
	return true;
}



const char* CMPPPacket::toString(const char* split)
{
	return split;	
}


void CMPPLogin::checkBuffer()
{
	sourceAddr.resize(6);
	authenticatorSource.resize(16);
}

bool CMPPLogin::encodeBody(cacti::Stream &content)
{
	content.putStream(&sourceAddr[0], sourceAddr.size());
	content.putStream(&authenticatorSource[0], authenticatorSource.size());
	content.put8(version);
	content.put32(timeStamp);
	

	return true;
}

bool CMPPLogin::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&sourceAddr[0], sourceAddr.size()));
	FAIL_RETURN(content.getStream(&authenticatorSource[0], authenticatorSource.size()));
	FAIL_RETURN(content.get8(version));
	FAIL_RETURN(content.get32(timeStamp));
	

	return true;
}


void CMPPLoginResp::checkBuffer()
{
	authenticatorISMG.resize(16);
}

bool CMPPLoginResp::encodeBody(cacti::Stream &content)
{
	content.put32(status);
	content.putStream(&authenticatorISMG[0], authenticatorISMG.size());
	content.put8(version);

	return true;
}

bool CMPPLoginResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(status));
	FAIL_RETURN(content.getStream(&authenticatorISMG[0], authenticatorISMG.size()));
	FAIL_RETURN(content.get8(version));

	return true;
}

CMPPSubmit::CMPPSubmit(u8 ver)
	:CMPPPacket(CMPP_Submit, ver)
	,TP_pId(0)
	,TP_udhi(0)
	,DestUsr_tl(1)
	,Dest_Terminal_Type(0)
{

};

void CMPPSubmit::checkBuffer()
{
	Service_Id.resize(10);
	Fee_terminal_Id.resize(32);
	Msg_src.resize(6);
	FeeType.resize(2);
	FeeCode.resize(6);
	ValId_Time.resize(17);
	At_Time.resize(17);
	Src_Id.resize(21);
	//Dest_terminal_Id.resize(32);
	LinkID.resize(20);
}

bool CMPPSubmit::encodeBody(cacti::Stream &content)
{
//	checkBuffer();

//	content.put32();

	content.put64(Msg_Id);
	content.put8(Pk_total);
	content.put8(Pk_number);
	content.put8(Registered_Delivery);
	content.put8(Msg_level);
	content.putStream(&Service_Id[0],Service_Id.size());
	content.put8(Fee_UserType);
	content.putStream(&Fee_terminal_Id[0],Fee_terminal_Id.size());
	content.put8(Fee_terminal_type);
	content.put8(TP_pId);
	content.put8(TP_udhi);
	content.put8(Msg_Fmt);
	content.putStream(&Msg_src[0],Msg_src.size());
	content.putStream(&FeeType[0],FeeType.size());
	content.putStream(&FeeCode[0],FeeCode.size());
	content.putStream(&ValId_Time[0],ValId_Time.size());
	content.putStream(&At_Time[0],At_Time.size());
	content.putStream(&Src_Id[0],Src_Id.size());
	content.put8(DestUsr_tl);
	Dest_Terminal_Id.resize(32*DestUsr_tl);
	content.putStream(&Dest_Terminal_Id[0],Dest_Terminal_Id.size());
	content.put8(Dest_Terminal_Type);
	content.put8(Msg_Length);
	if(Msg_Length>0){
		Msg_Content.resize(Msg_Length);
		content.putStream(&Msg_Content[0],Msg_Content.size());
	}else{
		return false;
	}
	content.putStream(&LinkID[0],LinkID.size());
	return true;
}

bool CMPPSubmit::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();
	FAIL_RETURN(content.get64(Msg_Id));
	FAIL_RETURN(content.get8(Pk_total));
	FAIL_RETURN(content.get8(Pk_number));
	FAIL_RETURN(content.get8(Registered_Delivery));
	FAIL_RETURN(content.get8(Msg_level));
	FAIL_RETURN(content.getStream(&Service_Id[0],Service_Id.size()));
	FAIL_RETURN(content.get8(Fee_UserType));
	FAIL_RETURN(content.getStream(&Fee_terminal_Id[0],Fee_terminal_Id.size()));
	FAIL_RETURN(content.get8(Fee_terminal_type));
	FAIL_RETURN(content.get8(TP_pId));
	FAIL_RETURN(content.get8(TP_udhi));
	FAIL_RETURN(content.get8(Msg_Fmt));
	FAIL_RETURN(content.getStream(&Msg_src[0],Msg_src.size()));
	FAIL_RETURN(content.getStream(&FeeType[0],FeeType.size()));
	FAIL_RETURN(content.getStream(&FeeCode[0],FeeCode.size()));
	FAIL_RETURN(content.getStream(&ValId_Time[0],ValId_Time.size()));
	FAIL_RETURN(content.getStream(&At_Time[0],At_Time.size()));
	FAIL_RETURN(content.getStream(&Src_Id[0],Src_Id.size()));
	FAIL_RETURN(content.get8(DestUsr_tl));
	Dest_Terminal_Id.resize(32*DestUsr_tl);
	FAIL_RETURN(content.getStream(&Dest_Terminal_Id[0],Dest_Terminal_Id.size()));
	FAIL_RETURN(content.get8(Dest_Terminal_Type));
	FAIL_RETURN(content.get8(Msg_Length));
	if(Msg_Length>0){
		Msg_Content.resize(Msg_Length);
		FAIL_RETURN(content.getStream(&Msg_Content[0],Msg_Content.size()));
	}else{
		return false;
	}
	FAIL_RETURN(content.getStream(&LinkID[0],LinkID.size()));
	
	return true;
}

 const char* CMPPSubmit::toString(const char* split){


	if(!m_tmp.empty())
	{
		return m_tmp.c_str();
	}
	string tmp;
	m_tmp.append(StringUtil::format("%I64d",Msg_Id));
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Pk_total));
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Pk_number));
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Registered_Delivery));
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Msg_level));
	m_tmp.append(split);
	tmp.assign(Service_Id.begin(),Service_Id.end());
	m_tmp.append(tmp.c_str());
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Fee_UserType));
	m_tmp.append(split);
	tmp.assign(Fee_terminal_Id.begin(),Fee_terminal_Id.end());
	m_tmp.append(tmp.c_str());
	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Fee_terminal_type));

	m_tmp.append(split);
	m_tmp.append(StringUtil::format("format:%d",Msg_Fmt));

	m_tmp.append(split);
	tmp.assign(Msg_src.begin(),Msg_src.end());
	m_tmp.append(tmp.c_str());

	m_tmp.append(split);
	tmp.assign(FeeType.begin(),FeeType.end());
	m_tmp.append(tmp.c_str());

	m_tmp.append(split);
	tmp.assign(FeeCode.begin(),FeeCode.end());
	m_tmp.append(tmp.c_str());

	//m_tmp.append(split);
	//tmp.assign(ValId_Time.begin(),ValId_Time.end());
	//m_tmp.append(tmp.c_str());

	//m_tmp.append(split);
	//tmp.assign(At_Time.begin(),At_Time.end());
	//m_tmp.append(tmp.c_str());

	m_tmp.append(split);
	tmp.assign(Src_Id.begin(),Src_Id.end());
	m_tmp.append(tmp.c_str());

	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",DestUsr_tl));

	m_tmp.append(split);
	tmp.assign(Dest_Terminal_Id.begin(),Dest_Terminal_Id.end());
	m_tmp.append(tmp.c_str());

	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Dest_Terminal_Type));

	m_tmp.append(split);
	m_tmp.append(StringUtil::format("%d",Msg_Length));

	m_tmp.append(split);
	tmp.assign(Msg_Content.begin(),Msg_Content.end());
	m_tmp.append(tmp.c_str());


	m_tmp.append(split);
	tmp.assign(LinkID.begin(),LinkID.end());
	m_tmp.append(tmp.c_str());

	return m_tmp.c_str();



}



void CMPPSubmitResp::checkBuffer()
{
	//Msg_Id.resize(8);
}

bool CMPPSubmitResp::encodeBody(cacti::Stream &content)
{
	content.put64(Msg_Id);
	content.put32(Result);

	return true;
}

bool CMPPSubmitResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get64(Msg_Id));
	FAIL_RETURN(content.get32(Result));

	return true;
}


CMPPDeliver::CMPPDeliver(u8 ver)
	:CMPPPacket(CMPP_Deliver, ver),
	TP_pid(CMPP::Tag_TP_pid),
	TP_udhi(CMPP::Tag_TP_udhi),
	LinkID(CMPP::Tag_LinkID),m_report(false)
	
{

};

void CMPPDeliver::checkBuffer()
{
	//Msg_Id.resize(8);
	Dest_Id.resize(21);
	Service_Id.resize(10);
	Src_terminal_Id.resize(32);
	LinkID.resize(20);
}

bool CMPPDeliver::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	content.put64(Msg_Id);
	content.putStream(&Dest_Id[0], Dest_Id.size());
	content.putStream(&Service_Id[0], Service_Id.size());
	content.put8(TP_pid);
	content.put8(TP_udhi);
	content.put8(Msg_Fmt);
	content.putStream(&Src_terminal_Id[0], Src_terminal_Id.size());
	content.put8(Src_terminal_type);
	content.put8(Registered_Delivery);
	content.put8(Msg_Length);
	if(Msg_Length>0){
		Msg_Content.resize(Msg_Length);
		content.putStream(&Msg_Content[0], Msg_Content.size());
	}
	
	content.putStream(&LinkID[0], LinkID.size());
	return true;
}

bool CMPPDeliver::decodeBody(cacti::Stream &content)
{
//	checkBuffer();

	
	FAIL_RETURN(content.get64(Msg_Id));
	FAIL_RETURN(content.getStream(&Dest_Id[0], Dest_Id.size()));
	FAIL_RETURN(content.getStream(&Service_Id[0], Service_Id.size()));
	FAIL_RETURN(content.get8(TP_pid));
	FAIL_RETURN(content.get8(TP_udhi));
	FAIL_RETURN(content.get8(Msg_Fmt));
	FAIL_RETURN(content.getStream(&Src_terminal_Id[0], Src_terminal_Id.size()));
	FAIL_RETURN(content.get8(Src_terminal_type));
	FAIL_RETURN(content.get8(Registered_Delivery));
	FAIL_RETURN(content.get8(Msg_Length));
	if(Msg_Length>0){
		Msg_Content.resize(Msg_Length);
		FAIL_RETURN(content.getStream(&Msg_Content[0], Msg_Content.size()));
	}

	FAIL_RETURN(content.getStream(&LinkID[0], LinkID.size()));

	if(Registered_Delivery==(u8)1){
		m_report=true;
		Stream reportcontent;
		reportcontent.putStream(&Msg_Content[0], Msg_Content.size());
		report.checkBuffer();
		report.decodeBody(reportcontent);
	}
	return true;
}

void CMPPReport::checkBuffer(){
	//Msg_Id.resize(8);
	Stat.resize(7);
	Submit_time.resize(10);
	Done_time.resize(10);
	Dest_terminal_Id.resize(32);
	
}


bool CMPPReport::encodeBody(cacti::Stream &content){
	content.put64(Msg_Id);
	content.putStream(&Stat[0], Stat.size());
	content.putStream(&Submit_time[0], Submit_time.size());
	content.putStream(&Done_time[0], Done_time.size());
	content.putStream(&Dest_terminal_Id[0], Dest_terminal_Id.size());
	content.put32(SMSC_sequence);
	return true;
}



bool CMPPReport::decodeBody(cacti::Stream &content){

	FAIL_RETURN(content.get64(Msg_Id));
	FAIL_RETURN(content.getStream(&Stat[0], Stat.size()));
	FAIL_RETURN(content.getStream(&Submit_time[0], Submit_time.size()));
	FAIL_RETURN(content.getStream(&Done_time[0], Done_time.size()));
	FAIL_RETURN(content.getStream(&Dest_terminal_Id[0], Dest_terminal_Id.size()));
	return true;
	
}


void CMPPDeliverResp::checkBuffer()
{
	//Msg_Id.resize(10);
}

bool CMPPDeliverResp::encodeBody(cacti::Stream &content)
{
	content.put64(Msg_Id);
	content.put32(Result);

	return true;
}

bool CMPPDeliverResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get64(Msg_Id));
	FAIL_RETURN(content.get32(Result));

	return true;
}

bool CMPPExit::encodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPExit::decodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPExitResp::encodeBody(cacti::Stream &content)
{
	return true;
}


bool CMPPExitResp::decodeBody(cacti::Stream &content)
{
	return true;
}


bool CMPPActiveTest::encodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPActiveTest::decodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPActiveTestResp::encodeBody(cacti::Stream &content)
{
	return true;
}

bool CMPPActiveTestResp::decodeBody(cacti::Stream &content)
{
	return true;
}

CMPPPacketPtr CMPPPacketFactory::createPacket(u32 cmd, u8 ver)
{
	switch(cmd)
	{
	case CMPP_Login:
		return CMPPPacketPtr(new CMPPLogin());
	case CMPP_Login_Resp:
		return CMPPPacketPtr(new CMPPLoginResp());
	case CMPP_Exit:
		return CMPPPacketPtr(new CMPPExit());
	case CMPP_Exit_Resp:
		return CMPPPacketPtr(new CMPPExitResp());
	case CMPP_Submit:
		return CMPPPacketPtr(new CMPPSubmit(ver));
	case CMPP_Submit_Resp:
		return CMPPPacketPtr(new CMPPSubmitResp());
	case CMPP_Deliver:
		return CMPPPacketPtr(new CMPPDeliver(ver));	
	case CMPP_Deliver_Resp:
		return CMPPPacketPtr(new CMPPDeliverResp());
	case CMPP_Active_Test:
		return CMPPPacketPtr(new CMPPActiveTest());
	case CMPP_Active_Test_Resp:
		return CMPPPacketPtr(new CMPPActiveTestResp());
	default:
		return CMPPPacketPtr(new CMPPPacket(cmd));
	}
}