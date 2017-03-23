#include "stdafx.h"
#include "BrowserEventHandler.h"

//#import "..\x64\Debug\WebBrowserInstance.exe"


CBrowserEventHandler::CBrowserEventHandler(const IID iid_event_src)
	:m_pEndPoint(NULL), m_dwCookie(0),m_pSrc(NULL), m_iidSrc(iid_event_src)
{
}


CBrowserEventHandler::~CBrowserEventHandler()
{
}

STDMETHODIMP CBrowserEventHandler::Invoke(DISPID dispIdMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS *pDispParams,
	VARIANT *pVarResult,
	EXCEPINFO *pExcepInfo,
	UINT *puArgErr)
{
	switch (dispIdMember)
	{
	case 0x1:
		// Make sure the is just three argument
		if (pDispParams->cArgs != 3)
			return DISP_E_BADPARAMCOUNT;

		// We don't support named arguments
		if (pDispParams->cNamedArgs)
			return DISP_E_NONAMEDARGS;

		// Coerce the argument into a long
		HRESULT hr;
		VARIANTARG var;
		VariantInit(&var);
		hr = VariantChangeTypeEx(&var,
			&(pDispParams->rgvarg[2]),
			lcid, 0, VT_UNKNOWN);
		if FAILED(hr)
			return DISP_E_BADVARTYPE;
		if(m_pEndPoint)
			m_pEndPoint->RenderStream((IStream*)var.punkVal, pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal);
		break;
	default:
		break;
	}
	return S_OK;
}

BOOL CBrowserEventHandler::AdviseEvent(IUnknown* pEventSrc, IBrowserEvents* pEndPoint)
{
	HRESULT hr = S_OK;
	if (pEventSrc == NULL || pEndPoint == NULL) return 0;
	m_pSrc = pEventSrc;
	hr = AtlAdvise(m_pSrc, this, m_iidSrc, &m_dwCookie);
	if (FAILED(hr)) return 0;
	m_pEndPoint = pEndPoint;
	return 1;
}

BOOL CBrowserEventHandler::ReleaseEvent()
{
	HRESULT hr = S_OK;
	if (m_pSrc == NULL || m_dwCookie == 0) return 0;
	hr = AtlUnadvise(m_pSrc, m_iidSrc, m_dwCookie);
	m_pEndPoint = NULL;
	m_pSrc = NULL;
	m_dwCookie = 0;
	if (FAILED(hr)) return 0;
	return 1;
}
