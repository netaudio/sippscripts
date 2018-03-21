#ifndef __SGIP_PACKET_H
#define __SGIP_PACKET_H

#pragma once
#include "base/baseprotocol.h"
#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include "base/msgtlv.h"
#include "sgipdef.h"

using namespace std;
using namespace cacti;
using namespace SGIP;

class SGIPHeader
{
public:
	SGIPHeader(u32 cmd)
		:totalLength(12),
		command(cmd),
		sequence(1),sourceCode(100001),sendDate(makeTimestamp())
	{};

public:
	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	SGIPHeader& operator = (const SGIPHeader& other);

public:
	u32 totalLength;
	u32 command;
	u32 sourceCode; //xxoo
	u32 sendDate;  //MMDDHHmmss
	u32 sequence;  //xxooxx
};

class SGIPPacket
{
public:
	SGIPPacket(u32 cmd)
		:header(cmd)
	
	{};

	bool fromStream(cacti::Stream &content, bool bOnlyBody=false);
	bool toStream(cacti::Stream &content);
	bool toArray(u8array &content);
	const char* getPacketSequence();
	void setSequence(u32 seq) { header.sequence = seq; };
	u32  getSequence()        { return header.sequence; };


	void setSourceCode(u32 souceCode) { header.sourceCode = souceCode; };
	u32  getSouceCode()        { return header.sourceCode; };

	void setSendDate(u32 sendDate) { header.sendDate = sendDate; };
	u32  getSendDate()        { return header.sendDate; };

	SGIPHeader& getHeader()   { return header; }
	void setHeader(SGIPHeader& head){ header = head; };
//	u32  getStatus() { return header.status; };

	
	
	virtual const char* toString(const char* split=" ");



protected:
	virtual void checkBuffer() {};
	virtual bool encodeBody(cacti::Stream &content);
	virtual bool decodeBody(cacti::Stream &content);
	

protected:
	SGIPHeader header;
	

private:
	char m_tmp[36];
	
};

class SGIPLogin : public SGIPPacket
{
public:
	SGIPLogin( )
		:SGIPPacket(SGIP_Login),loginType(SP_SMG)
		
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

	/*
	*登录类型。
	*1：SP向SMG建立的连接，用于发送命令
	*2：SMG向SP建立的连接，用于发送命令
	*3：SMG之间建立的连接，用于转发命令
	*4：SMG向GNS建立的连接，用于路由表的检索和维护
	*5：GNS向SMG建立的连接，用于路由表的更新
	*6：主备GNS之间建立的连接，用于主备路由表的一致性
	*11：SP与SMG以及SMG之间建立的测试连接，用于跟踪测试
	其它：保留
	*/
	u8 loginType;
	//[16]登录名
	u8array loginName;
	//[16]登录密码
	u8array password;
	//[8]保留，扩展用
	u8array reserve;


};

class SGIPLoginResp : public SGIPPacket
{
public:
	SGIPLoginResp()
		:SGIPPacket(SGIP_Login_Resp)
	{};

protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
	//请求返回结果
	u8			status;
	//[8]保留，扩展用
	u8array		reserve;


};

class SGIPSubmit : public SGIPPacket
{
public:
	SGIPSubmit( );
	virtual const char* toString(const char* split=" ");
protected:
	void checkBuffer();

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
	string m_str;

public:
		//	[21]	Text	SP的接入号码
		u8array spNumber;
		//[21]	Text	付费号码，手机号码前加“86”国别标志；
		//当且仅当群发且对用户收费时为空；如果为空，
		//则该条短消息产生的费用由UserNumber代表的用户支付；
		//如果为全零字符串“000000000000000000000”，表示该条短消息产生的费用由SP支付。
		u8array chargeNumber;
		//1	Integer	接收短消息的手机数量，取值范围1至100
		u8 userCount;
		//[21]	Text	接收该短消息的手机号，
		//该字段重复UserCount指定的次数，手机号码前加“86”国别标志
		u8array userNumber;
		//[5]	Text	企业代码，取值范围0-99999
		u8array corpId;	
		//[10]	Text	业务代码，由SP定义
		u8array serviceType	;
		//1	Integer	计费类型
		u8 feeType	;
		//[6]	Text	取值范围0-99999，该条短消息的收费值，单位为分，由SP定义
		//对于包月制收费的用户，该值为月租费的值
		u8array feeValue	;
		
