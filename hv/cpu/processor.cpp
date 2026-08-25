#include "processor.h"

#include "../common/logging.h"

ULONG cpu::processor::count()
{
	return KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
}

bool cpu::processor::get_number(ULONG processor_index, PROCESSOR_NUMBER* processor_number)
{
    if (!processor_number)
        return false;

    return (KeGetProcessorNumberFromIndex(processor_index, processor_number) >= 0);
}
