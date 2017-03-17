// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <atlctl.h>
#include <atltypes.h>
#include <atlstr.h>
using namespace ATL;
#include <vector>

#define _USE_WINDOWLESS

#include "CefClient.h"
#include "CefHandler.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "CefClientApp.h"

#define WBI_PCHECK(pointer) if(!pointer) return E_POINTER
#define WBI_REFCHECK(ref) if(!ref.get()) return E_POINTER
#define WBI_PREFCHECK(pointerref) if(!pointerref || !((*pointerref).get())) return E_POINTER