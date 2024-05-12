#include "serializer/conv/conv.h"
#include "serializer/conv/conv_compat.h"
#include "limits.h"
#include "stdint.h"

#include "stdio.h"

#if UINT_BYTE_SIZE == 2
    static inline size_t GetValBitsForUINT16(uint16_t inp) {
        return UINT16_BITS - __builtin_clz(inp);
    }
#endif

#if UINT_BYTE_SIZE <= 4
    static inline size_t GetValBitsfForUINT32(uint32_t inp) {
        size_t val_bits = 0;
        #if UINT_BYTE_SIZE == 2
            if (!(inp >> UINT16_BITS)) {
                val_bits = GetValBitsForUINT16(inp);
            } esle {
                val_bits = UINT16_BITS + GetValBitsForUINT16(inp >> UINT16_BITS);
            }
        #elif UINT_BYTE_SIZE == 4
            val_bits = UINT32_BITS - __builtin_clz(inp);
        #endif
        return val_bits;
}
#endif

static size_t GetValBits(uint64_t inp) {
    if (inp == 0) {
        return 0;
    }
    size_t val_bits = 0;
    #if UINT_BYTE_SIZE <= 4
        if (!(inp >> UINT32_BITS)) {
            val_bits = GetValBitsfForUINT32(inp);
        } else {
            val_bits = UINT32_BITS + GetValBitsfForUINT32(inp >> UINT32_BITS);
        }
    #elif UINT_BYTE_SIZE == 8
        val_bits = UINT64_BITS - __builtin_clz(inp); 
    #endif
    
    return val_bits;
}


size_t GetVarintSizeOfUINT(uint64_t inp) {
    size_t val_bits = GetValBits(inp);
    size_t one_bytes = val_bits / (CHAR_BIT - 1);
    if (val_bits % (CHAR_BIT - 1) == 0 && inp != 0) {
        --one_bytes;
    }
    return one_bytes + 1;
}

int ConvertVARINTtoUINT32(const char* varint_bebra, uint32_t* res) {
    *res = 0;
    if (*varint_bebra == 0) {
        return 0;
    }
    const unsigned char* varint = (unsigned char*)varint_bebra;
    const unsigned char* curr_byte_iter = varint;
    while (*curr_byte_iter >> (CHAR_BIT - 1)) {
        ++curr_byte_iter;
    }
    size_t varint_size = (curr_byte_iter - varint) / sizeof(char); //NOLINT
    size_t total_bits = varint_size * (CHAR_BIT - 1);
    if (*curr_byte_iter != 0) {
        total_bits += GetValBits(*curr_byte_iter);
    }
    if (total_bits > UINT32_BITS) {
        return 1;
    }
    size_t curr_shift = 0;
    for (size_t i = 0; i < varint_size; ++i) {
        unsigned char curr_byte = varint[i] << 1;
        curr_byte = curr_byte >> 1;
        *res += ((uint64_t)(curr_byte)) << (curr_shift);
        curr_shift += CHAR_BIT - 1;
    }
    unsigned char last_byte = *(unsigned char*)curr_byte_iter;
    *res += (uint64_t)last_byte << curr_shift;
    return 0;
}

int ConvertVARINTtoUINT64(const char* varint_bebra, uint64_t* res) {
    *res = 0;
    if (*varint_bebra == 0) {
        return 0;
    }
    const unsigned char* varint = (unsigned char*)varint_bebra;
    const unsigned char* curr_byte_iter = varint;
    while (*curr_byte_iter >> (CHAR_BIT - 1)) {
        ++curr_byte_iter;
    }
    size_t varint_size = (curr_byte_iter - varint) / sizeof(char); //NOLINT
    size_t total_bits = varint_size * (CHAR_BIT - 1);
    if (*curr_byte_iter != 0) {
        total_bits += GetValBits(*curr_byte_iter);
    }
    if (total_bits > UINT64_BITS) {
        return 1;
    }
    size_t curr_shift = 0;
    for (size_t i = 0; i < varint_size; ++i) {
        unsigned char curr_byte = varint[i] << 1;
        curr_byte = curr_byte >> 1;
        *res += ((uint64_t)(curr_byte)) << (curr_shift);
        curr_shift += CHAR_BIT - 1;
    }
    unsigned char last_byte = *(unsigned char*)curr_byte_iter;
    *res += (uint64_t)last_byte << curr_shift;
    return 0;
}

