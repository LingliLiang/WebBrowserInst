// BrowserApp.cpp : CBrowserApp ��ʵ��

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
