// -*- c++ -*- Commands.cpp : implementation file
//

#include "stdafx.h"
#include "meMsdev.h"
#include "Commands.h"
#include <process.h>                    // For _spawn
#include <comdef.h>                     // For _bstr_t
#include "MainDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Local definitions
#define MICROEMACS_INI_FILE           "me32.ini"
#define MICROEMACS_EXE_LOCATION       "c:\\Program Files\\JASSPA\\MicroEmacs\\me32.exe"


static BOOL bEmacsInit = FALSE;
static BOOL g_bEnableEmacs = FALSE;
static BOOL g_bAutoClose = FALSE;

// Local variables used by MicroEmacs commands. Note this is all a little
// dirty. To be honset I do not really know what I am doing with this
// stuff; however we have got the bugger going !! -- Jon -- 30th Dec 1998
static HWND lastHwnd = NULL;            // The last window handle
static HANDLE cmdHandle = INVALID_HANDLE_VALUE; // Command buffer handle
static char emacsExecutable [1024];     // Location of executable
static char tempDir [1024];             // Temporary directory
static char meUserName[128];            // Current user name

/////////////////////////////////////////////////////////////////////////////
// CCommands

//static FILE *fp=NULL ;
static void
MicroEmacsInit (void)
{
    char buff[128];
    char *temp, *ss;
    DWORD size=128 ;
    
    // Get the location of the executable
    GetPrivateProfileString ("Location",
                             "exe",
                             MICROEMACS_EXE_LOCATION,
                             emacsExecutable,
                             sizeof (emacsExecutable)-1,
                             MICROEMACS_INI_FILE);
    //    fp = fopen("c:\\log","w+") ;
    
    if (((ss = getenv ("MENAME")) != NULL) && (*ss != '\0'))
        strcpy (meUserName,ss);
    else if ((GetUserName (meUserName, &size) == TRUE) && (meUserName[0] != '\0'))
        ;
    else if (((ss = getenv ("LOGNAME")) != NULL) && (*ss != '\0'))
        strcpy (meUserName, ss);
    else
        strcpy (meUserName, "guest");
    //    fprintf(fp,"Here 5 %s\n",meUserName) ;
    //    fflush(fp) ;
    GetPrivateProfileString (meUserName,
                             "mename",
                             "",
                             buff,
                             sizeof (buff)-1,
                             MICROEMACS_INI_FILE);
    //    fprintf(fp,"Here 6 %s %s\n",meUserName,buff) ;
    //    fflush(fp) ;
    if(buff[0] != '\0')
        strcpy (meUserName, buff);
    
    // Get the temporary directory
    if ((temp = getenv ("TEMP")) != NULL)
        strcpy (tempDir, temp);
    else
        tempDir[0] = '\0';
    //    fprintf(fp,"Here 7 %s %s\n%s\n",tempDir,meUserName,emacsExecutable) ;
    //    fflush(fp) ;
    
    // Get the default settings out of the private profile
    GetPrivateProfileString (meUserName,
                             "msdevEnableEmacs",
                             "0", 
                             buff,
                             sizeof (buff) - 1,
                             MICROEMACS_INI_FILE);
    g_bEnableEmacs = ((strcmp (buff, "1") == 0) ? TRUE : FALSE);
    GetPrivateProfileString (meUserName,
                             "msdevAutoClose",
                             "0",
                             buff,
                             sizeof (buff) - 1,
                             MICROEMACS_INI_FILE);
    g_bAutoClose = ((strcmp (buff, "1") == 0) ? TRUE : FALSE);

    // Have initialised.
    bEmacsInit = TRUE;
}

