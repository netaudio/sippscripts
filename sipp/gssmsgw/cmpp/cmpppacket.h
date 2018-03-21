#ifndef __CMPP_PACKET_H
#define __CMPP_PACKET_H

#pragma once

#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "base/msgtlv.h"
#include "cmppdef.h"

using namespace std;
using namespace cacti;
using namespace CMPP;

class CMPPHeader
{
public:
	CMPPHeader(u32 cmd)
		:totalLength(12),
		command(cmd),
		sequence(1)
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	CMPPHeader& operator = (const CMPPHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 sequence;
};

class CMPPPacket
{
public:
	CMPPPacket(u32 cmd, u8 ver=CMPP::Version_30)
		:header(cmd)
		,version(ver)
		,smsType(0)
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	bool toArray(u8array &content);
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };
	CMPPHeader& getHeader()   { return header; }
	void setHeader(CMPPHeader& head){ header = head; };
//	u32  getStatus() { return header.status; };

	u8  getVersion() { return version; };
	

	virtual const char* toString(const char* split=" ");

	u8   getSmsType() { return smsType; }
	void setSmsType(u8 utype) { smsType = utype ;}

protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	
protected:
	CMPPHeader header;

private:
	u8  version;
	
	u8     smsType;//短信类型：0: 用户下行, 15: 群发下行, _SubmitType
};

class CMPPLogin : public CMPPPacket
{
public:
	CMPPLogin(u8 ver=CMPP::Version_30)
		:CMPPPacket(CMPP_Login),
		version(ver)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[6]客户端用来登录服务器端的用户账号
	//源地址，此处为SP_Id，即SP的企业代码。
	u8array		sourceAddr;
	//[16]客户端认证码，用来鉴别客户端的合法性
	//用于鉴别源地址。其值通过单向MD5 hash计算得出，表示如下：
	//AuthenticatorSource =
	//MD5（Source_Addr+9 字节的0 +shared secret+timestamp）
	//Shared secret 由中国移动与源地址实体事先商定，timestamp格式为：MMDDHHMMSS，即月日时分秒，10位。

	u8array		authenticatorSource;
	
	//客户端支持的协议版本号
	u8			version;
	//时间戳
	u32			timeStamp;	
	
};

