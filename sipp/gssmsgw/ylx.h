

#pragma once


#ifndef  __YLX_H__
#define __YLX_H__

#include "srvsock/srvsock.h"
#include "srvsock/Packet.h"

#pragma pack()

S_Msg	ParseCMDID_Submit(BYTE *pbuf);
void	OnMyRecv(char *buf,WORD len);
int		ParseMsg(BYTE *pbuf);


#endif//__YLX_H__


