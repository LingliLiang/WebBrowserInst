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


void CBrowser::FilpMemoryVer(void* pSrc, size_t len, size_t height, size_t width, size_t step)
{
	assert((len%step) == 0);
	assert(step);
	assert(pSrc);
	BYTE* pTemp = new BYTE[step];
	size_t times = height/2;
	BYTE* head = (BYTE*)pSrc,*tail = (BYTE*)pSrc+len-step;
	for(size_t index = 0; index < times; index++)
	{
		::memcpy_s(pTemp,step,head,step);
		::memcpy_s(head,step,tail,step);
		::memcpy_s(tail,step,pTemp,step);
		head += step;
		tail -= step;
	}
	delete pTemp;
}

void FilpMemory2(void* pSrc, size_t len, size_t height, size_t width, size_t step)
{
	assert((len%step) == 0);
	assert(step);
	assert(pSrc);
	BYTE* pTemp = new BYTE[step];
	size_t times = len / step / 2;
	BYTE* head = (BYTE*)pSrc,*tail = (BYTE*)pSrc+len-step;
	for(size_t index = 0; index < times; index++)
	{
		::memcpy_s(pTemp,step,head,step);
		//FilpMemory(pTemp,step,1);
		::memcpy_s(head,step,tail,step);
		//FilpMemory(head,step,1);
		::memcpy_s(tail,step,pTemp,step);
		head += step;
		tail -= step;
	}
	delete pTemp;
}

void CBrowser::FilpMemoryHor(void* pSrc, size_t len, size_t height, size_t width, size_t step)
{
	assert((width%step) == 0);
	assert(height);
	assert(step);
	assert(pSrc);
	size_t times = height;
	BYTE* cur = (BYTE*)pSrc;
	for(size_t index = 0; index < times; index++)
	{
		//FilpMemoryVer(cur,width,step);
		cur += width;
	}
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
	IStream* pDstStream = NULL;
	HRESULT hr = S_OK;
	ULONG cbLen = width * height * 4;
	VARIANT arrBuffer;
	VariantInit(&arrBuffer);
	arrBuffer.vt = VT_ARRAY  | VT_I1;
	SAFEARRAY*  pSArr = NULL;
	SAFEARRAYBOUND saBounds = {cbLen+1,0};	pSArr = SafeArrayCreate(VT_I1,1,&saBounds);
	DWORD sss = ::GetLastError();
	char* pdata = NULL;
	SafeArrayAccessData(pSArr,(VOID**)&pdata);
	::memcpy(pdata,buffer,cbLen);
	//FilpMemoryHor(pdata,cbLen,height,width,4);
	//FilpMemoryVer(pdata,cbLen,height,width,4);
	FilpMemoryVer(pdata,cbLen,height,width,width);
	pdata[cbLen] = '\0';
	SafeArrayUnaccessData(pSArr);
	arrBuffer.parray = pSArr;
	Fire_RenderArray(arrBuffer,width, height);
	SafeArrayDestroy(arrBuffer.parray);
	return S_OK;

	//a new hGlobal handle is to be allocated instead;
	//automatically free the hGlobal parameter
	hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream); //pStream had 1 ref
	ULONG ref = 0;
	if (SUCCEEDED(hr))
	{
		ULONG cbLen = width * height * 4;
		ULARGE_INTEGER size = { cbLen };
		LARGE_INTEGER cur = { 0 };
		ULONG cbWrite = 0;
		CComPtr<IStream> spStream;
		spStream.Attach(pStream);
		hr = spStream->SetSize(size);
		hr = spStream->Write(buffer, cbLen, &cbWrite);
		hr = spStream->Seek(cur, STREAM_SEEK_SET, NULL);//reset to begain
		hr = spStream->Commit(STGC_OVERWRITE);
		STATSTG statData = { 0 };
		hr = spStream->Stat(&statData, STATFLAG_NONAME);
		ATLTRACE("%s -- %x\n","pStream",pStream);
		// Marshal  pStream to pDstStream
		hr = CreateStreamOnHGlobal(NULL, TRUE, &pDstStream); //pDstStream had 1 ref
		if(SUCCEEDED(hr))
			ATLTRACE("%s -- %x\n","pDstStream",pDstStream);
		hr = ::CoMarshalInterface(pDstStream,IID_IStream,spStream,MSHCTX_LOCAL,NULL,MSHLFLAGS_TABLESTRONG);
		if(SUCCEEDED(hr))
		{
			pDstStream->Seek(cur, STREAM_SEEK_SET, NULL);//reset to begain

			if(FAILED(Fire_RenderStream((IUnknown**)&pDstStream, width, height)))
			{
				CComPtr<IStream> pFailedStream = NULL;
				hr = ::CoGetInterfaceAndReleaseStream(pDstStream,IID_IStream,(void**)&pFailedStream);
			}
			ref = pStream->AddRef();
			ref = pStream->Release();
		}
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
