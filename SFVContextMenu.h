// SFVContextMenu.h : Declaration of the CSFVContextMenu

#pragma once
#include "resource.h"       // main symbols

#include "SFVTool2.h"
#include "Defs.h"

struct __declspec(uuid("000214e4-0000-0000-c000-000000000046")) IContextMenu;

// CSFVContextMenu

class ATL_NO_VTABLE CSFVContextMenu : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSFVContextMenu, &CLSID_SFVContextMenu>,
	public IDispatchImpl<ISFVContextMenu, &IID_ISFVContextMenu, &LIBID_SFVTool2Lib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IShellExtInit,
    public IContextMenu
{
public:
	CSFVContextMenu()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SFVCONTEXTMENU)


BEGIN_COM_MAP(CSFVContextMenu)
	COM_INTERFACE_ENTRY(ISFVContextMenu)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

protected:
    StrList m_lstFileNames;
    bool m_bIsSFV;

public:
    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu
    STDMETHOD(GetCommandString)(UINT, UINT, UINT *, LPSTR, UINT);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
    STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
};

OBJECT_ENTRY_AUTO(__uuidof(SFVContextMenu), CSFVContextMenu)
