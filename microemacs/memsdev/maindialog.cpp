// MainDialog.cpp : implementation file
//

#include "stdafx.h"
#include "meMsdev.h"
#include "MainDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainDialog dialog


CMainDialog::CMainDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDialog)
	m_bEnableEmacs = TRUE;
	m_bAutoClose = TRUE;
	//}}AFX_DATA_INIT
}


void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDialog)
	DDX_Check(pDX, IDC_ENABLEEMACS, m_bEnableEmacs);
	DDX_Check(pDX, IDC_AUTOCLOSE, m_bAutoClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
	//{{AFX_MSG_MAP(CMainDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainDialog message handlers
