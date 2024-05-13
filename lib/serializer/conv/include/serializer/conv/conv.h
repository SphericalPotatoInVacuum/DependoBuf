#pragma once

#include "stdint.h"
#include "stddef.h"
#include "limits.h"

#if CHAR_BIT <= 1
#error "Uncompatible platform to run"
#endif

//Converts varint by its pointer to uint32_t, saves in res by its pointer, returns 0. If error has occurred, returns 1.
int ConvertVARINTtoUINT32(const char* varint, uint32_t* res);
//Converts varint by its pointer to uint64_t, saves in res by its pointer, returns 0. If error has occurred, returns 1.
int ConvertVARINTtoUINT64(const char* varint, uint64_t* res);
//Calculates number of bytes required to encode inp.
size_t GetVarintSizeOfUINT(uint64_t inp);
//Converts uint64_t(uint32_t is elevated to uint64_t), saves in varint by its pointer, returns 0. If error has occured, returns 1.
int ConvertUINTtoVARINT(char* varint, uint64_t inp, size_t varint_size);

//Calculates number of barray bytes required to encode array of booleans.
size_t GetBarraySizeOfBoolArray(size_t bool_array_size);
//Calculates number of boolean elements, which are encoded by given barray.
size_t GetBoolArraySizeofBarray(const char* barray);
//Encodes array of booleans into barray format, returns 0. If error has occured, returns 1.
int ConvertBoolArrayToBarray(char* barray, const char* bool_array, size_t bool_array_size, size_t barray_size);
//Decodes barray format into an array of booleans, returns 0. If error has occured, returns 1.
int ConvertBarrayToBoolArray(const char* barray, char* bool_array, size_t bool_array_size);