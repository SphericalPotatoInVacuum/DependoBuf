#include "serializer/conv/conv.h"

int ConvertVARINTtoUINT32(const char* varint, uint32_t* res) {
    *res = 0;

    const char* curr_byte_iter = varint;
    while (*curr_byte_iter >> 7) {
        ++curr_byte_iter;
    }
    size_t varint_size = curr_byte_iter - varint;
    size_t total_bits = varint_size * 7;
    if (*curr_byte_iter != 0) {
        total_bits += 8 - ((__builtin_clz((uint32_t)*curr_byte_iter)) - 24);
    }
    if (total_bits > 32) {
        return 1;
    }

    size_t curr_shift = 0;
    for (size_t i = 0; i < varint_size; ++i) {
        char curr_byte = varint[i] << 1;
        *res += ((uint32_t)(curr_byte >> 1)) << (curr_shift);
        curr_shift += 7;
    }
    *res += (uint32_t)*curr_byte_iter << curr_shift;
    return 0;
}

int ConvertVARINTtoUINT64(const char* varint, uint64_t* res) {
    *res = 0;
    const char* curr_byte_iter = varint;
    while (*curr_byte_iter >> 7) {
        ++curr_byte_iter;
    }
    size_t varint_size = curr_byte_iter - varint;
    size_t total_bits = varint_size * 7;
    if (*curr_byte_iter != 0) {
        total_bits += 8 - ((__builtin_clz((uint32_t)*curr_byte_iter)) - 24);
    }
    if (total_bits > 64) {
        return 1;
    }

    size_t curr_shift = 0;
    for (size_t i = 0; i < varint_size; ++i) {
        char curr_byte = varint[i] << 1;
        *res += ((uint64_t)(curr_byte >> 1)) << (curr_shift);
        curr_shift += 7;
    }
    *res += (uint64_t)*curr_byte_iter << curr_shift;
    return 0;
}

size_t GetVarintSizeOfUINT(uint64_t inp) {
    if (inp == 0) {
        return 1;
    }
    size_t val_bits = 64 - (__builtin_clz(inp));
    size_t one_bytes = val_bits / 7;
    if (val_bits % 7 == 0) {
        --one_bytes;
    }
    return one_bytes + 1;
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
        curr_byte = ((~(curr_byte >> 7)) << 7) ^ curr_byte;
        varint[i] = curr_byte;
        inp = inp >> 7;
    }
    varint[calculated_varint_size - 1] = inp;
    return 0;
}

size_t GetBarraySizeOfBoolArray(size_t bool_array_size) {
    if (bool_array_size == 0) {
        return 0;
    }
    size_t one_bytes = bool_array_size / 7;
    if (bool_array_size % 7 == 0) {
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
    while (*curr_byte_iter >> 7) {
        total_bits += 7;
        ++curr_byte_iter;
    }
    char indicator = *(curr_byte_iter + 1);
    total_bits += __builtin_ctz(indicator) + 1;
    return total_bits;
}

int ConvertBoolArrayToBarray(char* barray, const char* bool_array, size_t bool_array_size, size_t barray_size) {
    size_t calculated_barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    if (calculated_barray_size > barray_size || calculated_barray_size) {
        return 1;
    }
    size_t bool_array_iter = 0;
    for (size_t i = 0; i < calculated_barray_size - 2; ++i) {
        char curr_byte = 0x80;
        size_t end_point = bool_array_iter + 8;
        for (size_t j = 0; j < 7; ++j) {
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
    while (curr_byte >> 7) {
        for (size_t i = 0; i < 7; ++i) {
            bool_array[bool_array_iter] = curr_byte & (0x80);
            ++bool_array_iter;
            curr_byte = curr_byte >> 1;
        }
        ++barray;
        curr_byte = *barray;
    }
    char indicator = *(barray + 1);
    for (size_t i = 0; i < __builtin_ctz(indicator) + 1; ++i) {
        bool_array[bool_array_iter] = curr_byte & (0x80);
        curr_byte = curr_byte >> 1;
    }
    return 0;
}