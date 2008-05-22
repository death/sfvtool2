// SFVTool2.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include "SFVTool2.h"

HINSTANCE g_hInstance;

class CSFVTool2Module : public CAtlDllModuleT< CSFVTool2Module >
{
public :
	DECLARE_LIBID(LIBID_SFVTool2Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SFVTOOL2, "{99EC7331-345B-4A54-B0A7-D81110BB0DD5}")
};

CSFVTool2Module _AtlModule;


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    g_hInstance = hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}
