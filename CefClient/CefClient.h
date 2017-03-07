
//..\..\cef_binary_3.2924.1566.ga6a3da2_windows64;
//..\..\cef_binary_3.2924.1566.ga6a3da2_windows64\debug\;

//  从 Windowsx 头文件中排除宏定义,避免与cef_dom.h/cef_window.h冲突
#ifdef  GetNextSibling
#undef  GetNextSibling
#endif
#ifdef  GetFirstChild
#undef  GetFirstChild
#endif
#ifdef  IsMaximized
#undef  IsMaximized
#endif
#ifdef  IsMinimized
#undef  IsMinimized
#endif

extern HINSTANCE g_hInstance_;

int ClientEntry(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
