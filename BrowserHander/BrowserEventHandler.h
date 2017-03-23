#pragma once

#include <atlcom.h>
#include <initguid.h>
#include ".\x64\Debug\webbrowserinstance.tlh"
//#include "..\WebBrowserInst\WebBrowserInstance_i.h"
#include "..\WebBrowserInst\WebBrowserInstance_i.c"

using namespace WebBrowserInstanceLib;

class IBrowserEvents
{
public:
	virtual void RenderStream(IStream * pStream, LONG width, LONG height){};
};

class CBrowserEventHandler
	:public CComObjectRoot
	//,public IDispatch
	,public _IBrowserEvents
{
public:
	CBrowserEventHandler(const IID iid_event_src);
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
	// _IBrowserEvents
private:

public:
	BOOL AdviseEvent(IUnknown* pEventSrc, IBrowserEvents* pEndPoint);
	BOOL ReleaseEvent();
private:
	IBrowserEvents* m_pEndPoint;
	DWORD m_dwCookie;
	CComPtr<IUnknown> m_pSrc;
	const IID m_iidSrc;
};

class CBrowserBase
	:public IBrowserEvents
{
public:
	CBrowserBase();
	~CBrowserBase();
protected:
	HRESULT InitInstance()
	{
		HRESULT hr = S_OK;
		CComObject<CBrowserEventHandler>* pEHander = NULL;
		hr = m_spEHander.CreateInstance(&pEHander);
		if (FAILED(hr)) return hr;
		m_spEHander.SetVoid(pEHander);
		m_spEHander.AddRef();
		hr = CoCreateInstance(CLSID_BrowserApp,NULL,CLSCTX_SERVER, IID_IBrowserApp,(void**)&m_spApp);
		if (FAILED(hr)) return hr;
		hr = m_spApp->CreateBrowser(&m_spBrowser);
		if (FAILED(hr)) return hr;
		m_spBrowser->put_FrameRate(30);
		m_spBrowser->put_RenderMode(WindowLess);
		RECT rc = {0,0,1280,720};
		m_spBrowser->put_ViewRect(rc);
		hr = m_spApp->ShowBrowser(m_spBrowser);
		if (FAILED(hr)) return hr;
		m_spEHander.AdviseEvent(m_spBrowser, this);
		return hr;
	}

	HRESULT UnInit()
	{
		m_spEHander.AdviseEvent(m_spBrowser, this);
	}

	CComPtr<IBrowserApp> m_spApp;
	CComPtr<IBrowser> m_spBrowser;
	CComObject<CBrowserEventHandler> m_spEHander;
};

CBrowserBase::CBrowserBase()
	:m_spEHander(0)
{
}

CBrowserBase::~CBrowserBase()
{
}