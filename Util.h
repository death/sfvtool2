#pragma once

namespace Util
{

bool IsHexString(LPCTSTR pszString);
bool SaveWindowPlacement(LPCTSTR pszName, WINDOWPLACEMENT & wp);
bool LoadWindowPlacement(LPCTSTR pszName, WINDOWPLACEMENT & wp);

}