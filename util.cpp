#include "util.h"

#ifdef _DEBUG
void DLOG_(const wchar_t* fmt, ...)
{
	static wchar_t buff[1024 * 8] = {};

	va_list ap;
	va_start(ap, fmt);
	vswprintf_s(buff, fmt, ap);
	va_end(ap);
	OutputDebugString(buff);
}
#else
#endif