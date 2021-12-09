#include "pch.h"
#include "App.h"

using namespace std;
using namespace winrt;

struct Piyo
{
    int value;
    Piyo()
    {
        DLOG(L"piyo\n");
    }
    ~Piyo()
    {
        DLOG(L"~Piyo\n");
    }
};

struct Hoge
{
    Piyo value;

    ~Hoge() {
        DLOG(L"Destructor\n");
    }

    Piyo& GetValue() {
        return value;
    }
};

void Func(const Piyo& value)
{
    DLOG(L"%d %08X\n", value.value, &value.value);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdLine, int cmdShow)
{
	init_apartment(apartment_type::single_threaded);

    Func(Hoge().GetValue());

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
