#ifndef __CNGP_PACKET_H
#define __CNGP_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "cngpdef.h"

using namespace std;
using namespace cacti;
using namespace CNGP;

struct CNGPHeader
{
public:
	CNGPHeader(u32 cmd)
		:totalLength(16),
		command(cmd),
		status(0),
		sequence(1)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	CNGPHeader& operator = (const CNGPHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 status;
	u32 sequence;
};

struct CNGPPacket
{
public:
	CNGPPacket(u32 cmd)
		:header(cmd)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };
	CNGPHeader& getHeader()   { return header; }
	void setHeader(CNGPHeader& head){ header = head; };
	u32  getStatus() { return header.status; };
	void setStatus(u32 status) { header.status = status; };

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);

	CNGPHeader header;	
};

struct CNGPLogin : public CNGPPacket
{
public:
	CNGPLogin()
		:CNGPPacket(CNGP_Login)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]客户端用来登录服务器端的用户账号
	u8array		clientID;
	//[16]客户端认证码，用来鉴别客户端的合法性
	u8array		authenticatorClient;
	//客户端用来登录服务器端的登录类型
	u8			loginMode;
	//时间戳
	u32			timeStamp;	
	//客户端支持的协议版本号
	u8			clientVersion;
};

struct CNGPLoginResp : public CNGPPacket
{
public:
	CNGPLoginResp()
		:CNGPPacket(CNGP_Login_Resp)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[16]服务器端返回给客户端的认证码
	u8array		authenticatorServer;
	//服务器端支持的最高版本号
	u8			serverVersion;

};

struct CNGPSubmit : public CNGPPacket
{
public:
	CNGPSubmit()
		:CNGPPacket(CNGP_Submit)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]客户端用来登录服务器端的用户账号
	u8array		clientID;
	//短消息类型
	u8			subType;
	//SP是否要求返回状态报告
	u8			needReport;
	//短消息发送优先级
	u8			priority;
	//[10]业务代码
	u8array		serviceID;
	//[2]收费类型
	u8array		feeType;
	//[1]收费对象类型
	u8			feeUserType;
	//[6]资费代码
	u8array		feeCode;
	//短消息格式
	u8			msgFormat;
	//[17]短消息有效时间
	u8array		validTime;
	//[17]短消息定时发送时间
	u8array		atTime;
	//[21]短信息发送方号码
	u8array		srcTermID;
	//[21]计费用户号码
	u8array		chargeTermID;
	//短消息接收号码总数
	u8			destTermIDCount;
	//[21*DestTermCount]短消息接收号码 
	u8array		destTermID;
	//短消息长度
	u8			msgLength;
	//[MsgLength]短消息内容
	u8array		msgContent;
};

struct CNGPSubmitResp : public CNGPPacket
{
public:
	CNGPSubmitResp()
		:CNGPPacket(CNGP_Submit_Resp)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
}; 

struct CNGPDeliver : public CNGPPacket
{
public:
	CNGPDeliver()
		:CNGPPacket(CNGP_Deliver)
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
};

struct CNGPDeliverResp : public CNGPPacket
{
public:
	CNGPDeliverResp()
		:CNGPPacket(CNGP_Deliver_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
};

struct CNGPActiveTest : public CNGPPacket
{
public:
	CNGPActiveTest()
		:CNGPPacket(CNGP_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:

};

struct CNGPActiveTestResp : public CNGPPacket
{
public:
	CNGPActiveTestResp()
		:CNGPPacket(CNGP_Active_Test_Resp)
	{};

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

struct CNGPExit : public CNGPPacket
{
public:
	CNGPExit()
		:CNGPPacket(CNGP_Exit)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:

};

struct CNGPExitResp : public CNGPPacket
{
public:
	CNGPExitResp()
		:CNGPPacket(CNGP_Exit_Resp)
	{};

private:
	bool decodeBody(cacti::Stream &content);

public:
}; 

typedef boost::shared_ptr<CNGPPacket> CNGPPacketPtr;

class CNGPPacketFactory
{
public:
	static CNGPPacketPtr createPacket(u32 cmd);
};


struct CNGPQueryUserState : public CNGPPacket
{
public:
	CNGPQueryUserState()
		:CNGPPacket(CNGP_Query_UserState),
		userAddr(21),smgwNo(6)
	{};

	bool encodeBody(cacti::Stream &content);
	u8array userAddr;
	u8array	smgwNo;

};

struct CNGPQueryUserStateResp : public CNGPPacket
{
public:
	CNGPQueryUserStateResp()
		:CNGPPacket(CNGP_Query_UserState_Resp),userStatus(1)

	{};

	bool decodeBody(cacti::Stream &content);
	u8array	 userStatus;
	u32	 count;

};

struct CNGPPaymentRequest : public CNGPPacket
{
public:
	CNGPPaymentRequest()
		:CNGPPacket(CNGP_Payment_Request),
		messagId(10),chargeTermId(21),spId(10),destTermId(21),serviceId(10),
		feeType(2),feeCode(6),areaCode(4),smgwNo(6),fwdSmgwNo(6),smscNo(6),recvTime(14),doneTime(14)
	{};

	bool encodeBody(cacti::Stream &content);
	u8array		messagId;
	u8			smType;
	u8			subType;
	u8array		chargeTermId;
	u8array		spId;
	u8array		destTermId;
	u8array		serviceId;
	u8array		feeType;
	u8array		feeCode;
	u8			priority;
	u8			msgLength;
	u8array		areaCode;
	u8array     smgwNo;
	u8array		fwdSmgwNo;
	u8array		smscNo;
	u8array		recvTime;
	u8array		doneTime;

};

struct CNGPPaymentRequestResp : public CNGPPacket
{
public:
	CNGPPaymentRequestResp()
		:CNGPPacket(CNGP_Payment_Request_Resp)
	{};

	bool decodeBody(cacti::Stream &content);
	u8		resultNotifyCode;

};

struct CNGPPaymentAffirm : public CNGPPacket
{
public:
	CNGPPaymentAffirm()
		:CNGPPacket(CNGP_Payment_Affirm),
		messagId(10),feeAddr(21)
	{};

	bool encodeBody(cacti::Stream &content);
	u8array		messagId;
	u8			responseStatus;
	u8array		feeAddr;

};

struct CNGPPaymentAffirmResp : public CNGPPacket
{
public:
	CNGPPaymentAffirmResp()
		:CNGPPacket(CNGP_Payment_Affirm_Resp)
	{};

	bool decodeBody(cacti::Stream &content);
};

#endif
