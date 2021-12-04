#include "hypervisor/ept.h"
#include "hypervisor/common.h"
#include "freestanding/x86utils.h"
#include "freestanding/libc.h"

static u8 g_default_memory_type;

typedef union
{
    u8 types[8];
    u64 flags;
} ia32_mtrr_fixed;

typedef struct
{
    bool enabled;
    bool fixed;
    u8 type;
    u64 range_begin;
    u64 range_end;
} mtrr_data;

const auto max_fixed_range_mtrrs = (1 + 2 + 8) * 8;
const auto max_variable_range_mtrrs = 255;
const auto max_mtrrs = max_fixed_range_mtrrs + max_variable_range_mtrrs;

static mtrr_data g_mtrrs[max_mtrrs];

void ept_read_fixed_mtrrs(u64 msr, u64 base, u64* offset, u64 size, u64* index)
{
    auto fixed_range = read_msr<ia32_mtrr_fixed>(msr);

    for (auto mem_type : fixed_range.types)
    {
        auto curr_base = base + *offset;
        *offset += size;

        g_mtrrs[*index].enabled = true;
        g_mtrrs[*index].fixed = true;
        g_mtrrs[*index].range_begin = curr_base;
        g_mtrrs[*index].range_end = curr_base + size - 1;

        (*index)++;
    }
}

void ept_dump_mtrrs()
{
    auto index = 0;

    while (g_mtrrs[index].enabled)
    {
        auto& mtrr = g_mtrrs[index];
        corelog_hex("range start: ", mtrr.range_begin);
        corelog_hex("range end: ", mtrr.range_end);
        index++;
    }
}

void ept_init_mtrr()
{
    corelog("initializing ept mttrs\n");
    memset(g_mtrrs, 0, sizeof(g_mtrrs));

    auto default_type = read_msr<ia32_mtrr_def_type_register>(IA32_MTRR_DEF_TYPE);
    g_default_memory_type = default_type.default_memory_type;

    auto mtrr_capabilities = read_msr<ia32_mtrr_capabilities_register>(IA32_MTRR_CAPABILITIES);

    u64 index = 0;
    if (mtrr_capabilities.fixed_range_supported && default_type.fixed_range_mtrr_enable)
    {
        corelog("reading fixed range mtrrs\n");
        u64 offset = 0;
        ept_read_fixed_mtrrs(IA32_MTRR_FIX64K_00000, 0, &offset, 0x10000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX16K_80000, 0x80000, &offset, 0x4000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX16K_A0000, 0x80000, &offset, 0x4000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_C0000, 0xC0000, &offset, 0x1000, &index);;
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_C8000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_D0000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_D8000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_E0000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_E8000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_F0000, 0xC0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_F8000, 0xC0000, &offset, 0x1000, &index);
    }

    corelog_hex("reading variable mtrrs: ", mtrr_capabilities.variable_range_count);
    for (size_t i = 0; i < mtrr_capabilities.variable_range_count; i++)
    {
        auto phy_mask = read_msr<ia32_mtrr_physmask_register>(
            IA32_MTRR_PHYSMASK0 + i * 2
        );

        auto phy_base = read_msr<ia32_mtrr_physbase_register>(
            IA32_MTRR_PHYSBASE0 + i * 2
        );

        if (! phy_mask.valid) continue;

        auto& mtrr = g_mtrrs[i + index];
        mtrr.enabled = true;
        mtrr.type = phy_base.type;
        mtrr.fixed = false;
        mtrr.range_begin = phy_base.page_frame_number * 0x1000;

        u64 size = 1ull << call_bsf(phy_mask.page_frame_number * 0x1000);

        mtrr.range_end = mtrr.range_begin + size - 1;
    }

    ept_dump_mtrrs();
}

void ept_invalidate_ept_cache(void* ept_ptr)
{
    invept_descriptor descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.ept_pointer = reinterpret_cast<size_t>(ept_ptr);

    call_invept(ept_ptr == nullptr ? invept_type::invept_all_context : invept_type::invept_single_context, &descriptor);
}

void ept_invalidate_vpid_cache(u16 vproc_id)
{
    invvpid_descriptor descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.vpid = vproc_id;

    call_invvpid(vproc_id == 0 ? invvpid_type::invvpid_all_context : invvpid_type::invvpid_single_context, &descriptor);
}
