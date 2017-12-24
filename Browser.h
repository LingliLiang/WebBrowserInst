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
	public IDispatchImpl<IBrowserNotify, &__uuidof(IBrowserNotify)>
{
public:
	CBrowser();

	DECLARE_REGISTRY_RESOURCEID(IDR_BROWSER)


	BEGIN_COM_MAP(CBrowser)
		COM_INTERFACE_ENTRY(IBrowser)
		COM_INTERFACE_ENTRY2(IDispatch, IBrowser)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
		COM_INTERFACE_ENTRY(IBrowserNotify)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(CBrowser)
		CONNECTION_POINT_ENTRY(__uuidof(_IBrowserEvents))
	END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	void put_BrowserRefPoint(CefRefPtr<CefBrowser>** p);
private:
	virtual COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE MessageProcInter(const ULONG_PTR ulhWnd,\
		const  ULONG message, const  ULONGLONG wParam, const  LONGLONG lParam, const  HRESULT* hr);
	HRESULT OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam);
	HRESULT OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam);

	HRESULT OnSize();
	HRESULT OnFocus(bool setFocus);
	HRESULT OnCaptureLost();
	HRESULT OnPaint();
	HRESULT OnEraseBkgnd();

	HRESULT OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam);
	HRESULT OnIMEStartComposition();
	HRESULT OnIMEComposition(UINT message, WPARAM wParam, LPARAM lParam);
	HRESULT OnIMECancelCompositionEvent();

	// Manage popup bounds.
	bool IsOverPopupWidget(int x, int y) const;
	int GetPopupXOffset() const;
	int GetPopupYOffset() const;
	void ApplyPopupOffset(int& x, int& y) const;

	float device_scale_factor_;

	// Mouse state tracking.
	POINT last_mouse_pos_;
	POINT current_mouse_pos_;
	bool mouse_rotation_;
	bool mouse_tracking_;
	int last_click_x_;
	int last_click_y_;
	CefBrowserHost::MouseButtonType last_click_button_;
	int last_click_count_;
	double last_click_time_;
	bool last_mouse_down_on_view_;
private:

	CString  m_strHomePage;
	BrowserRenderMode m_mode;
	long m_frameRate;
	long m_identifier;
	CRect m_viewRect;
	HWND m_hParent;
	HWND m_hSelf;
	CefRefPtr<CefBrowser>*  m_BrowserRef;
	BOOL m_bKeybroad;
	BOOL m_bMouse;

public:
	// IBrowserNotify Methods
	STDMETHOD(OnTitleChange)(BSTR title);
	//window-less handle
	STDMETHOD(OnRender)(const BYTE* buffer, LONG width, LONG height, RECT* rects, ULONG rects_count);
	STDMETHOD(OnAddressChange)(BSTR url);
	STDMETHOD(OnStatusMessage)(BSTR msg);
	STDMETHOD(OnClose)();
	STDMETHOD(OnLoadStart)(ULONG transition_type);
	STDMETHOD(OnLoadEnd)(LONG httpStatusCode);
	STDMETHOD(OnLoadError)(LONG error_code, BSTR error_text, BSTR failed_url, BSTR* make_html);

	STDMETHOD(GetViewRect)(RECT* rect);
public:
	// IBrowser Methods
	STDMETHOD(get_HomePage)(BSTR* pVal);
	STDMETHOD(put_HomePage)(BSTR newVal);
	STDMETHOD(get_RenderMode)(BrowserRenderMode* pVal);
	STDMETHOD(put_RenderMode)(BrowserRenderMode newVal);
	STDMETHOD(get_FrameRate)(LONG* pVal);
	STDMETHOD(put_FrameRate)(LONG newVal);
	STDMETHOD(get_ViewRect)(RECT* pVal);
	STDMETHOD(put_ViewRect)(RECT newVal);
	STDMETHOD(get_Busy)(VARIANT_BOOL* pVal);
	STDMETHOD(get_LocationURL)(BSTR* pVal);
	STDMETHOD(get_IsInvalid)(VARIANT_BOOL* pVal);
	STDMETHOD(put_BrowserRefPoint)(IUnknown** newVal);
	STDMETHOD(get_Identifier)(LONG* pVal);
	STDMETHOD(put_Identifier)(LONG newVal);
	STDMETHOD(get_ParentWnd)(ULONG_PTR* pVal);
	STDMETHOD(put_ParentWnd)(ULONG_PTR newVal);
	STDMETHOD(get_BrowserWnd)(ULONG_PTR* pVal);
	STDMETHOD(put_BrowserWnd)(ULONG_PTR hWnd);

	STDMETHOD(GoHomePage)();
	STDMETHOD(GoForward)();
	STDMETHOD(GoBack)();
	STDMETHOD(Navigate)(BSTR url);
	STDMETHOD(Reload)();
	STDMETHOD(StopLoading)();
	STDMETHOD(Close)();
	STDMETHOD(RemoveBrowserRef)();

	STDMETHOD(MessageProc)(ULONG_PTR ulhWnd, ULONG message, ULONGLONG wParam, LONGLONG lParam);

	STDMETHOD(ConvertArray)(BYTE* pArry, ULONG uLenArry, WBRENDERDATA** ppData);
	STDMETHOD(get_EnableMouse)(VARIANT_BOOL* pVal);
	STDMETHOD(put_EnableMouse)(VARIANT_BOOL newVal);
	STDMETHOD(get_EnableKeybroad)(VARIANT_BOOL* pVal);
	STDMETHOD(put_EnableKeybroad)(VARIANT_BOOL newVal);
	STDMETHOD(GetScreenPoint)(INT viewX, INT viewY, INT* screenX, INT* screenY);
	STDMETHOD(GetScreenInfo)(ULONG* screen_info);
	STDMETHOD(OnPopupShow)(LONG show);
	STDMETHOD(OnPopupSize)(RECT* rect);
};

//OBJECT_ENTRY_AUTO(__uuidof(Browser), CBrowser)
OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO(__uuidof(Browser), CBrowser)