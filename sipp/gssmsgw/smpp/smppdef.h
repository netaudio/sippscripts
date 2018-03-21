#ifndef __SMPP_DEF_H
#define __SMPP_DEF_H
#pragma once

namespace SMPP
{

	//����
	static const u32 Max_Packet_Size		= 0x512;		//��������
	static const u32 Max_Once_Send			= 100;			//ÿ�η�������û���
	static const u32 Max_Message_Size		= 220;			//��Ϣ��󳤶�


	//�������
	enum _CommandCode
	{
		SMPP_BindRecv                       = 0x00000001,  //�ͻ��˵�¼����
		SMPP_BindRecv_Resp                  = 0x80000001,  //�ͻ��˵�¼Ӧ��
		SMPP_BindTrn						= 0x00000002,
		SMPP_BindTrn_Resp					= 0x80000002,
		SMPP_Submit                         = 0x00000004,  //�ύ����Ϣ
		SMPP_Submit_Resp                    = 0x80000004,  //�ύ����ϢӦ��
		SMPP_Deliver                        = 0x00000005,  //�·�����Ϣ
		SMPP_Deliver_Resp                   = 0x80000005,  //�·�����ϢӦ��
		SMPP_Unbind	                        = 0x00000006,  //�˳�����
		SMPP_Unbind_Resp	                    = 0x80000006,  //�˳�Ӧ��
		SMPP_Enquire						= 0x00000015,  //��ѯ��·״̬	
		SMPP_Enquire_Resp					= 0x80000015,  //��ѯ��·״̬Ӧ��
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
		Version_20                          = 0x20,        //SMPP 2.0
		Version_30                          = 0x30,        //SMPP 3.0
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

	enum _MsgType
	{
		MsgType_MO                          = 0,           //�ն˷���SP
		MsgType_MT                          = 6,           //SP�����ն�
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
		FeeType_Free                        = 1,           //���
		FeeType_Once                        = 2,           //�����Ʒ�
		FeeType_Month                       = 3,           //����
		FeeType_Maximal                     = 4,           //�ⶥ
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
