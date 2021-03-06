#include "StdAfx.h"
#include "CefHandler.h"

#include "include/cef_app.h"
//#include "include/cef_runnable.h"
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

//interface predefines
#include "WebBrowserInstance_i.h"

namespace {

	BOOL  GetBrowserNotify(CComPtr<IBrowser>& pBrowser, CComPtr<IBrowserNotify>& spNotify)
	{
		if(pBrowser)
		{
			if(SUCCEEDED(pBrowser->QueryInterface(IID_IBrowserNotify, (void**)&spNotify)))
			{
				assert(spNotify);
				return TRUE;
			}
		}
		return FALSE;
	}

	CefHandler* g_instance = NULL;

}  // namespace


CefHandler* CefHandler::GetInstance()
{
	return g_instance;
}

CefHandler::CefHandler(bool use_views)
	: m_bUseViews(use_views),
	m_bIsClose(false)
{
	DCHECK(!g_instance);
	g_instance = this;
	ibrowser_map_[IBROWSERCREATE] = CComPtr<IBrowser>();
}

CefHandler::CefHandler(const tstring& strUrl/*=TEXT("")*/)
	: m_bIsClose(false)
	, m_bUseViews(false)
{
}

CefHandler::~CefHandler()
{
	g_instance = NULL;
}


/**
* 	CefDisplayHandler methods: 状态改变回调接口
*/
void CefHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	CEF_REQUIRE_UI_THREAD();
	if (m_bUseViews) {
		// Set the title of the window using the Views framework.
		CefRefPtr<CefBrowserView> browser_view =
			CefBrowserView::GetForBrowser(browser);
		if (browser_view) {
			CefRefPtr<CefWindow> window = browser_view->GetWindow();
			if (window)
				window->SetTitle(title);
		}
	}
	else {
		// Set the title of the window using platform APIs.
		CComPtr<IBrowserNotify> spNotify;
		if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
		{
			BrowserRenderMode rmode = WindowLess;
			ibrowser_map_[browser->GetIdentifier()]->get_RenderMode(&rmode);
			if(rmode == AsPopup){
				CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
				SetWindowText(hwnd, std::wstring(title).c_str());
			}
			CComBSTR str = title.c_str();
			spNotify->OnTitleChange(str);
		}
	}
}

void CefHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		CComBSTR str = url.c_str();
		spNotify->OnAddressChange(str);
	}
}

bool CefHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
	return false;
}

void CefHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		CComBSTR str = value.c_str();
		spNotify->OnStatusMessage(str);
	}
}

/**
* CefLifeSpanHandler methods: 网页生命周期回调接口
*/
bool CefHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	const CefString& target_frame_name,
	CefLifeSpanHandler::WindowOpenDisposition target_disposition,
	bool user_gesture,
	const CefPopupFeatures& popupFeatures,
	CefWindowInfo& windowInfo,
	CefRefPtr<CefClient>& client,
	CefBrowserSettings& settings,
	bool* no_javascript_access)
{
	switch (target_disposition)
	{
	case WOD_NEW_FOREGROUND_TAB:
	case WOD_NEW_BACKGROUND_TAB:
	case WOD_NEW_POPUP:
	case WOD_NEW_WINDOW:
		frame->LoadURL(target_url);
		return true; //cancel create
	}
	return false;
}

void CefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();
	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
	CefRefPtr<CefBrowser> * p = &(*browser_list_.rbegin());
	base::AutoLock scopLock(m_lock_);
	ibrowser_map_[browser->GetIdentifier()].Attach(ibrowser_map_[IBROWSERCREATE].Detach());

	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		spNotify->put_Identifier(browser->GetIdentifier());
		spNotify->put_BrowserRefPoint(reinterpret_cast<IUnknown**>(&p));
		spNotify->put_BrowserWnd(reinterpret_cast<ULONG_PTR>(browser->GetHost()->GetWindowHandle()));
	}
}

