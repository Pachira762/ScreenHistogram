#pragma once

//#define USE_DIRECTCOMPOSITION
#define USE_WINRTCOMPOSITION

#include <memory>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <thread>
#include <atomic>
#include <functional>
#include <algorithm>

#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <d3dcompiler.h>
#include <Unknwn.h>
#include <inspectable.h>
#include <winrt\base.h>

#ifdef USE_DIRECTCOMPOSITION
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")
#elif defined USE_WINRTCOMPOSITION
#include <DispatcherQueue.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#endif

#include "util.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowsapp.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")