// MicroEmacsHwnd; Get the current microEmacs window handle. This is located
// in the head of the response file. We have to read this every time because
// the response file contents may have changed. I need to add some sort of
// cache here; but will leave that for another day. 
static long 
MicroEmacsHwnd (void)
{
    HANDLE rspHandle;                   // File handle.
    long hwndId = 0;                    // Base identity
    char buf [12];                      // Character buffer for handle
    char nambuf [1024];
    
    /* create the cammand file name */
    sprintf (nambuf, "%s\\me%s.rsp", tempDir, meUserName) ;
    
    // Try opening the file
    if ((rspHandle = CreateFile (nambuf,
                                 GENERIC_READ,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL)) != INVALID_HANDLE_VALUE)
    {
        DWORD numRead;                  // Bytes read
        
        // Back to the top and read the 1st line.
        SetFilePointer(rspHandle,0,NULL,FILE_BEGIN);
        if (ReadFile (rspHandle, buf, 12, &numRead, NULL) != 0)
        {
            char *p = buf;
            long sign = 1;
            
            // Convert the integer value to a window handle
            while (--numRead >= 0)
            {
                if (!isdigit (*p))
                {
                    if ((p == buf) && (*p == '-'))
                        sign = -sign;
                    else
                        break;
                }
                else 
                    hwndId = (hwndId * 10) + (*p - '0');
                p++;
            }
            hwndId *= sign;
        }
        
        // Close the file handle.
        CloseHandle (rspHandle);
    }
    
    return (hwndId);
}

// MicroEmacsOpen; Open the Emacs command file. This is located in the $TEMP
// directory and is called by the windows name. If MicroEamcs is not running
// then give it a jolt by spawing it off.
static int
MicroEmacsOpen (void)
{
    int retries = 0;
    char nambuf [1024];
    char buf [1024];
    
    // Initialise the editor variables.
    if (!bEmacsInit)
        MicroEmacsInit ();
    
    /* create the cammand file name */
    sprintf (nambuf, "%s\\me%s.cmd", tempDir, meUserName) ;
    
    while (retries++ < 10)
    {
        //        fprintf(fp,"%s %d\n",nambuf,retries) ;
        /* Try opening the file */
        if ((cmdHandle = CreateFile (nambuf,
                                     GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL)) != INVALID_HANDLE_VALUE)
        {
            /* Goto the end of the file */
            SetFilePointer (cmdHandle,0,NULL,FILE_END);
            return 0;                   /* OK */
        }
        
        if (retries == 1)
        {
            // try and launch a new ME
            sprintf (buf, "\"%s\"", emacsExecutable);
            
            _spawnlp (_P_NOWAIT, emacsExecutable, buf, "-c", NULL);
        }
        // Sleep for 2 seconds and give it time to come up
        Sleep (2000);
    }
    return 1;                           /* Error */
}


// MicroEmacsWrite; Provide our own formatted write cos Microsoft
// do not provide one.  Anybody know why Microsoft have to do
// everything differently ??
static int
MicroEmacsWrite (char *fmt, ...)
{
    DWORD written;
    char buf [2048];
    va_list ap;
    
    /* Quick check */
    if (cmdHandle == NULL)
        return 0;
    
    va_start (ap, fmt);
    if ((written = vsprintf (buf, fmt, ap)) > 0)
        WriteFile (cmdHandle, buf, written, &written, NULL);
    
    va_end (ap);
    return written;
}

// MicroEmacsClose; Close down the MicroEmacs access to the server file.
static void
MicroEmacsClose (void)
{
    HWND Hwnd;
    
    if (cmdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle (cmdHandle);
        cmdHandle = INVALID_HANDLE_VALUE;
    }
    
    /* Bring the editor to the foreground and/or uniconize */
    if ((Hwnd = (HWND)(MicroEmacsHwnd())) != 0)
    {
        if (IsIconic (Hwnd))
            ShowWindow (Hwnd, SW_SHOWNORMAL);
        else
            SetForegroundWindow (Hwnd);
    }
}

CCommands::CCommands()
{
    m_pApplication = NULL;
    m_pApplicationEventsObj = NULL;
    m_pDebuggerEventsObj = NULL;
    
    m_fOutputWnd = FALSE;
    m_fSaveOnlyEnabled = FALSE;
    m_dwAddInID = NULL;
    m_dwAppEvents = NULL;
    m_pDispBrkPnts = NULL;
}

