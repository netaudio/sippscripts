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
	*��¼���͡�
	*1��SP��SMG���������ӣ����ڷ�������
	*2��SMG��SP���������ӣ����ڷ�������
	*3��SMG֮�佨�������ӣ�����ת������
	*4��SMG��GNS���������ӣ�����·�ɱ�ļ�����ά��
	*5��GNS��SMG���������ӣ�����·�ɱ�ĸ���
	*6������GNS֮�佨�������ӣ���������·�ɱ��һ����
	*11��SP��SMG�Լ�SMG֮�佨���Ĳ������ӣ����ڸ��ٲ���
	����������
	*/
	u8 loginType;
	//[16]��¼��
	u8array loginName;
	//[16]��¼����
	u8array password;
	//[8]��������չ��
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
	//���󷵻ؽ��
	u8			status;
	//[8]��������չ��
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
		//	[21]	Text	SP�Ľ������
		u8array spNumber;
		//[21]	Text	���Ѻ��룬�ֻ�����ǰ�ӡ�86�������־��
		//���ҽ���Ⱥ���Ҷ��û��շ�ʱΪ�գ����Ϊ�գ�
		//���������Ϣ�����ķ�����UserNumber������û�֧����
		//���Ϊȫ���ַ�����000000000000000000000������ʾ��������Ϣ�����ķ�����SP֧����
		u8array chargeNumber;
		//1	Integer	���ն���Ϣ���ֻ�������ȡֵ��Χ1��100
		u8 userCount;
		//[21]	Text	���ոö���Ϣ���ֻ��ţ�
		//���ֶ��ظ�UserCountָ���Ĵ������ֻ�����ǰ�ӡ�86�������־
		u8array userNumber;
		//[5]	Text	��ҵ���룬ȡֵ��Χ0-99999
		u8array corpId;	
		//[10]	Text	ҵ����룬��SP����
		u8array serviceType	;
		//1	Integer	�Ʒ�����
		u8 feeType	;
		//[6]	Text	ȡֵ��Χ0-99999����������Ϣ���շ�ֵ����λΪ�֣���SP����
		//���ڰ������շѵ��û�����ֵΪ����ѵ�ֵ
		u8array feeValue	;
		
		//6	Text	ȡֵ��Χ0-99999�������û��Ļ��ѣ���λΪ�֣�
		//��SP���壬��ָ��SP���û����͹��ʱ�����ͻ���
		u8array givenValue	;
		//	1	Integer	���շѱ�־��0��Ӧ�գ�1��ʵ��
		u8 agentFlag;
		//1	Integer	����MT��Ϣ��ԭ��
		//0-MO�㲥����ĵ�һ��MT��Ϣ��
		//1-MO�㲥����ķǵ�һ��MT��Ϣ��
		//2-��MO�㲥�����MT��Ϣ��
		//3-ϵͳ���������MT��Ϣ
		u8 morelatetoMTFlag	;
		//1	Integer	���ȼ�0-9�ӵ͵��ߣ�Ĭ��Ϊ0
		u8 priority	;
		//[16]	Text	����Ϣ��������ֹʱ�䣬���Ϊ�գ�
		//��ʾʹ�ö���Ϣ���ĵ�ȱʡֵ��ʱ������Ϊ16���ַ���
		//��ʽΪ��yymmddhhmmsstnnp�� �����С�tnnp��ȡ�̶�ֵ��032+������Ĭ��ϵͳΪ����ʱ��
		u8array expireTime	;
		//[16]	Text	����Ϣ��ʱ���͵�ʱ�䣬���Ϊ�գ�
		//��ʾ���̷��͸ö���Ϣ��ʱ������Ϊ16���ַ���
		//��ʽΪ��yymmddhhmmsstnnp�� �����С�tnnp��ȡ�̶�ֵ��032+����
		//��Ĭ��ϵͳΪ����ʱ��
		u8array scheduleTime	;
		//1	Integer	״̬������
		//0-������Ϣֻ��������ʱҪ����״̬����
		//1-������Ϣ��������Ƿ�ɹ���Ҫ����״̬����
		//2-������Ϣ����Ҫ����״̬����
		//3-������Ϣ��Я�����¼Ʒ���Ϣ�����·����û���Ҫ����״̬����
		//����-����
		//ȱʡ����Ϊ0
		u8 reportFlag;
		//1	Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.9
		u8 tpPid	;
		//1	Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.23,��ʹ��1λ���Ҷ���
		u8 tpUdhi;	
		//	1	Integer	����Ϣ�ı����ʽ��
		//0����ASCII�ַ���
		//3��д������
		//4�������Ʊ���
		//8��UCS2����
		//15: GBK����
		//�����μ�GSM3.38��4�ڣ�SMS Data Coding Scheme
		u8 messageCoding;
		//	1	Integer	��Ϣ���ͣ�
		//0-����Ϣ��Ϣ
		//����������
		u8 messageType;
		//4	Integer	����Ϣ�ĳ���
		u32 messageLength;	
		//Message Length	Text	����Ϣ������
		u8array messageContent;	
		//[8]	Text	��������չ��
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
	//���󷵻ؽ��
	u8		status;
	//[8]��������չ��
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
		//[21]	Text	���Ͷ���Ϣ���û��ֻ��ţ��ֻ�����ǰ�ӡ�86�������־
		u8array userNumber;
		//[21]	Text	SP�Ľ������
		u8array spNumber;	
		//1	Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.9
		u8 tpPid;	
		//1	Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.23,��ʹ��1λ���Ҷ���
		u8 tpUdhi;	
		//	1	Integer	����Ϣ�ı����ʽ��
		//0����ASCII�ַ���
		//	3��д������
		//	4�������Ʊ���
		//	8��UCS2����
		//	15: GBK����
		//�����μ�GSM3.38��4�ڣ�SMS Data Coding Scheme
		u8 messageCoding;
		
		//	4	Integer	����Ϣ�ĳ���
		u32 messageLength;
		//Message Length	Text	����Ϣ������
		u8array messageContent;	
		//8	Text	��������չ��
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
		//1	Integer	Deliver�����Ƿ�ɹ����ա�
		//0�����ճɹ�
		//������������
		u8 status;	
		//	[8]	Text	��������չ��
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
		//SubmitSequenceNumber	12	Integer	���������漰��Submit��deliver��������к�
		u32 sourceCode;
		u32 sendDate;
		u32 sequence;
		//	1	Integer	Report��������
		//0������ǰһ��Submit�����״̬����
		//	1������ǰһ��ǰתDeliver�����״̬����
		u8 reportType;
		//	[21]	Text	���ն���Ϣ���ֻ��ţ��ֻ�����ǰ�ӡ�86�������־
		u8array userNumber;
		//	1	Integer	���������漰�Ķ���Ϣ�ĵ�ǰִ��״̬
		//0�����ͳɹ�
		//1���ȴ�����
		//	2������ʧ��
		u8 state;
		//	1	Integer	��State=2ʱΪ������ֵ������Ϊ0
		u8 errorCode;
		//[8]	Text	��������չ��
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
	//1	Integer	Deliver�����Ƿ�ɹ����ա�
	//0�����ճɹ�
	//������������
	u8 status;	
	//	[8]	Text	��������չ��
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
