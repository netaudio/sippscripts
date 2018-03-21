#ifndef __CNGP_DEF_H
#define __CNGP_DEF_H
#pragma once

namespace CNGP
{

	//����
	static const u32 Max_Packet_Size		= 0x512;		//��������
	static const u32 Max_Once_Send			= 100;			//ÿ�η�������û���
	static const u32 Max_Message_Size		= 254;			//��Ϣ��󳤶�


	//�������
	enum _CommandCode
	{
		CNGP_Login	                        = 0x00000001,  //�ͻ��˵�¼����
		CNGP_Login_Resp                     = 0x80000001,  //�ͻ��˵�¼Ӧ��
		CNGP_Submit                         = 0x00000002,  //�ύ����Ϣ
		CNGP_Submit_Resp                    = 0x80000002,  //�ύ����ϢӦ��
		CNGP_Deliver                        = 0x00000003,  //�·�����Ϣ
		CNGP_Deliver_Resp                   = 0x80000003,  //�·�����ϢӦ��
		CNGP_Active_Test                    = 0x00000004,  //���Ӽ������
		CNGP_Active_Test_Resp               = 0x80000004,  //���Ӽ�����Ի�Ӧ
		CNGP_Exit	                        = 0x00000006,  //�˳�����
		CNGP_Exit_Resp	                    = 0x80000006,  //�˳�Ӧ��
		CNGP_Payment_Request				= 0x0000000A,
		CNGP_Payment_Request_Resp			= 0x8000000A,	
		CNGP_Payment_Affirm					= 0x0000000B,
		CNGP_Payment_Affirm_Resp			= 0x8000000B,
		CNGP_Query_UserState				= 0x0000000C,
		CNGP_Query_UserState_Resp			= 0x8000000C,

	};


	//�ͻ��˵�¼ģʽ
	enum _LoginMode 
	{
		Mode_Send                           = 0,           //����      
		Mode_Receive                        = 1,           //����
		Mode_Transmit                       = 2,           //˫��
	};

	//�汾��
	enum _Version  
	{
		Version_10                          = 0x10,        //CNGP 1.0
		Version_20                          = 0x20,        //CNGP 2.0
		Version_30                          = 0x30,        //CNGP 3.0
	};

