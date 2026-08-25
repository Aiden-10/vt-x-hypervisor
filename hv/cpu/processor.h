#pragma once

#include <ntddk.h>

namespace cpu::processor
{
    ULONG count();

    bool get_number(ULONG processor_index, PROCESSOR_NUMBER* processor_number);

}