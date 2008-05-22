#include "StdAfx.h"
#include "sfvcreator.h"
#include "Resource.h"
#include "sfvhdr.h"
#include "CRC.h"
#include "CommonDialog.h"
#include "Util.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)

using namespace std;

#define WM_THREADFINISHED (WM_USER + 1)

const int cColFileName = 0;
const int cColFileCRC = 1;

// Constructor
CSFVCreator::CSFVCreator(StrList & lstFileNames)
: m_lstFileNames(lstFileNames)
, m_psm(0)
{
}

// Destructor
CSFVCreator::~CSFVCreator(void)
{
}

// Input SFV file name
bool CSFVCreator::InputSFVName(void)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_INPUTSFVNAME), NULL, InputDlgHelper, (LPARAM )this);

    if (m_strFileName.GetLength() > 0)
        return(true);

    return(false);
}

// Create SFV file
void CSFVCreator::Create(void)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SFVCHECK), NULL, CreateDlgHelper, (LPARAM )this);
}

BOOL CALLBACK CSFVCreator::InputDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
        SetWindowLong(hDlg, GWL_USERDATA, (LONG )lParam);
    
    CSFVCreator *pThis = reinterpret_cast<CSFVCreator *>(GetWindowLong(hDlg, GWL_USERDATA));
    return pThis->InputDlg(hDlg, uMsg, wParam, lParam);
}

// Input dialog procedure
BOOL CALLBACK CSFVCreator::InputDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            // Dialog initialization
            {
            // Extract directory from first filename
            const CString & strFileName = m_lstFileNames.front();
            TCHAR szDir[_MAX_DIR];
            TCHAR szDrive[_MAX_DRIVE];
            _tsplitpath(strFileName, szDrive, szDir, NULL, NULL);

            m_strPath = szDrive;
            m_strPath.Append(szDir);

            if (szDir[0]) {
                // Find end of directory name
                int nEnd = lstrlen(szDir) - 1;
                while (szDir[nEnd] == '\\') nEnd--;
                
                szDir[nEnd + 1] = '\0';

                // Find start of directory name
                LPTSTR p = szDir;
                LPTSTR s;
                while (s = _tcschr(p, '\\')) {
                    p = s;
                    p++;
                }
    
                if (p[0]) {
                    // Format file name
                    m_strFileName.Format(_T("%s.sfv"), p);
    
                    // Set field to default file name
                    SetDlgItemText(hDlg, IDC_FILENAME, m_strFileName);
                    SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETSEL, 0, -1);
                }
            }

            // Set focus to file name field
            SetFocus(GetDlgItem(hDlg, IDC_FILENAME));
            }
            return TRUE;
        case WM_COMMAND:
            // Dialog command
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case IDCANCEL:
                        // Cancel
                        PostMessage(hDlg, WM_CLOSE, 0, 0);
                        break;
                    case IDOK:
                        // OK
                        {
                        TCHAR szFileName[MAX_PATH];
                        GetDlgItemText(hDlg, IDC_FILENAME, szFileName, MAX_PATH);
                        m_strFileName = szFileName;
                        EndDialog(hDlg, IDOK);
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_CLOSE:
            // Close dialog
            m_strFileName = _T("");
            EndDialog(hDlg, IDCANCEL);
            break;
        default:
            break;
    }

    return(FALSE);
}

BOOL CALLBACK CSFVCreator::CreateDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
        SetWindowLong(hDlg, GWL_USERDATA, (LONG )lParam);
    
    CSFVCreator *pThis = reinterpret_cast<CSFVCreator *>(GetWindowLong(hDlg, GWL_USERDATA));
    return pThis->CreateDlg(hDlg, uMsg, wParam, lParam);
}

BOOL CALLBACK CSFVCreator::CreateDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            ListView_SetExtendedListViewStyle(hwndList, ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

            // Initialize size manager
            m_psm = new CommonDialog::CSizeManager(hDlg);

            // Set new window position
            WINDOWPLACEMENT wp;
            if (Util::LoadWindowPlacement(_T("SFVCreator"), wp) == true) {
                SetWindowPlacement(hDlg, &wp);
                m_psm->UpdatePositions();
            }

            // Get column width for CRCs
            m_nColWidth = ListView_GetStringWidth(hwndList, _T("DEADBEEFCAFEBABE"));
            
            // Get first column's width
            RECT rectList;
            GetClientRect(hwndList, &rectList);
            int nFirstColWidth = rectList.right - rectList.left - m_nColWidth - GetSystemMetrics(SM_CXHTHUMB);

            // Add columns 
            CommonDialog::AddColumn(hwndList, cColFileName, _T("FileName"), nFirstColWidth);
            CommonDialog::AddColumn(hwndList, cColFileCRC, _T("File CRC"), m_nColWidth);

            // Begin creating
            UpdateState(hDlg, true);
            _beginthread(CreateWorker, 0, this);
            }
            return(TRUE);
        case WM_COMMAND:
            // Dialog command
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case IDOK:
                    case IDC_STOP:
                        // OK / Stop
                        PostMessage(hDlg, WM_CLOSE, 0, 0);
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
                EndDialog(hDlg, IDOK);
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
                int nFirstColWidth = rectList.right - rectList.left - m_nColWidth - GetSystemMetrics(SM_CXHTHUMB);
                if (ListView_GetCountPerPage(hwndList) < ListView_GetItemCount(hwndList))
                    nFirstColWidth += GetSystemMetrics(SM_CXHTHUMB);

                // Update columns
                CommonDialog::UpdateColumn(hwndList, cColFileName, nFirstColWidth);
                CommonDialog::UpdateColumn(hwndList, cColFileCRC, m_nColWidth);

                // Invalidate window
                InvalidateRect(hDlg, NULL, TRUE);
            }
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
        case WM_DESTROY:
            // Destroy dialog
            // Save window placement
            {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hDlg, &wp)) {
                Util::SaveWindowPlacement(_T("SFVCreator"), wp);
            }
            }
            // Destroy size manager
            delete m_psm;
            m_psm = 0;

            if (m_hIcon) {
                SetClassLong(hDlg, GCL_HICON, 0);
                SetClassLong(hDlg, GCL_HICONSM, 0);
                DestroyIcon(m_hIcon);
                m_hIcon = 0;
            }
            break;
        default:
            break;
    }

    return(FALSE);
}

