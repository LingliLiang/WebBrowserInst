// WebBrowserInstance.cpp : WinMain µÄÊµÏÖ


#include "stdafx.h"
#include "resource.h"
#include "WebBrowserInstance_i.h"
#include "xdlldata.h"

using namespace ATL;


class CWebBrowserInstanceModule : public ATL::CAtlExeModuleT< CWebBrowserInstanceModule >
{
public :
	DECLARE_LIBID(LIBID_WebBrowserInstanceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WEBBROWSERINSTANCE, "{24DD94C1-17AF-4F94-B209-0AD0BB867CE9}")
public:
	int ClientEntry(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR    lpCmdLine,
		int       nCmdShow)
	{
		//get instance's full path 
		TCHAR lpszPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, lpszPath, MAX_PATH);
		::PathRemoveFileSpec(lpszPath);
		{
			CString dllDirectory = lpszPath;
			dllDirectory.Append(_T("\\cef_resources"));
			::SetDllDirectory(dllDirectory.GetBuffer());
		}

		// Enable High-DPI support on Windows 7 or newer.
		CefEnableHighDPISupport();

		void* sandbox_info = NULL;

		CefMainArgs main_args(hInstance);

		// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
		// that share the same executable. This function checks the command-line and,
		// if this is a sub-process, executes the appropriate logic.
		int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
		if (exit_code >= 0) {
			// The sub-process has completed so return here.
			return exit_code;
		}

		m_ModulePath = lpszPath;
		m_ModulePath.Append(_T("\\"));

		// Specify CEF global settings here.
		CefSettings settings;

		settings.no_sandbox = true;
		//settings.single_process = true;

		{
			CefString str_setting;
			CString strDir = m_ModulePath;
			strDir = m_ModulePath + _T("cef_resources");
			str_setting.Attach(&settings.log_file, false);
			str_setting.FromWString(strDir.GetBuffer());

			//str_setting.Attach(&settings.browser_subprocess_path, false);
			//str_setting.FromWString(strDir.GetBuffer());

			str_setting.Attach(&settings.resources_dir_path, false);
			str_setting.FromWString(strDir.GetBuffer());

			strDir = m_ModulePath + _T("cef_resources/locales");
			str_setting.Attach(&settings.locales_dir_path, false);
			str_setting.FromWString(strDir.GetBuffer());
		}
	
		

#if defined(_USE_WINDOWLESS)
		//enabled off screen rendering
		settings.windowless_rendering_enabled = true;
#endif
		// CefClientApp implements application-level callbacks for the browser process.
		// It will create the first browser instance in OnContextInitialized() after
		// CEF has initialized.
		CefRefPtr<CefClientApp> app(new CefClientApp);

		// Initialize CEF.
		CefInitialize(main_args, settings, app.get(), sandbox_info);
		return -1;
	}

	void RunMessageLoop() throw()
	{
		// Run the CEF message loop. This will block until CefQuitMessageLoop() is
		// called.
		CefRunMessageLoop();

		// Shut down CEF.
		CefShutdown();

		//MSG msg;
		//while (GetMessage(&msg, 0, 0, 0) > 0)
		//{
		//	TranslateMessage(&msg);
		//	DispatchMessage(&msg);
		//}
	}

private:
	CString m_ModulePath;
};

CWebBrowserInstanceModule _AtlModule;

//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
								LPTSTR lpCmdLine, int nShowCmd)
{
	//sub-process exit
	if (_AtlModule.ClientEntry(hInstance,hPrevInstance,lpCmdLine,nShowCmd) >= 0)
		return 0;
	return _AtlModule.WinMain(nShowCmd);
}

