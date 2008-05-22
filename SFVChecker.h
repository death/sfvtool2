#pragma once

#include "Defs.h"
#include "SizeManager.h"

class CSFVChecker
{
public:
    CSFVChecker(LPCTSTR pszFileName);
    ~CSFVChecker(void);

    void Check(void);

protected:
    bool ParseSFV(void);
    static BOOL CALLBACK CheckDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL CALLBACK CheckDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void SetListEntries(HWND hwnd, const FileList & lstFiles);
    static void CheckThread(void *pParam);
    static void UpdateState(HWND hwnd, bool bChecking);
    static void SelectAll(HWND hwnd);
    static void UpdateItem(HWND hwnd, int nItem, const FILE_ENTRY *pEntry);

private:
    FileList m_lstFiles;
    LPTSTR m_pszFileName;
    HWND m_hwnd;
    volatile bool m_bStop;
    HICON m_hIcon;

    // Dialog variables
    bool m_bIsClosing;
    CommonDialog::CSizeManager *m_psm;
    int m_nColWidth;
};
