#pragma once
#include "include/cef_client.h"
#include "include/base/cef_lock.h"

#include <string>
#include <list>

typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;

class CefHandler
	:public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefContextMenuHandler,
	public CefDownloadHandler,
	public CefRenderHandler
{
public:
	explicit CefHandler(bool use_views);
	CefHandler(const tstring& strUrl=TEXT(""));
	virtual ~CefHandler();
	//自定义方法
	CefRefPtr<CefBrowser> GetBrowser() { return m_pBrowser; }
	CefRefPtr<CefFrame>	GetMainFram() { return m_pBrowser.get()?m_pBrowser->GetMainFrame():NULL; }
	HWND	GetBrowserHostWnd() { return m_pBrowser.get()?m_pBrowser->GetHost()->GetWindowHandle():NULL; }
	void	SetHomePage(const tstring& strUrl) { m_strHomePage=strUrl; }
	const tstring& GetHomePage()const { return m_strHomePage; }
	tstring GetLoadingUrl();
	void	Navigate(const tstring& strUrl);

	void	CreateBrowser(HWND hParentWnd, const RECT& rect);

	// Provide access to the single global instance of this object.
	static CefHandler* GetInstance();


	//凡是继承了的处理功能都在这里返回this指针
	virtual CefRefPtr<CefDisplayHandler>		GetDisplayHandler()		{ return this; }
	virtual CefRefPtr<CefLifeSpanHandler>		GetLifeSpanHandler()	{ return this; }
	virtual CefRefPtr<CefLoadHandler>			GetLoadHandler()		{ return this; }
	virtual CefRefPtr<CefContextMenuHandler>	GetContextMenuHandler()	{ return this; }
	virtual CefRefPtr<CefDownloadHandler>		GetDownloadHandler()	{ return this; }
	virtual CefRefPtr<CefRenderHandler>		GetRenderHandler() { return this; }
	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url);
	virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text);
	virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value);

	// CefLifeSpanHandler methods:
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		CefLifeSpanHandler::WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access);
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser);
	virtual bool DoClose(CefRefPtr<CefBrowser> browser);
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser);

	// CefLoadHandler methods:
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type);
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl);

	//CefRequestHandler methods:
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request, bool is_redirect) 
	{
		return false;
	}

	virtual ReturnValue OnBeforeResourceLoad(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefRequestCallback> callback) 
	{
		return RV_CONTINUE;
	}

	// CefContextMenuHandler methods:
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model);

	virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags);

	// CefDownloadHandler methods:
	virtual void OnBeforeDownload( CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, 
		const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback);

	virtual void OnDownloadUpdated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
		CefRefPtr<CefDownloadItemCallback> callback);

	// Called to retrieve the view rectangle which is relative to screen
	// coordinates. Return true if the rectangle was provided.
	///
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect);

	///
	// Called when an element should be painted. Pixel values passed to this
	// method are scaled relative to view coordinates based on the value of
	// CefScreenInfo.device_scale_factor returned from GetScreenInfo. |type|
	// indicates whether the element is the view or the popup widget. |buffer|
	// contains the pixel data for the whole image. |dirtyRects| contains the set
	// of rectangles in pixel coordinates that need to be repainted. |buffer| will
	// be |width|*|height|*4 bytes in size and represents a BGRA image with an
	// upper-left origin.
	///
	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
		PaintElementType type,
		const RectList& dirtyRects,
		const void* buffer,
		int width, int height);

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool	IsClosing() const 
	{ 
		return m_bIsClose; 
	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message);
private:
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;
	CefRefPtr<CefBrowser>	m_pBrowser;
	bool	m_bIsClose;

	// True if the application is using the Views framework.
	const bool m_bUseViews;

	base::Lock m_lock_; //base::AutoLock lock_scope(m_lock_);

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(CefHandler);

private:
	tstring	m_strHomePage;
	static int	m_nBrowserCount;
};

