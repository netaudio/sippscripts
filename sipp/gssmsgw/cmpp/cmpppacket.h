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
	
	u8     smsType;//�������ͣ�0: �û�����, 15: Ⱥ������, _SubmitType
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
	//[6]�ͻ���������¼�������˵��û��˺�
	//Դ��ַ���˴�ΪSP_Id����SP����ҵ���롣
	u8array		sourceAddr;
	//[16]�ͻ�����֤�룬��������ͻ��˵ĺϷ���
	//���ڼ���Դ��ַ����ֵͨ������MD5 hash����ó�����ʾ���£�
	//AuthenticatorSource =
	//MD5��Source_Addr+9 �ֽڵ�0 +shared secret+timestamp��
	//Shared secret ���й��ƶ���Դ��ַʵ�������̶���timestamp��ʽΪ��MMDDHHMMSS��������ʱ���룬10λ��

	u8array		authenticatorSource;
	
	//�ͻ���֧�ֵ�Э��汾��
	u8			version;
	//ʱ���
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
	//���󷵻ؽ��
	//״̬
	//0����ȷ
	//	1����Ϣ�ṹ��
	//	2���Ƿ�Դ��ַ
	//	3����֤��
	//	4���汾̫��
	//	5~ ����������

	u32			status;
	//[16]�������˷��ظ��ͻ��˵���֤��
	//ISMG��֤�룬���ڼ���ISMG��
	//��ֵͨ������MD5 hash����ó�����ʾ���£�
	//	AuthenticatorISMG =MD5��Status+AuthenticatorSource+shared secret����
	//Shared secret ���й��ƶ���Դ��ַʵ�������̶���
	//AuthenticatorSourceΪԴ��ַʵ�巢�͸�ISMG�Ķ�Ӧ��ϢCMPP_Connect�е�ֵ��
	//	��֤����ʱ������Ϊ�ա�

	u8array		authenticatorISMG;
	//��������֧�ֵ���߰汾��
	//������֧�ֵ���߰汾�ţ�����3.0�İ汾����4bitΪ3����4λΪ0
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
		//8	Unsigned Integer	��Ϣ��ʶ��
		u64 Msg_Id	;
			//	1	Unsigned Integer	��ͬMsg_Id����Ϣ����������1��ʼ��
		u8 Pk_total;
		//1	Unsigned Integer	��ͬMsg_Id����Ϣ��ţ���1��ʼ��
		u8 Pk_number	;
		//1	Unsigned Integer	�Ƿ�Ҫ�󷵻�״̬ȷ�ϱ��棺
		//0������Ҫ��
		//	1����Ҫ��
		u8 Registered_Delivery	;
			//1	Unsigned Integer	��Ϣ����
		u8 Msg_level;
		//10	Octet String	ҵ���ʶ�������֡���ĸ�ͷ��ŵ���ϡ�
		u8array Service_Id;	
		//	1	Unsigned Integer	�Ʒ��û������ֶΣ�
		//0����Ŀ���ն�MSISDN�Ʒѣ�
		//1����Դ�ն�MSISDN�Ʒѣ�
		//2����SP�Ʒѣ�
		//3����ʾ���ֶ���Ч����˭�ƷѲμ�Fee_terminal_Id�ֶΡ�
		u8 Fee_UserType;
		//32	Octet String	���Ʒ��û��ĺ��룬��Fee_UserTypeΪ3ʱ��ֵ��Ч��
		//��Fee_UserTypeΪ0��1��2ʱ��ֵ�����塣
		u8array Fee_terminal_Id	;
		//1	Unsigned Integer	���Ʒ��û��ĺ������ͣ�0����ʵ���룻1��α�롣
		u8 Fee_terminal_type;
		//1	Unsigned Integer	GSMЭ�����͡���ϸ�ǽ�����ο�GSM03.40�е�9.2.3.9��
		u8 TP_pId;
		//	1	Unsigned Integer	GSMЭ�����͡���ϸ�ǽ�����ο�GSM03.40�е�9.2.3.23,��ʹ��1λ���Ҷ��롣
		u8 TP_udhi;
	//	1	Unsigned Integer	��Ϣ��ʽ��
		//0��ASCII����
		//	3������д��������
		//	4����������Ϣ��
		//	8��UCS2���룻
		//	15����GB���֡�����������
		u8 Msg_Fmt;
		//	6	Octet String	��Ϣ������Դ(SP_Id)��
		u8array Msg_src;
		//	2	Octet String	�ʷ����
		//01���ԡ��Ʒ��û����롱��ѣ�
		//02���ԡ��Ʒ��û����롱��������Ϣ�ѣ�
		//03���ԡ��Ʒ��û����롱��������ȡ��Ϣ�ѡ�
		u8array FeeType;
		//	6	Octet String	�ʷѴ��루�Է�Ϊ��λ����
		u8array FeeCode;
		//	17	Octet String	�����Ч�ڣ���ʽ��ѭSMPP3.3Э�顣
		u8array ValId_Time;
		//17	Octet String	��ʱ����ʱ�䣬��ʽ��ѭSMPP3.3Э�顣
		u8array At_Time	;
		//	21	Octet String	Դ���롣SP�ķ�������ǰ׺Ϊ�������ĳ�����, 
		//���ؽ��ú����������SMPPЭ��Submit_SM��Ϣ��Ӧ��source_addr�ֶΣ�
		//�ú����������û��ֻ�����ʾΪ����Ϣ�����к��롣
		u8array Src_Id;
		//	1	Unsigned Integer	������Ϣ���û�����(С��100���û�)��
		u8 DestUsr_tl;
		//	32*DestUsr_tl	Octet String	���ն��ŵ�MSISDN���롣
		u8array Dest_Terminal_Id;
			//1	Unsigned Integer	���ն��ŵ��û��ĺ������ͣ�0����ʵ���룻1��α�롣
		u8 Dest_Terminal_Type;
		//1	Unsigned Integer	��Ϣ����(Msg_FmtֵΪ0ʱ��<160���ֽڣ�����<=140���ֽ�)��
		//ȡֵ���ڻ����0��
		u8 Msg_Length;
		//Msg_length	Octet String	��Ϣ���ݡ�
		u8array Msg_Content	;
		//20	Octet String	�㲥ҵ��ʹ�õ�LinkID���ǵ㲥��ҵ���MT���̲�ʹ�ø��ֶΡ�
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
	//[8]����Ϣ��ˮ��
	//��Ϣ��ʶ�������㷨���£�
	//	����64λ��8�ֽڣ���������
	//	��1��	ʱ�䣨��ʽΪMMDDHHMMSS��������ʱ���룩��bit64~bit39������
	//	bit64~bit61���·ݵĶ����Ʊ�ʾ��
	//	bit60~bit56���յĶ����Ʊ�ʾ��
	//	bit55~bit51��Сʱ�Ķ����Ʊ�ʾ��
	//	bit50~bit45���ֵĶ����Ʊ�ʾ��
	//	bit44~bit39����Ķ����Ʊ�ʾ��
	//	��2��	�������ش��룺bit38~bit17���Ѷ������صĴ���ת��Ϊ������д�����ֶ��У�
	//	��3��	���кţ�bit16~bit1��˳�����ӣ�����Ϊ1��ѭ��ʹ�á�
	//	�������粻�����������㣬�Ҷ��롣
	//	��SP���������Ӧ����Ϣ��Sequence_Idһ���ԾͿɵõ�CMPP_Submit��Ϣ��Msg_Id��

	u64		Msg_Id;
	//���󷵻ؽ��
	u32			Result;
}; 

