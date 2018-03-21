#if !defined(AFX_COMMSOCKET_H__9983AADD_E747_48D4_A7DF_D4148469BEBA__INCLUDED_)
#define AFX_COMMSOCKET_H__9983AADD_E747_48D4_A7DF_D4148469BEBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommSocket.h : header file
//

#include "StdAfx.h"
#include "srvsock.h"

/////////////////////////////////////////////////////////////////////////////
// CCommSocket command target


class CCommSocket : public CSocket
{
// Attributes
public:
	CString     m_name;
	int         m_node;
    CString     m_rip;
    UINT        m_uPort;
// Operations
public:
	CCommSocket();
    virtual ~CCommSocket();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommSocket)
	public:
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnOutOfBandData(int nErrorCode);

	int Recv(void* lpBuf, int nBufLen, int nFlags = 0)
	{
		if (!SetTimeOut())
			return -1;
		int len = Receive(lpBuf, nBufLen, nFlags);
		KillTimeOut();
		return len;
	}


	int send(const void* lpBuf, int nBufLen, int nFlags = 0)
	{
		if (!SetTimeOut())
			return -1;
		int len = Send(lpBuf, nBufLen, nFlags);
		KillTimeOut();
		return len;
	}

	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CCommSocket)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	private:
		BOOL SetTimeOut(UINT uTimeOut = 1000)
		{
			m_nTimerID = SetTimer(NULL, 0, uTimeOut, NULL);
			return m_nTimerID;
		}

		BOOL KillTimeOut()
		{
			return KillTimer(NULL, m_nTimerID);
		}


	private:
		int m_nTimerID;

// Implementation
protected:
	virtual BOOL OnMessagePending()
	{
		// TODO: Add your specialized code here and/or call the base class
		MSG msg;
		if (::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_NOREMOVE))
		{
			if (msg.wParam == (UINT)m_nTimerID)
			{
				// Remove the message and call CancelBlockingCall.
				::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE);
				CancelBlockingCall();
				//ShutDown();
				return FALSE;
				// No need for idle time processing.
			}
		}
		return CSocket::OnMessagePending();
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMSOCKET_H__9983AADD_E747_48D4_A7DF_D4148469BEBA__INCLUDED_)

