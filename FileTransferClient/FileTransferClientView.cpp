// FileTransferClientView.cpp : implementation of the CFileTransferClientView class
//

#include "stdafx.h"
#include "FileTransferClient.h"

#include "FileTransferClientDoc.h"
#include "FileTransferClientView.h"
#include <stdlib.h>		// for rand and srand functions; used to simulate mismatch events

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT UWM_TCPEVENT = ::RegisterWindowMessage(
		_T( "UWM_TCPEVENT_F6A100CA_692D_478d_B28C_8E87A1991EA1" ) );



/////////////////////////////////////////////////////////////////////////////
// CFileTransferClientView

IMPLEMENT_DYNCREATE(CFileTransferClientView, CFormView)

BEGIN_MESSAGE_MAP(CFileTransferClientView, CFormView)
	//{{AFX_MSG_MAP(CFileTransferClientView)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS, OnFieldChangedIPAddress)
	ON_BN_CLICKED(IDC_BUTTON_GETFILE, OnButtonGetFile)
	ON_CBN_SELCHANGE(IDC_COMBOIPADDRESSES, OnSelChangeComboIPAddresses)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
	ON_REGISTERED_MESSAGE( UWM_TCPEVENT, OnGetFileEvent )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTransferClientView construction/destruction

CFileTransferClientView::CFileTransferClientView()
	: CFormView(CFileTransferClientView::IDD)
{
	//{{AFX_DATA_INIT(CFileTransferClientView)
	//}}AFX_DATA_INIT
	// TODO: add construction code here

//	m_iNumMismatches = 0;
	m_pThreadTCP = NULL;
}

CFileTransferClientView::~CFileTransferClientView()
{
	delete m_pThreadTCP;	// NULL deletes are OK
}

void CFileTransferClientView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileTransferClientView)
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctlStatus);
	DDX_Control(pDX, IDC_COMBOIPADDRESSES, m_cbIPAddresses);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctlTargetIP);
	DDX_Control(pDX, IDC_BUTTON_GETFILE, m_btnGetFile);
	//}}AFX_DATA_MAP
}

BOOL CFileTransferClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CFileTransferClientView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	
	// initialize controls based on .INI file settings

	CWinApp* pApp = AfxGetApp();

	int nDex = pApp->GetProfileInt( "Settings", "Combo Box", -1 );
	m_cbIPAddresses.SetCurSel( nDex );

	CString strIP = pApp->GetProfileString( "Settings", "IP Address", NULL );
	DWORD dwIP = ntohl( inet_addr( strIP.GetBuffer( MAX_PATH ) ) );
	m_ctlTargetIP.SetAddress( dwIP );

//	CString fName = pApp->GetProfileString( "Settings", "output_path", NULL );
//	m_ctlOutputPath.SetWindowText( fName );

//	BOOL bCheck = (BOOL)pApp->GetProfileInt( "Settings", "Inject Mismatch Events", 0 );
//	m_ctlSimulate.SetCheck( bCheck );

	
	// setup auxiliary controls

	CString msg;
	msg.Empty();

	msg.Format( "Status: Idle" );
	m_ctlStatus.SetWindowText( msg );

//	msg.Format( "Receive mismatch events = %d", m_iNumMismatches );
//	m_ctlNumMismatches.SetWindowText( msg );

//	m_ctlProgressGet.SetRange( 0, 128 );
//	m_ctlProgressGet.SetPos( 0 );

}


#define GFE_THREADSTART			0x00000001
#define GFE_THREADCOMPLETE		0x00000002
#define GFE_UPDATECONTROLS		0x00000004
#define GFE_STATUSCONNECTED		0x00000008