class CMPPReport{

public:
	void checkBuffer();


	bool encodeBody(cacti::Stream &content);
	bool decodeBody(cacti::Stream &content);
public:
	u64 Msg_Id;//	8	Unsigned Integer	��Ϣ��ʶ��
	//SP�ύ���ţ�CMPP_SUBMIT������ʱ����SP������ISMG������Msg_Id��
	u8array Stat;//	7	Octet String	���Ͷ��ŵ�Ӧ���������������һ��SP���ݸ��ֶ�ȷ��CMPP_SUBMIT��Ϣ�Ĵ���״̬��
	u8array Submit_time;//	10	Octet String	YYMMDDHHMM��YYΪ��ĺ���λ00-99��MM��01-12��DD��01-31��HH��00-23��MM��00-59����
	u8array Done_time;//	10	Octet String	YYMMDDHHMM��
	u8array Dest_terminal_Id;//	32	Octet String	Ŀ���ն�MSISDN����(SP����CMPP_SUBMIT��Ϣ��Ŀ���ն�)��
	u32 SMSC_sequence;//	4	Unsigned Integer	ȡ��SMSC����״̬�������Ϣ���е���Ϣ��ʶ��


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
		//	8	Unsigned Integer	��Ϣ��ʶ��
		//�����㷨���£�
		//����64λ��8�ֽڣ���������
		//��1��	ʱ�䣨��ʽΪMMDDHHMMSS��������ʱ���룩��bit64~bit39������
		//bit64~bit61���·ݵĶ����Ʊ�ʾ��
		//bit60~bit56���յĶ����Ʊ�ʾ��
		//bit55~bit51��Сʱ�Ķ����Ʊ�ʾ��
		//bit50~bit45���ֵĶ����Ʊ�ʾ��
		//bit44~bit39����Ķ����Ʊ�ʾ��
		//��2��	�������ش��룺bit38~bit17���Ѷ������صĴ���ת��Ϊ������д�����ֶ��У�
		//��3��	���кţ�bit16~bit1��˳�����ӣ�����Ϊ1��ѭ��ʹ�á�
		//�������粻�����������㣬�Ҷ��롣
		u64 Msg_Id;
	

