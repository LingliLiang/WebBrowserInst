#pragma once

template<class T>
class CProxy_IBrowserEvents :
	public ATL::IConnectionPointImpl<T, &__uuidof(_IBrowserEvents)>
{
public:
	HRESULT Fire_RenderStream(IUnknown ** pStream, LONG width, LONG height)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IDispatch * pConnection = static_cast<IDispatch *>(punkConnection.p);

			if (pConnection)
			{
				CComVariant avarParams[3];
				avarParams[2].byref = pStream;
				avarParams[2].vt = VT_UNKNOWN|VT_BYREF;
				avarParams[1] = width;
				avarParams[1].vt = VT_I4;
				avarParams[0] = height;
				avarParams[0].vt = VT_I4;
				CComVariant varResult;

				DISPPARAMS params = { avarParams, NULL, 3, 0 };
				hr = pConnection->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
			}
		}
		return hr;
	}
	HRESULT Fire_RenderArray(VARIANT arrBuffer, LONG width, LONG height)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IDispatch * pConnection = static_cast<IDispatch *>(punkConnection.p);

			if (pConnection)
			{
				CComVariant avarParams[3];
				avarParams[2] = arrBuffer;
				avarParams[1] = width;
				avarParams[1].vt = VT_I4;
				avarParams[0] = height;
				avarParams[0].vt = VT_I4;
				CComVariant varResult;

				DISPPARAMS params = { avarParams, NULL, 3, 0 };
				hr = pConnection->Invoke(2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
			}
		}
		return hr;
	}
};