afx_msg LRESULT CFileTransferClientView::OnGetFileEvent(WPARAM wParam, LPARAM lParam)
{

	switch ( wParam )
	{
	case GFE_THREADSTART:
		m_btnGetFile.EnableWindow( FALSE );
//		m_ctlOutputPath.EnableWindow( FALSE );
		m_cbIPAddresses.EnableWindow( FALSE );
		m_ctlTargetIP.EnableWindow( FALSE );
//		m_ctlSimulate.EnableWindow( FALSE );

		m_ctlStatus.SetWindowText( "Trying to connect ..." );

		break;

	case GFE_THREADCOMPLETE:
		m_btnGetFile.EnableWindow( TRUE );
//		m_ctlOutputPath.EnableWindow( TRUE );
		m_cbIPAddresses.EnableWindow( TRUE );
		m_ctlTargetIP.EnableWindow( TRUE );
//		m_ctlSimulate.EnableWindow( TRUE );

		m_ctlStatus.SetWindowText( "Status: Idle" );
//		m_ctlProgressGet.SetPos( 0 );

		m_pThreadTCP = NULL;

		break;

	case GFE_UPDATECONTROLS:
		{
//			CString msg;
//			msg.Format( "Receive mismatch events = %d", m_iNumMismatches );
//			m_ctlNumMismatches.SetWindowText( msg );
		}

		break;

	case GFE_STATUSCONNECTED:
		m_ctlStatus.SetWindowText( "Connection established -- receiving file ..." );


		break;

	default:
		ASSERT( FALSE );	// shouldn't get here
	}

	return 0L;
}

void CFileTransferClientView::OnDestroy() 
{
	// save settings in .INI file
	
	CWinApp* pApp = AfxGetApp();

	pApp->WriteProfileInt( "Settings", "Combo Box", m_cbIPAddresses.GetCurSel() );

	IN_ADDR ia;
	DWORD dwIP;
	m_ctlTargetIP.GetAddress( dwIP );
	ia.S_un.S_addr = htonl( dwIP );
	pApp->WriteProfileString( "Settings", "IP Address", inet_ntoa( ia ) );


//	CString fName;
//	m_ctlOutputPath.GetWindowText( fName );	
//	pApp->WriteProfileString( "Settings", "output_path", fName );


//	pApp->WriteProfileInt( "Settings", "Inject Mismatch Events", (BOOL)m_ctlSimulate.GetCheck() );


	CFormView::OnDestroy();
	
}

/////////////////////////////////////////////////////////////////////////////
// CFileTransferClientView printing

BOOL CFileTransferClientView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFileTransferClientView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFileTransferClientView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CFileTransferClientView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CFileTransferClientView diagnostics

