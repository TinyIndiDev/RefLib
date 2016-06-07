#include "stdafx.h"

void DebugPrint(char *format, ...)
{
#ifdef DEBUG
    va_list vl;
    char    dbgbuf[2048];

    if (pid == 0)
    {
        pid = GetCurrentProcessId();
    }

    va_start(vl, format);
    wvsprintf(dbgbuf, format, vl);
    va_end(vl);

    OutputDebugString(dbgbuf);
#endif
}
