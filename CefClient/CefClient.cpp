#include "StdAfx.h"
#include "CefClient.h"

#include "CefHandler.h"

#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "CefClientApp.h"

#ifdef MDd
#pragma comment(lib,"libcef_dll_wrapper_mtd.lib")
#else
#pragma comment(lib,"libcef_dll_wrapper.lib")
#endif
#pragma comment(lib,"libcef.lib")

HINSTANCE g_hInstance_ = NULL;

 int ClientEntry(HINSTANCE hInstance,
	 HINSTANCE hPrevInstance,
	 LPTSTR    lpCmdLine,
	 int       nCmdShow)
{
	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	void* sandbox_info = NULL;
	//// Parse command-line arguments.
	//CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	//command_line->InitFromString(::GetCommandLineW());

	CefMainArgs main_args(hInstance);

	// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
	// that share the same executable. This function checks the command-line and,
	// if this is a sub-process, executes the appropriate logic.
	int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return exit_code;
	}

	// Specify CEF global settings here.
	CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	//settings.single_process = true;

	// CefClientApp implements application-level callbacks for the browser process.
	// It will create the first browser instance in OnContextInitialized() after
	// CEF has initialized.
	CefRefPtr<CefClientApp> app(new CefClientApp);

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), sandbox_info);

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
	CefRunMessageLoop();

	// Shut down CEF.
	CefShutdown();

	return 0;
}
