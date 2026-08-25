#include "controls.h"

#include <ntddk.h>
#include <intrin.h>

ULONG controls::adjust_controls(ULONG control, ULONG msr)
{
    const ULONG64 value = __readmsr(msr);

    control &= static_cast<ULONG>(value >> 32);
    control |= static_cast<ULONG>(value);

    return control;
}
