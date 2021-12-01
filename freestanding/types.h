#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef u64 size_t;
typedef s64 ssize_t;

#define do_align(x) __attribute__((aligned(x)))
#define do_packed __attribute__((packed))
