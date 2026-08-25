#include "segments.h"

#include "../common/logging.h"

constexpr ULONG32 SEGMENT_UNUSABLE = 1u << 16;


//
// Returns true if the selector is a null selector.
//
// Null selectors are valid for some segment registers and should be
// represented as unusable when preparing VMCS guest segment state.
//
bool is_null_selector(USHORT selector)
{
    return (selector & 0xFFF8) == 0;
}

//
// Returns true if the selector references the Local Descriptor Table.
//
// This decoder currently supports descriptors from the GDT only, so
// selectors with TI=1 are rejected by the segment decoding helpers.
//
bool uses_ldt(USHORT selector)
{
    return (selector & 0x4) != 0;
}

//
// Converts a segment selector into the byte offset of its descriptor
// within the descriptor table.
//
// The selector index occupies bits 15:3 and each GDT entry is 8 bytes.
//
ULONG32 descriptor_offset(USHORT selector)
{
    return static_cast<ULONG32>(selector >> 3) * 8;
}

//
// Initializes a segment state as unusable.
//
// Used for valid null selectors such as an unused LDTR so the resulting
// state can be written directly into the corresponding VMCS fields.
//
void make_unusable(USHORT selector, segment_state_t* segment)
{
    segment->selector = selector;
    segment->base = 0;
    segment->limit = 0;
    segment->access_rights = SEGMENT_UNUSABLE;
}

//
// Decodes a normal 8-byte GDT descriptor into the format required by
// the VMCS guest segment fields.
//
// Used when initializing guest CS, SS, DS, ES, FS, and GS state before
// entering VMX non-root operation.
//
bool decode_segment(USHORT selector, ULONG64 gdt_base, USHORT gdt_limit, segment_state_t* segment)
{
    if (!segment || gdt_base == 0)
        return false;

    *segment = {};
    segment->selector = selector;

    // A null selector means this segment is not being used.
    // Mark it unusable so it can be written into the VMCS correctly.
    if (is_null_selector(selector))
    {
        make_unusable(selector, segment);
        return true;
    }

    // We only support descriptors stored in the GDT here.
    // If the TI bit is set, the selector points into the LDT instead.
    if (uses_ldt(selector))
        return false;

    const ULONG32 offset = descriptor_offset(selector);

    // A normal segment descriptor is 8 bytes.
    // Make sure the whole descriptor fits inside the GDT.
    if (offset > gdt_limit ||
        static_cast<ULONG64>(offset) + 7 > gdt_limit)
    {
        return false;
    }

    const ULONG64 descriptor_address = gdt_base + offset;

    const ULONG64 descriptor = *reinterpret_cast<const volatile ULONG64*>(descriptor_address);

    // Rebuild the segment base address from the three base fields
    // stored in different parts of the descriptor.
    segment->base =
        ((descriptor >> 16) & 0xFFFFull) |
        (((descriptor >> 32) & 0xFFull) << 16) |
        (((descriptor >> 56) & 0xFFull) << 24);

    // Rebuild the 20-bit segment limit from the descriptor.
    ULONG32 limit =
        static_cast<ULONG32>(descriptor & 0xFFFFull) |
        (static_cast<ULONG32>((descriptor >> 48) & 0xFull) << 16);

    // If granularity is enabled, the limit is measured in 4 KB pages
    // instead of bytes, so convert it into the actual byte limit.
    if (((descriptor >> 55) & 1ull) != 0)
        limit = (limit << 12) | 0xFFF;

    segment->limit = limit;

    // Copy the descriptor access/type flags into the layout
    // expected by the VMCS guest access-rights field.
    segment->access_rights =
        static_cast<ULONG32>((descriptor >> 40) & 0xFFull) |
        (static_cast<ULONG32>((descriptor >> 52) & 0xFull) << 12);

    segment->access_rights |= 1;

    return true;
}

//
// Decodes a 16-byte IA-32 system descriptor into VMCS segment state.
//
// Used for system segments such as the Task Register (TR) and
// LDTR, whose descriptors may contain a 64-bit base address.
//
bool decode_system_segment(USHORT selector, ULONG64 gdt_base, USHORT gdt_limit, segment_state_t* segment)
{
    if (!segment || gdt_base == 0)
        return false;

    *segment = {};
    segment->selector = selector;

    // A null system-segment selector means the segment is unused.
    if (is_null_selector(selector))
    {
        make_unusable(selector, segment);
        return true;
    }

    // This only reads system descriptors from the GDT.
    if (uses_ldt(selector))
        return false;

    const ULONG32 offset = descriptor_offset(selector);

    // In 64-bit mode, system descriptors such as the TSS descriptor
    // use 16 bytes instead of the normal 8 bytes.
    if (offset > gdt_limit ||
        static_cast<ULONG64>(offset) + 15 > gdt_limit)
    {
        return false;
    }

    const ULONG64 address = gdt_base + offset;

    // Read both 8-byte halves of the 16-byte system descriptor.
    const ULONG64 low = *reinterpret_cast<const volatile ULONG64*>(address);

    const ULONG64 high = *reinterpret_cast<const volatile ULONG64*>(address + 8);

    // Rebuild the full 64-bit base address.
    // The upper 32 bits come from the second half of the descriptor.
    segment->base =
        ((low >> 16) & 0xFFFFull) |
        (((low >> 32) & 0xFFull) << 16) |
        (((low >> 56) & 0xFFull) << 24) |
        ((high & 0xFFFFFFFFull) << 32);

    // Rebuild the 20-bit segment limit.
    ULONG32 limit =
        static_cast<ULONG32>(low & 0xFFFFull) |
        (static_cast<ULONG32>((low >> 48) & 0xFull) << 16);

    // Convert a page-granularity limit into its actual byte limit.
    if (((low >> 55) & 1ull) != 0)
        limit = (limit << 12) | 0xFFF;

    segment->limit = limit;

    // Extract the descriptor flags into the format expected by the VMCS.
    segment->access_rights =
        static_cast<ULONG32>((low >> 40) & 0xFFull) |
        (static_cast<ULONG32>((low >> 52) & 0xFull) << 12);

    segment->access_rights |= 1;

    return true;
}

//
// Logs the decoded state of a segment.
//
// Used while debugging VMCS guest-state setup to verify that selectors,
// bases, limits, and access-right fields were decoded correctly before
// VMLAUNCH.
//
void dump_segment(const char* name, const segment_state_t& s)
{
    log::printf(
        "[HV] %s SEL=%hx BASE=%llx LIMIT=%x AR=%x "
        "TYPE=%x S=%u DPL=%u P=%u L=%u DB=%u G=%u U=%u\n",
        name,
        s.selector,
        s.base,
        s.limit,
        s.access_rights,
        s.access_rights & 0xF,
        (s.access_rights >> 4) & 1,
        (s.access_rights >> 5) & 3,
        (s.access_rights >> 7) & 1,
        (s.access_rights >> 13) & 1,
        (s.access_rights >> 14) & 1,
        (s.access_rights >> 15) & 1,
        (s.access_rights >> 16) & 1
    );
}