bool CefHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();
	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		m_bIsClose = true;
		::OutputDebugString(L"CefHandler::DoClose 11\n");
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void CefHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();
	// Remove from the list of existing browsers.
	::OutputDebugString(L"CefHandler::OnBeforeClose\n");
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			IBrowserMapIt ibit = ibrowser_map_.find(browser->GetIdentifier());
			if (ibit != ibrowser_map_.end())
			{
				CComPtr<IBrowserNotify> spNotify;
				if(GetBrowserNotify(ibit->second, spNotify))
				{
					spNotify->RemoveBrowserRef();
					spNotify->OnClose();
				}
				ibrowser_map_.erase(ibit);
			}
			browser_list_.erase(bit);
			break;
		}
	}
	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		//CefQuitMessageLoop(); //not need, it handle in Atl Module_
	}
}

/**
* CefLoadHandler methods: 网页加载状态回调接口
*/
void CefHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		spNotify->OnLoadStart(transition_type);
	}
}

void CefHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	//CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		spNotify->OnLoadEnd(httpStatusCode);
	}
}

void CefHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, \
	ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
	CEF_REQUIRE_UI_THREAD();
	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;
	// Display a load error message.
	tstringstream ss;

	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		CComBSTR str_errorText = errorText.c_str();
		CComBSTR str_failedUrl = failedUrl.c_str();
		CComBSTR html;
		spNotify->OnLoadError(errorCode, str_errorText, str_failedUrl,&html);
		if (html.Length() != 0)
		{
			ss << html;
		}
		else
		{
			ss << L"<html><body bgcolor=\"white\">"
				L"<h2>Failed to load URL " << tstring(failedUrl) <<
				L" with error " << tstring(errorText) << L" (" << errorCode <<
				L").</h2></body></html>" << '\0';
		}
		frame->LoadString(ss.str(), failedUrl);
	}
}

/**
* CefContextMenuHandler methods: 菜单加载接口
*/
void CefHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, \
	CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	//在这里，我添加了自己想要的菜单
	cef_context_menu_type_flags_t flag = params->GetTypeFlags();
	if (flag & CM_TYPEFLAG_PAGE)
	{//普通页面的右键消息
		model->SetLabel(MENU_ID_BACK, L"后退");
		model->SetLabel(MENU_ID_FORWARD, L"前进");
		model->SetLabel(MENU_ID_VIEW_SOURCE, L"查看源代码");
		model->SetLabel(MENU_ID_PRINT, L"打印");
		model->SetLabel(MENU_ID_RELOAD, L"刷新");
		model->SetLabel(MENU_ID_RELOAD_NOCACHE, L"强制刷新");
		model->SetLabel(MENU_ID_STOPLOAD, L"停止加载");
		model->SetLabel(MENU_ID_REDO, L"重复");
	}
	if (flag & CM_TYPEFLAG_EDITABLE)
	{//编辑框的右键消息
		model->SetLabel(MENU_ID_UNDO, L"撤销");
		model->SetLabel(MENU_ID_REDO, L"重做");
		model->SetLabel(MENU_ID_CUT, L"剪切");
		model->SetLabel(MENU_ID_COPY, L"复制");
		model->SetLabel(MENU_ID_PASTE, L"粘贴");
		model->SetLabel(MENU_ID_DELETE, L"删除");
		model->SetLabel(MENU_ID_SELECT_ALL, L"全选");
	}
}

bool CefHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, \
	CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
	return false;
}

/**
* CefDownloadHandler methods: 下载管理接口
*/
void CefHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, \
	const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{

}

void CefHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, \
	CefRefPtr<CefDownloadItemCallback> callback)
{
	//取消CEF内部下载文件，使用默认浏览器打开链接去下载，下载过程就不需要我们关心了，毕竟不是做浏览器
	callback->Cancel();
	CefString strUrl = download_item->GetURL();
	//ShellExecute(NULL, L"open", strUrl.c_str(), NULL, NULL, SW_SHOW);
}

