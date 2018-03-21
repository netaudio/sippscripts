#ifndef __SC_SMS_DEF_H
#define __SC_SMS_DEF_H
#pragma once

namespace SCSMS
{

	//参数
	static const u32 Max_Packet_Size		= 0x512;		//最大包长度
	static const u32 Max_Once_Send			= 100;			//每次发送最多用户数
	static const u32 Max_Message_Size		= 107;			//消息最大长度
	static const u8  FILL_CHAR				= '\0';			//字符类型左对齐右填充字符

	//命令代码
	enum _CommandCode
	{
		SCSMS_Login	                        = 1,  //客户端登录请求
		SCSMS_Login_Resp                    = 2,  //客户端登录应答
		SCSMS_Deliver                       = 3,  //上行短消息
//		SCSMS_Deliver_Resp                  = 3,  //上行短消息应答
		SCSMS_Submit                        = 4,  //下行短消息
//		SCSMS_Submit_Resp                   = 4,  //下行短消息应答

		SCSMS_Submit_Group                  = 5,  //群发下行短消息
		SCSMS_Submit_Group_Resp             = 6,  //群发下行短消息应答

		SCSMS_Active_Test                   = 7,  //连接激活测试
		SCSMS_Active_Test_Resp              = 8,  //连接激活测试回应

		SCSMS_GW_REPORT						= 9,  //网关状态报告
		SCSMS_TERM_REPORT					= 10,  //终端状态报告
	};


	//返回值
	enum _StatusCode
	{
		Status_OK                           = 0,           //成功
		Status_Error_Unknown                = 1,           //未知错误
	};


	enum _FeeType
	{
		FeeType_Free                        = 1,           //免费
		FeeType_Once                        = 2,           //按条计费
		FeeType_Month                       = 3,           //包月
		FeeType_Maximal                     = 4,           //封顶
	};

	enum _MsgFormat
	{
		Format_Ascii                        = 0,           //ASCII编码
		Format_Write_Card                   = 3,           //短消息写卡操作
		Format_Binary                       = 4,           //二进制短消息
		Format_UCS2                         = 8,           //UCS2编码
		Format_GB                           = 15,          //GB18030编码
	};

}


#endif
