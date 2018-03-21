#pragma once

#ifndef  __PACKET_H__
#define __PACKET_H__
#include <windows.h>

#pragma  pack(1)

#define MAX_MSG_LEN 1500
#define COMM_CONNECTED  1
#define COMM_BROKEN		0
#define MAX_FILENAME_LENGTH 256


const short int PACKET_HEAD = 0x7FFE;


#define  PackName(a)  #a

#define	IC_BILL               68


//CMDID_Stat	        0x10000001	//心跳请求
//CMDID_Submit	        0x10000002	//短信下发请求
//CMDID_SubmitResp	    0x80000002	//短息下发应答
//CMDID_Report	        0x10000003	//状态报告请求
//CMDID_ReportResp	    0x80000003	//状态报告

enum{
    E_CMDID_Stat            = 0x10000001,
    E_CMDID_Submit          = 0x10000002,
    E_CMDID_SubmitResp      = 0x80000002,
    E_CMDID_Report          = 0x10000003,
    E_CMDID_ReportResp      = 0x80000003,
    E_CMDID_End,
};



struct S_Msg_Header{
    DWORD TotalLength;
    DWORD CommandId;    //包类型
    DWORD SequenceId;   //序列号             

};

struct  S_CMDID_Stat {

} ;

//短信下发
struct S_CMDID_Submit  {

    WORD    SerialID_length;
    BYTE    SerialID[21];

    WORD    type;
    WORD    OriginnumberLength;
    BYTE    Originnumber[21];

    WORD    CalleeLength;
    BYTE    Callee[21];
    
    BYTE    Msgcoding;
    WORD    MsgContentLength;
    BYTE    MsgContent[161];
} ;

 struct  S_CMDID_SubmitResp {
    DWORD   Result_Code;
    
    WORD    SerialIDLength;
    BYTE    SerialID[21];
    
    WORD    DescribtioBYTEnLength;
    BYTE    DescribtioBYTEn[161];

} ;

 struct  S_CMDID_Report {
    
    WORD    SerialIDLength;
    BYTE    SerialID[21];

    WORD    CalleeLength;
    BYTE    Callee[21];
    
    BYTE    Status;

    WORD    ErrcodeLength;
    BYTE    Errcode[21];

} ;

 struct   S_CMDID_ReportResp{
    DWORD Result_Code;
} ;


struct S_Msg{
    DWORD TotalLength;
    DWORD CommandId;    //包类型
    DWORD SequenceId;   //序列号             
    union {
        S_CMDID_Stat            CMDID_Stat;
        S_CMDID_Submit          CMDID_Submit;
        S_CMDID_SubmitResp      CMDID_SubmitResp;
        S_CMDID_Report          CMDID_Report;
        S_CMDID_ReportResp      CMDID_ReportResp;
    };
};


#pragma pack()

#endif//__PACKET_H__


