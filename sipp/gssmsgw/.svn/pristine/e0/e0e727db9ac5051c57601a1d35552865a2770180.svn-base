#ifndef __YLX_PACKET_H
#define __YLX_PACKET_H

#pragma once
#include "base/baseprotocol.h"
#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "base/msgtlv.h"
#include "ylxdef.h"

using namespace std;
using namespace cacti;
using namespace YLX;

class YLXHeader
{
public:
	YLXHeader(u32 cmd)
		:totalLength(12),
		command(cmd),
		sequence(1)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	YLXHeader& operator = (const YLXHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 sequence;  //xxooxx
};

class YLXPacket
{
public:
	YLXPacket(u32 cmd):header(cmd){};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	bool toArray(u8array &content);
	const char* getPacketSequence();
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };


	YLXHeader& getHeader()   { return header; }
	void setHeader(YLXHeader& head){ header = head; };
//	u32  getStatus() { return header.status; };
	
	virtual const char* toString(const char* split=" ");

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	

protected:
	YLXHeader header;
	

private:
	char m_tmp[36];
	
};

class YLXLogin : public YLXPacket
{
public:
	YLXLogin():YLXPacket(YLX_Login){};

protected:
	void checkBuffer(){};

private:
	bool encodeBody(cacti::Stream &content){};
	bool decodeBody(cacti::Stream &content){};

public:

	u8 loginType;
	//[16]登录名
	u8array loginName;
	//[16]登录密码
	u8array password;
	//[8]保留，扩展用
	u8array reserve;
};

//
//class YSMSSubmit : public YLXPacket
//{
//public:
//	YSMSSubmit()
//		:YSMSPacket(YSMS_Submit)
//		,Msg_Kind(10, YSMS::FILL_CHAR)
//		,LineNo(50, YSMS::FILL_CHAR)
//		,Pk_total(1)
//		,Pk_number(1)
//		,Fee_UserType(0)
//		,Service_Code(10, YSMS::FILL_CHAR)
//		,FeeType(6, YSMS::FILL_CHAR)
//		,FeeCode(6, YSMS::FILL_CHAR)
//		,FixedFee(6, YSMS::FILL_CHAR)
//		,GivenValue(6, YSMS::FILL_CHAR)
//		,Fee_terminal_Id(32, YSMS::FILL_CHAR)
//		,Fee_teminal_type(0)
//		,Msg_Fmt(0)
//		,Src_Id(21, YSMS::FILL_CHAR)
//		,Dest_Id(32, YSMS::FILL_CHAR)
//		,Dest_Id_type(0)
//		,Msg_Content(255, YSMS::FILL_CHAR)
//		,LinkID(50, YSMS::FILL_CHAR)
//		,MsgType(0)
//		,SpCode(10, YSMS::FILL_CHAR)
//		,ServiceID(10, YSMS::FILL_CHAR)
//	{};
//
//protected:
//	void checkBuffer();
//
//private:
//	bool encodeBody(cacti::Stream &content);
//	//bool decodeBody(cacti::Stream &content);
//
//public:
//	u8array		Msg_Kind;			//10字节
//	u8array		LineNo;				//50字节
//	u8			Pk_total;			//1字节
//	u8			Pk_number;			//1字节
//	u8			Fee_UserType;		//1字节
//	u8array		Service_Code;		//10字节
//	u8array		FeeType;			//6字节
//	u8array		FeeCode;			//6字节
//	u8array		FixedFee;			//6字节
//	u8array		GivenValue;			//6字节
//	u8array		Fee_terminal_Id;	//32字节
//	u8			Fee_teminal_type;	//1字节
//	u16			Msg_Fmt;			//2字节
//	u8array		Src_Id;				//21字节
//	u8array		Dest_Id;			//32字节
//	u8			Dest_Id_type;		//1字节
//	u8array		Msg_Content;		//255字节
//	u8array		LinkID;				//50字节
//	u8			MsgType;			//1字节
//	u8array		SpCode;				//10字节
//	u8array		ServiceID;			//10字节
//};
//


class YLXSubmit : public YLXPacket
{
public:
	YLXSubmit():
	  YLXPacket(YLX_Submit){};
	virtual const char* toString(const char* split=" ");
protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
	string m_str;

public:
	u8array reserve	;

};

class YLXStat : public YLXPacket
{
public:
	YLXStat():
	  YLXPacket(YLX_Stat){};
protected:
	void checkBuffer(){};

private:
	bool encodeBody(cacti::Stream &content){return true;};
	bool decodeBody(cacti::Stream &content){return true;};
	string m_str;

};


class YLXSubmitResp : public YLXPacket
{
public:
	YLXSubmitResp():YLXPacket(YLX_Submit_Resp){};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8		status;
	//[8]保留，扩展用
	u8array			reserve;
}; 

class YLXDeliver : public YLXPacket
{
public:
	YLXDeliver();

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
		//[21]	Text	发送短消息的用户手机号，手机号码前加“86”国别标志
		u8array userNumber;
		//[21]	Text	SP的接入号码
		u8array spNumber;	
		//1	Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.9
		u8 tpPid;	
		//1	Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.23,仅使用1位，右对齐
		u8 tpUdhi;	
		//	1:	Integer	短消息的编码格式。
		//  0： 纯ASCII字符串
		//	3： 写卡操作
		//	4： 二进制编码
		//	8： UCS2编码
		//	15: GBK编码
		//其它参见GSM3.38第4节：SMS Data Coding Scheme
		u8 messageCoding;
		
		//	4	Integer	短消息的长度
		u32 messageLength;
		//Message Length	Text	短消息的内容
		u8array messageContent;	
		//8	Text	保留，扩展用
		u8array reserve;
};


class YLXDeliverResp : public YLXPacket
{
public:
	YLXDeliverResp():YLXPacket(YLX_Deliver_Resp){};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
		//1	Integer	Deliver命令是否成功接收。
		//0：接收成功
		//其它：错误码
		u8 status;	
		//	[8]	Text	保留，扩展用
		u8array reserve;
};


class YLXReport : public YLXPacket
{
public:
	YLXReport():YLXPacket(YLX_Report){};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

	char m_sequence[36];
public:

		const char* getSubmitSequenceNumber();
		//SubmitSequenceNumber	12	Integer	该命令所涉及的Submit或deliver命令的序列号
		u32 sourceCode;
		u32 sendDate;
		u32 sequence;
		//	1	Integer	Report命令类型
		//0：对先前一条Submit命令的状态报告
		//	1：对先前一条前转Deliver命令的状态报告
		u8 reportType;
		//	[21]	Text	接收短消息的手机号，手机号码前加“86”国别标志
		u8array userNumber;
		//	1	Integer	该命令所涉及的短消息的当前执行状态
		//  0：发送成功
		//  1：等待发送
		//	2：发送失败
		u8 state;
		//	1	Integer	当State=2时为错误码值，否则为0
		u8 errorCode;
		//[8]	Text	保留，扩展用
		u8array reserve;


};

class YLXReportResp : public YLXPacket
{
public:
	YLXReportResp():YLXPacket(YLX_Report_Resp)	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//1	Integer	Deliver命令是否成功接收。
	//0：接收成功
	//其它：错误码
	u8 status;	
	//	[8]	Text	保留，扩展用
	u8array reserve;

};


class YLXActiveTest : public YLXPacket
{
public:
	YLXActiveTest()
		:YLXPacket(YLX_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};


typedef boost::shared_ptr<YLXPacket> YLXPacketPtr;

class YLXPacketFactory
{
public:
	static YLXPacketPtr createPacket(u32 cmd);
};



#endif