class CMPPLoginResp : public CMPPPacket
{
public:
	CMPPLoginResp()
		:CMPPPacket(CMPP_Login_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	//状态
	//0：正确
	//	1：消息结构错
	//	2：非法源地址
	//	3：认证错
	//	4：版本太高
	//	5~ ：其他错误

	u32			status;
	//[16]服务器端返回给客户端的认证码
	//ISMG认证码，用于鉴别ISMG。
	//其值通过单向MD5 hash计算得出，表示如下：
	//	AuthenticatorISMG =MD5（Status+AuthenticatorSource+shared secret），
	//Shared secret 由中国移动与源地址实体事先商定，
	//AuthenticatorSource为源地址实体发送给ISMG的对应消息CMPP_Connect中的值。
	//	认证出错时，此项为空。

	u8array		authenticatorISMG;
	//服务器端支持的最高版本号
	//服务器支持的最高版本号，对于3.0的版本，高4bit为3，低4位为0
	u8			version;

};

class CMPPSubmit : public CMPPPacket
{
public:
	CMPPSubmit(u8 ver=CMPP::Version_30);
	virtual const char* toString(const char* split=" ");
protected:
	void checkBuffer();
	
private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
	std::string m_tmp;
	
public:
		//8	Unsigned Integer	信息标识。
		u64 Msg_Id	;
			//	1	Unsigned Integer	相同Msg_Id的信息总条数，从1开始。
		u8 Pk_total;
		//1	Unsigned Integer	相同Msg_Id的信息序号，从1开始。
		u8 Pk_number	;
		//1	Unsigned Integer	是否要求返回状态确认报告：
		//0：不需要；
		//	1：需要。
		u8 Registered_Delivery	;
			//1	Unsigned Integer	信息级别。
		u8 Msg_level;
		//10	Octet String	业务标识，是数字、字母和符号的组合。
		u8array Service_Id;	
		//	1	Unsigned Integer	计费用户类型字段：
		//0：对目的终端MSISDN计费；
		//1：对源终端MSISDN计费；
		//2：对SP计费；
		//3：表示本字段无效，对谁计费参见Fee_terminal_Id字段。
		u8 Fee_UserType;
		//32	Octet String	被计费用户的号码，当Fee_UserType为3时该值有效，
		//当Fee_UserType为0、1、2时该值无意义。
		u8array Fee_terminal_Id	;
		//1	Unsigned Integer	被计费用户的号码类型，0：真实号码；1：伪码。
		u8 Fee_terminal_type;
		//1	Unsigned Integer	GSM协议类型。详细是解释请参考GSM03.40中的9.2.3.9。
		u8 TP_pId;
		//	1	Unsigned Integer	GSM协议类型。详细是解释请参考GSM03.40中的9.2.3.23,仅使用1位，右对齐。
		u8 TP_udhi;
	//	1	Unsigned Integer	信息格式：
		//0：ASCII串；
		//	3：短信写卡操作；
		//	4：二进制信息；
		//	8：UCS2编码；
		//	15：含GB汉字。。。。。。
		u8 Msg_Fmt;
		//	6	Octet String	信息内容来源(SP_Id)。
		u8array Msg_src;
		//	2	Octet String	资费类别：
		//01：对“计费用户号码”免费；
		//02：对“计费用户号码”按条计信息费；
		//03：对“计费用户号码”按包月收取信息费。
		u8array FeeType;
		//	6	Octet String	资费代码（以分为单位）。
		u8array FeeCode;
		//	17	Octet String	存活有效期，格式遵循SMPP3.3协议。
		u8array ValId_Time;
		//17	Octet String	定时发送时间，格式遵循SMPP3.3协议。
		u8array At_Time	;
		//	21	Octet String	源号码。SP的服务代码或前缀为服务代码的长号码, 
		//网关将该号码完整的填到SMPP协议Submit_SM消息相应的source_addr字段，
		//该号码最终在用户手机上显示为短消息的主叫号码。
		u8array Src_Id;
		//	1	Unsigned Integer	接收信息的用户数量(小于100个用户)。
		u8 DestUsr_tl;
		//	32*DestUsr_tl	Octet String	接收短信的MSISDN号码。
		u8array Dest_Terminal_Id;
			//1	Unsigned Integer	接收短信的用户的号码类型，0：真实号码；1：伪码。
		u8 Dest_Terminal_Type;
		//1	Unsigned Integer	信息长度(Msg_Fmt值为0时：<160个字节；其它<=140个字节)，
		//取值大于或等于0。
		u8 Msg_Length;
		//Msg_length	Octet String	信息内容。
		u8array Msg_Content	;
		//20	Octet String	点播业务使用的LinkID，非点播类业务的MT流程不使用该字段。
		u8array LinkID	;


};

class CMPPSubmitResp : public CMPPPacket
{
public:
	CMPPSubmitResp()
		:CMPPPacket(CMPP_Submit_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[8]短消息流水号
	//信息标识，生成算法如下：
	//	采用64位（8字节）的整数：
	//	（1）	时间（格式为MMDDHHMMSS，即月日时分秒）：bit64~bit39，其中
	//	bit64~bit61：月份的二进制表示；
	//	bit60~bit56：日的二进制表示；
	//	bit55~bit51：小时的二进制表示；
	//	bit50~bit45：分的二进制表示；
	//	bit44~bit39：秒的二进制表示；
	//	（2）	短信网关代码：bit38~bit17，把短信网关的代码转换为整数填写到该字段中；
	//	（3）	序列号：bit16~bit1，顺序增加，步长为1，循环使用。
	//	各部分如不能填满，左补零，右对齐。
	//	（SP根据请求和应答消息的Sequence_Id一致性就可得到CMPP_Submit消息的Msg_Id）

	u64		Msg_Id;
	//请求返回结果
	u32			Result;
}; 

class CMPPReport{

public:
	void checkBuffer();


	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
public:
	u64 Msg_Id;//	8	Unsigned Integer	信息标识。
	//SP提交短信（CMPP_SUBMIT）操作时，与SP相连的ISMG产生的Msg_Id。
	u8array Stat;//	7	Octet String	发送短信的应答结果，含义详见表一。SP根据该字段确定CMPP_SUBMIT消息的处理状态。
	u8array Submit_time;//	10	Octet String	YYMMDDHHMM（YY为年的后两位00-99，MM：01-12，DD：01-31，HH：00-23，MM：00-59）。
	u8array Done_time;//	10	Octet String	YYMMDDHHMM。
	u8array Dest_terminal_Id;//	32	Octet String	目的终端MSISDN号码(SP发送CMPP_SUBMIT消息的目标终端)。
	u32 SMSC_sequence;//	4	Unsigned Integer	取自SMSC发送状态报告的消息体中的消息标识。


};

class CMPPDeliver : public CMPPPacket
{
public:
	CMPPDeliver(u8 ver=CMPP::Version_30);
	bool isReport(){return m_report;};
protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
	bool m_report;

public:
		//	8	Unsigned Integer	信息标识。
		//生成算法如下：
		//采用64位（8字节）的整数：
		//（1）	时间（格式为MMDDHHMMSS，即月日时分秒）：bit64~bit39，其中
		//bit64~bit61：月份的二进制表示；
		//bit60~bit56：日的二进制表示；
		//bit55~bit51：小时的二进制表示；
		//bit50~bit45：分的二进制表示；
		//bit44~bit39：秒的二进制表示；
		//（2）	短信网关代码：bit38~bit17，把短信网关的代码转换为整数填写到该字段中；
		//（3）	序列号：bit16~bit1，顺序增加，步长为1，循环使用。
		//各部分如不能填满，左补零，右对齐。
		u64 Msg_Id;
	