CCommands::~CCommands()
{
    ASSERT (m_pApplication != NULL);
    m_pApplication->Release();
}

void CCommands::SetApplicationObject(IApplication* pApplication)
{
    // This function assumes pApplication has already been AddRef'd
    //  for us, which CDSAddIn did in its QueryInterface call
    //  just before it called us.
    m_pApplication = pApplication;
    
    // Create Application event handlers
    XApplicationEventsObj::CreateInstance(&m_pApplicationEventsObj);
    m_pApplicationEventsObj->AddRef();
    m_pApplicationEventsObj->Connect(m_pApplication);
    m_pApplicationEventsObj->m_pCommands = this;
    
    // Create Debugger event handler
    CComPtr<IDispatch> pDebugger;
    if (SUCCEEDED(m_pApplication->get_Debugger(&pDebugger))
        && pDebugger != NULL)
    {
        XDebuggerEventsObj::CreateInstance(&m_pDebuggerEventsObj);
        m_pDebuggerEventsObj->AddRef();
        m_pDebuggerEventsObj->Connect(pDebugger);
        m_pDebuggerEventsObj->m_pCommands = this;
    }
}

void CCommands::UnadviseFromEvents()
{
    ASSERT (m_pApplicationEventsObj != NULL);
    m_pApplicationEventsObj->Disconnect(m_pApplication);
    m_pApplicationEventsObj->Release();
    m_pApplicationEventsObj = NULL;
    
    if (m_pDebuggerEventsObj != NULL)
    {
        // Since we were able to connect to the Debugger events, we
        //  should be able to access the Debugger object again to
        //  unadvise from its events (thus the VERIFY_OK below--see stdafx.h).
        CComPtr<IDispatch> pDebugger;
        VERIFY_OK(m_pApplication->get_Debugger(&pDebugger));
        ASSERT (pDebugger != NULL);
        m_pDebuggerEventsObj->Disconnect(pDebugger);
        m_pDebuggerEventsObj->Release();
        m_pDebuggerEventsObj = NULL;
    }
}


/////////////////////////////////////////////////////////////////////////////
// Event handlers

// TODO: Fill out the implementation for those events you wish handle
//  Use m_pCommands->GetApplicationObject() to access the Developer
//  Studio Application object

// Application events

