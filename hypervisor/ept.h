#pragma once

#include "freestanding/types.h"

void ept_invalidate_ept_cache(void* ept_ptr);
void ept_invalidate_vpid_cache(u16 vproc_id);
