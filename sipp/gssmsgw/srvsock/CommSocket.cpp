// CommSocket.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <string.h>

#include "packet.h"
#include "CommSocket.h"
#include "srvsock.h"


//SOCKET gSocket;


OnRecv      gRecv = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MSG_RECV_TIMER_HANDLE 43211
#define ON_RECV_TIMEOUT 5
/////////////////////////////////////////////////////////////////////////////
// CCommSocket


void OutputLog(LPCTSTR DbgMessage,...)
{
	char buf[1024];
	memset(buf,0,1024);
	
	va_list args;
	va_start(args, DbgMessage);
	_vsnprintf_s (buf, sizeof(buf), DbgMessage, args);
	va_end(args);
    CTime now = CTime::GetCurrentTime();
    CString ss, s = now.Format("%c");
    ss.Format("%s(%3u)=>%s\n", s.GetBuffer(), GetTickCount() % 1000, buf);

	OutputDebugString(ss);
}


CCommSocket::CCommSocket()
{
	m_node = -1;
	m_name = "";
}


CCommSocket::~CCommSocket()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        //ShutDown(2);
        //Close();
    }
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CCommSocket, CSocket)
	//{{AFX_MSG_MAP(CCommSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CCommSocket member functions

#include "LsnSocket.h"

extern CLsnSocket gLsnSocket;

void CCommSocket::OnClose(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
    int node = m_node;
    if (m_hSocket == INVALID_SOCKET)
        return;
    //Close();
	ShutDown(2);
	m_hSocket = INVALID_SOCKET;
    OutputLog("node %d, socket closed", m_node);
    CSocket::OnClose(nErrorCode);
}

#define MAX_MSU_LEN  2048
#define MAX_BUFFER_SIZE MAX_MSU_LEN*10

#define MakeTLV(a)  \
    *(WORD *)&buf[pos] = (WORD)htons(strlen((char*)a));\
    pos += 2;\
    memcpy((void*)&buf[pos], a, strlen((char*)a));\
    pos += strlen((char*)a);

int  MakeReportPacket(BYTE *buf, BYTE *sn, BYTE *callee, BYTE *mtsn)
{
    int pos = 0;
    S_Msg_Header *hdr = (S_Msg_Header *)buf;
    hdr->SequenceId = htonl(0x12345678);
    hdr->CommandId = htonl(E_CMDID_Report);
    
    pos += sizeof(S_Msg_Header);

    MakeTLV(mtsn);

    MakeTLV(sn);
    MakeTLV(callee);
    BYTE status= 2;
    buf[pos] = status;
    pos++;

    BYTE eno[] = "0";

    MakeTLV(eno);

    hdr->TotalLength = htonl(pos);

    return pos;
}


int  MakeSubmitRespPacket(BYTE *buf,DWORD result,BYTE *sn,BYTE *errdesc ,BYTE *mtsn)
{
    int pos = 0;
    S_Msg_Header *hdr = (S_Msg_Header *)buf;
    hdr->SequenceId = htonl(0x12345678);
    hdr->CommandId = htonl(E_CMDID_SubmitResp);

    pos += sizeof(S_Msg_Header);
    MakeTLV(mtsn);

    *(DWORD*)&buf[pos] = htonl(result);
    pos += 4;

    MakeTLV(sn);
    MakeTLV(errdesc);
    hdr->TotalLength = htonl(pos);
    return pos;
}


void CCommSocket::OnReceive(int nErrorCode)
{
    // TODO: Add your specialized code here and/or call the base class
    static BYTE buf[MAX_BUFFER_SIZE];
    int len = Recv(buf, MAX_BUFFER_SIZE);
    CString s, sbin;
    for (int i = 0; i < len; i++)
    {
        sbin.Format("%02x ",buf[i]);
        s += sbin;
    }
    S_Msg_Header *phdr = (S_Msg_Header *)buf;
//    ParseMsg(buf);

    OutputLog("recv[%s],len=%d", s, len);

    if (gRecv != NULL)
        gRecv((char *)buf, len);
	CSocket::OnReceive(nErrorCode);
}


void CCommSocket::OnOutOfBandData(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	OutputLog("Server OnOutOfBandData");
	CancelBlockingCall();
	CSocket::OnOutOfBandData(nErrorCode);
}



int     MakeRecvSms(BYTE *buf, DWORD myID,const char *sn,const char *userPhone,const char *newContent, const char *linkId, const char *spNo, const char *gwname, BYTE messageCoding)
{
	return 0;
}
