#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>

#define LOG_TEXT_MAX_LEN    2048

void DebugPrint(char *format, ...)
{
#ifdef _DEBUG
    va_list vl;
    char dbgbuf[LOG_TEXT_MAX_LEN];

    va_start(vl, format);
    vsprintf_s(dbgbuf, LOG_TEXT_MAX_LEN, format, vl);
    va_end(vl);

    OutputDebugStringA(dbgbuf);
#endif
}
