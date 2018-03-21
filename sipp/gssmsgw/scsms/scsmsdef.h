#ifndef __SC_SMS_DEF_H
#define __SC_SMS_DEF_H
#pragma once

namespace SCSMS
{

	//����
	static const u32 Max_Packet_Size		= 0x512;		//��������
	static const u32 Max_Once_Send			= 100;			//ÿ�η�������û���
	static const u32 Max_Message_Size		= 107;			//��Ϣ��󳤶�
	static const u8  FILL_CHAR				= '\0';			//�ַ����������������ַ�

	//�������
	enum _CommandCode
	{
		SCSMS_Login	                        = 1,  //�ͻ��˵�¼����
		SCSMS_Login_Resp                    = 2,  //�ͻ��˵�¼Ӧ��
		SCSMS_Deliver                       = 3,  //���ж���Ϣ
//		SCSMS_Deliver_Resp                  = 3,  //���ж���ϢӦ��
		SCSMS_Submit                        = 4,  //���ж���Ϣ
//		SCSMS_Submit_Resp                   = 4,  //���ж���ϢӦ��

		SCSMS_Submit_Group                  = 5,  //Ⱥ�����ж���Ϣ
		SCSMS_Submit_Group_Resp             = 6,  //Ⱥ�����ж���ϢӦ��

		SCSMS_Active_Test                   = 7,  //���Ӽ������
		SCSMS_Active_Test_Resp              = 8,  //���Ӽ�����Ի�Ӧ

		SCSMS_GW_REPORT						= 9,  //����״̬����
		SCSMS_TERM_REPORT					= 10,  //�ն�״̬����
	};


	//����ֵ
	enum _StatusCode
	{
		Status_OK                           = 0,           //�ɹ�
		Status_Error_Unknown                = 1,           //δ֪����
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
