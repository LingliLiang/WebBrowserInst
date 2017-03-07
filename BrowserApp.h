// BrowserApp.h : CBrowserApp ������

#pragma once
#include "resource.h"       // ������



#include "WebBrowserInstance_i.h"
#include "_IBrowserAppEvents_CP.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;


// CBrowserApp
/**
* control all browser control
*
*
*/

class ATL_NO_VTABLE CBrowserApp :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CBrowserApp, &CLSID_BrowserApp>,
	public IConnectionPointContainerImpl<CBrowserApp>,
	public CProxy_IBrowserAppEvents<CBrowserApp>,
	public IDispatchImpl<IBrowserApp, &IID_IBrowserApp, &LIBID_WebBrowserInstanceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CBrowserApp()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERAPP)


BEGIN_COM_MAP(CBrowserApp)
	COM_INTERFACE_ENTRY(IBrowserApp)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CBrowserApp)
	CONNECTION_POINT_ENTRY(__uuidof(_IBrowserAppEvents))
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



};

OBJECT_ENTRY_AUTO(__uuidof(BrowserApp), CBrowserApp)
