// BrowserApp.cpp : CBrowserApp µÄÊµÏÖ

#include "stdafx.h"
#include "BrowserApp.h"

// CBrowserApp

CBrowserApp::CBrowserApp()
{
}

HRESULT CBrowserApp::FinalConstruct()
{
	return S_OK;
}

void CBrowserApp::FinalRelease()
{
}

STDMETHODIMP CBrowserApp::QuitApp()
{
	// TODO: Add your implementation code here
	CefQuitMessageLoop();
	Fire_Exit();
	return S_OK;
}


STDMETHODIMP CBrowserApp::CloseAll()
{
	// TODO: Add your implementation code here
	m_vecBrowsers.clear();
	return S_OK;
}


STDMETHODIMP CBrowserApp::Add(CBrowser* pBrowser)
{
	// TODO: Add your implementation code here
	m_vecBrowsers.push_back(CBrowserObj(pBrowser));
	return S_OK;
}


STDMETHODIMP CBrowserApp::Close(ULONG ulIndex)
{
	// index id 1-based
	if (ulIndex < 1 || ulIndex > m_vecBrowsers.size())
		return E_INVALIDARG;
	m_vecBrowsers.erase(m_vecBrowsers.begin() + ulIndex -1);
	return S_OK;
}


STDMETHODIMP CBrowserApp::CloseIdentify(LONG lId)
{
	// TODO: Add your implementation code here
	
	return S_OK;
}


STDMETHODIMP CBrowserApp::get_Count(LONG* pVal)
{
	// TODO: Add your implementation code here
	*pVal = (LONG)m_vecBrowsers.size();
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
	CBrowserObj& obj = m_vecBrowsers[index - 1];
	HRESULT hr = obj.QueryInterface(IID_IBrowser,(void**)pVal);
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


STDMETHODIMP CBrowserApp::CreateBrowser()
{
	// TODO: Add your implementation code here

	return S_OK;
}
