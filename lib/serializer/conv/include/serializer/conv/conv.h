#pragma once

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

//Converts varint by its pointer to uint32_t, saves in res by its pointer, returns 0. If error has occurred, returns 1.
int ConvertVARINTtoUINT32(char* varint, uint32_t* res);
//Converts varint by its pointer to uint64_t, saves in res by its pointer, returns 0. If error has occurred, returns 1.
int ConvertVARINTtoUINT64(char* varint, uint64_t* res);
//Calculates number of bytes required to encode inp.
static size_t GetVarintSizeOfUINT(uint64_t inp);
//Converts uint64_t(uint32_t is elevated to uint64_t), saves in varint by its pointer, returns 0. If error has occured, returns 1.
int ConvertUINTtoVARINT(char* varint, uint64_t inp, size_t varint_size);