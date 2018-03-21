// LsnSocket.cpp : implementation file
//

#include "stdafx.h"
#include "LsnSocket.h"
#include "CommSocket.h"
#include "srvsock.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLsnSocket
CLsnSocket gLsnSocket;

CLsnSocket::CLsnSocket()
{
    sock.m_hSocket = INVALID_SOCKET;
}


CLsnSocket::~CLsnSocket()
{
    sock.ShutDown(2);
    sock.m_hSocket = -1;
    ShutDown(2);
    m_hSocket = -1;
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CLsnSocket, CSocket)
    //{{AFX_MSG_MAP(CLsnSocket)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CLsnSocket member functions

void CLsnSocket::OnAccept(int nErrorCode)
{
    // TODO: Add your specialized code here and/or call the base class
    static int indexsock = 0;
    CString sip;
    sock.m_hSocket = INVALID_SOCKET;
    Accept(sock);
    //gSocket = sock.m_hSocket;
//    sock.GetPeerName(sock.m_rip, sock.m_uPort);
    //sock.GetSockName(sock.m_rip, sock.m_uPort);
    //OutputLog("socket[%s:%d] socket ÒÑÁ¬½Ó!", sock.m_rip, sock.m_uPort);
    ////m_Sock[indexsock].Close();
    //if(m_Sock[indexsock].m_hSocket==INVALID_SOCKET)
    //{
    //    m_Sock[indexsock].Create();
    //    m_Sock[indexsock].Attach(sock.m_hSocket);
    //    m_Sock[indexsock].m_node = indexsock++;
    //}
    ////OutputLog("accept connect,ip = %s !",sip);
//    Accept(m_Sock[indexsock]);

    CSocket::OnAccept(nErrorCode);
}


void CLsnSocket::OnClose(int nErrorCode)
{
    // TODO: Add your specialized code here and/or call the base class
	ShutDown();
	CSocket::OnClose(nErrorCode);
}


int CloseAll()
{
    if (gLsnSocket.m_hSocket == INVALID_SOCKET)
        return 0;
	
	for (int i = 0; i < MAX_NODE; i++){
		gLsnSocket.release(i);
	}

	gLsnSocket.ShutDown(2);
    return 0;
}

extern OnRecv      gRecv;
int StartSrv(int port, int max_user, void * recv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

        USES_CONVERSION;
    if (!AfxSocketInit())
    {
        AfxMessageBox("AfxSocketInit failed!");
        return -1;
    }

    if (!gLsnSocket.Create(port)){
        OutputLog("Srv socket Create Error,%d", GetLastError());
        return -1;
    }

    if (!gLsnSocket.Listen(max_user )){
        OutputLog("Srv socket Listen Error,%d", GetLastError());
        return -1;
    }
	gRecv  = (OnRecv)recv;

    return 0;
}


int   SendPacket(char *pack, WORD len)
{
    gLsnSocket.sock.Send(pack, len);
    return len;
}

