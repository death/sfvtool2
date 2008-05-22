#pragma once

#include "Defs.h"
#include "SizeManager.h"

class CSFVCreator
{
public:
    CSFVCreator(StrList & lstFileNames);
    ~CSFVCreator(void);

    bool InputSFVName(void);
    void Create(void);

protected:
    static BOOL CALLBACK InputDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL CALLBACK InputDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK CreateDlgHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL CALLBACK CreateDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void UpdateState(HWND hwnd, bool bCreating);
    static void CreateWorker(void *pParam);

private:
    StrList & m_lstFileNames;
    CString m_strFileName;
    CString m_strPath;
    HWND m_hwnd;
    volatile bool m_bStop;
    HICON m_hIcon;

    // Dialog variables
    bool m_bIsClosing;
    CommonDialog::CSizeManager *m_psm;
    int m_nColWidth;
};
