// Browser.h : Declaration of the CBrowser

#pragma once
#include "resource.h"       // main symbols



#include "WebBrowserInstance_i.h"
#include "_IBrowserEvents_CP.h"

#include ".\CefClient\CefHandler.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CBrowser

class ATL_NO_VTABLE CBrowser :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CBrowser, &CLSID_Browser>,
	public IConnectionPointContainerImpl<CBrowser>,
	public CProxy_IBrowserEvents<CBrowser>,
	public IDispatchImpl<IBrowser, &IID_IBrowser, &LIBID_WebBrowserInstanceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispatchImpl<IBrowserNotify, &__uuidof(IBrowserNotify), &LIBID_WebBrowserInstanceLib, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
	CBrowser()
	{
		m_BrowserRef = NULL;
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_BROWSER)


	BEGIN_COM_MAP(CBrowser)
		COM_INTERFACE_ENTRY(IBrowser)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
		COM_INTERFACE_ENTRY(IBrowserNotify)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(CBrowser)
		CONNECTION_POINT_ENTRY(__uuidof(_IBrowserEvents))
	END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:


private:
	CString  m_strHomePage;
	BrowserRenderMode m_mode;
	long m_frameRate; 
	CRect m_viewRect;
	CefRefPtr<CefBrowser>*  m_BrowserRef;

	// IBrowser Methods
public:
	STDMETHOD(get_HomePage)(BSTR* pVal);
	STDMETHOD(put_HomePage)(BSTR newVal);
	// IBrowserNotify Methods
public:
	STDMETHOD(OnTitleChange)(BSTR title);

	STDMETHOD(get_RenderMode)(BrowserRenderMode* pVal);
	STDMETHOD(put_RenderMode)(BrowserRenderMode newVal);
	STDMETHOD(get_FrameRate)(LONG* pVal);
	STDMETHOD(put_FrameRate)(LONG newVal);
	STDMETHOD(get_ViewRect)(RECT* pVal);
	STDMETHOD(put_ViewRect)(RECT newVal);
	STDMETHOD(get_Busy)(VARIANT_BOOL* pVal);
	STDMETHOD(get_LocationURL)(BSTR* pVal);
	STDMETHOD(get_IsInvalid)(VARIANT_BOOL* pVal);
	STDMETHOD(GoHomePage)();
	STDMETHOD(GoForward)();
	STDMETHOD(GoBack)();
	STDMETHOD(Navigate)(BSTR url);
	STDMETHOD(Reload)();
	STDMETHOD(StopLoading)();
	STDMETHOD(Close)();
};

OBJECT_ENTRY_AUTO(__uuidof(Browser), CBrowser)