int ConvertUINTtoVARINT(char* varint, uint64_t inp, size_t varint_size) {
    size_t calculated_varint_size = GetVarintSizeOfUINT(inp);
    if (calculated_varint_size > varint_size) {
        return 1;
    }
    if (inp == 0) {
        varint[0] = 0;
        return 0;
    }
    for (size_t i = 0; i < calculated_varint_size - 1; ++i) {
        char curr_byte = inp;
        curr_byte = ((~(curr_byte >> (CHAR_BIT - 1))) << (CHAR_BIT - 1)) ^ curr_byte;
        varint[i] = curr_byte;
        inp = inp >> (CHAR_BIT - 1);
    }
    varint[calculated_varint_size - 1] = inp;
    return 0;
}

size_t GetBarraySizeOfBoolArray(size_t bool_array_size) {
    if (bool_array_size == 0) {
        return 0;
    }
    size_t one_bytes = bool_array_size / (CHAR_BIT - 1);
    if (bool_array_size % (CHAR_BIT - 1) == 0) {
        --one_bytes;
    }
    return one_bytes + 2;
}

size_t GetBoolArraySizeofBarray(const char* barray) {
    if (barray == NULL) {
        return 0;
    }
    const char* curr_byte_iter = barray;
    size_t total_bits = 0;
    while (*curr_byte_iter >> (CHAR_BIT - 1)) {
        total_bits += CHAR_BIT - 1;
        ++curr_byte_iter;
    }
    char indicator = *(curr_byte_iter + 1);
    total_bits += __builtin_ctz(indicator) + 1;
    return total_bits;
}

int ConvertBoolArrayToBarray(char* barray, const char* bool_array, size_t bool_array_size, size_t barray_size) {
    size_t calculated_barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    if (calculated_barray_size > barray_size || calculated_barray_size == 0) {
        return 1;
    }
    size_t bool_array_iter = 0;
    for (size_t i = 0; i < calculated_barray_size - 2; ++i) {
        unsigned char curr_byte = 1 << (CHAR_BIT - 1);
        size_t end_point = bool_array_iter + CHAR_BIT;
        for (size_t j = 0; j < CHAR_BIT - 1; ++j) {
            if (bool_array[bool_array_iter]) {
                curr_byte = (1 << j) | curr_byte;
            }
            ++bool_array_iter;
        }
        barray[i] = curr_byte;
    }
    char last_byte = 0;
    size_t final_iter = 0;
    for (;final_iter < bool_array_size - bool_array_iter; ++final_iter) {
        if (bool_array[bool_array_iter]) {
            last_byte = (1 << final_iter) | last_byte;
        }
        ++bool_array_iter;
    }
    barray[calculated_barray_size - 2] = last_byte;
    barray[calculated_barray_size - 1] = 1 << (final_iter - 1);
    return 0;
}

int ConvertBarrayToBoolArray(const char* barray, char* bool_array, size_t bool_array_size) {
    size_t calculated_bool_array_size = GetBoolArraySizeofBarray(barray);
    if (calculated_bool_array_size > bool_array_size || calculated_bool_array_size == 0) {
        return 1;
    }
    size_t bool_array_iter = 0;
    char curr_byte = *barray;
    while (curr_byte >> (CHAR_BIT - 1)) {
        for (size_t i = 0; i < CHAR_BIT - 1; ++i) {
            bool_array[bool_array_iter] = curr_byte & 1;
            ++bool_array_iter;
            curr_byte = curr_byte >> 1;
        }
        ++barray;
        curr_byte = *barray;
    }
    char indicator = *(barray + 1);
    for (size_t i = 0; i < __builtin_ctz(indicator) + 1; ++i) {
        bool_array[bool_array_iter] = curr_byte & 1;
        curr_byte = curr_byte >> 1;
    }
    return 0;
}