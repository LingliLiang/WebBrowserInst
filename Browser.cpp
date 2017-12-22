// Browser.cpp : Implementation of CBrowser

#include "stdafx.h"
#include "Browser.h"
#include "shared\util_win.h"
#include "shared\geometry_util.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"
#include "BGRABlock.h"
using namespace client;

// CBrowser
CBrowser::CBrowser()
	:m_mode(AsPopup), m_frameRate(30), m_identifier(0),
	m_hParent(NULL), m_hSelf(NULL),m_BrowserRef(NULL), m_bMouse(0),m_bKeybroad(0),
	device_scale_factor_(client::GetDeviceScaleFactor()),
	last_mouse_pos_(),
	current_mouse_pos_(),
	mouse_rotation_(false),
	mouse_tracking_(false),
	last_click_x_(0),
	last_click_y_(0),
	last_click_button_(MBT_LEFT),
	last_click_count_(0),
	last_click_time_(0),
	last_mouse_down_on_view_(false)
{
}


HRESULT CBrowser::FinalConstruct()
{

	return S_OK;
}


void CBrowser::FinalRelease()
{
	Close();
	ATLTRACE("%s --\n", "CBrowser FinalRelease");
}


void FilpMemory(void* pSrc, size_t len, size_t step)
{
	assert((len%step) == 0);
	assert(step);
	assert(pSrc);
	BYTE* pTemp = new BYTE[step];
	if(!pTemp) return;
	size_t times = len / step / 2;
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


void FilpMemoryHor(void* pSrc,size_t height, size_t width, size_t step)
{
	assert((width%step) == 0);
	assert(height);
	assert(step);
	assert(pSrc);
	size_t times = height;
	size_t line = width*step;
	BYTE* cur = (BYTE*)pSrc;
	for(size_t index = 0; index < times; index++)
	{
		FilpMemory(cur, line, step);
		cur += line;
	}
}


void FilpMemoryVer(void* pSrc,size_t len, size_t width, size_t step)
{
	FilpMemory(pSrc, len, width*step);
}


STDMETHODIMP CBrowser::OnTitleChange(BSTR title)
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnRender(const BYTE* buffer, LONG width, LONG height, RECT* rects, ULONG rects_count)
{
	//IConnectionPointImpl had connect?
	if(!this->m_vec.GetSize()) return S_OK;

	// buffer -> BGRA
	CBGRABlock block(const_cast<BYTE*>(buffer), height, width, 4);

	HRESULT hr = S_OK;
	ULONG cbLen = (ULONG)block.GetBlockLength();
	VARIANT arrBuffer;
	VariantInit(&arrBuffer);
	arrBuffer.vt = VT_ARRAY | VT_I1;
	SAFEARRAY*  pSArr = NULL;
	SAFEARRAYBOUND saBounds = { cbLen + 1,0 };
	pSArr = SafeArrayCreate(VT_I1, 1, &saBounds);
	DWORD sss = ::GetLastError();
	char* pdata = NULL;
	SafeArrayAccessData(pSArr, (VOID**)&pdata);
	//::memcpy(pdata,buffer,cbLen);
	block.GetRectData(*rects, pdata);
	pdata[cbLen] = '\0';
	SafeArrayUnaccessData(pSArr);
	arrBuffer.parray = pSArr;
	Fire_RenderArray(arrBuffer, rects_count, width, height);
	SafeArrayDestroy(arrBuffer.parray);
	return S_OK;

	//a new hGlobal handle is to be allocated instead;
	//automatically free the hGlobal parameter
	IStream* pStream = NULL;
	IStream* pDstStream = NULL;
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
		ATLTRACE("%s -- %x\n", "pStream", pStream);
		// Marshal  pStream to pDstStream
		hr = CreateStreamOnHGlobal(NULL, TRUE, &pDstStream); //pDstStream had 1 ref
		if (SUCCEEDED(hr))
			ATLTRACE("%s -- %x\n", "pDstStream", pDstStream);
		hr = ::CoMarshalInterface(pDstStream, IID_IStream, spStream, MSHCTX_LOCAL, NULL, MSHLFLAGS_TABLESTRONG);
		if (SUCCEEDED(hr))
		{
			pDstStream->Seek(cur, STREAM_SEEK_SET, NULL);//reset to begain

			if (FAILED(Fire_RenderStream((IUnknown**)&pDstStream, width, height)))
			{
				CComPtr<IStream> pFailedStream = NULL;
				hr = ::CoGetInterfaceAndReleaseStream(pDstStream, IID_IStream, (void**)&pFailedStream);
			}
			ref = pStream->AddRef();
			ref = pStream->Release();
		}
	}

	return hr;
}