// Update button state
void CSFVCreator::UpdateState(HWND hwnd, bool bCreating)
{
    // Hide Select All, Recheck
    ShowWindow(GetDlgItem(hwnd, IDC_SELECTALL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_RECHECK), SW_HIDE);

    if (bCreating == true) {
        // Show Stop, Disable OK
        ShowWindow(GetDlgItem(hwnd, IDC_STOP), SW_SHOW);
        EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
    } else {
        // Hide Stop, Enable OK
        ShowWindow(GetDlgItem(hwnd, IDC_STOP), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
    }
}

// SFV Creator worker thread
void CSFVCreator::CreateWorker(void *pParam)
{
    const int cBufferSize = 64 * 1024;
    bool bError = false;
    BYTE *pBuffer = new BYTE [cBufferSize];

    CSFVCreator *pThis = reinterpret_cast<CSFVCreator *>(pParam);

    pThis->m_bStop = false;

    // Hide status text and show progress bar
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_STATUS), SW_HIDE);
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_PB), SW_SHOW);

    // Get list window handle
    HWND hwndList = GetDlgItem(pThis->m_hwnd, IDC_FILES);

    // Convert SFV filename to path
    CString strSFV = pThis->m_strPath + pThis->m_strFileName;

    // Try to open SFV for output
    ofstream output(T2CA((LPCTSTR )strSFV), ios_base::binary);

    if (output.good() == true) {
        // Write SFVTool header
        output.write((char *)aSFVHeader, sizeof(aSFVHeader));

        // Create a checksum for each file
        StrList::const_iterator i;
        int n = 0;

        for (i = pThis->m_lstFileNames.begin(); i != pThis->m_lstFileNames.end(); i++, n++) {
            CString strFileName = (*i);
            
            // Get filename/extension from path
            TCHAR szFName[_MAX_FNAME];
            TCHAR szExt[_MAX_EXT];
            _tsplitpath(strFileName, NULL, NULL, szFName, szExt);

            // Add file to list
            CString str;
            LVITEM item;
            item.iItem = n;
            item.iSubItem = cColFileName;
            str.Format(_T("%s%s"), szFName, szExt);
            item.pszText = const_cast<LPTSTR>((LPCTSTR )str);
            item.mask = LVIF_TEXT;

            ListView_InsertItem(hwndList, &item);

            // Set item's status
            str = _T("Checking...");
            item.pszText = const_cast<LPTSTR>((LPCTSTR )str);
            item.iSubItem = cColFileCRC;

            ListView_SetItem(hwndList, &item);

            // Ensure the item is visible
            ListView_EnsureVisible(hwndList, n, FALSE);

            // Check file
            DWORD dwCRC = CRC::cInitialCRC;

            bool bError = true;

            ifstream input(T2CA(strFileName), ios_base::binary);
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

                // Hash each block of file
                while (input.good() == true) {
                    input.read((char *)pBuffer, cBufferSize);
                    int nRead = input.gcount();

                    dwCRC = CRC::BlockCRC(pBuffer, static_cast<DWORD>(nRead), dwCRC);
                    
                    // Check for stop flag
                    if (pThis->m_bStop == true) break;

                    // Update progress bar
                    SendDlgItemMessage(pThis->m_hwnd, IDC_PB, PBM_STEPIT, 0, 0);
                }

                bError = false;
            }

            // Check for stop flag
            if (pThis->m_bStop == true) break;
            
            dwCRC ^= CRC::cXorFinalCRC;

            // Pass on if errorneous file
            if (bError == true) {
                // Set item's CRC
                str = _T("Error");
                item.pszText = const_cast<LPTSTR>((LPCTSTR )str);

                ListView_SetItem(hwndList, &item);
                continue;
            }

            // Set item's CRC
            str.Format(_T("%08lX"), dwCRC);
            item.pszText = const_cast<LPTSTR>((LPCTSTR )str);

            ListView_SetItem(hwndList, &item);

            // Write checksum to output
            str.Format(_T("%s%s %08lX\r\n"), szFName, szExt, dwCRC);
            output.write((LPCSTR )str, str.GetLength());
        }
    } else {
        // Error creating file
        bError = true;
    }

    // Deallocate memory
    delete [] pBuffer;

    // Set status
    if (bError == true)
        SetDlgItemText(pThis->m_hwnd, IDC_STATUS, _T("SFV Creation failed"));
    else
        SetDlgItemText(pThis->m_hwnd, IDC_STATUS, _T("SFV Written successfuly"));

    // Hide progress bar and show status text
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_STATUS), SW_SHOW);
    ShowWindow(GetDlgItem(pThis->m_hwnd, IDC_PB), SW_HIDE);

    pThis->m_bStop = true;

    // Tell parent window that we're finished
    PostMessage(pThis->m_hwnd, WM_THREADFINISHED, 0, 0);
}
