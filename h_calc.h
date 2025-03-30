#pragma once

#include <inttypes.h>

typedef char u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

struct Buffer
{
    void *data;
    size_t size;
};

struct Point
{
    f64 x0, y0, x1, y1;
};