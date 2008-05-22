#include "StdAfx.h"
#include "Util.h"

namespace Util
{

// Check if a string is a hex string (0-9a-fA-F)
bool IsHexString(LPCTSTR pszString)
{
    static LPCTSTR cszMask = _T("0123456789abcdefABCDEF");
    int i;

    // Scan string for invalid characters
    for (i = 0; pszString[i]; i++) {
        if (_tcschr(cszMask, pszString[i]) == NULL)
            return(false);
    }

    return(true);
}

bool SaveWindowPlacement(LPCTSTR pszName, WINDOWPLACEMENT & wp)
{
    HKEY hKey;
    bool bRet = false;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Execution\\SFVTool"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        HKEY hSubKey;
        if (RegCreateKeyEx(hKey, pszName, 0, NULL, 0, KEY_WRITE, NULL, &hSubKey, NULL) == ERROR_SUCCESS) {
            RegSetValueEx(hSubKey, _T("wp"), 0, REG_BINARY, (LPBYTE )&wp, sizeof(WINDOWPLACEMENT));
            RegCloseKey(hSubKey);
            
            bRet = true;
        }
        RegCloseKey(hKey);
    }

    return(bRet);
}

bool LoadWindowPlacement(LPCTSTR pszName, WINDOWPLACEMENT & wp)
{
    HKEY hKey;
    bool bRet = false;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Execution\\SFVTool"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        HKEY hSubKey;
        if (RegOpenKeyEx(hKey, pszName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            DWORD cbData = sizeof(WINDOWPLACEMENT);
            RegQueryValueEx(hSubKey, _T("wp"), NULL, NULL, (LPBYTE )&wp, &cbData);
            RegCloseKey(hSubKey);
            
            bRet = true;
        }
        RegCloseKey(hKey);
    }

    return(bRet);
}

}