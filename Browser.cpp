// Browser.cpp : Implementation of CBrowser

#include "stdafx.h"
#include "Browser.h"


// CBrowser

CBrowser::CBrowser()
{
	m_BrowserRef = NULL;
	m_mode = AsPopup;
	m_frameRate = 30;
	m_identifier = 0;
}


HRESULT CBrowser::FinalConstruct()
{

	return S_OK;
}


void CBrowser::FinalRelease()
{
}


void CBrowser::put_BrowserRefPoint(CefRefPtr<CefBrowser>** p)
{
	if (p && (*p)) {
		m_BrowserRef = *p;
	}
}


STDMETHODIMP CBrowser::get_HomePage(BSTR* pVal)
{
	if (*pVal) return E_INVALIDARG;
	*pVal = m_strHomePage.AllocSysString();
	return S_OK;
}


STDMETHODIMP CBrowser::put_HomePage(BSTR newVal)
{
	if (!newVal) return E_INVALIDARG;
	m_strHomePage = newVal;
	return S_OK;
}


STDMETHODIMP CBrowser::OnTitleChange(BSTR title)
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::get_RenderMode(BrowserRenderMode* pVal)
{
	*pVal = m_mode;
	return S_OK;
}


STDMETHODIMP CBrowser::put_RenderMode(BrowserRenderMode newVal)
{
	if (newVal<WindowLess || newVal>AsPopup)
		return E_INVALIDARG;
	m_mode = newVal;
	return S_OK;
}


STDMETHODIMP CBrowser::get_FrameRate(LONG* pVal)
{
	*pVal = m_frameRate;
	return S_OK;
}


STDMETHODIMP CBrowser::put_FrameRate(LONG newVal)
{
	if (newVal > 60)
		newVal = 60;
	if (newVal < 5)
		newVal = 5;
	m_frameRate = newVal;
	return S_OK;
}


STDMETHODIMP CBrowser::get_ViewRect(RECT* pVal)
{
	*pVal = m_viewRect;
	return S_OK;
}


STDMETHODIMP CBrowser::put_ViewRect(RECT newVal)
{
	CRect viewRect = newVal;
	if (viewRect.IsRectNull()) return E_INVALIDARG;
	m_viewRect = viewRect;
	return S_OK;
}


STDMETHODIMP CBrowser::get_Busy(VARIANT_BOOL* pVal)
{
	WBI_PREFCHECK(m_BrowserRef);
	*pVal = (*m_BrowserRef)->IsLoading() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


STDMETHODIMP CBrowser::get_LocationURL(BSTR* pVal)
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefFrame> pMainFram = (*m_BrowserRef)->GetMainFrame();
	WBI_REFCHECK(pMainFram);
	*pVal = ::SysAllocString(pMainFram->GetURL().c_str());
	return S_OK;
}


STDMETHODIMP CBrowser::get_IsInvalid(VARIANT_BOOL* pVal)
{
	if (!m_BrowserRef || !((*m_BrowserRef).get()))
		*pVal = VARIANT_FALSE;
	else
		*pVal = VARIANT_TRUE;
	return S_OK;
}


STDMETHODIMP CBrowser::put_BrowserRefPoint(IUnknown** newVal)
{
	// TODO: Add your implementation code here
	put_BrowserRefPoint(reinterpret_cast<CefRefPtr<CefBrowser>**>(newVal));
	return S_OK;
}


STDMETHODIMP CBrowser::get_Identifier(LONG* pVal)
{
	// TODO: Add your implementation code here
	*pVal = m_identifier;
	return S_OK;
}


STDMETHODIMP CBrowser::put_Identifier(LONG newVal)
{
	// TODO: Add your implementation code here
	m_identifier = newVal;
	return S_OK;
}


STDMETHODIMP CBrowser::GoHomePage()
{
	CComBSTR url;
	url.Attach(m_strHomePage.AllocSysString());
	return Navigate(url);
}


STDMETHODIMP CBrowser::GoForward()
{
	WBI_PREFCHECK(m_BrowserRef);
	if ((*m_BrowserRef)->CanGoForward())
		(*m_BrowserRef)->GoForward();
	else
		return S_FALSE;
	return S_OK;
}


STDMETHODIMP CBrowser::GoBack()
{
	WBI_PREFCHECK(m_BrowserRef);
	if ((*m_BrowserRef)->CanGoBack())
		(*m_BrowserRef)->GoBack();
	else
		return S_FALSE;
	return S_OK;
}


STDMETHODIMP CBrowser::Navigate(BSTR url)
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefFrame> pMainFram = (*m_BrowserRef)->GetMainFrame();
	WBI_REFCHECK(pMainFram);
	pMainFram->LoadURL(url);
	return S_OK;
}


STDMETHODIMP CBrowser::Reload()
{
	WBI_PREFCHECK(m_BrowserRef);
	(*m_BrowserRef)->Reload();
	return S_OK;
}


STDMETHODIMP CBrowser::StopLoading()
{
	WBI_PREFCHECK(m_BrowserRef);
	(*m_BrowserRef)->StopLoad();
	return S_OK;
}


STDMETHODIMP CBrowser::Close()
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);
	pHost->CloseBrowser(false);
	return S_OK;
}


STDMETHODIMP CBrowser::RemoveBrowserRef()
{
	// TODO: Add your implementation code here
	m_BrowserRef = NULL;
	return S_OK;
}


STDMETHODIMP CBrowser::MessageProc(LONGLONG hWnd, ULONG msg, ULONGLONG wParam, LONGLONG lParam)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBrowser::OnRender(const CHAR* buffer, LONG width, LONG height)
{
	// buffer -> BGRA
	IStream* pStream = NULL;
	HRESULT hr = S_OK;
	//a new hGlobal handle is to be allocated instead;
	//automatically free the hGlobal parameter
	hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (SUCCEEDED(hr))
	{
		ULONG cbLen = width * height * 4;
		ULARGE_INTEGER size = { cbLen };
		LARGE_INTEGER cur = { 0 };
		ULONG cbWrite = 0;
		CComPtr<IStream> spStream = pStream;
		hr = spStream->SetSize(size);
		hr = spStream->Write(buffer, cbLen, &cbWrite);
		hr = spStream->Seek(cur, STREAM_SEEK_SET, NULL);//reset to begain
		hr = spStream->Commit(STGC_OVERWRITE);
		Fire_RenderStream(spStream, width, height);
	}
	return hr;
}


STDMETHODIMP CBrowser::ConvertStream(IStream* pStream, ULONG* pLen, CHAR* pVal)
{
	HRESULT hr = S_OK;
	CComPtr<IStream> spStream = pStream;
	LARGE_INTEGER cur = { 0 };
	STATSTG statData = { 0 };
	hr = spStream->Seek(cur, STREAM_SEEK_SET, NULL);
	hr = spStream->Stat(&statData, STATFLAG_NONAME);//exclude name - reduces time and resources used in an allocation and free operation.
	if (SUCCEEDED(hr))
	{
		assert(pVal == NULL);
		pVal = new CHAR[statData.cbSize.QuadPart+1];
		if (pVal == NULL) return E_OUTOFMEMORY;
		ULONG cbLen = (ULONG)statData.cbSize.QuadPart;
		hr = spStream->Read(pVal, cbLen, pLen);
	}
	
	return hr;
}
