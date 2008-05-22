#include "StdAfx.h"
#include "sfvchecker.h"
#include "Util.h"
#include "Resource.h"
#include "CRC.h"
#include "CommonDialog.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)

using namespace std;

#define WM_THREADFINISHED (WM_USER + 1)

static const int cColFileName = 0;
static const int cColExpectedCRC = 1;
static const int cColFileCRC = 2;

// Constructor
CSFVChecker::CSFVChecker(LPCTSTR pszFileName)
: m_psm(0)
{
    m_pszFileName = new TCHAR [lstrlen(pszFileName) + 1];
    lstrcpy(m_pszFileName, pszFileName);
}

// Destructor
CSFVChecker::~CSFVChecker(void)
{
    delete [] m_pszFileName;
}

// Check SFV file
void CSFVChecker::Check(void)
{
    bool bCheck = true;

    // Parse the SFV
    if (ParseSFV() == false) {
        MessageBox(NULL, _T("Error parsing the SFV"), _T("SFVTool2"), MB_OK | MB_ICONERROR);
    } else if (m_lstFiles.empty() == true) {
        MessageBox(NULL, _T("No files in the SFV"), _T("SFVTool2"), MB_OK | MB_ICONERROR);
    } else {
        DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SFVCHECK), NULL, CheckDlgHelper, (LPARAM )this);
    }

    // Deallocate file list
    FileList::iterator i;
    for (i = m_lstFiles.begin(); i != m_lstFiles.end(); i++) {
        FILE_ENTRY *pEntry = (*i);
        delete pEntry;
    }

    m_lstFiles.clear();
}

// Parse SFV file and extract filenames/crcs from it
bool CSFVChecker::ParseSFV(void)
{
    USES_CONVERSION;

    // Create input object
    ifstream input(T2CA(m_pszFileName));
    if (input.good() == false) return(false);

    // Line loop
    string stringLine;
    while (input.good() == true) {
        getline(input, stringLine);
        CString strLine = stringLine.c_str();

        // Remove whitespace
        strLine.Trim(_T(" \r\n"));

        // Remove comment from line
        int nPos = strLine.Find(_T(';'));
        if (nPos != -1) {
            strLine = strLine.Left(nPos);
        }

        // Ignore blank lines
        if (strLine.GetLength() > 0) {
            // Must be in format XYZ
            // Where X is file name, Y is space(s) and Z is crc in hex

            // Look for the space(s) separator
            nPos = strLine.GetLength() - 1;
            while (nPos && strLine[nPos] != ' ') nPos--;

            if (nPos != 0) {
                // Allocate new file entry
                FILE_ENTRY *pEntry = new FILE_ENTRY;

                // Get file name
                pEntry->strFileName = strLine.Left(nPos);
                pEntry->strFileName = pEntry->strFileName.Trim(_T(' '));

                // Rest of the line
                strLine = strLine.Right(strLine.GetLength() - nPos);

                // Get CRC
                CString strCRC = strLine.Trim(_T(' '));
                
                // Check that the CRC really is a hex string
                if (Util::IsHexString(strCRC) == false) {
                    // Bad CRC string
                    return(false);
                }

                // Convert CRC string to CRC value
                pEntry->dwExpectedCRC = _tcstoul(strCRC, NULL, 16);

                // Add entry to file list
                pEntry->bCheck = true;
                pEntry->bError = false;
                pEntry->fs = FS_UNCHECKED;
                pEntry->dwFileCRC = 0;
                m_lstFiles.push_back(pEntry);

            } else {
                // Badly formatted line
                return(false);
            }
        }
    }

    return(true);
}

// Check dialog helper procedure
BOOL CALLBACK CSFVChecker::CheckDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
        SetWindowLong(hDlg, GWL_USERDATA, (LONG )lParam);

    CSFVChecker *pThis = reinterpret_cast<CSFVChecker *>(GetWindowLong(hDlg, GWL_USERDATA));
    return pThis->CheckDlg(hDlg, uMsg, wParam, lParam);
}