		//	21	Octet String	目的号码。
		//SP的服务代码，一般4--6位，或者是前缀为服务代码的长号码；该号码是手机用户短消息的被叫号码。
		u8array Dest_Id;
		
		u8array Service_Id;//	10	Octet String	业务标识，是数字、字母和符号的组合。
		u8 TP_pid;//	1	Unsigned Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.9。
		u8 TP_udhi;//	1	Unsigned Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.23，仅使用1位，右对齐。
		



		//	1	Unsigned Integer	信息格式：
		//0：ASCII串；
		//3：短信写卡操作；
		//4：二进制信息；
		//8：UCS2编码；
		//15：含GB汉字。
		u8 Msg_Fmt;
		
		u8array Src_terminal_Id;//	32	Octet String	源终端MSISDN号码（状态报告时填为CMPP_SUBMIT消息的目的终端号码）。
		u8 Src_terminal_type;//	1	Unsigned Integer	源终端号码类型，0：真实号码；1：伪码。
		
		
		//	1	Unsigned Integer	是否为状态报告：
		//0：非状态报告；
		//1：状态报告。
		u8 Registered_Delivery;
		
		u8 Msg_Length;//	1	Unsigned Integer	消息长度，取值大于或等于0。
		u8array Msg_Content;//	Msg_length	Octet String	消息内容。
		u8array LinkID;//	20	Octet String	点播业务使用的LinkID，非点播类业务的MT流程不使用该字段。

		CMPPReport report;

};



class CMPPDeliverResp : public CMPPPacket
{
public:
	CMPPDeliverResp()
		:CMPPPacket(CMPP_Deliver_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//[10]短消息流水号
	u64		Msg_Id;
	//请求返回结果
	u32			Result;
};

class CMPPActiveTest : public CMPPPacket
{
public:
	CMPPActiveTest()
		:CMPPPacket(CMPP_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class CMPPActiveTestResp : public CMPPPacket
{
public:
	CMPPActiveTestResp()
		:CMPPPacket(CMPP_Active_Test_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

class CMPPExit : public CMPPPacket
{
public:
	CMPPExit()
		:CMPPPacket(CMPP_Exit)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class CMPPExitResp : public CMPPPacket
{
public:
	CMPPExitResp()
		:CMPPPacket(CMPP_Exit_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

typedef boost::shared_ptr<CMPPPacket> CMPPPacketPtr;

class CMPPPacketFactory
{
public:
	static CMPPPacketPtr createPacket(u32 cmd, u8 ver=CMPP::Version_30);
};



#endif