#ifdef _DEBUG
void CFileTransferClientView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFileTransferClientView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CFileTransferClientDoc* CFileTransferClientView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFileTransferClientDoc)));
	return (CFileTransferClientDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFileTransferClientView message handlers

void CFileTransferClientView::OnFieldChangedIPAddress(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CFileTransferClientView::OnButtonGetFile() 
{
	if ( m_pThreadTCP != NULL )	// make certain thread is not already running
		return;

	DWORD dwIP = 0;
	IN_ADDR ia;
//	CString strLocalFileName;
	CString strTargetIP;

	// get IP address

	m_ctlTargetIP.GetAddress( dwIP );
	ia.S_un.S_addr = htonl( dwIP );
	strTargetIP = inet_ntoa( ia );

	// get file name and check for invalid characters

//	m_ctlOutputPath.GetWindowText( strLocalFileName );
//	if ( -1 != strLocalFileName.FindOneOf( ";*?\"<>|" ) )
///	{
//		CString sMessage;
//		sMessage.Format( "File name contains invalid characters (;*?\"<>| are not permitted)\n"
//			"%s\nEnter another name", strLocalFileName );
//		::MessageBox( NULL, sMessage, "Invalid File Name", MB_OK|MB_ICONEXCLAMATION );
//		return;
//	}

	// now get the file
	
///	BOOL bRet = GetFileFromRemoteSender( strTargetIP, strLocalFileName );	// now done in separate thread, below

//	m_strFileName = strLocalFileName;
	m_strTargetIP = strTargetIP;
//	m_bSimulateEvents = m_ctlSimulate.GetCheck();

	m_pThreadTCP = ::AfxBeginThread( ThreadedGetFileFromRemoteSender, (LPVOID) this, THREAD_PRIORITY_BELOW_NORMAL );

}

void CFileTransferClientView::OnSelChangeComboIPAddresses() 
{
	CString strIP, strTemp;
	strIP.Empty();

	m_cbIPAddresses.GetLBText( m_cbIPAddresses.GetCurSel(), strIP );

	DWORD dwIP = ntohl( inet_addr( strIP.GetBuffer( MAX_PATH ) ) );
	m_ctlTargetIP.SetAddress( dwIP );

/*******************

  use inet_addr function (above) instead of this code
  
********************/
/*
	int ii, nDex;
	BOOL bErr = TRUE;
	BYTE b[4];
	for ( ii=0; ii<3; ii++ )
	{
		nDex = strIP.Find( '.', 0 );

		if ( nDex == -1 )
		{
			bErr = FALSE;
			break;
		}

		strTemp = strIP.Mid( 0, nDex);
		strIP = strIP.Mid( nDex+1 );
		b[ii] = atoi( strTemp );

	}

	b[3] = atoi( strIP );

	if ( bErr )
	{
		m_ctlTargetIP.SetAddress( b[0], b[1], b[2], b[3] );
	}
*/

}

#define PRE_AGREED_PORT		8686
#define RECV_BUFFER_SIZE	16


UINT CFileTransferClientView::ThreadedGetFileFromRemoteSender(LPVOID pVoid)
{
	CFileTransferClientView* pThis = (CFileTransferClientView*)pVoid;
	
	// advise main thread that this thread has started
	
	pThis->PostMessage( UWM_TCPEVENT, GFE_THREADSTART, 0L );
	
	// create client socket and connect to server

	AfxSocketInit(NULL);	// make certain this is done somewhere in each thread (usually in InitInstance for main thread)
	CSocket sockClient;
	sockClient.Create();
	
	sockClient.Connect( pThis->m_strTargetIP, PRE_AGREED_PORT );	// PRE_AGREED_PORT is #define'd as 8686

	pThis->PostMessage( UWM_TCPEVENT, GFE_STATUSCONNECTED, 0L );


	// local variables used in file transfer (declared here to avoid "goto skips definition"-style compiler errors)

//	int simulate;		// used to simulate mismatch events (if designated by user)

//	BYTE* recdData = NULL;	// pointer to buffer for receiving data (memory is allocated after obtaining file size)

	int pbInterval = 0; int curPB = 0;	// progress bar variables

	

	// Inject mismatch events (if selected by user)
	// Seed the random-number generator with current time 
	srand( (unsigned)time( NULL ) );

	for (int i=0; i<3; i++)
	{
		#define PACKET_SIZE 1
		char szEvent[PACKET_SIZE] = { i+1 };
		int cbLeftToSend = PACKET_SIZE;	
		do
		{
			BYTE* bp = (BYTE*)(szEvent) + PACKET_SIZE - cbLeftToSend;
			int cbBytesSent = sockClient.Send( bp, cbLeftToSend );
			
			// test for errors and get out if they occurred
			if ( cbBytesSent == SOCKET_ERROR )
			{
				int iErr = ::GetLastError();
				TRACE( "SendFileToRemoteRecipient returned a socket error while sending file length\n"
					"\tNumber of Bytes sent = %d\n"
					"\tGetLastError = %d\n", cbBytesSent, iErr );
				
				/* you should handle the error here */
				
				goto PreReturnCleanup;
			}
			
			// data was successfully sent, so account for it with already-sent data
			cbLeftToSend -= cbBytesSent;
		}
		while ( cbLeftToSend>0 );
		Sleep(1000);
	}
	
PreReturnCleanup:		// labelled "goto" destination

	// free allocated memory
	// if we got here from a goto that skipped allocation, delete of NULL pointer
	// is permissible under C++ standard and is harmless
//	delete[] recdData;		

//	if ( bFileIsOpen )
//		destFile.Close();	// only close file if it's open (open might have failed above)

	sockClient.Close();

	pThis->PostMessage( UWM_TCPEVENT, GFE_THREADCOMPLETE, 0L );
	
	return 0;
}


