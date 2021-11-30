#pragma once

#include "freestanding/types.h"
#include "external/ia32-doc/out/ia32.hpp"

#define asmapi extern "C"

u64 pt_to_address(u32 pml4, u32 pdpt, u32 pd, u32 pt);

asmapi segment_descriptor_register_64* __attribute__((sysv_abi)) read_cr2();
asmapi segment_descriptor_register_64* __attribute__((sysv_abi)) read_cr3();
asmapi void __attribute__((sysv_abi)) read_gdtr(segment_descriptor_register_64* result);
asmapi void __attribute__((sysv_abi)) read_idtr(segment_descriptor_register_64* result);

asmapi void __attribute__((sysv_abi)) write_cr2(segment_descriptor_register_64* new_cr2);
asmapi void __attribute__((sysv_abi)) write_cr3(segment_descriptor_register_64* new_cr3);
asmapi void __attribute__((sysv_abi)) write_gdtr(segment_descriptor_register_64* new_gdtr);
asmapi void __attribute__((sysv_abi)) write_idtr(segment_descriptor_register_64* new_idtr);
