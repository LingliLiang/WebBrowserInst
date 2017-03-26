// BrowserApp.cpp : CBrowserApp µÄÊµÏÖ

#include "stdafx.h"
#include "BrowserApp.h"

// CBrowserApp

CBrowserApp::CBrowserApp()
{
}

HRESULT CBrowserApp::FinalConstruct()
{
	IBrowser* p = NULL;
	//create shadow browser incase close windowed-browser exit CefRunMessageLoop
	//if(SUCCEEDED(CreateBrowser(&p)))
	//{
	//	CComBSTR str;
	//	str = _T("http://global.bing.com/");
	//	p->put_HomePage(str);
	//	p->put_FrameRate(1);
	//	p->put_RenderMode(WindowLess);
	//	RECT rc = {0,0,128,72};
	//	p->put_ViewRect(rc);
	//	return ShowBrowser(p);
	//}
	//return E_UNEXPECTED;
	Add(new CBrowserObj);
	return S_OK;
}

void CBrowserApp::FinalRelease()
{
	ATLTRACE("%s --\n", "CBrowserApp FinalRelease");
}

STDMETHODIMP CBrowserApp::QuitApp()
{
	// TODO: Add your implementation code here
	//CefQuitMessageLoop();
	Fire_Exit();
	return S_OK;
}


STDMETHODIMP CBrowserApp::CloseAll()
{
	// TODO: Add your implementation code here
	BrowserObjIter iter = m_vecBrowsers.begin();
	while (iter != m_vecBrowsers.end())
	{
		(*iter)->Release();
		iter++;
	}
	m_vecBrowsers.clear();
	return S_OK;
}


STDMETHODIMP CBrowserApp::Add(CBrowserObj* pBrowser)
{
	// TODO: Add your implementation code here
	pBrowser->AddRef();
	m_vecBrowsers.push_back(pBrowser);
	return S_OK;
}


STDMETHODIMP CBrowserApp::Close(ULONG ulIndex)
{
	// index id 1-based, 0 is hide browser
	if (ulIndex < 1 || ulIndex > m_vecBrowsers.size())
		return E_INVALIDARG;
	BrowserObjIter iter = m_vecBrowsers.begin() + ulIndex;
	(*iter)->Release();
	m_vecBrowsers.erase(iter);
	return S_OK;
}


STDMETHODIMP CBrowserApp::CloseIdentify(LONG lId)
{
	// TODO: Add your implementation code here
	BrowserObjIter iter = m_vecBrowsers.begin();
	while (iter != m_vecBrowsers.end())
	{
		LONG id = 0;
		if (SUCCEEDED((*iter)->get_Identifier(&id)) && id == lId)
		{
			(*iter)->Release();
			m_vecBrowsers.erase(iter);
			break;
		}
		iter++;
	}
	return S_OK;
}


STDMETHODIMP CBrowserApp::get_Count(LONG* pVal)
{
	// TODO: Add your implementation code here
	*pVal = (LONG)m_vecBrowsers.size() - 1;
	return S_OK;
}


STDMETHODIMP CBrowserApp::get_Browser(IBrowser** pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}


STDMETHODIMP CBrowserApp::get_Item(LONG index, IBrowser** pVal)
{
	// TODO: Add your implementation code here
	if (index < 1 || index > m_vecBrowsers.size())
		return E_INVALIDARG;
	CBrowserObj* & obj = m_vecBrowsers[index];
	HRESULT hr = obj->QueryInterface(IID_IBrowser,(void**)pVal);
	if (FAILED(hr))
	{
		return E_UNEXPECTED;
	}
	return S_OK;
}


STDMETHODIMP CBrowserApp::get_NewEnum(IUnknown** pVal)
{
	// TODO: Add your implementation code here
	//typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, CBrowserObj, _Copy<CBrowserObj>>> enumVariant;
	//enumVariant* pEnum = new enumVariant;
	//HRESULT hr = pEnum->Init(m_vecBrowsers.begin(), m_vecBrowsers.end(), NULL, AtlFlagCopy);
	//if (SUCCEEDED(hr)) {
	//	hr = pEnum->QueryInterface<IID_IEnumVARIANT>((void**)pVal);
	//}
	//if (FAILED(hr))
	//{
	//	delete pEnum;
	//}
	//return hr;
	return E_NOTIMPL;
}


STDMETHODIMP CBrowserApp::CreateBrowser(IBrowser** pVal)
{
	// TODO: Add your implementation code here
	CComObject<CBrowser>* p = NULL;
	if(FAILED(p->CreateInstance(&p)))
	{
		return E_UNEXPECTED;
	}
	IBrowser* pIBrowser = NULL;
	if(FAILED(p->QueryInterface(IID_IBrowser,(void**)&pIBrowser)))
	{
		return E_UNEXPECTED;
	}
	CComBSTR str;
	str = _T("http://www.baidu.com/");
	pIBrowser->put_HomePage(str);
	Add(p);
	*pVal = pIBrowser;
	return S_OK;
}


STDMETHODIMP CBrowserApp::ShowBrowser(IBrowser* pVal)
{
	if(!pVal) return E_INVALIDARG;
	CefHandler* pInst = CefHandler::GetInstance();
	RECT rc={0};
	ULONG_PTR hWnd = NULL;
	pVal->get_ViewRect(&rc);
	pVal->get_ParentWnd(&hWnd);
	if(pInst)
	{
		SUCCEEDED(pInst->CreateBrowser(pVal, (HWND)hWnd,rc));
	}
	else
	{
		// SimpleHandler implements browser-level callbacks.
		CefRefPtr<CefHandler> handler(new CefHandler(false));
		pInst = CefHandler::GetInstance();
		SUCCEEDED(pInst->CreateBrowser(pVal, (HWND)hWnd,rc));
	}
	return S_OK;
}
