#include "test_includes.h"
#include "serializer/conv/conv.h"

static int TestUINT32VARINTConversion(uint32_t inp);
static int TestUINT64VARINTConversion(uint64_t inp);

static int TestUINT64VARINTConversion(uint64_t inp) {
    uint64_t out = 0;
    int exit_code = 0;

    size_t array_size = GetVarintSizeOfUINT(inp);
    char* varint_array = calloc(array_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint_array, inp, array_size);
    if (exit_code) {
        return !exit_code;
    }
    exit_code = ConvertVARINTtoUINT64(varint_array,&out);
    free(varint_array);
    return (!exit_code && inp == out);
}

static int TestUINT32VARINTConversion(uint32_t inp) {
    uint32_t out = 0;
    int exit_code = 0;

    size_t array_size = GetVarintSizeOfUINT(inp);
    char* varint_array = calloc(array_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint_array, inp, array_size);
    if (exit_code) {
        return !exit_code;
    }
    exit_code = ConvertVARINTtoUINT32(varint_array,&out);
    free(varint_array);
    return (!exit_code && inp == out);
}

int main() {
    assert(TestUINT32VARINTConversion(50));
    assert(TestUINT64VARINTConversion(50));
    assert(TestUINT32VARINTConversion(0));
    assert(TestUINT64VARINTConversion(0));
    assert(!TestUINT32VARINTConversion(UINT64_MAX));

}