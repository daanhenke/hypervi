#pragma once

#include "freestanding/x86utils.h"


asmapi void host_hv_entrypoint();
asmapi void host_vmexit(u64 rsp);

extern u64 host_vmexit_ptr;
