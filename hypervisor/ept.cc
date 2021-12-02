#include "hypervisor/ept.h"
#include "freestanding/x86utils.h"
#include "freestanding/libc.h"

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
