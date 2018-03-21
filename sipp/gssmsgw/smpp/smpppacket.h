#ifndef __SMPP_PACKET_H
#define __SMPP_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "smppdef.h"

using namespace std;
using namespace cacti;
using namespace SMPP;


struct SMPPHeader
{
public:
	SMPPHeader(u32 cmd , u32 status=0)
		:totalLength(12),
		command(cmd),
		command_status(status),
		sequence(1)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	SMPPHeader& operator = (const SMPPHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 command_status;
	u32 sequence;
};

struct SMPPPacket
{
public:
	SMPPPacket(u32 cmd)
		:header(cmd)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };
	SMPPHeader& getHeader()   { return header; }
	void setHeader(SMPPHeader& head){ header = head; };
//	u32  getStatus() { return header.status; };

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);

	SMPPHeader header;	
};

struct SMPPLogin : public SMPPPacket
{
public:
	SMPPLogin()
		:SMPPPacket(SMPP_BindTrn)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	std::string	 systemId;
	std::string  password;
	std::string  systemType;
	u8		version;
	u8		Ton;
	u8		Npi;
	std::string	 addressRange;
};

struct SMPPLoginResp : public SMPPPacket
{
public:
	SMPPLoginResp()
		:SMPPPacket(SMPP_BindTrn_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	std::string systemId;
};

struct SMPPSubmit : public SMPPPacket
{
public:
	SMPPSubmit()
		:SMPPPacket(SMPP_Submit)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	std::string		service_type;
	u8				source_address_ton;
	u8				source_address_npi;
	std::string		source_address;
	u8				dest_address_ton;
	u8				dest_address_np;
	std::string		destination_address;
	u8				esm_class;
	u8				protocol_ID;
	u8				priority_flag;
	std::string		schedule_delivery_time;
	std::string     validity_peroid;
	u8				registered_delivery_flag;
	u8				replace_if_present_flag;
	u8				data_coding;
	u8				sm_default_msg_id;
	u8				sm_length;
	std::string		short_message_text;

};

struct SMPPSubmitResp : public SMPPPacket
{
public:
	SMPPSubmitResp()
		:SMPPPacket(SMPP_Submit_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	std::string Message_id;
}; 

struct SMPPDeliver : public SMPPPacket
{
public:
	SMPPDeliver()
		:SMPPPacket(SMPP_Deliver)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
	//是否为状态报告
	u8			isReport;
	//短消息格式
	u8			msgFormat;
	//[14]短消息接收时间
	u8array		recvTime;
	//[21]短消息发送号码
	u8array		srcTermID;
	//[21]短消息接收号码
	u8array		destTermID;
	//短消息长度
	u8			msgLength;
	//短消息内容
	u8array		msgContent;
	//[8]保留
	u8array		reserve;
};

struct SMPPDeliverResp : public SMPPPacket
{
public:
	SMPPDeliverResp()
		:SMPPPacket(SMPP_Deliver_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
	//请求返回结果
	u32			status;
};



typedef boost::shared_ptr<SMPPPacket> SMPPPacketPtr;

class SMPPPacketFactory
{
public:
	static SMPPPacketPtr createPacket(u32 cmd);
};



#endif