HRESULT CCommands::XApplicationEvents::BeforeBuildStart()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::BuildFinish(long nNumErrors, long nNumWarnings)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeApplicationShutDown()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentOpen(IDispatch* theDocument)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    // Initialise the editor variables.
    if (!bEmacsInit)
        MicroEmacsInit ();
    
    if (g_bEnableEmacs)
    {
        HRESULT hr;
        
        CComQIPtr<ITextDocument, &IID_ITextDocument> pDoc(theDocument);
        if (!pDoc)
            return S_OK;
        
        BSTR FullName;
        hr = pDoc->get_FullName(&FullName);
        if (FAILED(hr))
            return S_OK;
        
        char Name[2048];
        
        /* Convert the name. This is a BSTR type and we want chars cos
         * MicroEmacs only deals with chars - how many different ways can one
         * hold a character ?? I hate Microsoft, they make life too bloody
         * difficult */
        {
            short *p = (short *)(FullName);
            char *q = Name;
            int cc;
            
            while ((cc = *p++) != 0)
            {
                if (cc == '"')
                    *q++ = '\\', *q++ = '"';
                else if (cc == '\\')
                    *q++ = cc, *q++ = cc;
                else
                    *q++ = cc;
            }
            *q++ = '\0';
        }
        
        char LineParam[64];
        
        long CurrentLine = -1;
        
        LPDISPATCH lpdisp;
        hr = pDoc->get_Selection(&lpdisp);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<ITextSelection, &IID_ITextSelection> pSel(lpdisp);
            if (!pSel)
            {
                lpdisp->Release();
                return S_OK;
            };
            
            hr = pSel->get_CurrentLine(&CurrentLine);
            if (SUCCEEDED(hr))
            {
                sprintf(LineParam, "%d", CurrentLine);
            }
            
            lpdisp->Release();
        }
        
        if(!MicroEmacsOpen ())
        {
            // Cannot find Line Number or does not know. Let Emacs sort
            // out where we are if the line number is 1 - this is the
            // default when we open a file.
            if ((CurrentLine == -1)||(CurrentLine == 1))
                MicroEmacsWrite ("C:MSDEV:find-file \"%s\"\n", Name);
            else  // Found Current Line
            {
                MicroEmacsWrite ("C:MSDEV:find-file \"%s\"\n", Name);
                MicroEmacsWrite ("C:MSDEV:goto-line %s\n", LineParam);
            }
            MicroEmacsClose ();
        }
        if (g_bAutoClose)
        {
            CComVariant vSaveChanges = dsSaveChangesPrompt;
            DsSaveStatus Saved;
            pDoc->Close(vSaveChanges, &Saved);
        }
    }
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeDocumentClose(IDispatch* theDocument)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentSave(IDispatch* theDocument)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewDocument(IDispatch* theDocument)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    // Initialise the editor variables.
    if (!bEmacsInit)
        MicroEmacsInit ();
    
    if (g_bEnableEmacs)
    {
        MicroEmacsWrite ("C:MSDEV:find-buffer \"*scratch*\"\n");
    }
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowActivate(IDispatch* theWindow)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#if 0
    if (g_bEnableEmacs)
    {
        HRESULT hr;
        CComQIPtr<IGenericWindow, &IID_IGenericWindow> pWindow(theWindow);
        if (!pWindow)
            return S_OK;
        
        if (g_bAutoClose)
        {
            CComVariant vSaveChanges = dsSaveChangesPrompt;
            DsSaveStatus Saved;
            pWindow->Close(vSaveChanges, &Saved);
        }
    }
#endif
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowDeactivate(IDispatch* theWindow)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceOpen()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceClose()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewWorkspace()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_OK;
}

// Debugger event

HRESULT CCommands::XDebuggerEvents::BreakpointHit(IDispatch* pBreakpoint)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#if 0
    CComQIPtr<IBreakpoint, &IID_IBreakpoint> pBreakpoint = pBP;
    
    // We only care about line# breakpoints. We can tell whether this
    // is a line# breakpoint by seeing whether its Location property
    // begins with a period (e.g., ".253")
    
    CComBSTR bstrLocation;
    
    pBreakpoint->get_Location(&bstrLocation);
    
    if (bstrLocation.Length() == 0 || *(BSTR)bstrLocation != '.')
        return S_OK;
    
    // Is it enabled as a tracepoint?
    
    CComBSTR bstrFile;
    
    pBreakpoint->get_File(&bstrFile);
    
    CString strFullInfo = (BSTR) bstrFile;
    
    strFullInfo += (BSTR) bstrLocation;
    
    BOOL* pEnable = NULL;
    
    m_pCommands->m_mapEnable.Lookup(strFullInfo, (void*&)pEnable);
    
    if (pEnable == NULL || *pEnable == FALSE)
        return S_OK;
    
    // Yes, it's a tracepoint. Let's output the expressions.
    CStringArray* pExprArray = NULL;
    
    m_pCommands->m_mapExpr.Lookup(strFullInfo, (void*&)pExprArray);
    
    ASSERT(pExprArray);
    
    IDebugger* pDebugger = m_pCommands->GetDebuggerObject();
    
    IApplication* pApplication = m_pCommands->GetApplicationObject();
    
    for (int nLoop = 0; nLoop < pExprArray->GetSize(); nLoop++)
    {
        CComBSTR bstrValue(_T("<Expression could not be evaluated>"));
        pDebugger->Evaluate(CComBSTR(pExprArray->GetAt(nLoop)),
                            &bstrValue);
        
        CComBSTR bstrOut(pExprArray->GetAt(nLoop));
        bstrOut += CComBSTR(_T(" = "));
        bstrOut += bstrValue;
        pApplication->PrintToOutputWindow(bstrOut);
    }
    pDebugger->Go();
