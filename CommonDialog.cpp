#include "Stdafx.h"
#include "CommonDialog.h"

namespace CommonDialog
{

// Add a scaled column to the listview
void AddScaledColumn(HWND hwnd, int nCol, LPCTSTR pszText, int nWidthPercent)
{
    // Get width
    RECT rect;
    GetClientRect(hwnd, &rect);

    int nWidth = nWidthPercent * (rect.right - rect.left) / 100;

    AddColumn(hwnd, nCol, pszText, nWidth);
}

// Update a scaled column in the listview
void UpdateScaledColumn(HWND hwnd, int nCol, int nWidthPercent)
{
    // Get width
    RECT rect;
    GetClientRect(hwnd, &rect);

    int nWidth = nWidthPercent * (rect.right - rect.left) / 100;

    UpdateColumn(hwnd, nCol, nWidth);
}

// Add a scaled column to the listview
void AddColumn(HWND hwnd, int nCol, LPCTSTR pszText, int nWidth)
{
    // Add column
    LVCOLUMN col;
    col.fmt = LVCFMT_LEFT;
    col.cx = nWidth;
    col.pszText = const_cast<LPTSTR>(pszText);
    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

    ListView_InsertColumn(hwnd, nCol, &col);
}

// Update a scaled column in the listview
void UpdateColumn(HWND hwnd, int nCol, int nWidth)
{
    // Set column
    LVCOLUMN col;
    col.cx = nWidth;
    col.mask = LVCF_WIDTH;

    ListView_SetColumn(hwnd, nCol, &col);
}

}