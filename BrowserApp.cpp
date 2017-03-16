// BrowserApp.cpp : CBrowserApp µÄÊµÏÖ

#include "stdafx.h"
#include "BrowserApp.h"


// CBrowserApp



STDMETHODIMP CBrowserApp::QuitApp()
{
	// TODO: Add your implementation code here
	CefQuitMessageLoop();
	Fire_Exit();
	return S_OK;
}
