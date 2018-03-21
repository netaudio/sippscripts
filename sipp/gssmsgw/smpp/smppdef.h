#ifndef __SMPP_DEF_H
#define __SMPP_DEF_H
#pragma once

namespace SMPP
{

	//参数
	static const u32 Max_Packet_Size		= 0x512;		//最大包长度
	static const u32 Max_Once_Send			= 100;			//每次发送最多用户数
	static const u32 Max_Message_Size		= 220;			//消息最大长度


	//命令代码
	enum _CommandCode
	{
		SMPP_BindRecv                       = 0x00000001,  //客户端登录请求
		SMPP_BindRecv_Resp                  = 0x80000001,  //客户端登录应答
		SMPP_BindTrn						= 0x00000002,
		SMPP_BindTrn_Resp					= 0x80000002,
		SMPP_Submit                         = 0x00000004,  //提交短消息
		SMPP_Submit_Resp                    = 0x80000004,  //提交短消息应答
		SMPP_Deliver                        = 0x00000005,  //下发短消息
		SMPP_Deliver_Resp                   = 0x80000005,  //下发短消息应答
		SMPP_Unbind	                        = 0x00000006,  //退出请求
		SMPP_Unbind_Resp	                    = 0x80000006,  //退出应答
		SMPP_Enquire						= 0x00000015,  //查询链路状态	
		SMPP_Enquire_Resp					= 0x80000015,  //查询链路状态应答
	};


	//客户端登录模式
	enum _LoginMode 
	{
		Mode_Send                           = 0,           //发送      
		Mode_Receive                        = 1,           //接收
		Mode_Transmit                       = 2,           //双向
	};

	//版本号
	enum _Version  
	{
		Version_20                          = 0x20,        //SMPP 2.0
		Version_30                          = 0x30,        //SMPP 3.0
	};

	//返回值
	enum _StatusCode
	{
		Status_OK                           = 0,           //成功
		Status_System_Busy                  = 1,           //系统忙
		Status_Overload_Connections         = 2,           //超过最大连接数
		Status_Timeout                      = 3,           //系统超时
		Status_Send_Fail                    = 4,           //发送失败
		//5-9	保留
		Status_Error_Frame                  = 10,          //消息结构错
		Status_Error_Command                = 11,          //命令字错
		Status_Error_Sequence               = 12,          //序列号错
		//13-19	保留
		Status_Error_IP                     = 20,          //IP地址错
		Status_Error_Authenticate           = 21,          //认证错
		Status_Not_Match_Version            = 22,          //版本号不匹配
		//23-29	保留
		Status_Invalid_MsgType              = 30,          //非法消息类型（MsgType）
		Status_Invalid_Priority             = 31,          //非法优先级（Priority）
		Status_Invalid_FeeType              = 32,          //非法资费类型（FeeType）
		Status_Invalid_FeeCode              = 33,          //非法资费代码（FeeCode）
		Status_Invalid_MsgFormat            = 34,          //非法短消息编码格式（MsgFormat）
		Status_Invalid_TimeFormat           = 35,          //非法时间格式
		Status_Invalid_MsgLength            = 36,          //非法短消息长度（MsgLength）
		Status_Expire_Date                  = 37,          //有效期已过
		Status_Invalid_QueryType            = 38,          //非法查询类别（QueryType）
		Status_Error_Route                  = 39,          //路由错误
		Status_Invalid_FixedFee             = 40,          //非法包月费/封顶费（FixedFee）
		Status_Invalid_ServiceID            = 43,          //非法业务代码（ServiceID）
		Status_Invalid_ValidTime            = 44,          //非法短消息有效时间（ValidTime）
		Status_Invalid_AtTime               = 45,          //非法定时发送时间（AtTime）
		Status_Invalid_SrcTermID            = 46,          //非法发送用户号码（SrcTermID）
		Status_Invalid_DestTermID           = 47,          //非法接收用户号码（DestTermID）
		Status_Invalid_ChargeTermID         = 48,          //非法计费用户号码（ChargeTermID）
		Status_Invalid_SPCode               = 49,          //非法SP代码
		Status_Not_PrepayUser               = 50,          //非预付费用户
		Status_Deficit                      = 51,          //余额不足
		Status_Not_RegisterUser             = 52,          //非注册用户：属于预付费用户，但该号码还未被注册
		Status_Not_RegisterSP               = 53,          //非注册SP
		Status_Invalid_Account              = 54,          //帐号不可用：属于预付费用户，但该帐号处于不可用状态，如已冻结等
		Status_Bill_Fail                    = 55,          //扣费失败
		//56-111	保留
		//112,终端存储部件不存在
		//113,终端存储部件满
		//114,终端不支持的加密模式
		//115,非法交互信息类型（IIType）
		//116,非法交互码（ICode）
		//12××××	ISMAP协议转换后的错误代码（其中××××为ISMAP协议中的错误码）
		//13××××	短消息过滤系统错误代码（其中××××为短消息过滤系统返回的错误代码）
	};

	enum _MsgType
	{
		MsgType_MO                          = 0,           //终端发给SP
		MsgType_MT                          = 6,           //SP发给终端
	};

	enum _NeedReport
	{
		Report_Nothing                      = 0,           //不要求返回状态报告
		Report_Need                         = 1,           //要求返回状态报告
	};

	enum _Priority
	{
		Priority_Low                        = 0,           //低优先级
		Priority_Normal                     = 1,           //普通
		Priority_High                       = 2,           //较高
		Priority_VeryHigh                   = 3,           //高
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
