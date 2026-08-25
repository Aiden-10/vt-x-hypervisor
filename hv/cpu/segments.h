#pragma once

#include <ntddk.h>

struct segment_state_t
{
    USHORT selector;
    ULONG64 base;
    ULONG32 limit;
    ULONG32 access_rights;
};

bool decode_segment(USHORT selector, ULONG64 gdt_base, USHORT gdt_limit, segment_state_t* segment);

bool decode_system_segment(USHORT selector, ULONG64 gdt_base, USHORT gdt_limit, segment_state_t* segment);

void dump_segment(const char* name, const segment_state_t& s);

extern "C" USHORT read_cs_asm();
extern "C" USHORT read_ss_asm();
extern "C" USHORT read_ds_asm();
extern "C" USHORT read_es_asm();
extern "C" USHORT read_fs_asm();
extern "C" USHORT read_gs_asm();
extern "C" USHORT read_tr_asm();
extern "C" USHORT read_ldtr_asm();