		//	21	Octet String	Ŀ�ĺ��롣
		//SP�ķ�����룬һ��4--6λ��������ǰ׺Ϊ�������ĳ����룻�ú������ֻ��û�����Ϣ�ı��к��롣
		u8array Dest_Id;
		
		u8array Service_Id;//	10	Octet String	ҵ���ʶ�������֡���ĸ�ͷ��ŵ���ϡ�
		u8 TP_pid;//	1	Unsigned Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.9��
		u8 TP_udhi;//	1	Unsigned Integer	GSMЭ�����͡���ϸ������ο�GSM03.40�е�9.2.3.23����ʹ��1λ���Ҷ��롣
		



		//	1	Unsigned Integer	��Ϣ��ʽ��
		//0��ASCII����
		//3������д��������
		//4����������Ϣ��
		//8��UCS2���룻
		//15����GB���֡�
		u8 Msg_Fmt;
		
		u8array Src_terminal_Id;//	32	Octet String	Դ�ն�MSISDN���루״̬����ʱ��ΪCMPP_SUBMIT��Ϣ��Ŀ���ն˺��룩��
		u8 Src_terminal_type;//	1	Unsigned Integer	Դ�ն˺������ͣ�0����ʵ���룻1��α�롣
		
		
		//	1	Unsigned Integer	�Ƿ�Ϊ״̬���棺
		//0����״̬���棻
		//1��״̬���档
		u8 Registered_Delivery;
		
		u8 Msg_Length;//	1	Unsigned Integer	��Ϣ���ȣ�ȡֵ���ڻ����0��
		u8array Msg_Content;//	Msg_length	Octet String	��Ϣ���ݡ�
		u8array LinkID;//	20	Octet String	�㲥ҵ��ʹ�õ�LinkID���ǵ㲥��ҵ���MT���̲�ʹ�ø��ֶΡ�

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
	//[10]����Ϣ��ˮ��
	u64		Msg_Id;
	//���󷵻ؽ��
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