#endif
    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CCommands methods

STDMETHODIMP CCommands::MeMsdevCommandMethod()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    // TODO: Replace this with the actual code to execute this command
    //  Use m_pApplication to access the Developer Studio Application object,
    //  and VERIFY_OK to see error strings in DEBUG builds of your add-in
    //  (see stdafx.h)
    
    VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
    //AfxMessageBox(_T("MeMsdev Command invoked."),
    //              MB_OK | MB_ICONINFORMATION);
    CMainDialog MyDialog;
    
    // If we have not initialised then now is the time to do it.
    if (!bEmacsInit)
        MicroEmacsInit ();
    
    MyDialog.m_bEnableEmacs = g_bEnableEmacs;
    MyDialog.m_bAutoClose = g_bAutoClose;
    MyDialog.DoModal();
    g_bEnableEmacs = MyDialog.m_bEnableEmacs;
    g_bAutoClose = MyDialog.m_bAutoClose;
    VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
    
    // Write the settings back into the .ini file. I guess we 
    // should really use the registry. However go old UNIX 
    // traditions tell us that with the .ini file we can
    // hack it and list it if we want to !!
    WritePrivateProfileString (meUserName,
                               "msdevEnableEmacs",
                               (g_bEnableEmacs ? "1" : "0"),
                               MICROEMACS_INI_FILE);
    WritePrivateProfileString (meUserName,
                               "msdevAutoClose",
                               (g_bAutoClose ? "1" : "0"),
                               MICROEMACS_INI_FILE);
    // Force a flush back to the file system
    WritePrivateProfileString (NULL, NULL, NULL, MICROEMACS_INI_FILE);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////
// CCommands Debugger Methods

HRESULT
CCommands::GetBrkPnts()
{
    HRESULT hr = S_OK;
    if (m_pDispBrkPnts == NULL)
    {
        CComPtr<IDispatch> pDispDebugger;
        CComQIPtr<IDebugger, &IID_IDebugger> pDebugger;
        
        hr = m_pApplication->get_Debugger(&pDispDebugger);
        pDebugger = pDispDebugger;
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr) && pDebugger)
        {
            hr = pDebugger->get_Breakpoints(&m_pDispBrkPnts);
            m_pBrkPnts = m_pDispBrkPnts;
        }
    }
    if (!m_pBrkPnts)
        hr = E_FAIL;
    return(hr);
}

HRESULT
CCommands::BrkPntMgr()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    m_pApplication->EnableModeless(VARIANT_FALSE);
#if 0
    HRESULT hr;
    int iRes;
    CDlgBrkPnts dlgBrkPnts;
    
    hr = GetBrkPnts();
    ASSERT(SUCCEEDED(hr));
    if (m_pBrkPnts)
    {
        dlgBrkPnts.SetBrkPnts(this);
        iRes = dlgBrkPnts.DoModal();
        
    }
#endif
    m_pApplication->EnableModeless((VARIANT_BOOL)VARIANT_TRUE);
    return S_OK;
}


