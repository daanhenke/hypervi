#pragma once


int itoa(unsigned long long value, char* buff, unsigned int radix, int sign, unsigned int pad);

#include "freestanding/types.h"

extern "C" __attribute__((sysv_abi)) void* _movsb(void* buf, void* val, size_t size);
extern "C" __attribute__((sysv_abi)) void* _stosb(void* buf, size_t val, size_t size);

extern "C" void* memcpy(void* dest, const void* source, size_t num);

extern "C" int strcmp(const char* a, const char* b);
extern "C" size_t strlen(const char* str);
