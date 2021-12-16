#include "App.h"
#include <wrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowsapp.lib")

using namespace std;
using namespace winrt;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdLine, int cmdShow)
{
    InitCommonControls();
	init_apartment(apartment_type::single_threaded);

    App app{};
    if (!app.Initialize()) {
        return false;
    }

    MSG msg;
    while(true)
    {
        BOOL br = GetMessage(&msg, NULL, 0, 0);
        if (br == 0 || br == -1) {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}