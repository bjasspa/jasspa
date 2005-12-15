/*******************************************************************************
 ***                                                                          ***
 ***                                                                          ***
 ***            Copyright (c) 1997 - 1999 Engineering Animation, Inc.         ***
 ***                                                                          ***
 ***    All rights reserved.  This program or documentation contains          ***
 ***    proprietary confidential information and trade secrets of             ***
 ***    Engineering Animation, Inc.  Reverse engineering of object code       ***
 ***    is prohibited.                                                        ***
 ***                                                                          ***
 ***                       ENGINEERING ANIMATION, INC.                        ***
 ***                                                                          ***
 ***                           ISU Research Park                              ***
 ***                         2625 North Loop Drive                            ***
 ***                            Ames, IA  50010                               ***
 ***                                                                          ***
 ***                                                                          ***
 *******************************************************************************/

// ExportManager.cpp : Implementation of CExportManager

#include "StdAfx.h"
#include "resource.h"  

#include "ExportManager.h"
#include "ExportProgress.h"
#include "VP3DExporterDefines.h"

#include <visAPI/eaiCADImporter.h>
#include <visAPI/eaiEntityFactory.h>
#include <visAPI/eaiTraverser.h>

#include "Defines.h"
#include "Vis3D.h"
#ifndef _WIN32
#include "VP3DExporterMenu.h"
#include "VP3DExporterStrings.h"
#endif

#include "Jt/JtBrep.h"
#include <Jt/JtModelContext.h>
#include <Jt/JtPartition.h>
#include <JtExtensions/VisPartNode.h>
#include <JtExtensions/VisMetaDataNode.h>
#include <Jt/JtNode.h>
#include <Jt/JtInstance.h>
#include <JtStd/ptrvec.h>

#include <iostream.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define strrchr _tcsrchr
#define access _access
#define chdir _chdir
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define GETINTERFACE_ARG3TYPE(c) (IUnknown **) c
#else
#define GETINTERFACE_ARG3TYPE(c) (void **) c
#endif

// a wrapper class for visAPI's importer class 
CImporter::CImporter()
{
}

CImporter::~CImporter()
{
}

void CImporter::import(JtNode *pNode, eaiHierarchy *&pRootNode)
{
    _buildHierarchy(pNode);
    pRootNode = _root;
} 

#ifndef _WIN32
IMPLEMENT_CLASS_FACTORY(CExportManager)

BEGIN_CATID_MAP(CExportManager)
CATID_MAP_ENTRY(s_CATID_EAISecondaryObject)
END_CATID_MAP()
#endif

IDirectModelView *CExportManager::m_pIDirectModelView = NULL;
IVisibilityManager *CExportManager::m_pIVisibilityManager = NULL;
eaiStack *CExportManager::m_pParentInstanceStack = NULL;
unsigned int CExportManager::m_iCurrentViewMask = 0;
JtPtrVec<IVFExportFilter *> CExportManager::m_ExportFilters;
JtPtrVec<IVF3DExporter *> CExportManager::m_ExportClients;
CExportDlg *CExportManager::m_pExportDialog = NULL;
CExportProgress *CExportManager::m_pProgressDialog = NULL;