STDMETHODIMP CBrowser::OnAddressChange(BSTR url)
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnStatusMessage(BSTR msg) 
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnClose() {
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnLoadStart(ULONG transition_type) 
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnLoadEnd(LONG httpStatusCode) 
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::OnLoadError(LONG error_code, BSTR error_text, BSTR failed_url, BSTR* make_html) 
{
	return E_NOTIMPL;
}


STDMETHODIMP CBrowser::GetViewRect(RECT * rect)
{
	*rect = m_viewRect;
	return S_OK;
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
	if (pVal == NULL || m_identifier == 0)
	{
		return E_INVALIDARG;
	}
	*pVal = m_identifier;
	return S_OK;
}


STDMETHODIMP CBrowser::put_Identifier(LONG newVal)
{
	// TODO: Add your implementation code here
	m_identifier = newVal;
	return S_OK;
}


STDMETHODIMP CBrowser::get_ParentWnd(ULONG_PTR* pVal)
{
	*pVal = reinterpret_cast<ULONG_PTR>(m_hParent);
	return S_OK;
}


STDMETHODIMP CBrowser::put_ParentWnd(ULONG_PTR newVal)
{
	m_hParent = reinterpret_cast<HWND>(newVal);
	return S_OK;
}


STDMETHODIMP CBrowser::get_BrowserWnd(ULONG_PTR* pVal)
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);
	*pVal = reinterpret_cast<ULONG_PTR>(pHost->GetWindowHandle());
	return S_OK;
}


STDMETHODIMP CBrowser::put_BrowserWnd(ULONG_PTR hWnd)
{
	m_hSelf = reinterpret_cast<HWND>(hWnd);
	return S_OK;
}