void CefHandler::CloseAllBrowsers(bool force_close)
{
	if (!CefCurrentlyOn(TID_UI))
	{
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&CefHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}

bool CefHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message)
{
	return false;
}

/**
* CefRenderHandler methods:渲染回馈
*/

bool CefHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
	CefRect& rect) {
		return false;
}

bool CefHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	CComPtr<IBrowserNotify> spNotify;
	if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
	{
		RECT rectClid = { 0 };
		::OutputDebugString(L"CefHandler::GetViewRect\n");
		spNotify->GetViewRect(&rectClid);
		rect.Set(rectClid.left, rectClid.top, rectClid.right, rectClid.bottom);
	}
	return true;
}

bool CefHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
	int viewX,
	int viewY,
	int& screenX,
	int& screenY) {
		CComPtr<IBrowserNotify> spNotify;
		if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
		{
			spNotify->GetScreenPoint(viewX, viewY,&screenX,&screenY);
		}
		return false;
}

bool CefHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,
	CefScreenInfo& screen_info) {
		return false;
}

void CefHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
	bool show) {}

void CefHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
	const CefRect& rect) {}

void CefHandler::OnPaint(CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const void* buffer,
	int width, int height) {
		//if(!dirtyRects.size()) return;
		auto it = dirtyRects.begin();
		RECT rc = {it->x,it->y,it->x + it->width,it->y + it->height};
		//ATLTRACE("CefHandler::OnPaint dirtyRects [%d %d %d %d]\n",it->x,it->y,it->width,it->height);
		CComPtr<IBrowserNotify> spNotify;
		if(GetBrowserNotify(ibrowser_map_[browser->GetIdentifier()], spNotify))
		{
			spNotify->OnRender((const BYTE*)buffer, width, height, &rc, 1);
		}
}

void CefHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
	CefCursorHandle cursor,
	CursorType type,
	const CefCursorInfo& custom_cursor_info) {
		SetCursor(cursor);
}

bool CefHandler::StartDragging(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefDragData> drag_data,
	DragOperationsMask allowed_ops,
	int x, int y) {
		return false;
}


void CefHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
	DragOperation operation) {

}

void CefHandler::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,
	double x,
	double y) {
}

void CefHandler::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
	const CefRange& selected_range,
	const RectList& character_bounds) {
}

/**
* methods: 自定义接口
*/
HRESULT CefHandler::CreateBrowser(IBrowser * pBrowser, HWND hParentWnd, const RECT& rect)
{
	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	// Information used when creating the native window.
	CefWindowInfo window_info;

#if defined(OS_WIN)
	CComBSTR url;
	tstring strHomePage;
	pBrowser->get_HomePage(&url);

#ifdef UNICODE
	strHomePage = (LPCTSTR)url;
#else
	strHomePage = T2A((LPCWSTR)url);
#endif // UNICODE

#if defined(_USE_WINDOWLESS)
	long frame_rate = 0;
	pBrowser->get_FrameRate(&frame_rate);
	browser_settings.windowless_frame_rate = frame_rate;
#endif //_USE_WINDOWLESS
	BrowserRenderMode render_mode;
	pBrowser->get_RenderMode(&render_mode);
	switch (render_mode)
	{
	case WindowLess:
		window_info.SetAsWindowless(hParentWnd, true);
		break;
	case AsChild:
		if (hParentWnd)
			window_info.SetAsChild(hParentWnd, rect);
		else
			return E_INVALIDARG;
		break;
	case AsPopup:
	default:
		// On Windows we need to specify certain flags that will be passed to
		// CreateWindowEx().
		window_info.SetAsPopup(hParentWnd, "CefClient");
		break;
	}


#endif //OS_WIN
	// Create the browser window.
	base::AutoLock scopLock(m_lock_);
	ibrowser_map_[IBROWSERCREATE] = pBrowser;
	CefBrowserHost::CreateBrowser(window_info, this, strHomePage, browser_settings, NULL);
	return S_OK;
}