// SFVContextMenu.cpp : Implementation of CSFVContextMenu

#include "stdafx.h"
#include "SFVContextMenu.h"
#include "SFVChecker.h"
#include "SFVCreator.h"

using namespace std;

// CSFVContextMenu

// Initialize
HRESULT CSFVContextMenu::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    FORMATETC   fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM   stg = { TYMED_HGLOBAL };
    HDROP       hDrop;
    
    // Look for CF_HDROP data in the data object
    if (FAILED(pDataObj->GetData(&fmt, &stg))) {
        // Invalid argument
        return(E_INVALIDARG);
    }

    // Get a pointer to actual data
    hDrop = reinterpret_cast<HDROP>(GlobalLock(stg.hGlobal));
    
    if (hDrop == NULL) {
        // Release medium
        ReleaseStgMedium(&stg);

        // Invalid argument
        return(E_INVALIDARG);
    }

    // Sanity check - make sure there is at least one filename
    UINT uNumFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    if (uNumFiles == 0) {
        // Release medium
        GlobalUnlock(stg.hGlobal);
        ReleaseStgMedium(&stg);

        // Invalid argument
        return(E_INVALIDARG);
    }

    // Return result
    HRESULT hr = S_OK;

    // Empty filename list
    m_lstFileNames.clear();

    // Add file names to list (not directories)
    for (UINT i = 0; i < uNumFiles; i++) {
        TCHAR szFileName[MAX_PATH];
        if (DragQueryFile(hDrop, i, szFileName, MAX_PATH)) {
            DWORD dwAttr = GetFileAttributes(szFileName);
            if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
                m_lstFileNames.push_back(CString(szFileName));
            }
        }
    }

    // Should we show the 'Check SFV' option?
    m_bIsSFV = false;

    // Check if it's an SFV
    if (m_lstFileNames.size() == 1) {
        // One file was selected, check file's extension
        const CString & strFileName = m_lstFileNames.front();
        if (strFileName.GetLength() > 4) {
            // Get extension
            CString strExtension = strFileName.Right(4);
            strExtension = strExtension.MakeUpper();

            // Is it .SFV ?
            if (strExtension.Compare(_T(".SFV")) == 0) {
                m_bIsSFV = true;
            }
        }
    }

    // Release medium
    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);

    return(hr);
}

// Add our entries to the context menu
HRESULT CSFVContextMenu::QueryContextMenu(HMENU hMenu, UINT uMenuIndex, UINT uidFirstCmd, UINT uidLastCmd, UINT uFlags)
{
    // If the flags include CMF_DEFAULTONLY we shouldn't do anything
    if (uFlags & CMF_DEFAULTONLY) {
        return(MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0));
    }

    // Insert our menu item
    if (m_bIsSFV == true) {
        InsertMenu(hMenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, _T("Check SFV..."));
    } else {
        InsertMenu(hMenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, _T("Create SFV..."));
    }

    // Return number of items added
    return(MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1));
}

HRESULT CSFVContextMenu::GetCommandString(UINT uidCmd, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    USES_CONVERSION;
    
    // We only added one command, therefore uidCmd must be zero
    if (uidCmd != 0)
        return(E_INVALIDARG);

    // We only want to supply a help text
    if (!(uFlags & GCS_HELPTEXT)) 
        return(E_INVALIDARG);

    TCHAR szHelp[256];

    // Determine which string to use
    if (m_bIsSFV == true) {
        lstrcpy(szHelp, _T("Check file(s) against SFV hashes"));
    } else {
        lstrcpy(szHelp, _T("Create SFV hashes from file(s)"));
    }

    if (uFlags & GCS_UNICODE) {
        // Supply a unicode string
        lstrcpynW((LPWSTR )pszName, T2CW(szHelp), cchMax);
    } else {
        // Supply an ANSI string
        lstrcpynA(pszName, T2CA(szHelp), cchMax);
    }

    return(S_OK);
}

HRESULT CSFVContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
    // lpVerb should not be a pointer
    if (HIWORD(pCmdInfo->lpVerb) != 0)
        return(E_INVALIDARG);

    switch (LOWORD(pCmdInfo->lpVerb)) {
        case 0:
            {
                // Execute command
                if (m_bIsSFV == true) {
                    // Check SFV
                    CSFVChecker checker(m_lstFileNames.front());
                    checker.Check();
                } else {
                    // Create SFV
                    CSFVCreator creator(m_lstFileNames);
                    if (creator.InputSFVName() == true) {
                        creator.Create();
                    }
                }
            }
            break;
        default:
            return(E_INVALIDARG);
    }

    return(S_OK);
}