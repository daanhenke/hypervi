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

mtrr_data g_mtrrs[max_mtrrs];

void ept_read_fixed_mtrrs(u64 msr, u64 base, u64* offset, u64 size, u64* index)
{
    auto fixed_range = read_msr<ia32_mtrr_fixed>(msr);

    for (u64 i = 0; i < 8; i++)
    {
        auto mem_type = fixed_range.types[i];
        auto curr_base = base + (size * i);
        *offset += size;

        u64 real_idx = *index;
        g_mtrrs[real_idx].enabled = 1;
        g_mtrrs[real_idx].fixed = 1;
        g_mtrrs[real_idx].type = mem_type;
        g_mtrrs[real_idx].range_begin = curr_base;
        g_mtrrs[real_idx].range_end = curr_base + size;

        (*index)++;
    }
}

void ept_dump_mtrrs()
{
    auto index = 0;

    corelog_hex("dumping mtrrs\n", g_mtrrs[0].range_begin);
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
        ept_read_fixed_mtrrs(IA32_MTRR_FIX16K_A0000, 0xA0000, &offset, 0x4000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_C0000, 0xC0000, &offset, 0x1000, &index);;
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_C8000, 0xC8000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_D0000, 0xD0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_D8000, 0xD8000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_E0000, 0xE0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_E8000, 0xE8000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_F0000, 0xF0000, &offset, 0x1000, &index);
        ept_read_fixed_mtrrs(IA32_MTRR_FIX4K_F8000, 0xF8000, &offset, 0x1000, &index);
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

        auto* mtrr = &g_mtrrs[index++];
        mtrr->enabled = true;
        mtrr->type = phy_base.type;
        mtrr->fixed = false;
        mtrr->range_begin = phy_base.page_frame_number * 0x1000;

        u64 size = 1ull << call_bsf(phy_mask.page_frame_number);
        size = size << 12;

        mtrr->range_end = mtrr->range_begin + size;
    }

    g_mtrrs[index].enabled = 0;

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

    // memset(&structs.pml4, 0, sizeof(ept_pml4));
    // memset(&structs.pdpt, 0, sizeof(epdpte) * 512);
    // memset(&structs.pde, 0, sizeof(epde_2mb) * 512);
    auto pml4 = reinterpret_cast<ept_pml4*>(ldr_malloc(0x1000));
    corelog_hex("pml4 allocated @ ", reinterpret_cast<u64>(pml4));
    memset(pml4, 0, 0x1000);

    auto pdpts = reinterpret_cast<epdpte*>(ldr_malloc(0x1000));
    corelog_hex("pdpts allocated @ ", reinterpret_cast<u64>(pdpts));
    memset(structs.pdpt, 0, 0x1000);

    pml4->flags = 0;
    pml4->read_access = true;
    pml4->write_access = true;
    pml4->execute_access = true;
    pml4->page_frame_number = reinterpret_cast<u64>(pdpts) >> 12;
    for (size_t pdpt_i = 0; pdpt_i < 512; pdpt_i++)
    {
        auto pdpt = &pdpts[pdpt_i];

        auto pde = reinterpret_cast<epde*>(ldr_malloc(0x1000));
        memset(pde, 0, 0x1000);

        pdpt->flags = 0;
        pdpt->read_access = true;
        pdpt->write_access = true;
        pdpt->execute_access = true;
        pdpt->page_frame_number = reinterpret_cast<u64>(pde) >> 12;

        for (size_t pde_i = 0; pde_i < 512; pde_i++)
        {
            auto pde_big = reinterpret_cast<epde_2mb*>(&pde[pde_i]);
            auto addr = pt_to_address(0, pdpt_i, pde_i, 0);

            if (pde_i == 0x3E)
            {
                //corelog_hex("pd addr @ ", addr);
            }

            auto mem_type = ept_get_memory_type(addr, 0x200000);

            pde_big->flags = 0;
            pde_big->read_access = true;
            pde_big->write_access = true;
            pde_big->execute_access = true;

            if (mem_type != MEMORY_TYPE_INVALID)
            {
                pde_big->large_page = true;
                pde_big->memory_type = mem_type;
                pde_big->page_frame_number = (addr >> 12);
            }
            else
            {
                corelog("splitting pde\n");

                auto pts = reinterpret_cast<epte*>(ldr_malloc(0x1000));
                memset(pts, 0, 0x1000);

                pde->flags = 0;
                pde->page_frame_number = reinterpret_cast<u64>(pts) >> 12;
                pde->read_access = 1;
                pde->write_access = 1;
                pde->execute_access = 1;

                for (size_t pt_i = 0; pt_i < 512; pt_i++)
                {
                    auto pt = &pts[pt_i];

                    pt->flags = 0;
                    pt->read_access = true;
                    pt->write_access = true;
                    pt->execute_access = true;

                    auto addr = pt_to_address(0, pdpt_i, pde_i, pt_i);
                    pt->memory_type = ept_get_memory_type(addr, 0x1000);
                    pt->page_frame_number = addr >> 12;
                    //corelog_hex("pt for mem @ ", pt->page_frame_number);

                    if (pt->memory_type == MEMORY_TYPE_INVALID)
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
    structs.ptr.page_frame_number = reinterpret_cast<u64>(pml4) >> 12;
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


void ept_resolve_entries(void* address, ept_pml4** pml4, epdpte** pdpt, epde** pd, epte** pt)
{
    u32 pml4_i, pdpt_i, pd_i, pt_i;
    address_to_pt(address, &pml4_i, &pdpt_i, &pd_i, &pt_i);

    ept_pointer ept_ptr;
    ept_ptr.flags = cpu_vmxread(VMCS_CTRL_EPT_POINTER);

    auto pml4e = &reinterpret_cast<ept_pml4*>(ept_ptr.page_frame_number << 12)[pml4_i];
    if (pml4 != nullptr)
    {
        *pml4 = pml4e;
    }

    auto pdpte = &reinterpret_cast<epdpte*>(pml4e->page_frame_number << 12)[pdpt_i];
    if (pdpt != nullptr)
    {
        *pdpt = pdpte;
    }

    auto pde = &reinterpret_cast<epde*>(pdpte->page_frame_number << 12)[pd_i];
    if (pd != nullptr)
    {
        *pd = pde;
    }

    auto pde_big = reinterpret_cast<epde_2mb*>(pde);
    if (pde_big->large_page)
    {
        *pt = nullptr;
        return;
    }

    corelog_hex("bad pde pfn : ", pde_big->page_frame_number);

    auto pte = &reinterpret_cast<epte*>(pde->page_frame_number << 12)[pt_i];
    if (pt != nullptr)
    {
        *pt = pte;
    }
}
