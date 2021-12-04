#pragma once

#include "freestanding/x86utils.h"

typedef struct
{
    size_t status_code;
    size_t core_index;
    do_align(4096) vmxon core_vmxon;
    do_align(4096) vmcs core_vmcs;
    do_align(4096) char host_stack[0xFFFF];
} cpu_state;

#define cpu_max 8
extern cpu_state g_cpu_states[cpu_max];

size_t cpu_init();
u64 cpu_vmxread(u64 field);
