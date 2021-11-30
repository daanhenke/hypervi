#include "loader/efi/common.h"
#include "loader/efi/paging.h"
#include "freestanding/x86utils.h"
#include "external/ia32-doc/out/ia32.hpp"

static do_align(0x1000) pml4e_64 pml4;
static do_align(0x1000) pdpte_64 pdpt[512];
static do_align(0x1000) pde_2mb_64 pdt[512][512];

void efi_paging_init()
{
    log("initializing pagetables for 512gb identity map\n");

    pml4.present = true;
    pml4.write = true;
    pml4.page_frame_number = reinterpret_cast<u64>(pdpt) >> PAGE_SHIFT;

    for (size_t i = 0; i < 512; i++)
    {
        auto pdp = pdpt[i];

        pdp.present = true;
        pdp.write = true;
        pdp.large_page = true;
        pdp.page_frame_number = reinterpret_cast<u64>(pdt[i]) >> PAGE_SHIFT;

        for (size_t j = 0; j < 512; j++)
        {
            auto physical = pt_to_address(0, i, j, 0);
            auto pde = pdt[i][j];

            pde.present = true;
            pde.write = true;
            pde.large_page = true;
            pde.page_frame_number = physical >> 21;
        }
    }
}
