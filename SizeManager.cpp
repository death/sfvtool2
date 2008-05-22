#include "StdAfx.h"
#include "resource.h"
#include "sizemanager.h"

namespace CommonDialog
{

// Constructor
CSizeManager::CSizeManager(HWND hwnd)
: m_hwnd(hwnd)
{
    // Get original position rectangles
    GetClientRect(hwnd, &m_rectClient);

    GetRect(IDOK, m_rectOKButton);
    GetRect(IDC_SELECTALL, m_rectSelectAllButton);
    GetRect(IDC_RECHECK, m_rectRecheckButton);
    GetRect(IDC_STOP, m_rectStopButton);

    GetRect(IDC_STATUSGROUP, m_rectStatusGroup);
    GetRect(IDC_FILES, m_rectFilesList);

    GetRect(IDC_STATUS, m_rectStatusText);
    GetRect(IDC_PB, m_rectProgressBar);
}

// Destructor
CSizeManager::~CSizeManager(void)
{
}

// Get prespective rect of a child control
void CSizeManager::GetRect(int nId, RECT & rect)
{
    RECT rectTemp;
    HWND hwnd = GetDlgItem(m_hwnd, nId);

    GetWindowRect(hwnd, &rectTemp);
    
    POINT pt;
    
    // Convert from screen coordinates to prespective
    pt.x = rectTemp.left;
    pt.y = rectTemp.top;
    ScreenToClient(m_hwnd, &pt);
    rect.left = pt.x;
    rect.top = pt.y;

    pt.x = rectTemp.right;
    pt.y = rectTemp.bottom;
    ScreenToClient(m_hwnd, &pt);
    rect.right = pt.x;
    rect.bottom = pt.y;
}

// Update positions of children windows
void CSizeManager::UpdatePositions(void)
{
    // Get new client rectangle
    RECT rect;
    GetClientRect(m_hwnd, &rect);

    // Update button positions
    UpdateButtonPosition(rect, IDOK, m_rectOKButton);
    UpdateButtonPosition(rect, IDC_SELECTALL, m_rectSelectAllButton);
    UpdateButtonPosition(rect, IDC_RECHECK, m_rectRecheckButton);
    UpdateButtonPosition(rect, IDC_STOP, m_rectStopButton);

    // Update fixed position controls
    UpdateFixedPosition(rect, IDC_STATUSGROUP, m_rectStatusGroup);
    UpdateFixedPosition(rect, IDC_FILES, m_rectFilesList);

    // Update text-style position controls
    UpdateTextPosition(rect, IDC_STATUS, m_rectStatusText);
    UpdateTextPosition(rect, IDC_PB, m_rectProgressBar);
}

// Update button position
void CSizeManager::UpdateButtonPosition(RECT & rectNewClient, int nId, RECT & rectOriginal)
{
    // Buttons are right/bottom aligned
    int nDeltaX = m_rectClient.right - rectOriginal.right;
    int nDeltaY = m_rectClient.bottom - rectOriginal.bottom;

    int nWidth = rectOriginal.right - rectOriginal.left;
    int nHeight = rectOriginal.bottom - rectOriginal.top;
    int x = rectNewClient.right - nWidth - nDeltaX;
    int y = rectNewClient.bottom - nHeight - nDeltaY;

    MoveWindow(GetDlgItem(m_hwnd, nId), x, y, nWidth, nHeight, FALSE);
}

void CSizeManager::UpdateFixedPosition(RECT & rectNewClient, int nId, RECT & rectOriginal)
{
    // Only modify the height/width
    int nDeltaX = m_rectClient.right - rectOriginal.right;
    int nDeltaY = m_rectClient.bottom - rectOriginal.bottom;

    int nWidth = rectNewClient.right - nDeltaX - rectOriginal.left;
    int nHeight = rectNewClient.bottom - nDeltaY - rectOriginal.top;
    int x = rectOriginal.left;
    int y = rectOriginal.top;

    MoveWindow(GetDlgItem(m_hwnd, nId), x, y, nWidth, nHeight, FALSE);
}

void CSizeManager::UpdateTextPosition(RECT & rectNewClient, int nId, RECT & rectOriginal)
{
    // Only modify width and y position
    int nDeltaX = m_rectClient.right - rectOriginal.right;
    int nDeltaY = m_rectClient.bottom - rectOriginal.bottom;

    int nWidth = rectNewClient.right - nDeltaX - rectOriginal.left;
    int nHeight = rectOriginal.bottom - rectOriginal.top;
    int x = rectOriginal.left;
    int y = rectNewClient.bottom - nDeltaY - nHeight;

    MoveWindow(GetDlgItem(m_hwnd, nId), x, y, nWidth, nHeight, FALSE);
}

}