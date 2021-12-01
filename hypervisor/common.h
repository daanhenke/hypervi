#pragma once

#include "loader/exports.h"

#define corelog_prefix ldr_log("[core "); ldr_log_hex(ldr_core_whoami()); ldr_log("]: ")
#define corelog(msg) corelog_prefix; ldr_log(msg);
#define corelog_hex(msg, value) corelog_prefix; ldr_log(msg); ldr_log_hex(value); ldr_log("\n")
