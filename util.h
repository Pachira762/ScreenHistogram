#pragma once
#include <stdarg.h>
#include <Windows.h>
#include <winrt/base.h>

#ifdef _DEBUG
void	DLOG_(const wchar_t* fmt, ...);
#define DLOG(...) DLOG_(__VA_ARGS__)
#else
#define DLOG __noop
#endif

template<typename Ty>
std::tuple<Ty, DWORD> LoadResourcePtr(LPCTSTR name, LPCTSTR type)
{
    auto instance = GetModuleHandle(NULL);
    auto resource = FindResource(instance, name, type);
    winrt::check_pointer(resource);

    auto handle = LoadResource(instance, resource);
    winrt::check_pointer(handle);

    auto size = SizeofResource(instance, resource);

    return { reinterpret_cast<Ty>(LockResource(handle)), size };
}
