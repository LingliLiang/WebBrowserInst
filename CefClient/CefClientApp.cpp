#include "StdAfx.h"
#include "CefClientApp.h"
#include "CefHandler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

namespace {

	// When using the Views framework this object provides the delegate
	// implementation for the CefWindow that hosts the Views-based browser.
	class ClientWindowDelegate : 
		public CefWindowDelegate
	{
	public:
		explicit ClientWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
			: browser_view_(browser_view) 
		{
		}

		void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE 
		{
			// Add the browser view and show the window.
			window->AddChildView(browser_view_);
			window->Show();

			// Give keyboard focus to the browser view.
			browser_view_->RequestFocus();
		}

		void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE 
		{
			browser_view_ = NULL;
		}

		bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE 
		{
			// Allow the window to close if the browser says it's OK.
			CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
			if (browser)
				return browser->GetHost()->TryCloseBrowser();
			return true;
		}

	private:
		CefRefPtr<CefBrowserView> browser_view_;

		IMPLEMENT_REFCOUNTING(ClientWindowDelegate);
		DISALLOW_COPY_AND_ASSIGN(ClientWindowDelegate);
	};

}  // namespace


CefClientApp::CefClientApp()
{
}

CefClientApp::~CefClientApp()
{
}


void CefClientApp::OnContextInitialized()
{
	CEF_REQUIRE_UI_THREAD();

	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

#if defined(OS_WIN) || defined(OS_LINUX)
	// Create the browser using the Views framework if "--use-views" is specified
	// via the command-line. Otherwise, create the browser using the native
	// platform framework. The Views framework is currently only supported on
	// Windows and Linux.
	const bool use_views = command_line->HasSwitch("use-views");
#else
	const bool use_views = false;
#endif

	// SimpleHandler implements browser-level callbacks.
	//CefRefPtr<CefHandler> handler(new CefHandler(use_views));


	std::string url;

	// Check if a "--url=" value was provided via the command-line. If so, use
	// that instead of the default URL.
	url = command_line->GetSwitchValue("url");
	if (url.empty())
		url = "https://www.baidu.com";

//	// Specify CEF browser settings here.
//	CefBrowserSettings browser_settings;
//	if (use_views) {
//
//		// Create the BrowserView.
//		CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
//			handler, url, browser_settings, NULL, NULL);
//
//		// Create the Window. It will show itself after creation.
//		CefWindow::CreateTopLevelWindow(new ClientWindowDelegate(browser_view));
//	} else {
//		// Information used when creating the native window.
//		CefWindowInfo window_info;
//
//#if defined(OS_WIN)
//
//#if !defined(_USE_WINDOWLESS)
//		browser_settings.windowless_frame_rate = 30;
//		//::FindWindow(L"WIN32PROJECT1", L"Win32Project1")
//		window_info.SetAsWindowless(NULL,true);
//#else
//		// On Windows we need to specify certain flags that will be passed to
//		// CreateWindowEx().
//		window_info.SetAsPopup(NULL, "CefClient");
//#endif
//
//#endif
//		// Create the first browser window.
//		CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,NULL);
	//}
}
