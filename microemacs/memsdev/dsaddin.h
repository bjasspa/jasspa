// DSAddIn.h : header file
//

#if !defined(AFX_DSADDIN_H__43CE4E55_9FFA_11D2_950C_444553540000__INCLUDED_)
#define AFX_DSADDIN_H__43CE4E55_9FFA_11D2_950C_444553540000__INCLUDED_

#include "commands.h"

// {43CE4E42-9FFA-11D2-950C-444553540000}
DEFINE_GUID(CLSID_DSAddIn,
0x43ce4e42, 0x9ffa, 0x11d2, 0x95, 0xc, 0x44, 0x45, 0x53, 0x54, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public IDSAddIn,
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>
{
public:
	DECLARE_REGISTRY(CDSAddIn, "MeMsdev.DSAddIn.1",
		"MEMSDEV Developer Studio Add-in", IDS_MEMSDEV_LONGNAME,
		THREADFLAGS_BOTH)

	CDSAddIn() {}
	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

// IDSAddIns
public:
	STDMETHOD(OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD(OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

protected:
	CCommandsObj* m_pCommands;
	DWORD m_dwCookie;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSADDIN_H__43CE4E55_9FFA_11D2_950C_444553540000__INCLUDED)
