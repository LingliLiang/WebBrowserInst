
//..\..\cef_binary_3.2924.1566.ga6a3da2_windows64;
//..\..\cef_binary_3.2924.1566.ga6a3da2_windows64\debug\;

//  �� Windowsx ͷ�ļ����ų��궨��,������cef_dom.h/cef_window.h��ͻ
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
