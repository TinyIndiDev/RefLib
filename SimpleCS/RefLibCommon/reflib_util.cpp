#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <vector>

void DebugPrint(char *format, ...)
{
#ifdef _DEBUG
    va_list args;
    int len;
    std::vector<char> buffer;

    va_start(args, format);
    len = _vscprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    buffer.resize(len, 0x00);
    vsprintf_s(buffer.data(), len, format, args);
    va_end(args);

    std::cout << buffer.data() << std::endl;

    OutputDebugStringA(buffer.data());
#endif
}
