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