// Check dialog procedure
BOOL CALLBACK CSFVChecker::CheckDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            // Dialog initialization
            {
            m_hwnd = hDlg;
            m_bIsClosing = false;

            // Set icon
            m_hIcon = static_cast<HICON>(LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR));
            SetClassLong(hDlg, GCL_HICON, reinterpret_cast<LONG>(m_hIcon));
            SetClassLong(hDlg, GCL_HICONSM, reinterpret_cast<LONG>(m_hIcon));

            // Set style
            HWND hwndList = GetDlgItem(hDlg, IDC_FILES);
            ListView_SetExtendedListViewStyle(hwndList, ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

            // Initialize size manager
            m_psm = new CommonDialog::CSizeManager(hDlg);

            // Set new window position
            WINDOWPLACEMENT wp;
            if (Util::LoadWindowPlacement(_T("SFVChecker"), wp) == true) {
                SetWindowPlacement(hDlg, &wp);
                m_psm->UpdatePositions();
            }

            // Get column width for CRCs
            m_nColWidth = ListView_GetStringWidth(hwndList, _T("DEADBEEFCAFEBABE"));
            
            // Get first column's width
            RECT rectList;
            GetClientRect(hwndList, &rectList);
            int nFirstColWidth = rectList.right - rectList.left - m_nColWidth * 2  - GetSystemMetrics(SM_CXHTHUMB);;

            // Add columns 
            CommonDialog::AddColumn(hwndList, cColFileName, _T("FileName"), nFirstColWidth);
            CommonDialog::AddColumn(hwndList, cColExpectedCRC, _T("Expected CRC"), m_nColWidth);
            CommonDialog::AddColumn(hwndList, cColFileCRC, _T("File CRC"), m_nColWidth);

            // Set entries
            SetListEntries(hwndList, m_lstFiles);
            SelectAll(hwndList);
            
            // Start checking files
            UpdateState(hDlg, true);
            _beginthread(CheckThread, 0, this);
            }
            return(TRUE);
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case IDOK:
                        // OK
                        PostMessage(hDlg, WM_CLOSE, 0, 0);
                        break;
                    case IDC_STOP:
                        // Stop checking
                        m_bStop = true;
                        break;
                    case IDC_RECHECK:
                        // Recheck files
                        UpdateState(hDlg, true);
                        _beginthread(CheckThread, 0, this);
                        break;
                    case IDC_SELECTALL:
                        // Select all items
                        SelectAll(GetDlgItem(hDlg, IDC_FILES));
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_THREADFINISHED:
            // Thread has finished
            UpdateState(hDlg, false);
            if (m_bIsClosing == true)
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            break;
        case WM_CLOSE:
            // Close dialog
            if (m_bStop == false) {
                m_bIsClosing = true;
                m_bStop = true;
            } else {
                EndDialog(hDlg, IDOK);
            }
            break;
        case WM_NOTIFY:
            // Dialog notification
            {
            LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lParam);
            if (pHeader->idFrom == IDC_FILES) {
                
                if (pHeader->code == NM_CUSTOMDRAW) {
                    // Custom draw

                    LPNMLVCUSTOMDRAW pInfo = reinterpret_cast<LPNMLVCUSTOMDRAW>(pHeader);
                    
                    if (pInfo->nmcd.dwDrawStage == CDDS_PREPAINT) {

                        // Color prepaint, tell windows we want a notification for each item
                        SetWindowLong(hDlg, DWL_MSGRESULT, (LONG )CDRF_NOTIFYITEMDRAW);
                        return TRUE;

                    } else if (pInfo->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                        
                        // Pre-paint of an item
                        const FILE_ENTRY *pEntry = m_lstFiles[pInfo->nmcd.dwItemSpec];
                        if (pEntry->bError == true) {
                            // Colour it red
                            pInfo->clrTextBk = RGB(255, 0, 0);
                            pInfo->clrText = RGB(0, 0, 0);
                        }

                    }
                    
                    SetWindowLong(hDlg, DWL_MSGRESULT, (LONG )CDRF_DODEFAULT);
                    return TRUE;

                } else if (pHeader->code == LVN_ITEMCHANGED) {
                    
                    // Item changed
                    LPNMLISTVIEW pInfo = reinterpret_cast<LPNMLISTVIEW>(pHeader);

                    FILE_ENTRY *pEntry = m_lstFiles[pInfo->iItem];

                    if (pEntry) {
                        
                        // Set check according to listview check
                        BOOL bCheck = ListView_GetCheckState(pInfo->hdr.hwndFrom, pInfo->iItem);
    
                        if (bCheck == TRUE) {
                            pEntry->bCheck = true;
                        } else {
                            pEntry->bCheck = false;
                        }

                    }

                    return TRUE;
                }

            }
            }
            break;
        case WM_GETMINMAXINFO:
            // Get minimum/maximum window sizes information
            {
            LPMINMAXINFO pmmi = reinterpret_cast<LPMINMAXINFO>(lParam);
            pmmi->ptMinTrackSize.x = CommonDialog::cMinWidth;
            pmmi->ptMinTrackSize.y = CommonDialog::cMinHeight;
            }
            break;
        case WM_SIZE:
            // Size dialog
            if (m_psm) {
                m_psm->UpdatePositions();

                // Get first column's width
                HWND hwndList = GetDlgItem(hDlg, IDC_FILES);
                RECT rectList;
                GetClientRect(hwndList, &rectList);
                int nFirstColWidth = rectList.right - rectList.left - m_nColWidth * 2 - GetSystemMetrics(SM_CXHTHUMB);
                if (ListView_GetCountPerPage(hwndList) < ListView_GetItemCount(hwndList))
                    nFirstColWidth += GetSystemMetrics(SM_CXHTHUMB);

                // Update columns
                CommonDialog::UpdateColumn(hwndList, cColFileName, nFirstColWidth);
                CommonDialog::UpdateColumn(hwndList, cColExpectedCRC, m_nColWidth);
                CommonDialog::UpdateColumn(hwndList, cColFileCRC, m_nColWidth);

                // Invalidate window
                InvalidateRect(hDlg, NULL, TRUE);
            }
            break;
        case WM_DESTROY:
            // Destroy dialog
            // Save window placement
            {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hDlg, &wp))
                Util::SaveWindowPlacement(_T("SFVChecker"), wp);

            // Destroy size manager
            delete m_psm;
            m_psm = 0;

            if (m_hIcon) {
                SetClassLong(hDlg, GCL_HICON, 0);
                SetClassLong(hDlg, GCL_HICONSM, 0);
                DestroyIcon(m_hIcon);
                m_hIcon = 0;
            }
            }
            break;
        default:
            break;
    }

    return(FALSE);
}

