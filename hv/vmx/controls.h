#pragma once

#include <ntddk.h>

struct vcpu_t;

namespace controls
{
    ULONG adjust_controls(ULONG control, ULONG msr);
}