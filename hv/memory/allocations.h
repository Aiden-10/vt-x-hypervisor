// allocations.h
#pragma once

#include <ntddk.h>

namespace memory
{
    PVOID allocate_contiguous_page();
    void free_contiguous_page(PVOID address);
}