STDMETHODIMP CBrowser::get_EnableMouse(VARIANT_BOOL* pVal)
{
	*pVal = m_bMouse ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


STDMETHODIMP CBrowser::put_EnableMouse(VARIANT_BOOL newVal)
{
	m_bMouse = newVal == VARIANT_TRUE ? 1 : 0;
	return S_OK;
}


STDMETHODIMP CBrowser::get_EnableKeybroad(VARIANT_BOOL* pVal)
{
	*pVal = m_bKeybroad ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


STDMETHODIMP CBrowser::put_EnableKeybroad(VARIANT_BOOL newVal)
{
	m_bKeybroad = newVal == VARIANT_TRUE ? 1 : 0;
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
	m_identifier = 0;
	m_BrowserRef = NULL;
	return S_OK;
}


STDMETHODIMP CBrowser::ConvertStream(IStream* pStream, ULONG* pLen, CHAR** pVal)
{
	HRESULT hr = S_OK;
	CComPtr<IStream> spStream = pStream;
	LARGE_INTEGER cur = { 0 };
	STATSTG statData = { 0 };
	hr = spStream->Seek(cur, STREAM_SEEK_SET, NULL);
	hr = spStream->Stat(&statData, STATFLAG_NONAME);//exclude name - reduces time and resources used in an allocation and free operation.
	if (SUCCEEDED(hr))
	{
		assert(*pVal == NULL);
		*pVal = new CHAR[statData.cbSize.QuadPart + 1];
		if (*pVal == NULL) return E_OUTOFMEMORY;
		ULONG cbLen = (ULONG)statData.cbSize.QuadPart;
		hr = spStream->Read(*pVal, cbLen, pLen);
	}

	return hr;
}


STDMETHODIMP CBrowser::MessageProc(ULONG_PTR ulhWnd, ULONG message, ULONGLONG wParam, LONGLONG lParam)
{
	HRESULT hr = S_OK;
	if (!CefCurrentlyOn(TID_UI))
	{
		// Execute on the UI thread.
		const ULONG_PTR c_ulhWnd = ulhWnd;
		const ULONG c_message = message;
		const ULONGLONG c_wParam = wParam;
		const LONGLONG c_lParam = lParam;
		const HRESULT* c_hr = NULL;
		CefPostTask(TID_UI, base::Bind(&CBrowser::MessageProcInter, base::Unretained(this),\
			c_ulhWnd, c_message, c_wParam, c_lParam, c_hr/*, base::Unretained(this)*/));
		return hr;
	}
	//ATLTRACE("CBrowser::MessageProc %08x %x %d %d\n", ulhWnd, message, wParam, lParam);
	MessageProcInter(ulhWnd, message, wParam, lParam, &hr);
	return hr;
}

void CBrowser::MessageProcInter(const ULONG_PTR ulhWnd, const  ULONG message,
	const  ULONGLONG wParam, const  LONGLONG lParam, const  HRESULT* hr)
{
	auto SetResult = [&](HRESULT rhr)->void
	{
		if(hr) *(const_cast<HRESULT*>(hr)) = rhr;
	};
	HWND hWnd = reinterpret_cast<HWND>(ulhWnd);
	//Check is right window
	bool pass = false;
	hr = S_OK;
	if (m_hSelf && m_hSelf == hWnd)
	{
		pass = true;
	}
	else if (m_hParent && m_hParent == hWnd)
	{
		pass = true;
	}
	if (!pass) {
		SetResult(S_FALSE);
		return;
	}
	
	// We want to handle IME events before the OS does any default handling.
	switch (message) {
	case WM_IME_SETCONTEXT:
		SetResult(this->OnIMESetContext(message, wParam, lParam));
		break;
	case WM_IME_STARTCOMPOSITION:
		SetResult(this->OnIMEStartComposition());
		break;
	case WM_IME_COMPOSITION:
		SetResult(this->OnIMEComposition(message, wParam, lParam));
		break;
	case WM_IME_ENDCOMPOSITION:
		SetResult(this->OnIMECancelCompositionEvent());
		// Let WTL call::DefWindowProc() and release its resources.
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
		if(m_bMouse)
			SetResult(this->OnMouseEvent(message, wParam, lParam));
		break;

	case WM_SIZE:
		SetResult(this->OnSize());
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		SetResult(this->OnFocus(message == WM_SETFOCUS));
		break;

	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		SetResult(this->OnCaptureLost());
		break;

	case WM_SYSCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		if(m_bKeybroad)
			SetResult(this->OnKeyEvent(message, wParam, lParam));
		break;

	case WM_PAINT:
		//this->OnPaint();
		break;

	case WM_ERASEBKGND:
		if (SUCCEEDED(this->OnEraseBkgnd()))
			break;
		// Don't erase the background.
		SetResult(S_FALSE);
		break;
	case WM_NCDESTROY:
		break;
	}


}

HRESULT CBrowser::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd_ = NULL;
	if (m_hParent) hwnd_ = m_hParent;
	if(m_hSelf) hwnd_ = m_hSelf;
	if (!hwnd_ || !::IsWindow(hwnd_)) return E_UNEXPECTED;
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);

	LONG currentTime = 0;
	bool cancelPreviousClick = false;

	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
		message == WM_MOUSELEAVE) {
		currentTime = GetMessageTime();
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		cancelPreviousClick =
			(abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2))
			|| (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2))
			|| ((currentTime - last_click_time_) > GetDoubleClickTime());
		if (cancelPreviousClick &&
			(message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
			last_click_count_ = 0;
			last_click_x_ = 0;
			last_click_y_ = 0;
			last_click_time_ = 0;
		}
	}

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN: {
		::SetCapture(hwnd_);
		::SetFocus(hwnd_);
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		if (wParam & MK_SHIFT) {
			// Start rotation effect.
			last_mouse_pos_.x = current_mouse_pos_.x = x;
			last_mouse_pos_.y = current_mouse_pos_.y = y;
			mouse_rotation_ = true;
		}
		else {
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONDOWN ? MBT_LEFT : (
					message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
			if (!cancelPreviousClick && (btnType == last_click_button_)) {
				++last_click_count_;
			}
			else {
				last_click_count_ = 1;
				last_click_x_ = x;
				last_click_y_ = y;
			}
			last_click_time_ = currentTime;
			last_click_button_ = btnType;

			if (pHost) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				pHost->SendMouseClickEvent(mouse_event, btnType, false,
					last_click_count_);
			}
		}
	} break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (GetCapture() == hwnd_)
			ReleaseCapture();
		if (mouse_rotation_) {
			// End rotation effect.
			mouse_rotation_ = false;
			//renderer_.SetSpin(0, 0);
			//Invalidate();
		}
		else {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONUP ? MBT_LEFT : (
					message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
			if (pHost) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				if (last_mouse_down_on_view_ &&
					IsOverPopupWidget(x, y) &&
					(GetPopupXOffset() || GetPopupYOffset())) {
					break;
				}
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				pHost->SendMouseClickEvent(mouse_event, btnType, true,
					last_click_count_);
			}
		}
		break;

	case WM_MOUSEMOVE: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		if (mouse_rotation_) {
			// Apply rotation effect.
			current_mouse_pos_.x = x;
			current_mouse_pos_.y = y;
			//renderer_.IncrementSpin(
			//	current_mouse_pos_.x - last_mouse_pos_.x,
			//	current_mouse_pos_.y - last_mouse_pos_.y);
			last_mouse_pos_.x = current_mouse_pos_.x;
			last_mouse_pos_.y = current_mouse_pos_.y;
			//Invalidate();
		}
		else {
			if (!mouse_tracking_) {
				// Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
				// be generated.
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd_;
				TrackMouseEvent(&tme);
				mouse_tracking_ = true;
			}

			if (pHost) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				pHost->SendMouseMoveEvent(mouse_event, false);
			}
		}
		break;
	}

	case WM_MOUSELEAVE: {
		if (mouse_tracking_) {
			// Stop tracking mouse leave.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE & TME_CANCEL;
			tme.hwndTrack = hwnd_;
			TrackMouseEvent(&tme);
			mouse_tracking_ = false;
		}

		if (pHost) {
			// Determine the cursor position in screen coordinates.
			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(hwnd_, &p);

			CefMouseEvent mouse_event;
			mouse_event.x = p.x;
			mouse_event.y = p.y;
			DeviceToLogical(mouse_event, device_scale_factor_);
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			pHost->SendMouseMoveEvent(mouse_event, true);
		}
	} break;

	case WM_MOUSEWHEEL:
		if (pHost) {
			POINT screen_point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			HWND scrolled_wnd = ::WindowFromPoint(screen_point);
			if (scrolled_wnd != hwnd_)
				break;

			ScreenToClient(hwnd_, &screen_point);
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);

			CefMouseEvent mouse_event;
			mouse_event.x = screen_point.x;
			mouse_event.y = screen_point.y;
			ApplyPopupOffset(mouse_event.x, mouse_event.y);
			DeviceToLogical(mouse_event, device_scale_factor_);
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			pHost->SendMouseWheelEvent(mouse_event,
				IsKeyDown(VK_SHIFT) ? delta : 0,
				!IsKeyDown(VK_SHIFT) ? delta : 0);
		}
		break;
	}
	return S_OK;
}

