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
* 	CefDisplayHandler methods: ״̬�ı�ص��ӿ�
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
		//PlatformTitleChange(browser, title);
		assert(ibrowser_map_[browser->GetIdentifier()].p);
		{
			CComPtr<IBrowserNotify> spNotify;
			ibrowser_map_[browser->GetIdentifier()]->QueryInterface(IID_IBrowserNotify,(void**)&spNotify);
			CComBSTR str = title.c_str(); 
			if(spNotify) spNotify->OnTitleChange(str);
		}
	}
}

void CefHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{

}

bool CefHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
	return false;
}

void CefHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{

}

/**
* CefLifeSpanHandler methods: ��ҳ�������ڻص��ӿ�
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
		//browser->GetMainFrame()->LoadURL(target_url);
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
	assert(ibrowser_map_[browser->GetIdentifier()].p);
	ibrowser_map_[browser->GetIdentifier()]->put_Identifier(browser->GetIdentifier());
	ibrowser_map_[browser->GetIdentifier()]->put_BrowserRefPoint(reinterpret_cast<IUnknown**>(&p));
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
				ibit->second->RemoveBrowserRef();
				ibrowser_map_.erase(ibit);
			}
			browser_list_.erase(bit);
			break;
		}
	}
	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		//CefQuitMessageLoop();
	}
}

/**
* CefLoadHandler methods: ��ҳ����״̬�ص��ӿ�
*/
void CefHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{

}

void CefHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	//CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
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
	//std::tstring 
	ss << L"<html><body bgcolor=\"white\">"
		L"<h2>Failed to load URL " << tstring(failedUrl) <<
		L" with error " << tstring(errorText) << L" (" << errorCode <<
		L").</h2></body></html>" << '\0';
	frame->LoadString(ss.str(), failedUrl);
}

/**
* CefContextMenuHandler methods: �˵����ؽӿ�
*/
void CefHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, \
	CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	//�������������Լ���Ҫ�Ĳ˵�
	cef_context_menu_type_flags_t flag = params->GetTypeFlags();
	if (flag & CM_TYPEFLAG_PAGE)
	{//��ͨҳ����Ҽ���Ϣ
		model->SetLabel(MENU_ID_BACK, L"����");
		model->SetLabel(MENU_ID_FORWARD, L"ǰ��");
		model->SetLabel(MENU_ID_VIEW_SOURCE, L"�鿴Դ����");
		model->SetLabel(MENU_ID_PRINT, L"��ӡ");
		model->SetLabel(MENU_ID_RELOAD, L"ˢ��");
		model->SetLabel(MENU_ID_RELOAD_NOCACHE, L"ǿ��ˢ��");
		model->SetLabel(MENU_ID_STOPLOAD, L"ֹͣ����");
		model->SetLabel(MENU_ID_REDO, L"�ظ�");
	}
	if (flag & CM_TYPEFLAG_EDITABLE)
	{//�༭����Ҽ���Ϣ
		model->SetLabel(MENU_ID_UNDO, L"����");
		model->SetLabel(MENU_ID_REDO, L"����");
		model->SetLabel(MENU_ID_CUT, L"����");
		model->SetLabel(MENU_ID_COPY, L"����");
		model->SetLabel(MENU_ID_PASTE, L"ճ��");
		model->SetLabel(MENU_ID_DELETE, L"ɾ��");
		model->SetLabel(MENU_ID_SELECT_ALL, L"ȫѡ");
	}
}

bool CefHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, \
	CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
	return false;
}

/**
* CefDownloadHandler methods: ���ع���ӿ�
*/
void CefHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, \
	const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{

}

void CefHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, \
	CefRefPtr<CefDownloadItemCallback> callback)
{
	//ȡ��CEF�ڲ������ļ���ʹ��Ĭ�������������ȥ���أ����ع��̾Ͳ���Ҫ���ǹ����ˣ��Ͼ������������
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
* CefRenderHandler methods:��Ⱦ����
*/

bool CefHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
	CefRect& rect) {
	return false;
}

bool CefHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	RECT rectClid = {0};
	assert(ibrowser_map_[browser->GetIdentifier()].p);
	ibrowser_map_[browser->GetIdentifier()]->get_ViewRect(&rectClid);
	::OutputDebugString(L"CefHandler::GetViewRect\n");
	rect.Set(rectClid.left, rectClid.top, rectClid.right, rectClid.bottom);
	return true;
}

bool CefHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
	int viewX,
	int viewY,
	int& screenX,
	int& screenY) {
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
	::OutputDebugString(L"CefHandler::OnPaint\n");
	assert(ibrowser_map_[browser->GetIdentifier()].p);
	{
		CComPtr<IBrowserNotify> spNotify;
		ibrowser_map_[browser->GetIdentifier()]->QueryInterface(IID_IBrowserNotify,(void**)&spNotify);
		if(spNotify) spNotify->OnRender((const CHAR*)buffer,width,height);
	}
}

void CefHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
	CefCursorHandle cursor,
	CursorType type,
	const CefCursorInfo& custom_cursor_info) {
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
* methods: �Զ���ӿ�
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