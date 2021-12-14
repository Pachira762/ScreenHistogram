#include "pch.h"
#include "App.h"
#include <wrl.h>

using namespace std;
using namespace winrt;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdLine, int cmdShow)
{
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