HRESULT
CCommands::Save(BOOL fToOutputWindow, LPCTSTR szFile, BOOL fSaveOnlyEnabled, CString& strComment)
{
    long l, cBrks = 0;
    CStringArray rgStrBrks;
    HRESULT hr;
    
    m_strFile = szFile;
    _ASSERTE(m_pBrkPnts);
    hr = m_pBrkPnts->get_Count(&cBrks);
    l = 0;
    rgStrBrks.Add(strComment); // put the comment string first.
    while (l < cBrks)
    {
        CComPtr<IDispatch> pDispBrkPnt;
        CComQIPtr<IBreakpoint, &IID_IBreakpoint> pBrkPnt;
        CComBSTR bstrLocation;
        CComBSTR bstrFile;
        CComBSTR bstrFunction;
        //long lElements;
        short sEnabled;
        //DsWindowsMessage dsWndMsg;
        //long lPassCount;
        CComBSTR bstrExecutable;
        DsBreakpointType lType;
        CComBSTR bstrWndProc;
        CComVariant varItem = l;
        /* Type:
           dsChangedExpression
           dsLocation
           dsLocationWithTrueExpression
           dsLocationWithChangedExpression
           dsMessage
           dsTrueExpression
         */
        
        hr = m_pBrkPnts->Item(varItem, &pDispBrkPnt);
        pBrkPnt = pDispBrkPnt;
        
        hr = pBrkPnt->get_Enabled(&sEnabled);
        CString strOut, strT;
        CComBSTR bstrT ;
        hr = pBrkPnt->get_Type((long *)&lType);
        hr = pBrkPnt->get_File(&bstrFile);
        hr = pBrkPnt->get_Function(&bstrFunction);
        hr = pBrkPnt->get_Executable(&bstrExecutable);
        hr = pBrkPnt->get_Location(&bstrLocation);
        strOut.Format("%d:",lType);
        bstrT = bstrFile;
        bstrT.Append(_T("("));
        strT = bstrLocation;
        if (strT.GetAt(0) == _T('.'))
#if 0
            strT.Delete(0); // remove leading "."
#else
        {
            CString mystrT = strT.Mid(1);
            strT = mystrT;
        }
#endif   
        bstrT.Append(strT);
        bstrT.Append(_T("): "));
        bstrT.Append(strOut);
        bstrT.AppendBSTR(bstrExecutable);
        bstrT.Append(_T(";"));
        bstrT.AppendBSTR(bstrFunction);
        bstrT.Append(_T(";"));
        if (fToOutputWindow)
        {
            m_pApplication->PrintToOutputWindow(bstrT);
        }
        
        if (sEnabled || !fSaveOnlyEnabled)
        {
            strT = bstrT;
            rgStrBrks.Add(strT);
        }
        
        l++;
    }
    // save out the breakpoints
    if (!m_strFile.IsEmpty())
    {
        CFile fileBrkList;
        if (m_strFile.ReverseFind(_T('.')) == -1)
        {
            m_strFile += _T(".brk"); // add the missing extension
        }
        TRY
        {
            if (fileBrkList.Open(m_strFile, CFile::modeCreate | CFile::modeWrite))
            {
                CArchive ar( &fileBrkList, CArchive::store);
                
                rgStrBrks.Serialize(ar);
            }
        }
        CATCH(CFileException, e)
        {
            CString strMsg;
            CString strErr;
            e->GetErrorMessage(strErr.GetBuffer(_MAX_PATH), _MAX_PATH);
            strErr.ReleaseBuffer();
            strMsg.Format("%s \n    %s", strErr, m_strFile);
            MessageBox(NULL, strMsg, _T("Breakpoint Files"), MB_OK);
        }
        END_CATCH
              }
    return(S_OK);
}

