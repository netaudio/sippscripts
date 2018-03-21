
#pragma once


#ifndef  __SRVSOCK_H__
#define __SRVSOCK_H__
#include <windows.h>

#define MAX_NODE 1000

#pragma  pack(1)

typedef void(*OnRecv)(char *buf, WORD len);

int     CloseAll();
int     SendPacket(char *pack, WORD len);
int     StartSrv(int port, int max_user, void* recv);
int     MakeReportPacket(BYTE *buf, BYTE *sn, BYTE *callee, BYTE *mtsn);
int     MakeSubmitRespPacket(BYTE *buf, DWORD result, BYTE *sn, BYTE *errdesc, BYTE *mtsn);
int     MakeRecvSms(BYTE *buf, DWORD myID,const char *sn,const char *userPhone,const char *newContent, const char *linkId, const char *spNo, const char *gwname, BYTE messageCoding);


#pragma pack()

#endif//__SRVSOCK_H__

