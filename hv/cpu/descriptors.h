// descriptors.h

#pragma once

#include <ntddk.h>

#pragma pack(push, 1)
struct descriptor_table_register_t
{
    USHORT limit;
    ULONG64 base;
};
#pragma pack(pop)

extern "C" void read_gdtr_asm(descriptor_table_register_t* gdtr);

namespace cpu
{
    descriptor_table_register_t read_gdtr();
    descriptor_table_register_t read_idtr();
}