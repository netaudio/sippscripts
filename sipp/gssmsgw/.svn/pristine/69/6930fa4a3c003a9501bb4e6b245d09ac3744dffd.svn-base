#ifndef __YSMS_PACKET_H
#define __YSMS_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "YSMSdef.h"

using namespace std;
using namespace cacti;
using namespace YSMS;

class YSMSHeader
{
public:
	YSMSHeader(u8 cmd)
		:totalLength(0),
		command(cmd)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	YSMSHeader& operator = (const YSMSHeader& other);

public:
	u8  command;
	u16 totalLength;
};

class YSMSPacket
{
public:
	YSMSPacket(u8 cmd)
		:header(cmd)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);

	YSMSHeader& getHeader()   { return header; }
	void setHeader(YSMSHeader& head){ header = head; };

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	
protected:
	YSMSHeader header;
};

class YSMSLogin : public YSMSPacket
{
public:
	YSMSLogin()
		:YSMSPacket(YSMS_Login)
		,clientID(10, YSMS::FILL_CHAR)
		,password(50, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]客户端用来登录服务器端的用户账号
	u8array		clientID;
	//[50]客户端认证密码
	u8array		password;
};

class YSMSLoginResp : public YSMSPacket
{
public:
	YSMSLoginResp()
		:YSMSPacket(YSMS_Login_Resp)
	{};

protected:
	//void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8		status;

};

