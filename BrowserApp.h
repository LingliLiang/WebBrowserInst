// BrowserApp.h : CBrowserApp 的声明

#pragma once
#include "resource.h"       // 主符号



#include "WebBrowserInstance_i.h"
#include "_IBrowserAppEvents_CP.h"
#include "Browser.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;


// CBrowserApp
/**
* control all browser control
*
*
*/
typedef CComObject<CBrowser> CBrowserObj;
class ATL_NO_VTABLE CBrowserApp :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CBrowserApp, &CLSID_BrowserApp>,
	public IConnectionPointContainerImpl<CBrowserApp>,
	public CProxy_IBrowserAppEvents<CBrowserApp>,
	public IDispatchImpl<IBrowserApp, &IID_IBrowserApp, &LIBID_WebBrowserInstanceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CBrowserApp();

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERAPP)


BEGIN_COM_MAP(CBrowserApp)
	COM_INTERFACE_ENTRY(IBrowserApp)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CBrowserApp)
	CONNECTION_POINT_ENTRY(__uuidof(_IBrowserAppEvents))
END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

private:
	typedef std::vector<CBrowserObj*>::iterator BrowserObjIter;
	std::vector<CBrowserObj*> m_vecBrowsers;
public:

	STDMETHOD(QuitApp)();
	STDMETHOD(CloseAll)();
	STDMETHOD(Add)(CBrowserObj* pBrowser);
	STDMETHOD(Close)(ULONG ulIndex);
	STDMETHOD(CloseBrowser)(IBrowser* pVal);
	STDMETHOD(CloseIdentify)(LONG lId);
	STDMETHOD(get_Count)(LONG* pVal);
	STDMETHOD(get_Browser)(IBrowser** pVal);
	STDMETHOD(get_Item)(LONG index, IBrowser** pVal);
	STDMETHOD(get_NewEnum)(IUnknown** pVal);
	STDMETHOD(CreateBrowser)(IBrowser** pVal);
	STDMETHOD(ShowBrowser)(IBrowser* pVal);
};

OBJECT_ENTRY_AUTO(__uuidof(BrowserApp), CBrowserApp)
