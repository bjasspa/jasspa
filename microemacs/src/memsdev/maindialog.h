#if !defined(AFX_MAINDIALOG_H__AC72671A_2977_11D1_B2F3_006008040780__INCLUDED_)
#define AFX_MAINDIALOG_H__AC72671A_2977_11D1_B2F3_006008040780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MainDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainDialog dialog

class CMainDialog : public CDialog
{
// Construction
public:
	CMainDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMainDialog)
	enum { IDD = IDD_ADDINMAIN };
	BOOL	m_bEnableEmacs;
	BOOL	m_bAutoClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMainDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDIALOG_H__AC72671A_2977_11D1_B2F3_006008040780__INCLUDED_)
