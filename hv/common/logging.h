// logging.h
#pragma once

#include <ntddk.h>
#include <stdarg.h>

namespace log
{
    inline void printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        vDbgPrintExWithPrefix(
            "[HV] ",
            DPFLTR_IHVDRIVER_ID,
            DPFLTR_INFO_LEVEL,
            format,
            args
        );

        va_end(args);
    }

    inline void error(const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        vDbgPrintExWithPrefix(
            "[HV][Error] ",
            DPFLTR_IHVDRIVER_ID,
            DPFLTR_INFO_LEVEL,
            format,
            args
        );

        va_end(args);
    }
}