HRESULT CBrowser::OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);

	CefKeyEvent event;
	event.windows_key_code = (int)wParam;
	event.native_key_code = (int)lParam;
	event.is_system_key = message == WM_SYSCHAR ||
		message == WM_SYSKEYDOWN ||
		message == WM_SYSKEYUP;

	if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
		event.type = KEYEVENT_RAWKEYDOWN;
	else if (message == WM_KEYUP || message == WM_SYSKEYUP)
		event.type = KEYEVENT_KEYUP;
	else
		event.type = KEYEVENT_CHAR;
	event.modifiers = GetCefKeyboardModifiers(wParam, lParam);

	pHost->SendKeyEvent(event);
	return S_OK;
}

HRESULT CBrowser::OnSize()
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);
	// Keep |client_rect_| up to date.
	//::GetClientRect(hwnd_, &client_rect_);

	pHost->WasResized();
	return S_OK;
}

HRESULT CBrowser::OnFocus(bool setFocus)
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);
	pHost->SendFocusEvent(setFocus);
	return S_OK;
}

HRESULT CBrowser::OnCaptureLost()
{
	WBI_PREFCHECK(m_BrowserRef);
	CefRefPtr<CefBrowserHost> pHost = (*m_BrowserRef)->GetHost();
	WBI_REFCHECK(pHost);
	if (mouse_rotation_)
		return S_OK;

	pHost->SendCaptureLostEvent();
	return S_OK;
}

