// EZLogTCPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EZLogTCP.h"
#include "EZLogTCPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PRE_AGREED_PORT		8686
#define TIMESTAMP_FORMAT	_T("%H:%M:%S")

const UINT UWM_TCPEVENT = ::RegisterWindowMessage(
		_T( "UWM_TCPEVENT_27580DB4_1360_4062_BB4E_C5C894E90CCC" ) );

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEZLogTCPDlg dialog




CEZLogTCPDlg::CEZLogTCPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEZLogTCPDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strOutputPath.Empty();
	m_pThreadTCP = NULL;
	m_pThreadLPT = NULL;
	m_bAbortThreads = false;
}

void CEZLogTCPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OUTPUT_PATH, m_ctlOutputPath);
	DDX_Control(pDX, IDC_BTN_LISTEN, m_btnStartListen);
	DDX_Control(pDX, IDC_STATUS, m_ctlStatus);
	DDX_Control(pDX, IDC_MESSAGES, m_txtMessages);
}

BEGIN_MESSAGE_MAP(CEZLogTCPDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE( UWM_TCPEVENT, OnTcpEvent )
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_LISTEN, &CEZLogTCPDlg::OnBnClickedBtnListen)
	ON_WM_KEYDOWN()
	ON_WM_DEADCHAR()
END_MESSAGE_MAP()


// CEZLogTCPDlg message handlers

BOOL CEZLogTCPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CWinApp* pApp = AfxGetApp();
	CString fName = pApp->GetProfileString( _T("Settings"), _T("output_path"), NULL );
	m_ctlOutputPath.SetWindowText( fName );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEZLogTCPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEZLogTCPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEZLogTCPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CEZLogTCPDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN   )  
	{
		if(pMsg->wParam == _T('t') || pMsg->wParam == _T('T'))
		{
			LARGE_INTEGER llFrequency,llCount;
			CString strTime;

			if (!QueryPerformanceFrequency(&llFrequency))
				throw GetLastError();

			QueryPerformanceCounter(&llCount); // get time a.s.a. we received the first and only byte
			__int64 timetick = llFrequency.QuadPart ? ((1000*llCount.QuadPart)/llFrequency.QuadPart) : 0;

			strTime = CTime::GetCurrentTime().Format(TIMESTAMP_FORMAT);
			WriteLog(timetick,(int)(pMsg->wParam),_T("KEY"),strTime);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);  
}


#define FSE_THREADSTART			0x00000001
#define FSE_THREADCOMPLETE		0x00000002
#define FSE_UPDATESTATUS		0x00000004
#define FSE_STATUSCONNECTED		0x00000008
#define FSE_UPDATEMESSAGE		0x00000010

afx_msg LRESULT CEZLogTCPDlg::OnTcpEvent(WPARAM wParam, LPARAM lParam)
{
	CString str;
	switch ( wParam )
	{
	case FSE_THREADSTART:
		m_btnStartListen.EnableWindow( FALSE );
		m_ctlOutputPath.EnableWindow( FALSE );
		str.Format(_T("Waiting for incoming connection on port %d..."),PRE_AGREED_PORT);
		WriteStatus(str, false);
		break;

	case FSE_THREADCOMPLETE:
		m_bAbortThreads = true;
		WaitForSingleObject(m_pThreadTCP->m_hThread,10000); // wait for thread to finish
		m_btnStartListen.EnableWindow( TRUE );
		m_ctlOutputPath.EnableWindow( TRUE );
		m_logfile.Close();
		m_pThreadTCP = NULL;
		WriteStatus(_T("Connection closed."), false);
		break;

	case FSE_UPDATESTATUS:
		m_ctlStatus.SetWindowText( m_strStatus );
		break;

	case FSE_UPDATEMESSAGE:
		m_txtMessages.ReplaceSel( CString(_T("\r\n")) + m_strNewMessage );
		break;

	case FSE_STATUSCONNECTED:
		WriteStatus(_T("Connection accepted -- waiting for events ..."), false);
		break;

	default:
		ASSERT( FALSE );	// shouldn't get here
	}

	return 0L;
}

void CEZLogTCPDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CWinApp* pApp = AfxGetApp();
	CString fName;
	m_ctlOutputPath.GetWindowText( fName );	
	pApp->WriteProfileString( _T("Settings"), _T("output_path"), fName );
}

void CEZLogTCPDlg::OnBnClickedBtnListen()
{
	if ( m_pThreadTCP != NULL )	// make certain thread is not already running
		return;

	CString strPath;
	m_ctlOutputPath.GetWindowText( strPath );

	// confirm that directory exists
	const DWORD attr = GetFileAttributes(strPath);
	if (attr==INVALID_FILE_ATTRIBUTES || (attr&FILE_ATTRIBUTE_DIRECTORY)==0 )
	{
		CString sMessage;
		sMessage.Format( _T("Cannot open directory:\n\"%s\""), strPath );
		AfxMessageBox(sMessage, MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	m_strOutputPath = strPath;

	// construct a filename for event log (based on current time stamp)
	TCHAR path_buffer[_MAX_PATH];
	_tcscpy_s(path_buffer,_MAX_PATH,m_strOutputPath);
	size_t path_len = _tcslen(path_buffer);
	if (path_buffer[path_len-1]!=_T('\\'))
	{
		path_buffer[path_len++] = _T('\\');
		path_buffer[path_len] = _T('\0');
	}
	CString strTime = CTime::GetCurrentTime().Format(_T("%y%m%d_%H%M%S"));
	_tcscat_s(path_buffer,_MAX_PATH,_T("ez_"));
	_tcscat_s(path_buffer,_MAX_PATH,strTime);
	_tcscat_s(path_buffer,_MAX_PATH,_T(".log"));

	try
	{
		m_txtMessages.SetWindowText(_T(""));
		m_logfile.Open(path_buffer, (CFile::modeCreate | CFile::modeWrite | CFile::typeText));
		m_bAbortThreads = false;
		m_pThreadLPT = ::AfxBeginThread( ThreadedLptMonitor, (LPVOID) this, THREAD_PRIORITY_ABOVE_NORMAL );
		m_pThreadTCP = ::AfxBeginThread( ThreadedTcpReceive, (LPVOID) this, THREAD_PRIORITY_ABOVE_NORMAL );
	}
	catch (CFileException& ex)
	{
		m_bAbortThreads = true;
		CString sMessage;
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		sMessage.Format(_T("Cannot open log file\n%1024s\n"), szError);
		AfxMessageBox(sMessage, MB_OK | MB_ICONEXCLAMATION );
	}
}

void CEZLogTCPDlg::WriteLog(const TCHAR* sz)
{
	CSingleLock lck(&m_csLogfile);
	if (m_logfile.m_hFile!=m_logfile.hFileNull)
		m_logfile.WriteString(sz);
}

void CEZLogTCPDlg::WriteLog(__int64 timetick, int code, const TCHAR* source, const TCHAR* message)
{
	CString str;
	if (source==NULL && message==NULL)
		str.Format(_T("%I64d\t%d"),timetick,code);
	else
	{
		if (source==NULL) source = _T("-");
		if (message==NULL) message = _T("");
		str.Format(_T("%I64d\t%d\t%s\t%s"),timetick,code,source,message);
	}
	WriteLog(str+_T('\n'));
//	WriteStatus(str,true); 
//	m_txtMessages.ReplaceSel( CString("\r\n") + m_strStatus );
		m_strNewMessage = str;
	PostMessage( UWM_TCPEVENT, FSE_UPDATEMESSAGE, 0L );
}

void CEZLogTCPDlg::WriteStatus(const TCHAR* sz, bool bSendUpdate)
{
	{
		CSingleLock lck(&m_csStatus);
		m_strStatus = sz;
	}
	if (bSendUpdate)
		PostMessage( UWM_TCPEVENT, FSE_UPDATESTATUS, 0L );
	else
	{
		m_ctlStatus.SetWindowText( m_strStatus );
	}
}

UINT CEZLogTCPDlg::ThreadedTcpReceive(LPVOID pVoid)
{
	LARGE_INTEGER llFrequency,llCount;
	CString str, strTime;
	CEZLogTCPDlg* pThis = (CEZLogTCPDlg*)pVoid;
	pThis->PostMessage( UWM_TCPEVENT, FSE_THREADSTART, 0L );
	
	AfxSocketInit(NULL);	// make certain this is done somewhere in each thread (usually in InitInstance for main thread)
	CSocket sockSrvr; 
	sockSrvr.Create(PRE_AGREED_PORT); // Creates our server socket
	sockSrvr.Listen(); // Start listening for the client at PORT
	CSocket sockConnection;
	sockSrvr.Accept(sockConnection); // Use another CSocket to accept the connection
	
	pThis->PostMessage( UWM_TCPEVENT, FSE_STATUSCONNECTED, 0L );
	
	try
	{
		if (!QueryPerformanceFrequency(&llFrequency))
			throw GetLastError();

		do
		{
//			#define PACKET_SIZE 1
//			char szEvent[PACKET_SIZE];
//			int cbLeftToReceive = PACKET_SIZE;
//			SecureZeroMemory(szEvent,sizeof(szEvent));
//			do
//			{
//				BYTE* bp = (BYTE*)(szEvent) + PACKET_SIZE - cbLeftToReceive;
//				int cbBytesRet = sockConnection.Receive( bp, cbLeftToReceive );

				// use this instead for a single byte:
				BYTE evt1 = 0;
				const int cbBytesRet = sockConnection.Receive( &evt1, sizeof(evt1) );
				QueryPerformanceCounter(&llCount); // get time a.s.a. we received the first and only byte

				// test for errors and get out if they occurred
				if ( cbBytesRet == SOCKET_ERROR || cbBytesRet == 0 )
				{
					DWORD iErr = ::GetLastError();
					TRACE( "GetFileFromRemoteSite returned a socket error while getting file length\n"
						"\tNumber of Bytes received (zero means connection was closed) = %d\n"
						"\tGetLastError = %d\n", cbBytesRet, iErr );

					/* you should handle the error here */
					throw iErr;
				}

				// good data was retrieved, so accumulate it with already-received data
//				cbLeftToReceive -= cbBytesRet;
//			}
//			while ( cbLeftToReceive > 0 );

			strTime = CTime::GetCurrentTime().Format(TIMESTAMP_FORMAT);
			__int64 timetick = llFrequency.QuadPart ? ((1000*llCount.QuadPart)/llFrequency.QuadPart) : 0;
			pThis->WriteLog(timetick,evt1,_T("TCP"),strTime);
		}
		while ( !pThis->m_bAbortThreads );
	//	dataLength = ntohl( dataLength );
	}
	catch (DWORD iErr)
	{
		const TCHAR* szMessage = NULL;
		switch (iErr)
		{
		case 0L:				szMessage = _T("Connection closed"); break;
		case WSANOTINITIALISED: szMessage = _T("Socket not initialized"); break;
		case WSAENETDOWN:		szMessage = _T("Network subsystem failed"); break;
		case WSAENOTCONN:		szMessage = _T("The socket is not connected"); break;
		case WSAEINPROGRESS:	szMessage = _T("A blocking Windows Sockets operation is in progress"); break;
		case WSAENOTSOCK:		szMessage = _T("The descriptor is not a socket"); break;
		case WSAEOPNOTSUPP:		szMessage = _T("MSG_OOB was specified, but the socket is not of type SOCK_STREAM"); break;
		case WSAESHUTDOWN:		szMessage = _T("The socket has been shut down"); break;
		case WSAEWOULDBLOCK:	szMessage = _T("The socket is marked as nonblocking and the Receive operation would block"); break;
		case WSAEMSGSIZE:		szMessage = _T("The datagram was too large to fit into the specified buffer and was truncated"); break;
		case WSAEINVAL:			szMessage = _T("The socket has not been bound with Bind"); break;
		case WSAECONNABORTED:	szMessage = _T("The virtual circuit was aborted due to timeout or other failure"); break;
		case WSAECONNRESET:		szMessage = _T("The virtual circuit was reset by the remote side"); break;
		default: break;
		}
		if (szMessage==NULL)
			str.Format(_T("ERROR %08x\n"),iErr);
		else if (iErr==0)
			str = szMessage;
		else
			str.Format(_T("ERROR %08x - %s\n"),iErr,szMessage);

		QueryPerformanceCounter(&llCount); // get time a.s.a. we received the first and only byte
		__int64 timetick = llFrequency.QuadPart ? ((1000*llCount.QuadPart)/llFrequency.QuadPart) : 0;
		pThis->WriteLog(timetick,0,_T("TCP"),str);
//		pThis->WriteStatus(str,true);
	}

	sockConnection.Close();
	
	// advise main thread that we're completed
	pThis->PostMessage( UWM_TCPEVENT, FSE_THREADCOMPLETE, 0L );
	
	return 0;
	
}

// prototypes for inpout32.dll
short _stdcall Inp32(short PortAddress);
//void _stdcall Out32(short PortAddress, short data); not used

// select one of the following ports:
//inline unsigned int GetPortState() { return Inp32(0x03fe) & 0x20; } // this will return the DSR (Data Set Ready) status of COM1 (note: only when this port is opened with another application)
inline unsigned int GetPortState() { return Inp32(0x0379) & 0x40; } // this will return the nAck status of LPT1 (pin 10)
//inline unsigned int GetPortState() { return Inp32(0xB030) & 0x40; } // this will return the nAck status of LPT1 (pin 10) at spinoza centre

UINT CEZLogTCPDlg::ThreadedLptMonitor(LPVOID pVoid)
{
	LARGE_INTEGER llFrequency,llCount;
	CString str,strTime;
	CEZLogTCPDlg* pThis = (CEZLogTCPDlg*)pVoid;

	try
	{
		if (!QueryPerformanceFrequency(&llFrequency))
			throw GetLastError();
		unsigned int prevdata = GetPortState(); // get initial state
		unsigned int n_onsets = 0;
		do
		{
			unsigned int data = GetPortState();
			if (data!=0 && data!=prevdata) // use data!=0 to trap low to high transitions
			{
				QueryPerformanceCounter(&llCount); 
				strTime = CTime::GetCurrentTime().Format(TIMESTAMP_FORMAT);
				__int64 timetick = llFrequency.QuadPart ? ((1000*llCount.QuadPart)/llFrequency.QuadPart) : 0;
				n_onsets++;
				str.Format(_T("\t%d"), n_onsets);
				pThis->WriteLog(timetick,data,_T("LPT"),(strTime + str));
			}
			prevdata = data;
			Sleep(10); // for fMRI this is accurate enough. (1ms could overload some systems...)
		} while (!pThis->m_bAbortThreads);
	}
	catch (DWORD iErr)
	{
		const TCHAR* szMessage = NULL;
		switch (iErr)
		{
		default: break;
		}
		if (szMessage==NULL)
			str.Format(_T("ERROR %08x\n"),iErr);
		else if (iErr==0)
			str = szMessage;
		else
			str.Format(_T("ERROR %08x - %s\n"),iErr,szMessage);

		QueryPerformanceCounter(&llCount); // get time a.s.a. we received the first and only byte
		__int64 timetick = llFrequency.QuadPart ? ((1000*llCount.QuadPart)/llFrequency.QuadPart) : 0;
		pThis->WriteLog(timetick,0,_T("LPT"),str);
	}
	return 0;
}
