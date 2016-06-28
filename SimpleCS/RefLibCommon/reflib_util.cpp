#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <sstream>
#include "reflib_safelock.h"

void DebugPrint(char *format, ...)
{
#ifdef _DEBUG
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsprintf_s(buffer, 1024, format, args);
    va_end(args);

    std::string msg(buffer);

    std::cout << msg << std::endl;

    OutputDebugStringA(msg.c_str());
#endif
}
