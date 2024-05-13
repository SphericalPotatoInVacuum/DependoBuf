#include "test_includes.h"
#include "serializer/conv/conv.h"

static int TestBARRAYNotEnoughSize();
static int TestBARRAYConversion(const char* bool_array, size_t bool_array_size);
static int TestBARRAYNotEnoughSizeReverse();

static int TestBARRAYConversion(const char* bool_array, size_t bool_array_size) {
    size_t barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    char* barray = calloc(barray_size, sizeof(char));
    int exit_code = ConvertBoolArrayToBarray(barray, bool_array, bool_array_size, barray_size);

    size_t out_bool_array_size = GetBoolArraySizeofBarray(barray);
    char* out_bool_array = calloc(out_bool_array_size, sizeof(char));

    if (exit_code) {
        free(barray);
        free(out_bool_array);
        return !exit_code;
    }
    exit_code = ConvertBarrayToBoolArray(barray, out_bool_array, bool_array_size);
    if (exit_code) {
        free(barray);
        free(out_bool_array);
        return !exit_code;
    }
    for (size_t i = 0; i < out_bool_array_size; ++i) {
        if ((bool_array[i] && !out_bool_array[i]) || (!bool_array[i] && out_bool_array[i])) {
            return 0;
        }
    }
    free(barray);
    free(out_bool_array);
    return out_bool_array_size == bool_array_size;
}

static int TestBARRAYNotEnoughSize() {
    char bool_array[100] = {0};
    char barray[1] = {0};
    size_t barray_size = 1;

    int exit_code = ConvertBoolArrayToBarray(barray, bool_array, 100, barray_size);
    return exit_code;
}

static int TestBARRAYNotEnoughSizeReverse() {
    int exit_code = 0;

    char bool_array[100] = {0};
    size_t bool_array_size = 100;
    size_t barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    char* barray = calloc(barray_size, sizeof(char));

    exit_code = ConvertBoolArrayToBarray(barray, bool_array, bool_array_size, barray_size);
    if (exit_code) {
        free(barray);
        return !exit_code;
    }
    char out_bool_array[1] = {0};
    size_t out_bool_array_size = 1;

    exit_code = ConvertBarrayToBoolArray(barray, out_bool_array, out_bool_array_size);
    free(barray);
    return exit_code;
}

int main() {
    size_t bool_array_size = 50;
    char* bool_array = calloc(bool_array_size, sizeof(char));
    for (size_t i = 0; i < bool_array_size; ++i) {
        bool_array[i] = i % 2;
    }

    assert(TestBARRAYConversion(bool_array, bool_array_size));
    assert(!TestBARRAYConversion(bool_array, 0));
    assert(TestBARRAYNotEnoughSize());
    assert(TestBARRAYNotEnoughSizeReverse());
    
    free(bool_array);
}