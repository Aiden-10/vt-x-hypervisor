#include "allocations.h"

PVOID memory::allocate_contiguous_page()
{
    PHYSICAL_ADDRESS lowest{};
    PHYSICAL_ADDRESS highest{};
    PHYSICAL_ADDRESS boundary{};

    highest.QuadPart = MAXLONGLONG;

    return MmAllocateContiguousMemorySpecifyCache(
        PAGE_SIZE,
        lowest,
        highest,
        boundary,
        MmCached
    );
}

void memory::free_contiguous_page(PVOID address)
{
    if (address)
        MmFreeContiguousMemory(address);
}
