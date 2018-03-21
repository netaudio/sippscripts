#ifndef __YLX_DEF_H
#define __YLX_DEF_H
#pragma once

namespace YLX
{

	//参数
	static const u32 Max_Packet_Size		= 0x512;		//最大包长度
	static const u32 Max_Once_Send			= 100;			//每次发送最多用户数
	static const u32 Max_Message_Size		= 140;			//消息最大长度


	//命令代码
	enum _CommandCode
	{
		YLX_Login	                       = 0x00000001,  //客户端登录请求
		YLX_Login_Resp                     = 0x80000001,  //客户端登录应答
		YLX_Stat						   = 0x10000001,  //状态+心跳
		YLX_Submit                         = 0x10000002,  //提交短消息
		YLX_Submit_Resp                    = 0x80000002,  //提交短消息应答
		YLX_Deliver                        = 0x00000004,  //下发短消息
		YLX_Deliver_Resp                   = 0x80000004,  //下发短消息应答

		YLX_Report                         = 0x10000003,  //下发短消息
		YLX_Report_Resp                    = 0x80000003,  //下发短消息应答

		YLX_Active_Test                    = 0x00000008,  //连接激活测试
		YLX_Active_Test_Resp               = 0x80000008,  //连接激活测试回应
		YLX_Exit	                       = 0x00000002,  //退出请求
		YLX_Exit_Resp	                   = 0x80000002,  //退出应答
		YLX_Query	                       = 0x00000007,  //SP统计查询请求
		YLX_Query_Resp                     = 0x80000007,  //SP统计查询应答

	};


	//客户端登录模式
	enum _LoginMode 
	{
	};

	//版本号
	enum _Version  
	{
	};

	//返回值
	enum _StatusCode
	{
		Status_OK                           = 0,           //成功
		Status_System_Busy                  = 1,           //系统忙
		Status_Overload_Connections         = 2,           //超过最大连接数
		Status_Timeout                      = 3,           //系统超时
		Status_Send_Fail                    = 4,           //发送失败
	};

	enum _MsgType
	{
		MsgType_MO                          = 0,           //终端发给SP
		MsgType_MT                          = 6,           //SP发给终端
	};

		enum _MTFlag
	{
		MO_Order_First                      = 0,    //-MO点播引起的第一条MT消息
		MO_Order							= 1,	//MO点播引起的非第一条MT消息
		NOT_MO_Order                        = 2,    //非MO点播引起的MT消息
		System_MT							=3,		//系统反馈引起的MT消息
	};

	enum _NeedReport
	{
		Report_Error						= 0,

		Report_Need                         = 1,           //要求返回状态报告
		Report_Nothing                      = 2,           //不要求返回状态报告
		Report_Charge						= 3,
		//0-该条消息只有最后出错时要返回状态报告
		//1-该条消息无论最后是否成功都要返回状态报告
		//2-该条消息不需要返回状态报告
		//3-该条消息仅携带包月计费信息，不下发给用户，要返回状态报告
		//其它-保留
		//缺省设置为0

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
	};

	enum _TagName
	{
		Tag_TP_pid							= 0x0001,
		Tag_TP_udhi							= 0x0002,
		Tag_LinkID							= 0x0003,
		Tag_ChargeUserType					= 0x0004,
		Tag_ChargeTermType					= 0x0005,
		Tag_ChargeTermPseudo				= 0x0006,
		Tag_DestTermType					= 0x0007,
		Tag_DestTermPseudo					= 0x0008,
		Tag_PkTotal							= 0x0009,
		Tag_PkNumber						= 0x000A,
		Tag_SubmitMsgType					= 0x000B,
		Tag_SPDealReslt						= 0x000C,
		Tag_SrcTermType						= 0x000D,
		Tag_SrcTermPseudo					= 0x000E,
		Tag_NodesCount						= 0x000F,
		Tag_MsgSrc							= 0x0010,
		Tag_SrcType							= 0x0011,
		Tag_MServiceID						= 0x0012,
	};

	enum _SubmitType
	{
		Type_NormalMsg						= 0,			//普通短消息
		Type_WebCustomResut					= 1,			//WEB方式定制结果消息
		Type_WebCancelCustomResult			= 2,			//WEB方式取消定制结果消息
		Type_TermCustomResult				= 3,			//终端方式定制结果消息
		Type_TermCancelCustomResult			= 4,			//终端方式取消定制结果消息
		Type_MonthFeeNotify					= 5,			//包月扣费通知消息
		Type_WebCustomConfirm				= 6,			//WEB方式定制二次确认消息
		Type_WebCancelCustomConfirm			= 7,			//WEB方式取消定制二次确认消息
		Type_TermCustomConfirm				= 8,			//终端方式定制二次确认消息
		Type_TermCancelCustomConfirm		= 9,			//终端方式取消定制二次确认消息
		Type_WebOrderConfirm				= 10,			//WEB方式点播二次确认消息
		Type_TermOrderConfirm				= 11,			//终端方式点播二次确认消息（暂保留）
		Type_BatchSendRequest				= 12,			//群发请求
		Type_SyncBookRequest				= 13,			//同步订购（包括点播和定制）关系
		Type_BatchSendResult				= 14,			//群发结果通知消息,无该字段时，默认为“普通短消息”
		Type_SyncBookResult					= 15,			//同步订购（包括点播和定制）关系回复
	};
}


#endif
