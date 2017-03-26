#include "stdafx.h"
#include "BrowserEventHandler.h"

CComModule _Module;
extern __declspec(selectany) CAtlModule* _pAtlModule=&_Module;


CBrowserEventHandler::CBrowserEventHandler()
	:m_pEndPoint(NULL), m_dwCookie(0),m_pSrc(NULL), m_iidSrc(IID_NULL)
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
		//HRESULT hr;
		//VARIANTARG var;
		//VariantInit(&var);
		//hr = VariantChangeTypeEx(&var,
		//	&(pDispParams->rgvarg[2]),
		//	lcid, 0, VT_UNKNOWN);
		//if FAILED(hr)
		//	return DISP_E_BADVARTYPE;
		if(m_pEndPoint)
			m_pEndPoint->RenderStream((IStream**)pDispParams->rgvarg[2].ppunkVal, pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal);
		break;
	case 0x2:
		// Make sure the is just three argument
		if (pDispParams->cArgs != 3)
			return DISP_E_BADPARAMCOUNT;

		// We don't support named arguments
		if (pDispParams->cNamedArgs)
			return DISP_E_NONAMEDARGS;

		if(m_pEndPoint)
			m_pEndPoint->RenderArray(pDispParams->rgvarg[2], pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal);
		break;
	default:
		break;
	}
	return S_OK;
}


BOOL CBrowserEventHandler::AdviseEvent(IUnknown* pEventSrc, const IID iid_event_src, IBrowserEvents* pEndPoint)
{
	HRESULT hr = S_OK;
	if (pEventSrc == NULL || pEndPoint == NULL) return 0;
	m_pSrc = pEventSrc;
	m_iidSrc = iid_event_src;
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
	m_iidSrc = IID_NULL;
	m_dwCookie = 0;
	if (FAILED(hr)) return 0;
	return 1;
}


/**
*
*  CBrowserBase Class
*
*/

CBrowserBase::CBrowserBase()
	:m_spEHander(0), m_viewRc(0, 0, 1280, 720)
	, m_fps(30), m_bInit(0), m_hbrowserWnd(NULL), m_renderMode(WindowLess)
{
}


CBrowserBase::~CBrowserBase() {}


void CBrowserBase::RenderBuffer(void * pBuffer, ULONG len)
{
}


HRESULT CBrowserBase::InitInstance()
{
	HRESULT hr = S_OK;
	if (m_bInit) return E_INVALIDARG;
	hr = CComObject<CBrowserEventHandler>::CreateInstance(&m_spEHander);
	if (FAILED(hr)) return hr;
	m_spEHander->AddRef();
	hr = CoCreateInstance(CLSID_BrowserApp, NULL, CLSCTX_SERVER, IID_IBrowserApp, (void**)&m_spApp);
	if (FAILED(hr)) return hr;
	m_spBrowser = m_spApp->CreateBrowser();
	if (!m_spBrowser) return hr;
	m_spBrowser->put_FrameRate(m_fps);
	m_spBrowser->put_RenderMode(m_renderMode);
	m_spBrowser->put_ViewRect(m_viewRc);
	if (!m_homePage.IsEmpty())
	{
		CComBSTR url = (LPCTSTR)m_homePage;
		m_spBrowser->put_HomePage(url);
	}
	hr = m_spApp->ShowBrowser(m_spBrowser);
	if (FAILED(hr)) return hr;
	m_spEHander->AdviseEvent(m_spBrowser, DIID__IBrowserEvents, this);
	m_bInit = 1;
	return hr;
}


HRESULT CBrowserBase::UnInit()
{
	if (!m_bInit) return E_INVALIDARG;
	m_spEHander->ReleaseEvent();
	m_spEHander->Release();
	m_spBrowser->Close();
	m_spApp->CloseAll();
	m_spApp = NULL;
	m_spBrowser = NULL;
	m_spEHander = NULL;
	m_bInit = 0;
	return S_OK;
}


BOOL CBrowserBase::IsInitialized() { return m_bInit; }


void CBrowserBase::RenderStream(IStream ** pStream, LONG width, LONG height)
{

	ULONG len = 0;
	char* buffer = NULL;
	HRESULT hr = S_OK;
	ATLTRACE("%s -- %x\n", "RenderStream -- pStream", *pStream);
	CComPtr<IStream> pMarshal = *pStream;
	CComPtr<IStream> spStream;
	LARGE_INTEGER cur = { 0 };
	STATSTG statData = { 0 };
	hr = ::CoGetInterfaceAndReleaseStream(pMarshal, IID_IStream, (void**)&spStream);
	if (pMarshal)
		hr = pMarshal->Stat(&statData, STATFLAG_NONAME);//exclude name - reduces time and resources used in an allocation and free operation.
	ATLTRACE("%d -- %d\n", statData.type, statData.cbSize);
	return;
	if (SUCCEEDED(hr))
	{
		HGLOBAL hMem = NULL;
		GetHGlobalFromStream(spStream, &hMem);
		ULONG cbLen = (ULONG)GlobalSize(hMem);
		BYTE *pData = (BYTE *)GlobalLock(hMem);
		buffer = new CHAR[cbLen + 1];
		if (buffer != NULL)
		{
			CopyMemory(buffer, pData, cbLen);
			RenderBuffer(buffer, cbLen);
			delete buffer;
		}
		::GlobalUnlock(hMem);
	}

}


void CBrowserBase::RenderArray(VARIANT arrBuffer, LONG width, LONG height)
{
	LPSTR* pdata = NULL;
	ULONG cbLen = width*height * 4;
	SafeArrayAccessData(arrBuffer.parray, (VOID**)&pdata);
	if (pdata != NULL)
	{
		RenderBuffer(pdata, cbLen);
	}
	SafeArrayUnaccessData(arrBuffer.parray);
}
