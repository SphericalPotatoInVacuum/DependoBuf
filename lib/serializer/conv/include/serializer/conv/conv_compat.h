#pragma once

#include "stdint.h"
#include "limits.h"

#define UINT32_BITS 32
#define UINT64_BITS 64
#define UINT16_BITS 16

#if UINT_MAX == UINT16_MAX
#define UINT_BYTE_SIZE 2

#elif UINT_MAX == UINT32_MAX
#define UINT_BYTE_SIZE 4

#elif UINT_MAX == UINT64_MAX
#define UINT_BYTE_SIZE 8

#else
#erorr "unsigned integer size not defined"

#endif