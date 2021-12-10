#pragma once

#include "freestanding/types.h"
#include "external/ia32-doc/out/ia32.hpp"

#define asmapi extern "C" __attribute__((sysv_abi))

u64 pt_to_address(u32 pml4, u32 pdpt, u32 pd, u32 pt);
u64 get_segment_base(u64 descriptor_table_base, u16 segment_selector);
u16 get_segment_limit(u16 segment_selector);
u32 get_segment_access_rights_vmx(u16 selector);

size_t get_current_elf_base();

asmapi size_t read_cr0();
asmapi void write_cr0(size_t new_cr0);

asmapi size_t read_cr1();
asmapi void write_cr1(size_t new_cr1);

asmapi size_t read_cr2();
asmapi void write_cr2(size_t new_cr2);

asmapi size_t read_cr3();
asmapi void write_cr3(size_t new_cr3);

asmapi size_t read_cr4();
asmapi void write_cr4(size_t new_cr4);

asmapi size_t read_dr7();
asmapi void write_dr7(size_t new_dr7);

asmapi u16 read_es();
asmapi void write_es(u16 new_es);

asmapi u16 read_cs();
asmapi void write_cs(u16 new_es);

asmapi u16 read_ss();
asmapi void write_ss(u16 new_es);

asmapi u16 read_ds();
asmapi void write_ds(u16 new_es);

asmapi u16 read_fs();
asmapi void write_fs(u16 new_es);

asmapi u16 read_gs();
asmapi void write_gs(u16 new_es);

asmapi size_t read_msr(u32 msr_id);
asmapi void write_msr(u32 msr_id, u64 value);

asmapi u64 read_rflags();

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

asmapi u16 read_ldtr();

asmapi void set_cs(u64 selector);
asmapi u64 call_bsf(u64 value);

asmapi u64 get_rsp();
asmapi u64 get_rip();

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

asmapi u64 call_lsl(u16 selector);
asmapi u64 call_lar(u16 selector);

asmapi size_t call_vmxon(void* vmxon);
asmapi size_t call_vmclear(void* vmcs);
asmapi size_t call_vmptrld(void* vmcs);
asmapi size_t call_vmread(u64 field, u64* value);
asmapi size_t call_vmwrite(u64 field, u64 value);
asmapi size_t call_vmlaunch();

asmapi size_t call_invept(invept_type type, const invept_descriptor* descriptor);
asmapi size_t call_invvpid(invvpid_type type, const invvpid_descriptor* descriptor);
