#include "hypervisor/ept.h"
#include "hypervisor/cpu.h"
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

    //ept_dump_mtrrs();
}

u64 ept_get_memory_type(u64 start, u64 size)
{
    u64 result = MEMORY_TYPE_INVALID;

    for (auto i = 0; i < max_mtrrs; i++)
    {
        auto& mtrr = g_mtrrs[i];

        if (! mtrr.enabled) break;

        if (start < mtrr.range_begin || start >= mtrr.range_end)
        {
            continue;
        }

        if (start + size - 1 >= mtrr.range_end)
        {
            return MEMORY_TYPE_INVALID;
        }

        if (mtrr.fixed)
        {
            return mtrr.type;
        }

        if (mtrr.type == MEMORY_TYPE_UNCACHEABLE)
        {
            return MEMORY_TYPE_UNCACHEABLE;
        }

        if (mtrr.type == MEMORY_TYPE_WRITE_THROUGH && result == MEMORY_TYPE_WRITE_BACK)
        {
            result = MEMORY_TYPE_WRITE_THROUGH;
        }
        else if (mtrr.type == MEMORY_TYPE_WRITE_BACK && result == MEMORY_TYPE_WRITE_THROUGH)
        {
            result = MEMORY_TYPE_WRITE_THROUGH;
        }
        else
        {
            result = mtrr.type;
        }
    }

    if (result == MEMORY_TYPE_INVALID)
    {
        return g_default_memory_type;
    }

    return result;
}

void ept_init_core(cpu_state* state)
{
    corelog("setting up ept\n");

    auto& structs = state->core_ept;

    memset(&structs.pml4, 0, sizeof(ept_pml4));
    memset(&structs.pdpt, 0, sizeof(epdpte) * 512);
    memset(&structs.pde, 0, sizeof(epde_2mb) * 512);

    structs.pml4.read_access = true;
    structs.pml4.write_access = true;
    structs.pml4.execute_access = true;
    structs.pml4.page_frame_number = reinterpret_cast<u64>(structs.pdpt) >> 12;

    u64 old_mem_type = 0; // debug var
    u64 current_pt_index = 0;
    for (size_t pdpt_i = 0; pdpt_i < 512; pdpt_i++)
    {
        auto& pdpt = structs.pdpt[pdpt_i];

        pdpt.read_access = true;
        pdpt.write_access = true;
        pdpt.execute_access = true;
        pdpt.page_frame_number = reinterpret_cast<u64>(structs.pde[pdpt_i]) >> 12;

        for (size_t pde_i = 0; pde_i < 512; pde_i++)
        {
            auto pde_big = reinterpret_cast<epde_2mb*>(&structs.pde[pdpt_i][pde_i]);
            auto addr = pt_to_address(0, pdpt_i, pde_i, 0);

            auto mem_type = ept_get_memory_type(addr, 0x200000);

            if (mem_type != old_mem_type)
            {
                old_mem_type = mem_type;
                corelog_hex("different memory type: ", mem_type);
            }

            pde_big->read_access = true;
            pde_big->write_access = true;
            pde_big->execute_access = true;

            if (mem_type != MEMORY_TYPE_INVALID)
            {
                pde_big->large_page = true;
                pde_big->memory_type = mem_type;
                pde_big->page_frame_number = addr >> 12;
            }
            else
            {
                corelog("splitting pde\n");
                auto pde = reinterpret_cast<epde*>(pde_big);

                pde->page_frame_number = reinterpret_cast<size_t>(structs.pt[pdpt_i][pde_i]) >> 12;

                for (size_t pt_i = 0; pt_i < 512; pt_i++)
                {
                    auto& pt = structs.pt[pdpt_i][pde_i][pt_i];

                    pt.read_access = true;
                    pt.write_access = true;
                    pt.execute_access = true;

                    auto addr = pt_to_address(0, pdpt_i, pde_i, pt_i);
                    pt.memory_type = ept_get_memory_type(addr, 0x1000);
                    pt.page_frame_number = addr >> 12;

                    if (pt.memory_type == MEMORY_TYPE_INVALID)
                    {
                        corelog("ERR in ept: fucked up pt entries!!!\n");
                    }
                }
            }
        }
    }

    structs.ptr.flags = 0;
    structs.ptr.memory_type = MEMORY_TYPE_WRITE_BACK;
    structs.ptr.page_walk_length = EPT_PAGE_WALK_LENGTH_4;
    structs.ptr.page_frame_number = reinterpret_cast<u64>(&structs.pml4) >> 12;
}

// u64 ept_get_ptr(cpu_state* state)
// {
//     return reinterpret_cast<u64>(&state->core_ept.ptr);
// }

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
