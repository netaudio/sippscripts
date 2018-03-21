#ifndef __SMGP_PACKET_H
#define __SMGP_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "base/msgtlv.h"
#include "smgpdef.h"

using namespace std;
using namespace cacti;
using namespace SMGP;

class SMGPHeader
{
public:
	SMGPHeader(u32 cmd)
		:totalLength(12),
		command(cmd),
		sequence(1)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	SMGPHeader& operator = (const SMGPHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 sequence;
};

class SMGPPacket
{
public:
	SMGPPacket(u32 cmd, u8 ver=SMGP::Version_20)
		:header(cmd)
		,version(ver)
		,smsType(0)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	bool toArray(u8array &content);
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };
	SMGPHeader& getHeader()   { return header; }
	void setHeader(SMGPHeader& head){ header = head; };
//	u32  getStatus() { return header.status; };

	u8  getVersion() { return version; };
	const MsgTLV&  getTLV(u16 tlvType);

	virtual const char* toString(const char* split=" ");

	u8   getSmsType() { return smsType; }
	void setSmsType(u8 utype) { smsType = utype ;}

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	

protected:
	SMGPHeader header;
	MsgTLVList tlvList;

private:
	u8  version;
	MsgTLV m_tlv;
	u8     smsType;//短信类型：0: 用户下行, 15: 群发下行, _SubmitType
};

class SMGPLogin : public SMGPPacket
{
public:
	SMGPLogin(u8 ver=SMGP::Version_20)
		:SMGPPacket(SMGP_Login),
		clientVersion(ver)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[8]客户端用来登录服务器端的用户账号
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

class SMGPLoginResp : public SMGPPacket
{
public:
	SMGPLoginResp()
		:SMGPPacket(SMGP_Login_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u32			status;
	//[16]服务器端返回给客户端的认证码
	u8array		authenticatorServer;
	//服务器端支持的最高版本号
	u8			serverVersion;

};

class SMGPSubmit : public SMGPPacket
{
public:
	SMGPSubmit(u8 ver=SMGP::Version_20);

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//短消息类型
	u8			msgType;
	//SP是否要求返回状态报告
	u8			needReport;
	//短消息发送优先级
	u8			priority;
	//[10]业务代码
	u8array		serviceID;
	//[2]收费类型
	u8array		feeType;
	//[6]资费代码
	u8array		feeCode;
	//[6]包月费/封顶费
	u8array		fixedFee;
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
	//[8]保留 
	u8array		reserve;

	//[1]GSM协议类型
	MsgTLV		TP_pid;
	//[1]GSM协议类型
	MsgTLV		TP_udhi;
	//[20]交易标识
	MsgTLV		LinkID;
	//[8]信息内容的来源
	MsgTLV		MsgSrc;
	//[1]计费用户类型
	MsgTLV		ChargeUserType;
	//[1]计费用户的号码类型
	MsgTLV		ChargeTermType;
	//[Length]计费用户的伪码
	MsgTLV		ChargeTermPseudo;
	//[1]短消息接收方号码的类型
	MsgTLV		DestTermType;
	//[Length]短消息接收方的伪码
	MsgTLV		DestTermPseudo;
	//[1]相同MsgID的消息总条数
	MsgTLV		PkTotal;
	//[1]相同MsgID的消息序号
	MsgTLV		PkNumber;
	//[1]SP发送的消息类型
	MsgTLV		SubmitMsgType;
	//[1]SP对消息的处理结果
	MsgTLV		SPDealResult;
	//[21]业务代码（用于移动网业务）
	MsgTLV		MServiceID;
};

class SMGPSubmitResp : public SMGPPacket
{
public:
	SMGPSubmitResp()
		:SMGPPacket(SMGP_Submit_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
	//请求返回结果
	u32			status;
}; 

class SMGPDeliver : public SMGPPacket
{
public:
	SMGPDeliver(u8 ver=SMGP::Version_20);

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
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

	//[1]GSM协议类型
	MsgTLV		TP_pid;
	//[1]GSM协议类型
	MsgTLV		TP_udhi;
	//[20]交易标识
	MsgTLV		LinkID;
	//[1]短消息发送方的号码类型
	MsgTLV		SrcTermType;
	//[Length]短消息发送方的伪码
	MsgTLV		SrcTermPseudo;
	//[1]SP发送的消息类型
	MsgTLV		SubmitMsgType;
	//[1]SP对消息的处理结果
	MsgTLV		SPDealResult;
};

class SMGPDeliverResp : public SMGPPacket
{
public:
	SMGPDeliverResp()
		:SMGPPacket(SMGP_Deliver_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u8array		msgID;
	//请求返回结果
	u32			status;
};

class SMGPActiveTest : public SMGPPacket
{
public:
	SMGPActiveTest()
		:SMGPPacket(SMGP_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class SMGPActiveTestResp : public SMGPPacket
{
public:
	SMGPActiveTestResp()
		:SMGPPacket(SMGP_Active_Test_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

class SMGPExit : public SMGPPacket
{
public:
	SMGPExit()
		:SMGPPacket(SMGP_Exit)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class SMGPExitResp : public SMGPPacket
{
public:
	SMGPExitResp()
		:SMGPPacket(SMGP_Exit_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

typedef boost::shared_ptr<SMGPPacket> SMGPPacketPtr;

class SMGPPacketFactory
{
public:
	static SMGPPacketPtr createPacket(u32 cmd, u8 ver=SMGP::Version_20);
};



#endif
