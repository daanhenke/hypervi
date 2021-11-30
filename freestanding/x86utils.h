#pragma once

#include "freestanding/types.h"
#include "external/ia32-doc/out/ia32.hpp"

#define asmapi extern "C" __attribute__((sysv_abi))

u64 pt_to_address(u32 pml4, u32 pdpt, u32 pd, u32 pt);

asmapi segment_descriptor_register_64* read_cr2();
asmapi void write_cr2(segment_descriptor_register_64* new_cr2);

asmapi segment_descriptor_register_64* read_cr3();
asmapi void write_cr3(segment_descriptor_register_64* new_cr3);

asmapi size_t read_msr(u32 msr_id);

template <typename T>
T read_msr(u32 msr_id)
{
    size_t raw = read_msr(msr_id);
    T val;
    val.flags = raw;
    return val;
}

asmapi void read_gdtr(segment_descriptor_register_64* result);
asmapi void write_gdtr(segment_descriptor_register_64* new_gdtr);

asmapi void read_idtr(segment_descriptor_register_64* result);
asmapi void write_idtr(segment_descriptor_register_64* new_idtr);

asmapi u16 read_tr();
asmapi void write_tr(u16 new_idtr);
asmapi void clear_tr();

asmapi void set_cs(u64 selector);

typedef union
{
    struct
    {
        u32 eax;
        u32 ebx;

        union
        {
            u32 ecx;
            struct
            {
                u32 sse3: 1;
                u32 pclmulqdq: 1;
                u32 dtes64: 1;
                u32 monitor: 1;
                u32 ds_cpl: 1;
                u32 vmx: 1;
            };
        };

        u32 edx;
    };

    char as_string[12];
} cpuid_t;

asmapi void call_cpuid(cpuid_t* regs);
