#include "x86utils.h"

typedef union
{
    u64 as_u64;
    struct
    {
        u64 unused: 12;
        u64 pt: 9;
        u64 pd: 9;
        u64 pdpt: 9;
        u64 pml4: 9;
    } indexed;
} pt_helper;

u64 pt_to_address(u32 pml4, u32 pdpt, u32 pd, u32 pt)
{
    pt_helper helper;

    helper.as_u64 = 0;
    helper.indexed.pml4 = pml4;
    helper.indexed.pdpt = pdpt;
    helper.indexed.pd = pd;
    helper.indexed.pt = pt;

    return helper.as_u64;
}

segment_descriptor_32* get_segment_descriptor(u64 descriptor_table_base, u16 selector_value)
{
    segment_selector selector;
    selector.flags = selector_value;

    auto descriptors = reinterpret_cast<segment_descriptor_32*>(descriptor_table_base);
    return &descriptors[selector.index];
}

u64 get_descriptor_base(segment_descriptor_32* segment_descriptor)
{
    u64 base_hi, base_mid, base_low;

    base_hi = segment_descriptor->base_address_high << 24;
    base_mid = segment_descriptor->base_address_middle << 16;
    base_low = segment_descriptor->base_address_low;

    u64 base = (base_hi | base_mid | base_low) & 0xFFFFFFFF;

    // Detect long descriptors and expand the base (only tss for now)
    if (segment_descriptor->system == 0 &&
        (
            segment_descriptor->type == SEGMENT_DESCRIPTOR_TYPE_TSS_AVAILABLE ||
            segment_descriptor->type == SEGMENT_DESCRIPTOR_TYPE_TSS_BUSY
        ))
    {
        auto descriptor64 = reinterpret_cast<segment_descriptor_64*>(segment_descriptor);
        base |= (static_cast<u64>(descriptor64->base_address_upper) << 32);
    }

    return base;
}

u64 get_segment_base(u64 descriptor_table_base, u16 segment_selector_value)
{
    segment_selector selector;
    selector.flags = segment_selector_value;

    // null segment
    if (selector.table == 0 && selector.index == 0)
    {
        return 0;
    }

    // we dont support reading ldt's
    if (selector.table != 0)
    {
        return 0;
    }

    return get_descriptor_base(get_segment_descriptor(descriptor_table_base, segment_selector_value));
}

u16 get_segment_limit(u16 selector)
{
    return call_lsl(selector);
}

u32 get_segment_access_rights_vmx(u16 selector)
{
    vmx_segment_access_rights output;
    segment_selector input;
    input.flags = selector;

    if (input.table == 0 && input.index == 0)
    {
        output.flags = 0;
        output.unusable = 1;
        return output.flags;
    }

    u32 rights = call_lar(selector);
    output.flags = (rights >> 8);
    output.reserved1 = 0;
    output.reserved2 = 0;
    output.unusable = 0;

    return output.flags;
}

//const u64 elf_base_test_var = 1337;
size_t get_current_elf_base()
{
    // const u64 elf_base_test_var = 1337;
    // auto page_aligned_ptr_inside_elf = reinterpret_cast<size_t>(&elf_base_test_var) & 0xFFFFFFFFFFFFF000;
    // auto magic_finder = reinterpret_cast<char*>(page_aligned_ptr_inside_elf);

    // u64 count = 0;
    // while (magic_finder != nullptr)
    // {
    //     auto magic = reinterpret_cast<u32*>(magic_finder);
    //     auto extra = reinterpret_cast<u8*>(magic_finder) + 4;
    //     if (*magic == 0x464c457F && *extra == 2 )
    //     {
    //         if (count == 1)
    //         {
    //             return reinterpret_cast<u64>(magic_finder);
    //         }

    //         count++;
    //     }
    //     magic_finder -= 0x1000;
    // }

    return 0;
}
