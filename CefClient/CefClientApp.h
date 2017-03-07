#pragma once

#include "include/cef_app.h"

class CefClientApp : 
	public CefApp,
	public CefBrowserProcessHandler
{
public:
	CefClientApp();
	~CefClientApp();

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE
	{ return this; }

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;
private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(CefClientApp);
};