// Set listview entries by file list
void CSFVChecker::SetListEntries(HWND hwnd, const FileList & lstFiles)
{
    // Reset list view
    ListView_DeleteAllItems(hwnd);

    // Add items to list view
    FileList::const_iterator i;
    int nItem = 0;
    for (i = lstFiles.begin(); i != lstFiles.end(); i++, nItem++) {
        const FILE_ENTRY *pEntry = (*i);
        
        // File name
        LVITEM item;
        item.pszText = const_cast<LPTSTR>((LPCTSTR )pEntry->strFileName);
        item.iItem = nItem;
        item.iSubItem = cColFileName;
        item.mask = LVIF_TEXT;

        ListView_InsertItem(hwnd, &item);
        UpdateItem(hwnd, nItem, pEntry);
    }
}

// The check thread
void CSFVChecker::CheckThread(void *pParam)
{
    const int cBufferSize = 64 * 1024;
    BYTE *pBuffer;

    try {
        pBuffer = new BYTE [cBufferSize];
    }
    catch(bad_alloc) {
        return;
    }

    CSFVChecker *pThis = reinterpret_cast<CSFVChecker *>(pParam);

    // Set stop flag
    pThis->m_bStop = false;

    // Hide status text and show progress bar
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_STATUS), SW_HIDE);
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_PB), SW_SHOW);

    // Get window handles
    HWND hwndList = GetDlgItem(pThis->m_hwnd, IDC_FILES);

    // Check each file
    FileList::iterator i;
    int n = 0;
    bool bSuccess = true;
    int nScanned = 0;
    FileList & lst = pThis->m_lstFiles;
    
    for (i = lst.begin(); i != lst.end(); i++, n++) {
        FILE_ENTRY *pEntry = (*i);
        bool bError = true;

        if (pEntry) {
            // Determine if we should check this file        
            if (pEntry->bCheck == true) {
                nScanned++;

                // Ensure the item is visible
                ListView_EnsureVisible(hwndList, n, FALSE);

                // Open file and assume it is errorneous and unchecked
                TCHAR szDrive[_MAX_DRIVE];
                TCHAR szDir[_MAX_DIR];
                _tsplitpath(pThis->m_pszFileName, szDrive, szDir, NULL, NULL);
                
                CString strFileName;
                strFileName.Format(_T("%s%s%s"), szDrive, szDir, (LPCTSTR )pEntry->strFileName);

                ifstream input(T2CA(strFileName), ios_base::binary);
                bError = true;
                pEntry->fs = FS_UNCHECKED;

                if (input.good() == true) {
                    // Get size of file
                    input.seekg(0, ios_base::end);
                    int nSize = static_cast<int>(input.tellg());
                    input.seekg(0, ios_base::beg);
    
                    // Calculate and set progress bar size/step/position
                    int nBarSize = nSize / cBufferSize;
                    if ((nSize % cBufferSize) != 0) nBarSize++;
                    SendDlgItemMessage(pThis->m_hwnd, IDC_PB, PBM_SETRANGE32, 0, nBarSize);
                    SendDlgItemMessage(pThis->m_hwnd, IDC_PB, PBM_SETSTEP, 1, 0);
                    SendDlgItemMessage(pThis->m_hwnd, IDC_PB, PBM_SETPOS, 0, 0);

                    // Check the file
                    DWORD & dwCRC(pEntry->dwFileCRC);
                    
                    dwCRC = CRC::cInitialCRC;
                    
                    // Scan each block
                    while (input.good() == true) {
                        // Read block
                        input.read((char *)pBuffer, cBufferSize);
                        int nRead = input.gcount();

                        // CRC check block
                        dwCRC = CRC::BlockCRC(pBuffer, static_cast<DWORD>(nRead), dwCRC);
                        
                        if (pThis->m_bStop == true) break;

                        // Update progress bar
                        SendDlgItemMessage(pThis->m_hwnd, IDC_PB, PBM_STEPIT, 0, 0);
                    }

                    dwCRC ^= CRC::cXorFinalCRC;
                } else {
                    // Could not open the file
                    pEntry->fs = FS_MISSING;
                }
                
                // Check for stop
                if (pThis->m_bStop == true) {
                    nScanned--;
                    break;
                }

                // Check if CRCs are the same
                if (pEntry->dwFileCRC == pEntry->dwExpectedCRC) {
                    bError = false;
                    pEntry->bCheck = false;
                } else {
                    bSuccess = false;
                    SetDlgItemText(pThis->m_hwnd, IDC_STATUS, _T("Errorneous file(s) found"));
                }

                pEntry->bError = bError;
                if (pEntry->fs == FS_UNCHECKED)
                    pEntry->fs = FS_CHECKED;

                // Update file CRC result and check state
                UpdateItem(hwndList, n, pEntry);
            }
        }
    }

    // Deallocate memory
    delete [] pBuffer;

    // Set final status
    CString str;
    if (nScanned == 0) {
        str = _T("No files were scanned");
    } else if (bSuccess == true) {
        if (nScanned == 1) {
            str = _T("Scanned file is correct");
        } else {
            str.Format(_T("Scanned files are correct (%d files)"), nScanned);
        }
    }

    if (nScanned == 0 || bSuccess == true)
        SetDlgItemText(pThis->m_hwnd, IDC_STATUS, str);

    // Hide progress bar and show status text
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_STATUS), SW_SHOW);
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_PB), SW_HIDE);

    pThis->m_bStop = true;

    // Tell parent window we're finished
    PostMessage(pThis->m_hwnd, WM_THREADFINISHED, 0, 0);
}