// function that finds the first partition with a datastore name, given the root partition 
// of a DirectModel doc
JtPartition *FindDSPartition(JtPartition *pRootPartition)
{
    if (!pRootPartition)
        return(NULL);
    
    // the assumption is that a partition with a datastore name and location exist
    // at most 2 levels below the root partition for a DirectModel doc
    if (pRootPartition && pRootPartition->numChildren())
    {
        // get the child of the root partition
        JtNode *pNode = pRootPartition->child(0);
        
        // if child is not a partition, get the grandchild if the root partition
        if (!pNode->isOfType(JtPartition::classTypeID()) && pNode->numChildren())
            pNode = pNode->child(0);
        
        // if found partition node
        if (pNode && pNode->isOfType(JtPartition::classTypeID()))
        {
            JtString temp = ((JtPartition *) pNode)->datastoreName();
            if (!temp.isEmpty())
                return((JtPartition *) pNode);
        }
    }
    return(NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CExportManager

// -----------------------------------------------------------------------------
// Class        : CExportManager
// Method       : constructor
// Description  : 
// Input        : 1. IUnknown ptr
// Output       : 
// Return       : 
// -----------------------------------------------------------------------------
CExportManager::CExportManager(IUnknown *pUnk):
#ifndef _WIN32
CUnknown(pUnk),
#endif
m_pObjectBroker(NULL)
{
    m_pLicenseCheck = NULL;
    m_pFrameBroker = NULL;
    m_pIDirectModelDoc = NULL;
    m_pCurrentModelContext = NULL;
    m_pRootNode = NULL;
    m_pExportDialog = NULL;
    m_pIVisibilityManager = NULL;
    m_pParentInstanceStack = NULL;
    m_DMViews.setLength(0);
    m_nCurrentClient = -1;
    m_bMemoryOnlyExport = FALSE;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager
// Method       : destructor
// Description  : 
// Input        : 
// Output       : 
// Return       : 
// -----------------------------------------------------------------------------
CExportManager::~CExportManager()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              if (m_pExportDialog)
                  delete m_pExportDialog;
    m_pExportDialog = NULL;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetDirectModelView
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK if success, E_FAIL if failure
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetDirectModelView(DWORD **pIDirectModelView)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              if (!m_pIDirectModelView)
              {
                  *pIDirectModelView = NULL;
                  return(E_FAIL);
              }
    
    *pIDirectModelView = (DWORD *) m_pIDirectModelView;
    m_pIDirectModelView->AddRef();
    
    return(S_OK);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : SilentExport
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK if success, E_FAIL if failure
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::SilentExport(TCHAR *pFileName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              if (!pFileName)
                  return(E_FAIL);
    
    // get the index to the export client corresponding to this 
    // filename
    int curExp = -1;
    TCHAR curExt[20];
    TCHAR *pExt = strrchr(pFileName, '.');
    
    for (int i=0; i<m_ExportClients.length(); i++)
    {
        // get the file extension for current client
        m_ExportClients[i]->GetExtension(curExt);
        if (!_tcscmp(curExt, pExt))
        {
            curExp = i;
            break;
        }
    }
    
    // if the correct export client is not found, return failure
    if (curExp == -1)
        return(E_FAIL);
    
    // reset the export client to its default settings
    m_ExportClients[i]->ResetDefaults();
    
    // do the export
    CString pathName = pFileName;
    if (doExport(pathName, curExp, 1))
        return(S_OK);
    
    return(E_FAIL);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetInProcResourceHandle
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetInProcResourceHandle(DWORD *hInst)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
#ifdef _WIN32
              *hInst = (DWORD) _Module.GetResourceInstance();
#endif
    
    return(S_OK);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetDefaultToolBarData
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetDefaultToolBarData(LPTSTR szTBName, 
                                                   UINT* numDefToolBars, 
                                                   UINT* numItems, UINT* pItems, 
                                                   ULONG* pPageOwner)
{
    strcpy(szTBName, "");
    *numDefToolBars = 0;
    *numItems = 0;
    *pItems = NULL;
    *pPageOwner = 0;
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetMenuID
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetMenuID(int* menuID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              *menuID = IDR_FILE;
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetIconID
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetIconID(int* IconID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              *IconID = 0;
    
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : UpdateUIHandler
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::UpdateUIHandler(int menuid, int* UIFlags)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              if (menuid == IDR_SAVE_AS)
              {
                  // check for license
                  if (!m_pLicenseCheck)
                      m_pLicenseCheck = new LicenseCheck<VISVIEW_3D_EXPORT>(m_pObjectBroker);
                  
                  if (m_pLicenseCheck)
                      if (!SUCCEEDED(m_pLicenseCheck->checkForLicense()))
                          return(S_OK);
                  
                  m_bMemoryOnlyExport = TRUE;
                  
                  // disable Save As menu item if current document is not a jt-document
                  if (m_pCurrentModelContext)
                  {
                      JtPartition *pRootPartition = m_pCurrentModelContext->rootPartition();
                      if (pRootPartition)
                      {
                          // if we can find the named partition, then enable the menu, else disable it
                          if (FindDSPartition(pRootPartition))
                          {
                              m_bMemoryOnlyExport = FALSE;
                              *UIFlags |= UI_ENABLE;
                          }
                          else
                          {
                              // else check if any export clients supports export of in memory only models
                              for (int i=0; i<m_ExportClients.length(); i++)
                              {
                                  BOOL bSupport = FALSE;
                                  HRESULT hr = m_ExportClients[i]->SupportInMemoryOnlyModels(&bSupport);
                                  if (SUCCEEDED(hr) && bSupport)
                                  {
                                      // found export clients that supports it
                                      *UIFlags |= UI_ENABLE;
                                      break;
                                  }
                              }
                          }
                      }
                  }
              }
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetExecutablePath
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetExecutablePath(LPTSTR, ULONG)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : GetContextMenuKey
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::GetContextMenuKey(int* key,int* AddPosition)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              *key = 0;
    *AddPosition = -1;
    
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : BindObjectBroker
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::BindObjectBroker(IVFObjectBroker *pObjectBroker)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              if (m_pObjectBroker)
                  m_pObjectBroker->Release();
    m_pObjectBroker = NULL;
    
    if(pObjectBroker)
    {
        m_pObjectBroker = pObjectBroker;
        m_pObjectBroker->AddRef();
        
        // check for license
        if (m_pLicenseCheck)
            delete m_pLicenseCheck;
        
        m_pLicenseCheck = new LicenseCheck<VISVIEW_3D_EXPORT>(m_pObjectBroker);
    }
    
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : Notify
// Description  : 
// Input        : 
// Output       : 
// Return       : S_OK 
// -----------------------------------------------------------------------------
STDMETHODIMP CExportManager::Notify(const WORD Command, const DWORD MessageLength, 
                                    const BYTE *Message)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              ULONG i=0, num=1;
    HRESULT hr;
    int j=0;
    
    switch(Command)
    {
        //-------------------------------------------------------------------
        // N_START
        //-------------------------------------------------------------------
    case N_START:
        {
            // get the handle
            m_hDeviceManager = *((ULONG*)Message);
            
            // register interested in EAI3DView
            m_pObjectBroker->RegisterInterest(m_hDeviceManager, 1, &s_CATID_EAI3DViewer);
            
            // get the interface of FrameBroker
            m_pObjectBroker->EnumRunningObjectsByClass(s_CATID_UIBroker, &num, &i);
            m_pObjectBroker->GetInterface(i, s_IID_IVFFrameBroker, GETINTERFACE_ARG3TYPE(&m_pFrameBroker));
            m_pObjectBroker->EnumRunningObjectsByClass(s_CLSID_Vis3DModuleManager, &num, &i);
            
            // collect all the exporter clients that has registered interest in the export manager
            if (num) 
                _pollExportClients();
            else 
                return (E_FAIL);
            
            // if we have any available module, then add a menu item called "Save As..."
            if (m_ExportClients.length())
            {
                m_pFrameBroker->AddMenuToMenuPage(i, m_hDeviceManager, IDR_FILE);
                m_h3DModuleManager = i;  // Cache the handle for later use
            }
            else
                return E_FAIL;
            
            // collect all export filters
            _pollExportFilters();
        }
        
        break;
        
        //-------------------------------------------------------------------
        // N_OBJ_CREATE
        //-------------------------------------------------------------------
    case N_OBJ_CREATE:
        {
            m_pThreeDViewerGlobalHandle = *((ULONG*)Message);
        }
        break;
        //-------------------------------------------------------------------
        // N_OBJ_DESTROY
        //-------------------------------------------------------------------
    case N_OBJ_DESTROY:
        {
            ULONG refCount = 0; 
            
            // get DirectModel view being destroyed 
            ULONG vis3DViewHandle = *((ULONG*)Message);
            IDirectModelView *pCurView = NULL;
            m_pObjectBroker->GetInterface( vis3DViewHandle, s_IID_IDirectModelView, 
                                           GETINTERFACE_ARG3TYPE(&pCurView));
            
            if (pCurView)
            {
                // cycle through list of views and delete the current view from the list
                for (int i=0; i<m_DMViews.length(); i++)
                {
                    if (m_DMViews[i] == pCurView)
                    {
                        refCount = m_DMViews[i]->Release();
                        m_DMViews.remove(i, 1);
                        break;
                    }
                }
                
                refCount = pCurView->Release();
            }
            
            // if no more views exist
            if (!m_DMViews.length())
            {
                m_pIDirectModelView = NULL;
                
                if (m_pIDirectModelDoc)
                    m_pIDirectModelDoc->Release();
                m_pIDirectModelDoc = NULL;
                
                if (m_pCurrentModelContext)
                    m_pCurrentModelContext->unref();
                m_pCurrentModelContext = NULL;
            }
        }
        break;	
        //-------------------------------------------------------------------
        // N_OBJ_GOTFOCUS
        //-------------------------------------------------------------------
    case N_OBJ_GOTFOCUS:
        {
            ULONG vis3DViewHandle = *((ULONG*)Message);
            ULONG refCount = 0;
            if (m_pObjectBroker)
            {
                // get a new IDirectModelView pointer
                IDirectModelView *pCurView = NULL;
                m_pObjectBroker->GetInterface( vis3DViewHandle, s_IID_IDirectModelView, 
                                               GETINTERFACE_ARG3TYPE(&pCurView));
                
                if (pCurView)
                {
                    // cycle through list of opened DirectModel documents to see if current
                    // document has been opened
                    BOOL bOpenDoc = FALSE;
                    for (j=0; j<m_DMViews.length(); j++)
                    {
                        if (m_DMViews[j] == pCurView)
                        {
                            bOpenDoc = TRUE;
                            break;
                        }
                    }
                    
                    // update active DirectModelView
                    m_pIDirectModelView = pCurView;
                    
                    // update visibility manager
                    if (m_pIVisibilityManager)
                        m_pIVisibilityManager->Release();
                    
                    IManage3DObjects *pIManage3DObjects = NULL;
                    hr = m_pIDirectModelView->QueryInterface(s_IID_IManage3DObjects, (void **) &pIManage3DObjects);
                    if (SUCCEEDED(hr) && pIManage3DObjects)
                    {
                        pIManage3DObjects->GetVisibilityManager(&m_pIVisibilityManager);
                        pIManage3DObjects->Release();
                    }
                    
                    // if document not already opened, add doc to list
                    if (!bOpenDoc)
                        m_DMViews.append(pCurView);
                    else
                    {
                        // else release the pointer so we always ref a DMView exactly once
                        pCurView->Release();
                    }
                    
                    // update document info
                    if (m_pCurrentModelContext)
                        refCount = m_pCurrentModelContext->unref();
                    m_pCurrentModelContext = NULL;
                    
                    if (m_pIDirectModelDoc)
                        refCount = m_pIDirectModelDoc->Release();
                    m_pIDirectModelDoc = NULL;
                    
                    m_pIDirectModelView->GetDocument(&m_pIDirectModelDoc);
                    if (m_pIDirectModelDoc)
                    {
                        refCount = m_pIDirectModelDoc->AddRef();
                        m_pIDirectModelDoc->GetData((DWORD *)&m_pCurrentModelContext);
                        if (m_pCurrentModelContext)
                            m_pCurrentModelContext->ref();
                    }
                    m_pIDirectModelView->GetViewNum(&m_iCurrentViewMask);
                    
                    // bind DirectModel view to export filters
                    for (j=0; j<m_ExportFilters.length(); j++)
                        m_ExportFilters[j]->BindDirectModelView((DWORD *) m_pIDirectModelView);
                }
            }
        }
        break;
        //-------------------------------------------------------------------
        // N_OBJ_LOSTFOCUS
        //-------------------------------------------------------------------
	/*	case N_OBJ_LOSTFOCUS:
           {
           ULONG refCount = 0; 
           if ( m_pIDirectModelView )
           refCount = m_pIDirectModelView->Release();
           m_pIDirectModelView = NULL;
           
           for (j=0; j<m_ExportFilters.length(); j++)
           m_ExportFilters[j]->bindDirectModelView(NULL);
           
           if (m_pIDirectModelDoc)
           refCount = m_pIDirectModelDoc->Release();
           m_pIDirectModelDoc = NULL;
           
           if (m_pCurrentModelContext)
           m_pCurrentModelContext->unref();
           m_pCurrentModelContext = NULL;
           }			
           break;*/
        
        //-------------------------------------------------------------------
        // N_STOP
        //-------------------------------------------------------------------
    case N_STOP:
        {
            // return the license
            if (m_pLicenseCheck)
            {
                m_pLicenseCheck->returnLicense();
                delete m_pLicenseCheck;
                m_pLicenseCheck = NULL;
            }
            
            if (m_pObjectBroker)
            {	
                m_pObjectBroker->UnregisterInterest( m_hDeviceManager, 1, &s_CATID_EAI3DViewer );
                m_pObjectBroker->Release();
                m_pObjectBroker = NULL;
            }
            
            if (m_pFrameBroker)
                m_pFrameBroker->Release();
            m_pFrameBroker = NULL;
            
            // release all export filters
            for (j=0; j<m_ExportFilters.length(); j++)
            {
                m_ExportFilters[j]->TerminateFilter();
                m_ExportFilters[j]->Release();
            }
            
            // release all export clients
            for (j=0; j<m_ExportClients.length(); j++)
            {
                m_ExportClients[j]->TerminateClient();
                m_ExportClients[j]->Release();
            }
            
            // release current DirectModel doc
            if ( m_pIDirectModelDoc) 
                m_pIDirectModelDoc->Release();
            m_pIDirectModelDoc = NULL; 
            
            // release current model context
            if (m_pCurrentModelContext)
                m_pCurrentModelContext->unref();
            m_pCurrentModelContext = NULL;
            
            // release visibility manager
            if (m_pIVisibilityManager)
                m_pIVisibilityManager->Release();
            m_pIVisibilityManager = NULL;
            
            // release all DirectModelViews
            for (int i=0; i<m_DMViews.length(); i++)
                m_DMViews[i]->Release();
        }
        break;
        
        //-------------------------------------------------------------------
        // N_MENUCOMMAND
        //-------------------------------------------------------------------
    case N_MENUCOMMAND:
        handleMenuCommand( *((UINT *)Message) );
        break;
        
    default:
        break;
    }
    
    return S_OK;
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : preNodeFilterHook
// Description  : 
// Input        : 
// Output       : 
// Return       : return value
// -----------------------------------------------------------------------------
int CExportManager::preNodeFilterHook(JtNode *&pNode)
{
    // return codes -- 0 - abort traversal, 1 - skip this node, 2 - continue as usual
    
    JtNode *pOrigNode = pNode;
    pNode->ref();
    
    // hard coded filter that skips all part nodes that are not selected
    //if (pNode->isOfType(JtInstance::classTypeID()) ||
    //	pNode->isOfType(JtPartition::classTypeID()) ||
    //		pNode->isOfType(VisMetaDataNode::classTypeID()))
    //{
    if (!_isVisibleNode(pNode))
        return(0);
    //}
    
    // pass the node through the active filters
    int retValue = 2;
    for (int i=0; i<m_ExportFilters.length() && retValue == 2; i++)
    {
        HRESULT hr;
        if (m_pExportDialog->isChecked(i))
        {
            JtNode *pNewNode = NULL;
            hr = m_ExportFilters[i]->Filter((DWORD *) pNode, (DWORD *) m_pProgressDialog, (DWORD **) &pNewNode, &retValue);
            if (!SUCCEEDED(hr))
                break;
            
            if (pNewNode)
            {
                pNode->unref();
                pNode = pNewNode;
            }
            
            if (retValue != 2)
                break;
        } 
    }
    
    if (pNode == pOrigNode)
        pNode->unref();
    
    return(retValue);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : postNodeFilterHook
// Description  : 
// Input        : 
// Output       : 
// Return       : 
// -----------------------------------------------------------------------------
void CExportManager::postNodeFilterHook(JtNode *pNode, eaiHierarchy *pParent, 
                                        eaiEntity *&pEntity)
{
    //if (pNode->isOfType(JtPartition::classTypeID()) || 
    //		pNode->isOfType(JtInstance::classTypeID()) ||
    //			pNode->isOfType(VisMetaDataNode::classTypeID()))
    m_pParentInstanceStack->pop();
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : doExport
// Description  : 
// Input        : 
// Output       : 
// Return       : export status
// -----------------------------------------------------------------------------
DWORD CExportManager::doExport(CString fileName, int curExp, int remDegenAssem)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              DWORD success = 0; 
    HRESULT hr;
    
    // get root partition
    JtPartition *pRootPartition = m_pCurrentModelContext->rootPartition();
    
    // get the export interface implemented by the current exporter
    IVF3DJtExporter *pJtExporter = NULL;
    IVF3DVisAPIExporter *pVisAPIExporter = NULL;
    hr = m_ExportClients[curExp]->QueryInterface(s_IID_IVF3DJtExporter, (void **) &pJtExporter);
    if (FAILED(hr) || !pJtExporter)
        hr = m_ExportClients[curExp]->QueryInterface(s_IID_IVF3DVisAPIExporter, (void **) &pVisAPIExporter);
    
    if (!pJtExporter && !pVisAPIExporter)
        return(0);
    
    // init progress dialog
    hr = CoCreateInstance(s_CLSID_ExportProgress, NULL, CLSCTX_INPROC_SERVER, s_IID_IExportProgress, (void **)&m_pProgressDialog);
    if (FAILED(hr))
        return(0);
    
    // if Jt export interface is implemented
    if (pJtExporter)
    {
        hr = pJtExporter->Export((DWORD *) m_pCurrentModelContext, (int) m_iCurrentViewMask, (TCHAR *) LPCTSTR(fileName), 
                                 (DWORD *) m_pProgressDialog, &success);
        
        pJtExporter->Release();
    }
    // else visAPI export interface is implemented
    else
    {
        // import the Jt scene to visAPI
        eaiHierarchy *pNode = NULL;
        
        //m_pProgressDialog->setParent(m_pExportDialog);
        //m_pProgressDialog->SetMessage(_T(" "));
        
        CImporter *pImporter = new CImporter;	
        
        // ask export client to setup importer options
        eaiCADImporter::BrepLoadOption brepOption = eaiCADImporter::eaiBREP_ONLY;
        eaiCADImporter::ShapeLoadOption lodOption = eaiCADImporter::eaiALL_LODS;
        
        m_ExportClients[curExp]->SetupImporter((int *) &brepOption, (int *) &lodOption);
        pImporter->setBrepLoadOption(brepOption);
        pImporter->setShapeLoadOption(lodOption);
        
        // set remove degenerate assemblies option
        pImporter->setDeleteDegenAssem(remDegenAssem); 
        
        // setup filter hooks
        pImporter->setPreActionCB(preNodeFilterHook);
        pImporter->setPostActionCB(postNodeFilterHook);
        
        // setup assembly instancing option
        pImporter->setAssemblyOption(eaiCADImporter::eaiEXPLODE_ASSEMBLY);
        
        // create a holder assembly for docs with inserted assembly/parts
        if (pRootPartition->numChildren() > 1)
        {
            m_pRootNode = eaiEntityFactory::createAssembly("root"); 
            m_pRootNode->ref();
        }
        
        BOOL cont = FALSE; 
        for (int i=0; i<pRootPartition->numChildren(); i++)
        {
            // init the assembly stack
            m_pParentInstanceStack = eaiEntityFactory::createStack();
            m_pParentInstanceStack->ref();
            
            // add node instance of root partition into stacl
            JtNodeInstance *pRootInstance = NULL;
            int nNumInstances = 0;
            pRootPartition->instances((const JtNodeInstance *&) pRootInstance, nNumInstances);
            m_pParentInstanceStack->push(pRootInstance[0]);
            
            // import the scene 
            pImporter->import(pRootPartition->child(i), pNode);
            
            m_pParentInstanceStack->unref();
            m_pParentInstanceStack = NULL;
            
            if (pNode)   
            { 
                if (m_pRootNode) 
                    m_pRootNode->addChild(pNode);   
                else
                    m_pRootNode = (eaiAssembly *) pNode;
                
                cont = TRUE;  
            }
        }
        
        // export the visAPI scene
        if (cont)
        {	
            // export
            hr = pVisAPIExporter->Export((DWORD *) m_pRootNode, (TCHAR *) LPCTSTR(fileName), 
                                         (DWORD *) m_pProgressDialog, &success); 	
        }
        
        if (m_pRootNode) 
        {
            m_pRootNode->unref();
            m_pRootNode = NULL;
        }  
	
        if (pImporter)
            delete pImporter;
        pImporter = NULL;
        
        pVisAPIExporter->Release();
    }
    
    //if (m_pProgressDialog)
    //  ((IExportProgress *) m_pProgressDialog)->Release(); 
    m_pProgressDialog = NULL;
    
    return(success);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : handleMenuCommand
// Description  : 
// Input        : 
// Output       : 
// Return       : TRUE / FALSE 
// -----------------------------------------------------------------------------
BOOL CExportManager::handleMenuCommand(UINT uMsg)
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
              
              _initExportDialog();
    
    // get the datastore location for the currently opened document
    CString sOrigFilename =_T("");
    JtString temp;
    JtChar buf[2048];
    JtPartition *pRootPartition = m_pCurrentModelContext->rootPartition();
    pRootPartition = FindDSPartition(pRootPartition);
    if (pRootPartition)
    {
        pRootPartition->getDatastoreLocation(temp);
        if (!temp.isEmpty())
        {
            temp.stringToAscii(buf, 2048);
            for (int i=0; i<strlen(buf); i++)
                if (buf[i] == '\\')
                    buf[i] = '/';
            sOrigFilename = buf;
        }
        temp = pRootPartition->datastoreName();
        if (!temp.isEmpty())
        {
            temp.stringToAscii(buf, 2048);
            for (int i=0; i<strlen(buf); i++)
                if (buf[i] == '\\')
                    buf[i] = '/';
            sOrigFilename += buf;
        }
    }
    
    // show dialog
    int nRet = m_pExportDialog->DoModal();
    
    if (nRet == IDOK)
    {	
        // get the export filename
        CString sFilename = m_pExportDialog->getPathname();
        sprintf(buf, LPCTSTR(sFilename));
        for (int i=0; i<strlen(buf); i++)
            if (buf[i] == '\\')
                buf[i] = '/';
        sFilename = buf;
        
        // get current export client
        int nCurrentClient = m_pExportDialog->curExportClient();
        for (int j=0; j<m_ExportClients.length(); j++)
        {
            if (m_ExportClients[j] == m_ActiveExportClients[nCurrentClient])
            {
                m_nCurrentClient = j;
                break;
            }
        }
        
        // get the remove degenerate assembly option
        int remDegenAssem = m_pExportDialog->delDegenAssem();
        
        DWORD success = doExport(sFilename, m_nCurrentClient, remDegenAssem);
	
        if (success == EXPORT_SUCCESS)
        {
            // if original pathname is the same as the saved pathname, then display
            // different confirmation message      
            if (sOrigFilename == sFilename)
            {
#ifdef _WIN32
                AfxMessageBox(IDS_EXP_OK2, MB_ICONINFORMATION);
#else
                AfxMessageBox(IDS_EXP_OK2);
#endif
            }
            else
            {
#ifdef _WIN32
                AfxMessageBox(IDS_EXP_OK, MB_ICONINFORMATION);
#else
                AfxMessageBox(IDS_EXP_OK);
#endif
            }
        }
        else if (success == EXPORT_ABORT)
        {
#ifdef _WIN32
            AfxMessageBox(IDS_EXP_ABORT, MB_ICONINFORMATION);
#else
            AfxMessageBox(IDS_EXP_ABORT);
#endif
        }
        else
            AfxMessageBox(IDS_EXP_FAIL);  
    }
    
    delete m_pExportDialog;
    m_pExportDialog = NULL;
    
    return(TRUE);
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : _isVisibleNode
// Description  : 
// Input        : 
// Output       : 
// Return       : current Jt node
// -----------------------------------------------------------------------------
int CExportManager::_isVisibleNode(JtNode *pNode)
{
    JtNodeInstance *pNodeInstances = NULL;
    int nNumInstances = 0, nNumChildren = 0;
    BOOL bVisible = TRUE;
    
    // if node is a partition node
    int i = 0;
    pNode->instances((const JtNodeInstance *&) pNodeInstances, nNumInstances);
    if (nNumInstances == 1) 
    {
        // if node is an instance or partition, get the visibility of the node
        //if (pNode->isOfType(JtInstance::classTypeID()) || pNode->isOfType(JtPartition::classTypeID()) ||
        //	pNode->isOfType(VisMetaDataNode::classTypeID()))
        m_pIVisibilityManager->GetVisibility((DWORD) pNodeInstances[0], &bVisible);
    }
    else 
    {
        // get the parent instance of the node 
        DWORD parentInstance = (DWORD) m_pParentInstanceStack->top();
        for (i=0; i<nNumInstances; i++)
        {
            if (parentInstance == (DWORD) JtNode::parentInstance(pNodeInstances[i]))
            {
                // if node is an instance or partition, get the visibility of the node
                //if (pNode->isOfType(JtInstance::classTypeID()) || pNode->isOfType(JtPartition::classTypeID()) ||
                //	pNode->isOfType(VisMetaDataNode::classTypeID()))
                m_pIVisibilityManager->GetVisibility((DWORD) pNodeInstances[i], &bVisible);
                
                break; 
            }
        } 
    } 
    
    m_pParentInstanceStack->push(pNodeInstances[i]);
    
    return(bVisible); 
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : _pollExportClients
// Description  : 
// Input        : 
// Output       : 
// Return       : number of export clients
// -----------------------------------------------------------------------------
int CExportManager::_pollExportClients()
{
    GUID GUIDs[20];
    ULONG num = 20;
    
    // get CLSIDs for all objects who implements the exporter category
    HRESULT hr = m_pObjectBroker->EnumClassesByCategories(1, &s_CATID_EAI3DExporter, &num, GUIDs);
    
    if (!SUCCEEDED(hr))
        return(0);
    
    for (ULONG i=0; i<num; i++)
    {
        HRESULT hr;
        IVF3DExporter *pExportClient = NULL;
        hr = CoCreateInstance(GUIDs[i], NULL, CLSCTX_INPROC_SERVER, s_IID_IVF3DExporter, (void **)&pExportClient);
        
        // call the client to initialize importer options
        //pExportClient->init();
	
        if (SUCCEEDED(hr))
        {
            pExportClient->InitClient();
            IExportManager *pExportManager = (IExportManager *) this;
            pExportClient->BindExportManager((DWORD *) pExportManager);
            m_ExportClients.append((IVF3DExporter *) pExportClient);
        }
    }	
    return(m_ExportClients.length());
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : _pollExportFilters
// Description  : 
// Input        : 
// Output       : 
// Return       : number of export filters
// -----------------------------------------------------------------------------
int CExportManager::_pollExportFilters()
{ 
    GUID GUIDs[20];
    ULONG num = 20;
    
    // get CLSIDs for all objects who implements the export filter category
    HRESULT hr = m_pObjectBroker->EnumClassesByCategories(1, &s_CATID_EAIExportFilter, &num, GUIDs);
    
    if (!SUCCEEDED(hr))
        return(0);
    
    for (ULONG i=0; i<num; i++)
    {
        HRESULT hr;
        IVFExportFilter *pExportFilter = NULL;
        hr = CoCreateInstance(GUIDs[i], NULL, CLSCTX_INPROC_SERVER, s_IID_IVFExportFilter, (void **)&pExportFilter);
        
        if (SUCCEEDED(hr))
        {
            pExportFilter->InitFilter();
            pExportFilter->BindDirectModelView((DWORD *) m_pIDirectModelView);
            m_ExportFilters.append((IVFExportFilter *) pExportFilter);
        }
    }	
    return(m_ExportFilters.length());
}

// -----------------------------------------------------------------------------
// Class        : CExportManager 
// Method       : _initExportDialog
// Description  : 
// Input        : 
// Output       : 
// Return       : 
// -----------------------------------------------------------------------------
void CExportManager::_initExportDialog() 
{
    if (m_pExportDialog) 
        delete m_pExportDialog; 
    m_pExportDialog = NULL;
    
    // get the jt filename for active DirectModel doc
    JtString temp;
    JtChar buf[2048];
    
    // if currently active export client is not set, the set it to 
    // the Jt export client if it is available
    if (m_nCurrentClient == -1)
    {
        for (int i=0; i<m_ExportClients.length(); i++)
        {
            TCHAR ext[20];
            m_ExportClients[i]->GetExtension(ext);
            if (!strcmp(ext, ".jt"))
            {
                m_nCurrentClient = i;
                break;
            }
        }
        
        // Jt export client not found, set default client to the first on in the list
        if (m_nCurrentClient == -1)
            m_nCurrentClient = 0;
    }
    
    // get the root partition for the DM doc
    CString sFilename = _T("");
    JtPartition *pRootPartition = m_pCurrentModelContext->rootPartition();
    if (pRootPartition)
    {
        // get the partition node with the datastore name
        JtPartition *pNamedPartition = FindDSPartition(pRootPartition);
        
        // if partition node with datastore name is found
        if (pNamedPartition)
        { 
            pNamedPartition->getDatastoreLocation(temp);
            if (!temp.isEmpty())
            {
                temp.stringToAscii(buf, 2048);
                sFilename = buf;
            }
            temp = pNamedPartition->datastoreName();
            if (!temp.isEmpty())
            {
                temp.stringToAscii(buf, 2048);
                sFilename += buf;
            }
        }
        else
            sFilename = _T("model.jt");
        
        // generate a list of active export clients
        m_ActiveExportClients.setLength(0);
        int nCurrentClient = 0;
        
        // if current export in memory only model, then only select export clients
        // that support export of in memory only models
        if (m_bMemoryOnlyExport)
        {
            for (int i=0; i<m_ExportClients.length(); i++)
            {
                BOOL bSupported = FALSE;
                HRESULT hr = m_ExportClients[i]->SupportInMemoryOnlyModels(&bSupported);
                if (SUCCEEDED(hr) && bSupported)
                {
                    m_ActiveExportClients.append(m_ExportClients[i]);
                    
                    if (i == m_nCurrentClient)
                        nCurrentClient = m_ActiveExportClients.length() - 1;
                }
            }
        }
        // else if we found a datastore name for the model
        else if (pNamedPartition)
        {
            m_ActiveExportClients = m_ExportClients;
            nCurrentClient = m_nCurrentClient;
        }
        
        if (m_ActiveExportClients.length())
            m_pExportDialog = new CExportDlg(sFilename, m_ActiveExportClients, m_ExportFilters, 
                                             nCurrentClient);
    }
}