HRESULT
CCommands::Load(LPCTSTR szFile, CString& strComment)
{
    CStringArray rgStrBrks, rgStrBrksFailed;
    CString strBrk;
    CComBSTR bstrT;
    HRESULT hr;
    
    m_strFile = szFile;
    if (!m_strFile.IsEmpty())
    {
        CFile fileBrkList;
        if (m_strFile.ReverseFind(_T('.')) == -1)
        {
            m_strFile += _T(".brk"); // add the missing extension
        }
        TRY
        {
            if (fileBrkList.Open(m_strFile, CFile::modeRead))
            {
                CArchive ar( &fileBrkList, CArchive::load);
                
                rgStrBrks.Serialize(ar);
            }
        }
        CATCH(CFileException, e)
        {
            CString strMsg;
            CString strErr;
            e->GetErrorMessage(strErr.GetBuffer(_MAX_PATH), _MAX_PATH);
            strErr.ReleaseBuffer();
            strMsg.Format("%s \n    %s", strErr, m_strFile);
            MessageBox(NULL, strMsg, _T("Breakpoint Files"), MB_OK);
        }
        END_CATCH;
        if (rgStrBrks.GetUpperBound() >= 0)
        {
            int i, ich;
            long l;
            i = 0;
            if (i <= rgStrBrks.GetUpperBound())
            {
                strComment = rgStrBrks.GetAt(i);
                i++;
            }
            while (i <= rgStrBrks.GetUpperBound())
            {
                CString strLine;
                CComVariant varSel;
                CComPtr<IGenericDocument> pDoc = NULL;
                CComQIPtr<ITextDocument, &IID_ITextDocument> pTextDoc;
                CComPtr<IDispatch> pDispBrkPnt;
                CComPtr<IDispatch> pDispSel;
                CComQIPtr<ITextSelection, &IID_ITextSelection> pSel;
                strBrk = rgStrBrks.GetAt(i);
                ich = strBrk.Find(_T('('));
                _ASSERTE(ich >= 0);
                hr = FindDoc(strBrk.Left(ich), pDoc);
                if (SUCCEEDED(hr) && pDoc)
                {
                    hr = pDoc->put_Active((VARIANT_BOOL)VARIANT_TRUE);
                    pTextDoc = pDoc;
                    if (pTextDoc)
                    {
                        strBrk = strBrk.Mid(ich+1);
                        ich = strBrk.Find(_T(')'));
                        strLine = strBrk.Left(ich);
                        l = atol(strLine);
                        
                        hr = pTextDoc->get_Selection(&pDispSel);
                        if (SUCCEEDED(hr) && pDispSel)
                        {
                            CComVariant varSelMode = dsMove;
                            pSel = pDispSel;
                            hr = pSel->GoToLine(l, varSelMode);
                            
                            varSel = l;
                            hr = m_pBrkPnts->AddBreakpointAtLine(varSel, &pDispBrkPnt);
                        }
                    }
                }
                if (FAILED(hr))
                {
                    rgStrBrksFailed.Add(rgStrBrks.GetAt(i));
                }
                i++;
            }
        }
    }
    return(S_OK);
}



HRESULT
CCommands::FindDoc(LPCTSTR szFile, CComPtr<IGenericDocument>& pDoc, BOOL fOkToOpen /*= TRUE*/)
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispatch> pDispDocuments;
    CComQIPtr<IDocuments, &IID_IDocuments> pDocuments;
    CComBSTR bstrFile;
    m_pApplication->get_Documents(&pDispDocuments);
    pDocuments = pDispDocuments;
    pDispDocuments = NULL;
    BOOL fFound = FALSE;
    
    if (pDocuments)
    {	// ENSURE DOC IS OPEN -- WE KEEP FILES AROUND THAT MAY HAVE BEEN CLOSED
        CComVariant vtDocType = _T("Auto");
        CComVariant vtBoolReadOnly = FALSE;
        CComPtr<IDispatch> pDispDoc;
        bstrFile = szFile;
        CComQIPtr<IGenericDocument, &IID_IGenericDocument> pDocument;
        if (fOkToOpen)
        {
            hr = pDocuments->Open(bstrFile, vtDocType, vtBoolReadOnly, &pDispDoc);
            pDocument = pDispDoc;
            if (SUCCEEDED(hr) && pDocument)
            {
                fFound = TRUE;
                pDoc = pDocument;
            }
        }
        if (!fFound)
        {
            CComPtr<IUnknown> pUnk;
            CComQIPtr<IEnumVARIANT, &IID_IEnumVARIANT> pNewEnum;
            if (SUCCEEDED(pDocuments->get__NewEnum(&pUnk)) && pUnk != NULL)
            {
                pNewEnum = pUnk;
                VARIANT varGenericDocument;
                
                while (!fFound && pNewEnum->Next(1, &varGenericDocument, NULL) == S_OK)
                {
                    CComQIPtr<IGenericDocument, &IID_IGenericDocument> pGenericDocument;
                    ASSERT (varGenericDocument.vt == VT_DISPATCH);
                    pGenericDocument = varGenericDocument.pdispVal;
                    VariantClear(&varGenericDocument);
                    CComBSTR bstrFullName;
                    pGenericDocument->get_FullName(&bstrFullName);
                    if ((CString) (bstrFullName) == (CString)(szFile))
                    {
                        fFound = TRUE;
                        pDoc = pGenericDocument;
                    }
                }
            }
            
            
        }
    }
    
    return(hr);
}


