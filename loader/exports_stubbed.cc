#include "loader/exports.h"

void ldr_log(const char* string) {}
void ldr_log_hex(size_t value) {}

size_t ldr_core_whoami() {}
size_t ldr_core_count() {}

void ldr_call_on_all_cores(ldr_coac_cb function) {}
