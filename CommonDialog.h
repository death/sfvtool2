#pragma once

namespace CommonDialog
{

const int cMinWidth = 300;
const int cMinHeight = 200;

void AddScaledColumn(HWND hwnd, int nCol, LPCTSTR pszText, int nWidthPercent);
void UpdateScaledColumn(HWND hwnd, int nCol, int nWidthPercent);
void AddColumn(HWND hwnd, int nCol, LPCTSTR pszText, int nWidth);
void UpdateColumn(HWND hwnd, int nCol, int nWidth);

}