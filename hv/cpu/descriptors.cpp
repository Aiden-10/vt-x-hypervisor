// descriptors.cpp

#include "descriptors.h"

#include <intrin.h>

descriptor_table_register_t cpu::read_gdtr()
{
    descriptor_table_register_t gdtr{};

    read_gdtr_asm(&gdtr);

    return gdtr;
}

descriptor_table_register_t cpu::read_idtr()
{
    descriptor_table_register_t idtr{};

    __sidt(&idtr);

    return idtr;
}