class YSMSDeliver : public YSMSPacket
{
public:
	YSMSDeliver()
		:YSMSPacket(YSMS_Deliver)
		,LineNo(50, YSMS::FILL_CHAR)
		,Msg_Id(50, YSMS::FILL_CHAR)
		,Dest_Id(21, YSMS::FILL_CHAR)
		,Dest_Id_Type(0)
		,Src_Id(32, YSMS::FILL_CHAR)
		,Src_Id_Type(0)
		,Service_Code(50, YSMS::FILL_CHAR)
		,Msg_Fmt(0)
		,Msg_Content(255, YSMS::FILL_CHAR)
		,LinkId(50, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50字节
	u8array		Msg_Id;				//50字节
	u8array		Dest_Id;			//21字节
	u8			Dest_Id_Type;		//1字节
	u8array		Src_Id;				//32字节
	u8			Src_Id_Type;		//1字节
	u8array		Service_Code;		//50字节
	u16			Msg_Fmt;			//2字节
	u8array		Msg_Content;		//255字节
	u8array		LinkId;				//50字节
};


class YSMSSubmit : public YSMSPacket
{
public:
	YSMSSubmit()
		:YSMSPacket(YSMS_Submit)
		,Msg_Kind(10, YSMS::FILL_CHAR)
		,LineNo(50, YSMS::FILL_CHAR)
		,Pk_total(1)
		,Pk_number(1)
		,Fee_UserType(0)
		,Service_Code(10, YSMS::FILL_CHAR)
		,FeeType(6, YSMS::FILL_CHAR)
		,FeeCode(6, YSMS::FILL_CHAR)
		,FixedFee(6, YSMS::FILL_CHAR)
		,GivenValue(6, YSMS::FILL_CHAR)
		,Fee_terminal_Id(32, YSMS::FILL_CHAR)
		,Fee_teminal_type(0)
		,Msg_Fmt(0)
		,Src_Id(21, YSMS::FILL_CHAR)
		,Dest_Id(32, YSMS::FILL_CHAR)
		,Dest_Id_type(0)
		,Msg_Content(255, YSMS::FILL_CHAR)
		,LinkID(50, YSMS::FILL_CHAR)
		,MsgType(0)
		,SpCode(10, YSMS::FILL_CHAR)
		,ServiceID(10, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	u8array		Msg_Kind;			//10字节
	u8array		LineNo;				//50字节
	u8			Pk_total;			//1字节
	u8			Pk_number;			//1字节
	u8			Fee_UserType;		//1字节
	u8array		Service_Code;		//10字节
	u8array		FeeType;			//6字节
	u8array		FeeCode;			//6字节
	u8array		FixedFee;			//6字节
	u8array		GivenValue;			//6字节
	u8array		Fee_terminal_Id;	//32字节
	u8			Fee_teminal_type;	//1字节
	u16			Msg_Fmt;			//2字节
	u8array		Src_Id;				//21字节
	u8array		Dest_Id;			//32字节
	u8			Dest_Id_type;		//1字节
	u8array		Msg_Content;		//255字节
	u8array		LinkID;				//50字节
	u8			MsgType;			//1字节
	u8array		SpCode;				//10字节
	u8array		ServiceID;			//10字节
};

class YSMSSubmitGroup : public YSMSPacket
{
public:
	YSMSSubmitGroup()
		:YSMSPacket(YSMS_Submit_Group)
		,Msg_Kind(10, YSMS::FILL_CHAR)
		,LineNo(50, YSMS::FILL_CHAR)
		,Pk_total(1)
		,Pk_number(1)
		,Fee_UserType(0)
		,Service_Code(10, YSMS::FILL_CHAR)
		,FeeType(6, YSMS::FILL_CHAR)
		,FeeCode(6, YSMS::FILL_CHAR)
		,FixedFee(6, YSMS::FILL_CHAR)
		,GivenValue(6, YSMS::FILL_CHAR)
		,Fee_terminal_Id(32, YSMS::FILL_CHAR)
		,Fee_teminal_type(0)
		,Msg_Fmt(0)
		,ValidTime(19, YSMS::FILL_CHAR)
		,AtTime(19, YSMS::FILL_CHAR)
		,Src_Id(21, YSMS::FILL_CHAR)
		,Dest_Id(32, YSMS::FILL_CHAR)
		,Dest_Id_type(0)
		,Msg_Content(255, YSMS::FILL_CHAR)
		,LinkID(50, YSMS::FILL_CHAR)
		,MsgType(0)
		,SpCode(10, YSMS::FILL_CHAR)
		,ServiceID(10, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	u8array		Msg_Kind;			//10字节
	u8array		LineNo;				//50字节
	u8			Pk_total;			//1字节
	u8			Pk_number;			//1字节
	u8			Fee_UserType;		//1字节
	u8array		Service_Code;		//10字节
	u8array		FeeType;			//6字节
	u8array		FeeCode;			//6字节
	u8array		FixedFee;			//6字节
	u8array		GivenValue;			//6字节
	u8array		Fee_terminal_Id;	//32字节
	u8			Fee_teminal_type;	//1字节
	u16			Msg_Fmt;			//2字节
	u8array		ValidTime;			//2007-02-11 13:23:56必须按该格式提交，19字节
	u8array		AtTime;				//2007-02-11 13:23:56必须按该格式提交，19字节
	u8array		Src_Id;				//21字节
	u8array		Dest_Id;			//32字节
	u8			Dest_Id_type;		//1字节
	u8array		Msg_Content;		//255字节
	u8array		LinkID;				//50字节
	u8			MsgType;			//1字节
	u8array		SpCode;				//10字节
	u8array		ServiceID;			//10字节
};

class YSMSSubmitGroupResp : public YSMSPacket
{
public:
	YSMSSubmitGroupResp()
		:YSMSPacket(YSMS_Submit_Group_Resp)
	{};

protected:
	//void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8		status;

};

class YSMSActiveTest : public YSMSPacket
{
public:
	YSMSActiveTest()
		:YSMSPacket(YSMS_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8		status;
};

class YSMSActiveTestResp : public YSMSPacket
{
public:
	YSMSActiveTestResp()
		:YSMSPacket(YSMS_Active_Test_Resp)
	{};

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8		status;
}; 

class YSMSGWReport: public YSMSPacket
{
public:
	YSMSGWReport()
		:YSMSPacket(YSMS_GW_REPORT)
		,LineNo(50, YSMS::FILL_CHAR)
		,ReportFlag(50, YSMS::FILL_CHAR)
		,ReportFlagTime(19, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50字节
	u8array		ReportFlag;			//50字节
	u8array		ReportFlagTime;		//19字节
}; 

class YSMSTermReport: public YSMSPacket
{
public:
	YSMSTermReport()
		:YSMSPacket(YSMS_TERM_REPORT)
		,LineNo(50, YSMS::FILL_CHAR)
		,ReportFlag(50, YSMS::FILL_CHAR)
		,ReportFlagTime(19, YSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50字节
	u8array		ReportFlag;			//50字节
	u8array		ReportFlagTime;		//19字节
}; 

typedef boost::shared_ptr<YSMSPacket> YSMSPacketPtr;

class YSMSPacketFactory
{
public:
	static YSMSPacketPtr createPacket(u8 cmd);
};



#endif
