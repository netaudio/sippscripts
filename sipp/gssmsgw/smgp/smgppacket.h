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
	u8     smsType;//�������ͣ�0: �û�����, 15: Ⱥ������, _SubmitType
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
	//[8]�ͻ���������¼�������˵��û��˺�
	u8array		clientID;
	//[16]�ͻ�����֤�룬��������ͻ��˵ĺϷ���
	u8array		authenticatorClient;
	//�ͻ���������¼�������˵ĵ�¼����
	u8			loginMode;
	//ʱ���
	u32			timeStamp;	
	//�ͻ���֧�ֵ�Э��汾��
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
	//���󷵻ؽ��
	u32			status;
	//[16]�������˷��ظ��ͻ��˵���֤��
	u8array		authenticatorServer;
	//��������֧�ֵ���߰汾��
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
	//����Ϣ����
	u8			msgType;
	//SP�Ƿ�Ҫ�󷵻�״̬����
	u8			needReport;
	//����Ϣ�������ȼ�
	u8			priority;
	//[10]ҵ�����
	u8array		serviceID;
	//[2]�շ�����
	u8array		feeType;
	//[6]�ʷѴ���
	u8array		feeCode;
	//[6]���·�/�ⶥ��
	u8array		fixedFee;
	//����Ϣ��ʽ
	u8			msgFormat;
	//[17]����Ϣ��Чʱ��
	u8array		validTime;
	//[17]����Ϣ��ʱ����ʱ��
	u8array		atTime;
	//[21]����Ϣ���ͷ�����
	u8array		srcTermID;
	//[21]�Ʒ��û�����
	u8array		chargeTermID;	
	//����Ϣ���պ�������
	u8			destTermIDCount;
	//[21*DestTermCount]����Ϣ���պ��� 
	u8array		destTermID;
	//����Ϣ����
	u8			msgLength;
	//[MsgLength]����Ϣ����
	u8array		msgContent;
	//[8]���� 
	u8array		reserve;

	//[1]GSMЭ������
	MsgTLV		TP_pid;
	//[1]GSMЭ������
	MsgTLV		TP_udhi;
	//[20]���ױ�ʶ
	MsgTLV		LinkID;
	//[8]��Ϣ���ݵ���Դ
	MsgTLV		MsgSrc;
	//[1]�Ʒ��û�����
	MsgTLV		ChargeUserType;
	//[1]�Ʒ��û��ĺ�������
	MsgTLV		ChargeTermType;
	//[Length]�Ʒ��û���α��
	MsgTLV		ChargeTermPseudo;
	//[1]����Ϣ���շ����������
	MsgTLV		DestTermType;
	//[Length]����Ϣ���շ���α��
	MsgTLV		DestTermPseudo;
	//[1]��ͬMsgID����Ϣ������
	MsgTLV		PkTotal;
	//[1]��ͬMsgID����Ϣ���
	MsgTLV		PkNumber;
	//[1]SP���͵���Ϣ����
	MsgTLV		SubmitMsgType;
	//[1]SP����Ϣ�Ĵ�����
	MsgTLV		SPDealResult;
	//[21]ҵ����루�����ƶ���ҵ��
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
	//[10]����Ϣ��ˮ��
	u8array		msgID;
	//���󷵻ؽ��
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
	//[10]����Ϣ��ˮ��
	u8array		msgID;
	//�Ƿ�Ϊ״̬����
	u8			isReport;
	//����Ϣ��ʽ
	u8			msgFormat;
	//[14]����Ϣ����ʱ��
	u8array		recvTime;
	//[21]����Ϣ���ͺ���
	u8array		srcTermID;
	//[21]����Ϣ���պ���
	u8array		destTermID;
	//����Ϣ����
	u8			msgLength;
	//����Ϣ����
	u8array		msgContent;
	//[8]����
	u8array		reserve;

	//[1]GSMЭ������
	MsgTLV		TP_pid;
	//[1]GSMЭ������
	MsgTLV		TP_udhi;
	//[20]���ױ�ʶ
	MsgTLV		LinkID;
	//[1]����Ϣ���ͷ��ĺ�������
	MsgTLV		SrcTermType;
	//[Length]����Ϣ���ͷ���α��
	MsgTLV		SrcTermPseudo;
	//[1]SP���͵���Ϣ����
	MsgTLV		SubmitMsgType;
	//[1]SP����Ϣ�Ĵ�����
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
	//[10]����Ϣ��ˮ��
	u8array		msgID;
	//���󷵻ؽ��
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