HRESULT CBrowser::OnPaint()
{
	return S_OK;
}

HRESULT CBrowser::OnEraseBkgnd()
{
	return S_OK;
}

HRESULT CBrowser::OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

HRESULT CBrowser::OnIMEStartComposition()
{
	return S_OK;
}

HRESULT CBrowser::OnIMEComposition(UINT message, WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

HRESULT CBrowser::OnIMECancelCompositionEvent()
{
	return S_OK;
}

bool CBrowser::IsOverPopupWidget(int x, int y) const {
	CEF_REQUIRE_UI_THREAD();
	//const CefRect& rc = renderer_.popup_rect();
	//int popup_right = rc.x + rc.width;
	//int popup_bottom = rc.y + rc.height;
	//return (x >= rc.x) && (x < popup_right) &&
	//	(y >= rc.y) && (y < popup_bottom);
	return false;
}

int CBrowser::GetPopupXOffset() const {
	CEF_REQUIRE_UI_THREAD();
	//return renderer_.original_popup_rect().x - renderer_.popup_rect().x;
	return 0;
}

int CBrowser::GetPopupYOffset() const {
	CEF_REQUIRE_UI_THREAD();
	//return renderer_.original_popup_rect().y - renderer_.popup_rect().y;
	return 0;
}

void CBrowser::ApplyPopupOffset(int& x, int& y) const {
	//if (IsOverPopupWidget(x, y)) {
	//	x += GetPopupXOffset();
	//	y += GetPopupYOffset();
	//}
}

STDMETHODIMP CBrowser::GetScreenPoint(INT viewX, INT viewY, INT* screenX, INT* screenY)
{
	// TODO: Add your implementation code here

	return S_OK;
}


STDMETHODIMP CBrowser::GetScreenInfo(ULONG* screen_info)
{
	// TODO: Add your implementation code here

	return S_OK;
}


STDMETHODIMP CBrowser::OnPopupShow(LONG show)
{
	// TODO: Add your implementation code here

	return S_OK;
}


STDMETHODIMP CBrowser::OnPopupSize(RECT* rect)
{
	// TODO: Add your implementation code here

	return S_OK;
}