// Update state
void CSFVChecker::UpdateState(HWND hwnd, bool bChecking)
{
    if (bChecking == true) {
        // Disable OK, Show Stop, Hide Recheck, Hide Select All, Disable List
        EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
        ShowWindow(GetDlgItem(hwnd, IDC_STOP), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_RECHECK), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SELECTALL), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_FILES), FALSE);
    } else {
        // Enable OK, Hide Stop, Show Recheck, Show Select All, Enable List
        EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
        ShowWindow(GetDlgItem(hwnd, IDC_STOP), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_RECHECK), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_SELECTALL), SW_SHOW);
        EnableWindow(GetDlgItem(hwnd, IDC_FILES), TRUE);
    }
}

// Select all items in the list
void CSFVChecker::SelectAll(HWND hwnd)
{
    int nCount = ListView_GetItemCount(hwnd);
    
    // Select each item
    for (int i = 0; i < nCount; i++)
        ListView_SetCheckState(hwnd, i, TRUE);
}

// Update an item in listview
void CSFVChecker::UpdateItem(HWND hwnd, int nItem, const FILE_ENTRY *pEntry)
{
    LVITEM item;
    item.iItem = nItem;
    item.iSubItem = cColFileName;
    item.mask = LVIF_TEXT;

    if (pEntry->bCheck == true) {
        ListView_SetCheckState(hwnd, nItem, TRUE);
    } else {
        ListView_SetCheckState(hwnd, nItem, FALSE);
    }
    
    // Expected CRC
    item.iSubItem = cColExpectedCRC;
    CString str;
    str.Format(_T("%08lX"), pEntry->dwExpectedCRC);
    item.pszText = const_cast<LPTSTR>((LPCTSTR )str);
        
    ListView_SetItem(hwnd, &item);

    // File CRC field
    item.iSubItem = cColFileCRC;
        
    switch (pEntry->fs) {
        case FS_CHECKED:
            // File checked
            str.Format(_T("%08lX"), pEntry->dwFileCRC);
            break;
        case FS_MISSING:
            // File is missing
            str = _T("Missing");
            break;
        case FS_UNCHECKED:
        default:
            // File not yet checked
            str = _T("---");
            break;
    }

    item.pszText = const_cast<LPTSTR>((LPCTSTR )str);
        
    ListView_SetItem(hwnd, &item);
}