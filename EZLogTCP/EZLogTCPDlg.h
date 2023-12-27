// EZLogTCPDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CEZLogTCPDlg dialog
class CEZLogTCPDlg : public CDialog
{
// Construction
public:
	CEZLogTCPDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EZLOGTCP_DIALOG };

	void WriteLog(const TCHAR* sz);
	void WriteLog(__int64 timetick, int code, const TCHAR* source=NULL, const TCHAR* message=NULL);
	void WriteStatus(const TCHAR* sz, bool bSendUpdate);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CWinThread* m_pThreadTCP;
	static UINT ThreadedTcpReceive( LPVOID pVoid );

	CWinThread* m_pThreadLPT;
	static UINT ThreadedLptMonitor( LPVOID pVoid );

	bool m_bAbortThreads;
	CString m_strOutputPath;

	CString m_strNewMessage;
	CString m_strStatus;
	CCriticalSection m_csStatus;

	CStdioFile m_logfile;
	CCriticalSection m_csLogfile;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg); // for trapping key events 
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnListen();
	afx_msg LRESULT OnTcpEvent(WPARAM, LPARAM);
	CEdit m_ctlOutputPath;
	CButton m_btnStartListen;
	CStatic m_ctlStatus;
	CEdit m_txtMessages;
};