		//6	Text	取值范围0-99999，赠送用户的话费，单位为分，
		//由SP定义，特指由SP向用户发送广告时的赠送话费
		u8array givenValue	;
		//	1	Integer	代收费标志，0：应收；1：实收
		u8 agentFlag;
		//1	Integer	引起MT消息的原因
		//0-MO点播引起的第一条MT消息；
		//1-MO点播引起的非第一条MT消息；
		//2-非MO点播引起的MT消息；
		//3-系统反馈引起的MT消息
		u8 morelatetoMTFlag	;
		//1	Integer	优先级0-9从低到高，默认为0
		u8 priority	;
		//[16]	Text	短消息寿命的终止时间，如果为空，
		//表示使用短消息中心的缺省值。时间内容为16个字符，
		//格式为”yymmddhhmmsstnnp” ，其中“tnnp”取固定值“032+”，即默认系统为北京时间
		u8array expireTime	;
		//[16]	Text	短消息定时发送的时间，如果为空，
		//表示立刻发送该短消息。时间内容为16个字符，
		//格式为“yymmddhhmmsstnnp” ，其中“tnnp”取固定值“032+”，
		//即默认系统为北京时间
		u8array scheduleTime	;
		//1	Integer	状态报告标记
		//0-该条消息只有最后出错时要返回状态报告
		//1-该条消息无论最后是否成功都要返回状态报告
		//2-该条消息不需要返回状态报告
		//3-该条消息仅携带包月计费信息，不下发给用户，要返回状态报告
		//其它-保留
		//缺省设置为0
		u8 reportFlag;
		//1	Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.9
		u8 tpPid	;
		//1	Integer	GSM协议类型。详细解释请参考GSM03.40中的9.2.3.23,仅使用1位，右对齐
		u8 tpUdhi;	
		//	1	Integer	短消息的编码格式。
		//0：纯ASCII字符串
		//3：写卡操作
		//4：二进制编码
		//8：UCS2编码
		//15: GBK编码
		//其它参见GSM3.38第4节：SMS Data Coding Scheme
		u8 messageCoding;
		//	1	Integer	信息类型：
		//0-短消息信息
		//其它：待定
		u8 messageType;
		//4	Integer	短消息的长度
		u32 messageLength;	
		//Message Length	Text	短消息的内容
		u8array messageContent;	
		//[8]	Text	保留，扩展用
		u8array reserve	;

};

class SGIPSubmitResp : public SGIPPacket
{
public:
	SGIPSubmitResp()
		:SGIPPacket(SGIP_Submit_Resp)
	{};

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

class SGIPDeliver : public SGIPPacket
{
public:
	SGIPDeliver();

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
		//	1	Integer	短消息的编码格式。
		//0：纯ASCII字符串
		//	3：写卡操作
		//	4：二进制编码
		//	8：UCS2编码
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

class SGIPDeliverResp : public SGIPPacket
{
public:
	SGIPDeliverResp()
		:SGIPPacket(SGIP_Deliver_Resp)
	{};

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


class SGIPReport : public SGIPPacket
{
public:
	SGIPReport();

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
		//0：发送成功
		//1：等待发送
		//	2：发送失败
		u8 state;
		//	1	Integer	当State=2时为错误码值，否则为0
		u8 errorCode;
		//[8]	Text	保留，扩展用
		u8array reserve;


};

class SGIPReportResp : public SGIPPacket
{
public:
	SGIPReportResp()
		:SGIPPacket(SGIP_Report_Resp)
	{};

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

class SGIPActiveTest : public SGIPPacket
{
public:
	SGIPActiveTest()
		:SGIPPacket(SGIP_Active_Test)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class SGIPActiveTestResp : public SGIPPacket
{
public:
	SGIPActiveTestResp()
		:SGIPPacket(SGIP_Active_Test_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

class SGIPExit : public SGIPPacket
{
public:
	SGIPExit()
		:SGIPPacket(SGIP_Exit)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:

};

class SGIPExitResp : public SGIPPacket
{
public:
	SGIPExitResp()
		:SGIPPacket(SGIP_Exit_Resp)
	{};

private:
	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);

public:
}; 

typedef boost::shared_ptr<SGIPPacket> SGIPPacketPtr;

class SGIPPacketFactory
{
public:
	static SGIPPacketPtr createPacket(u32 cmd);
};



#endif
