#ifndef __SCSMS_PACKET_H
#define __SCSMS_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "scsmsdef.h"

using namespace std;
using namespace cacti;
using namespace SCSMS;

class SCSMSHeader
{
public:
	SCSMSHeader(u8 cmd)
		:totalLength(0),
		command(cmd)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	SCSMSHeader& operator = (const SCSMSHeader& other);

public:
	u8  command;
	u16 totalLength;
};

class SCSMSPacket
{
public:
	SCSMSPacket(u8 cmd)
		:header(cmd)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);

	SCSMSHeader& getHeader()   { return header; }
	void setHeader(SCSMSHeader& head){ header = head; };

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	
protected:
	SCSMSHeader header;
};

class SCSMSLogin : public SCSMSPacket
{
public:
	SCSMSLogin()
		:SCSMSPacket(SCSMS_Login)
		,clientID(10, SCSMS::FILL_CHAR)
		,password(50, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//[10]�ͻ���������¼�������˵��û��˺�
	u8array		clientID;
	//[50]�ͻ�����֤����
	u8array		password;
};

class SCSMSLoginResp : public SCSMSPacket
{
public:
	SCSMSLoginResp()
		:SCSMSPacket(SCSMS_Login_Resp)
	{};

protected:
	//void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//���󷵻ؽ��
	u8		status;

};

class SCSMSDeliver : public SCSMSPacket
{
public:
	SCSMSDeliver()
		:SCSMSPacket(SCSMS_Deliver)
		,LineNo(50, SCSMS::FILL_CHAR)
		,Msg_Id(50, SCSMS::FILL_CHAR)
		,Dest_Id(21, SCSMS::FILL_CHAR)
		,Dest_Id_Type(0)
		,Src_Id(32, SCSMS::FILL_CHAR)
		,Src_Id_Type(0)
		,Service_Code(50, SCSMS::FILL_CHAR)
		,Msg_Fmt(0)
		,Msg_Content(255, SCSMS::FILL_CHAR)
		,LinkId(50, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50�ֽ�
	u8array		Msg_Id;				//50�ֽ�
	u8array		Dest_Id;			//21�ֽ�
	u8			Dest_Id_Type;		//1�ֽ�
	u8array		Src_Id;				//32�ֽ�
	u8			Src_Id_Type;		//1�ֽ�
	u8array		Service_Code;		//50�ֽ�
	u16			Msg_Fmt;			//2�ֽ�
	u8array		Msg_Content;		//255�ֽ�
	u8array		LinkId;				//50�ֽ�
};


class SCSMSSubmit : public SCSMSPacket
{
public:
	SCSMSSubmit()
		:SCSMSPacket(SCSMS_Submit)
		,Msg_Kind(10, SCSMS::FILL_CHAR)
		,LineNo(50, SCSMS::FILL_CHAR)
		,Pk_total(1)
		,Pk_number(1)
		,Fee_UserType(0)
		,Service_Code(10, SCSMS::FILL_CHAR)
		,FeeType(6, SCSMS::FILL_CHAR)
		,FeeCode(6, SCSMS::FILL_CHAR)
		,FixedFee(6, SCSMS::FILL_CHAR)
		,GivenValue(6, SCSMS::FILL_CHAR)
		,Fee_terminal_Id(32, SCSMS::FILL_CHAR)
		,Fee_teminal_type(0)
		,Msg_Fmt(0)
		,Src_Id(21, SCSMS::FILL_CHAR)
		,Dest_Id(32, SCSMS::FILL_CHAR)
		,Dest_Id_type(0)
		,Msg_Content(255, SCSMS::FILL_CHAR)
		,LinkID(50, SCSMS::FILL_CHAR)
		,MsgType(0)
		,SpCode(10, SCSMS::FILL_CHAR)
		,ServiceID(10, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	u8array		Msg_Kind;			//10�ֽ�
	u8array		LineNo;				//50�ֽ�
	u8			Pk_total;			//1�ֽ�
	u8			Pk_number;			//1�ֽ�
	u8			Fee_UserType;		//1�ֽ�
	u8array		Service_Code;		//10�ֽ�
	u8array		FeeType;			//6�ֽ�
	u8array		FeeCode;			//6�ֽ�
	u8array		FixedFee;			//6�ֽ�
	u8array		GivenValue;			//6�ֽ�
	u8array		Fee_terminal_Id;	//32�ֽ�
	u8			Fee_teminal_type;	//1�ֽ�
	u16			Msg_Fmt;			//2�ֽ�
	u8array		Src_Id;				//21�ֽ�
	u8array		Dest_Id;			//32�ֽ�
	u8			Dest_Id_type;		//1�ֽ�
	u8array		Msg_Content;		//255�ֽ�
	u8array		LinkID;				//50�ֽ�
	u8			MsgType;			//1�ֽ�
	u8array		SpCode;				//10�ֽ�
	u8array		ServiceID;			//10�ֽ�
};

class SCSMSSubmitGroup : public SCSMSPacket
{
public:
	SCSMSSubmitGroup()
		:SCSMSPacket(SCSMS_Submit_Group)
		,Msg_Kind(10, SCSMS::FILL_CHAR)
		,LineNo(50, SCSMS::FILL_CHAR)
		,Pk_total(1)
		,Pk_number(1)
		,Fee_UserType(0)
		,Service_Code(10, SCSMS::FILL_CHAR)
		,FeeType(6, SCSMS::FILL_CHAR)
		,FeeCode(6, SCSMS::FILL_CHAR)
		,FixedFee(6, SCSMS::FILL_CHAR)
		,GivenValue(6, SCSMS::FILL_CHAR)
		,Fee_terminal_Id(32, SCSMS::FILL_CHAR)
		,Fee_teminal_type(0)
		,Msg_Fmt(0)
		,ValidTime(19, SCSMS::FILL_CHAR)
		,AtTime(19, SCSMS::FILL_CHAR)
		,Src_Id(21, SCSMS::FILL_CHAR)
		,Dest_Id(32, SCSMS::FILL_CHAR)
		,Dest_Id_type(0)
		,Msg_Content(255, SCSMS::FILL_CHAR)
		,LinkID(50, SCSMS::FILL_CHAR)
		,MsgType(0)
		,SpCode(10, SCSMS::FILL_CHAR)
		,ServiceID(10, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	u8array		Msg_Kind;			//10�ֽ�
	u8array		LineNo;				//50�ֽ�
	u8			Pk_total;			//1�ֽ�
	u8			Pk_number;			//1�ֽ�
	u8			Fee_UserType;		//1�ֽ�
	u8array		Service_Code;		//10�ֽ�
	u8array		FeeType;			//6�ֽ�
	u8array		FeeCode;			//6�ֽ�
	u8array		FixedFee;			//6�ֽ�
	u8array		GivenValue;			//6�ֽ�
	u8array		Fee_terminal_Id;	//32�ֽ�
	u8			Fee_teminal_type;	//1�ֽ�
	u16			Msg_Fmt;			//2�ֽ�
	u8array		ValidTime;			//2007-02-11 13:23:56���밴�ø�ʽ�ύ��19�ֽ�
	u8array		AtTime;				//2007-02-11 13:23:56���밴�ø�ʽ�ύ��19�ֽ�
	u8array		Src_Id;				//21�ֽ�
	u8array		Dest_Id;			//32�ֽ�
	u8			Dest_Id_type;		//1�ֽ�
	u8array		Msg_Content;		//255�ֽ�
	u8array		LinkID;				//50�ֽ�
	u8			MsgType;			//1�ֽ�
	u8array		SpCode;				//10�ֽ�
	u8array		ServiceID;			//10�ֽ�
};

class SCSMSSubmitGroupResp : public SCSMSPacket
{
public:
	SCSMSSubmitGroupResp()
		:SCSMSPacket(SCSMS_Submit_Group_Resp)
	{};

protected:
	//void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//���󷵻ؽ��
	u8		status;

};

class SCSMSActiveTest : public SCSMSPacket
{
public:
	SCSMSActiveTest()
		:SCSMSPacket(SCSMS_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	//bool decodeBody(cacti::Stream &content);

public:
	//���󷵻ؽ��
	u8		status;
};

class SCSMSActiveTestResp : public SCSMSPacket
{
public:
	SCSMSActiveTestResp()
		:SCSMSPacket(SCSMS_Active_Test_Resp)
	{};

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//���󷵻ؽ��
	u8		status;
}; 

class SCSMSGWReport: public SCSMSPacket
{
public:
	SCSMSGWReport()
		:SCSMSPacket(SCSMS_GW_REPORT)
		,LineNo(50, SCSMS::FILL_CHAR)
		,ReportFlag(50, SCSMS::FILL_CHAR)
		,ReportFlagTime(19, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50�ֽ�
	u8array		ReportFlag;			//50�ֽ�
	u8array		ReportFlagTime;		//19�ֽ�
}; 

class SCSMSTermReport: public SCSMSPacket
{
public:
	SCSMSTermReport()
		:SCSMSPacket(SCSMS_TERM_REPORT)
		,LineNo(50, SCSMS::FILL_CHAR)
		,ReportFlag(50, SCSMS::FILL_CHAR)
		,ReportFlagTime(19, SCSMS::FILL_CHAR)
	{};

protected:
	void checkBuffer();

private:
	//bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	u8array		LineNo;				//50�ֽ�
	u8array		ReportFlag;			//50�ֽ�
	u8array		ReportFlagTime;		//19�ֽ�
}; 

typedef boost::shared_ptr<SCSMSPacket> SCSMSPacketPtr;

class SCSMSPacketFactory
{
public:
	static SCSMSPacketPtr createPacket(u8 cmd);
};



#endif