	//����ֵ
	enum _StatusCode
	{
		Status_OK                           = 0,           //�ɹ�
		Status_System_Busy                  = 1,           //ϵͳæ
		Status_Overload_Connections         = 2,           //�������������
		Status_Timeout                      = 3,           //ϵͳ��ʱ
		Status_Send_Fail                    = 4,           //����ʧ��
		//5-9	����
		Status_Error_Frame                  = 10,          //��Ϣ�ṹ��
		Status_Error_Command                = 11,          //�����ִ�
		Status_Error_Sequence               = 12,          //���кŴ�
		//13-19	����
		Status_Error_IP                     = 20,          //IP��ַ��
		Status_Error_Authenticate           = 21,          //��֤��
		Status_Not_Match_Version            = 22,          //�汾�Ų�ƥ��
		//23-29	����
		Status_Invalid_MsgType              = 30,          //�Ƿ���Ϣ���ͣ�MsgType��
		Status_Invalid_Priority             = 31,          //�Ƿ����ȼ���Priority��
		Status_Invalid_FeeType              = 32,          //�Ƿ��ʷ����ͣ�FeeType��
		Status_Invalid_FeeCode              = 33,          //�Ƿ��ʷѴ��루FeeCode��
		Status_Invalid_MsgFormat            = 34,          //�Ƿ�����Ϣ�����ʽ��MsgFormat��
		Status_Invalid_TimeFormat           = 35,          //�Ƿ�ʱ���ʽ
		Status_Invalid_MsgLength            = 36,          //�Ƿ�����Ϣ���ȣ�MsgLength��
		Status_Expire_Date                  = 37,          //��Ч���ѹ�
		Status_Invalid_QueryType            = 38,          //�Ƿ���ѯ���QueryType��
		Status_Error_Route                  = 39,          //·�ɴ���
		Status_Invalid_FixedFee             = 40,          //�Ƿ����·�/�ⶥ�ѣ�FixedFee��
		Status_Invalid_ServiceID            = 43,          //�Ƿ�ҵ����루ServiceID��
		Status_Invalid_ValidTime            = 44,          //�Ƿ�����Ϣ��Чʱ�䣨ValidTime��
		Status_Invalid_AtTime               = 45,          //�Ƿ���ʱ����ʱ�䣨AtTime��
		Status_Invalid_SrcTermID            = 46,          //�Ƿ������û����루SrcTermID��
		Status_Invalid_DestTermID           = 47,          //�Ƿ������û����루DestTermID��
		Status_Invalid_ChargeTermID         = 48,          //�Ƿ��Ʒ��û����루ChargeTermID��
		Status_Invalid_SPCode               = 49,          //�Ƿ�SP����
		Status_Not_PrepayUser               = 50,          //��Ԥ�����û�
		Status_Deficit                      = 51,          //����
		Status_Not_RegisterUser             = 52,          //��ע���û�������Ԥ�����û������ú��뻹δ��ע��
		Status_Not_RegisterSP               = 53,          //��ע��SP
		Status_Invalid_Account              = 54,          //�ʺŲ����ã�����Ԥ�����û��������ʺŴ��ڲ�����״̬�����Ѷ����
		Status_Bill_Fail                    = 55,          //�۷�ʧ��
		//56-111	����
		//112,�ն˴洢����������
		//113,�ն˴洢������
		//114,�ն˲�֧�ֵļ���ģʽ
		//115,�Ƿ�������Ϣ���ͣ�IIType��
		//116,�Ƿ������루ICode��
		//12��������	ISMAPЭ��ת����Ĵ�����루���С�������ΪISMAPЭ���еĴ����룩
		//13��������	����Ϣ����ϵͳ������루���С�������Ϊ����Ϣ����ϵͳ���صĴ�����룩
	};

	enum _SubType
	{
		MsgType_CancelOrder                 = 0,           //ȡ������
		MsgType_Order						= 1,           //���Ļ�㲥����
		MsgType_OrderSend					= 2,           //�㲥�·�
		MsgType_Subscibe                    = 3,           //�����·�
	};

	enum _NeedReport
	{
		Report_Nothing                      = 0,           //��Ҫ�󷵻�״̬����
		Report_Need                         = 1,           //Ҫ�󷵻�״̬����
	};

	enum _Priority
	{
		Priority_Low                        = 0,           //�����ȼ�
		Priority_Normal                     = 1,           //��ͨ
		Priority_High                       = 2,           //�ϸ�
		Priority_VeryHigh                   = 3,           //��
	};

	enum _FeeType
	{
		FeeType_Free                        = 0,           //���
		FeeType_Once                        = 1,           //�����Ʒ�
		FeeType_Month                       = 2,           //����
		FeeType_Maximal                     = 3,           //�ⶥ
		FeeType_MonthBill					= 4,           //���¿۷�����
		FeeType_CR							= 5,           //CR����
	};

	enum _FeeUserType
	{
		FeeUserType_Obj						= 0,           //��Ŀ���ն˼Ʒ�
		FeeUserType_Src						= 1,           //��Դ�ն˼Ʒ�
		FeeUserType_Sp						= 2,           //��SP�Ʒ�
		FeeUserType_BillTel					= 3,           //���ռƷ��û�����Ʒ�
	};

	enum _MsgFormat
	{
		Format_Ascii                        = 0,           //ASCII����
		Format_Write_Card                   = 3,           //����Ϣд������
		Format_Binary                       = 4,           //�����ƶ���Ϣ
		Format_UCS2                         = 8,           //UCS2����
		Format_GB                           = 15,          //GB18030����
	};

}


#endif