HRESULT
CCommands::ClearAll()
{
    HRESULT hr;
    long l, cBrks, lLine;
    CString strT;
    short sEnabled;
    VARIANT_BOOL fRemoved;
    CComBSTR bstrCommand;
    
    bstrCommand = _T("DebugRemoveAllBreakpoints");
    hr = m_pApplication->ExecuteCommand(bstrCommand);
    if (SUCCEEDED(hr))
    {
        return(hr);
    }
    
    hr = GetBrkPnts();
    if (SUCCEEDED(hr) && m_pBrkPnts)
    {
        hr = m_pBrkPnts->get_Count(&cBrks);
        l = 0;
        while (l < cBrks)
        {
            CComPtr<IDispatch> pDispBrkPnt;
            CComQIPtr<IBreakpoint, &IID_IBreakpoint> pBrkPnt;
            CComBSTR bstrLocation;
            CComBSTR bstrFile;
            CComVariant varItem = l;
            
            hr = m_pBrkPnts->Item(varItem, &pDispBrkPnt);
            if (SUCCEEDED(hr) && pDispBrkPnt)
            {
                pBrkPnt = pDispBrkPnt;
                
                hr = pBrkPnt->get_Enabled(&sEnabled);
                hr = pBrkPnt->get_File(&bstrFile);
                hr = pBrkPnt->get_Location(&bstrLocation);
                
                CComPtr<IGenericDocument> pDoc = NULL;
                CComQIPtr<ITextDocument, &IID_ITextDocument> pTextDoc;
                
                strT = bstrFile;
                hr = FindDoc(strT, pDoc);
                if (SUCCEEDED(hr) && pDoc)
                {
                    hr = pDoc->put_Active((VARIANT_BOOL)VARIANT_TRUE);
                    pTextDoc = pDoc;
                    if (SUCCEEDED(hr) && pTextDoc)
                    {
                        strT = bstrLocation;
#if 0                        
                        // JON : Does not work for MSDEV 5.0
                        strT.Remove(_T('.'));
#endif
                        lLine = atol(strT);
                        varItem = lLine;
                        hr = m_pBrkPnts->RemoveBreakpointAtLine(varItem, &fRemoved);
                    }
                }
            }
            l++;
        }
    }
    return(hr);
}

void
CCommands::SetSaveOnlyEnabled(BOOL fSaveOnlyEnabled)
{
    m_fSaveOnlyEnabled = fSaveOnlyEnabled;
}

void
CCommands::SetFile(LPCTSTR szFile)
{
    m_strFile = szFile;
}


void
CCommands::GetCounts(long &lTotal, long &lEnabled)
{
    HRESULT hr;
    long l;
    lTotal = 0;
    lEnabled = 0;
    hr = m_pBrkPnts->get_Count(&lTotal);
    _ASSERTE(SUCCEEDED(hr));
    
    l = 0;
    while (l < lTotal)
    {
        CComPtr<IDispatch> pDispBrkPnt;
        CComQIPtr<IBreakpoint, &IID_IBreakpoint> pBrkPnt;
        short sEnabled;
        CComVariant varItem = l;
        
        hr = m_pBrkPnts->Item(varItem, &pDispBrkPnt);
        pBrkPnt = pDispBrkPnt;
        
        hr = pBrkPnt->get_Enabled(&sEnabled);
        if (sEnabled)
        {
            lEnabled++;
        }
        
        l++;
    }
    
}
