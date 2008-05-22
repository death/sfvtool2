#pragma once

namespace CommonDialog
{

class CSizeManager
{
public:
    CSizeManager(HWND hwnd);
    ~CSizeManager(void);

    void UpdatePositions(void);

protected:
    void GetRect(int nId, RECT & rect);
    void UpdateButtonPosition(RECT & rectNewClient, int nId, RECT & rectOriginal);
    void UpdateFixedPosition(RECT & rectNewClient, int nId, RECT & rectOriginal);
    void UpdateTextPosition(RECT & rectNewClient, int nId, RECT & rectOriginal);

private:
    HWND m_hwnd;

    // Original sizes
    RECT m_rectClient;
    
    RECT m_rectOKButton;
    RECT m_rectSelectAllButton;
    RECT m_rectRecheckButton;
    RECT m_rectStopButton;

    RECT m_rectStatusGroup;
    RECT m_rectFilesList;

    RECT m_rectStatusText;
    RECT m_rectProgressBar;
};

}