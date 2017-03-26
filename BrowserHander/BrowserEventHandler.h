#pragma once

#include <atlcom.h>
#include <atltypes.h>
#include <initguid.h>
#import "..\x64\Debug\WebBrowserInstance.exe"
//#include ".\x64\Debug\webbrowserinstance.tlh"
//#include "..\WebBrowserInst\WebBrowserInstance_i.h"
#include "..\WebBrowserInst\WebBrowserInstance_i.c"

using namespace WebBrowserInstanceLib;

class IBrowserEvents
{
public:
	virtual void RenderStream(IStream ** pStream, LONG width, LONG height){}
	virtual void RenderArray(VARIANT arrBuffer, LONG width, LONG height){}
};


class CBrowserEventHandler
	:public CComObjectRoot
	//,public IDispatch
	,public _IBrowserEvents
{
public:
	CBrowserEventHandler();
	~CBrowserEventHandler();

	BEGIN_COM_MAP(CBrowserEventHandler)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(_IBrowserEvents)
	END_COM_MAP()

public:
	STDMETHODIMP GetTypeInfoCount(UINT*)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetTypeInfo(UINT iTInfo,
		LCID lcid,
		ITypeInfo **ppTInfo)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetIDsOfNames(REFIID riid,
		LPOLESTR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID *rgDispId)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP Invoke(DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr);

public:
	BOOL AdviseEvent(IUnknown* pEventSrc, const IID iid_event_src, IBrowserEvents* pEndPoint);
	BOOL ReleaseEvent();
private:
	IBrowserEvents* m_pEndPoint; // _IBrowserEvents
	DWORD m_dwCookie;
	CComPtr<IUnknown> m_pSrc;
	IID m_iidSrc;
};


class CBrowserBase
	:public IBrowserEvents
{
public:
	CBrowserBase();
	~CBrowserBase();

	virtual void RenderBuffer(void * pBuffer, ULONG len);

	HRESULT InitInstance();

	HRESULT UnInit();

	BOOL IsInitialized();

	CRect m_viewRc;
	CString m_homePage;
	UINT m_fps;
	HWND m_hbrowserWnd;
	BrowserRenderMode m_renderMode;
protected:

	virtual void RenderStream(IStream ** pStream, LONG width, LONG height);

	virtual void RenderArray(VARIANT arrBuffer, LONG width, LONG height);

	CComPtr<IBrowserApp> m_spApp;
	CComPtr<IBrowser> m_spBrowser;
	CComObject<CBrowserEventHandler>* m_spEHander;
private:
	BOOL m_bInit; 
};