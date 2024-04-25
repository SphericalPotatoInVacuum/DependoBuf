#include "serializer/error_handler/error_handler.h"
#include "serializer/serialization/serialization.h"
#include "serializer/layout/layout.h"
#include "serializer/conv/conv.h"

#include "stdio.h"
#include "limits.h"
#include "stdint.h"
#include "assert.h"
#include "stdlib.h"

static int CompareValues(const Layout* layout, const Value* first_val, const Value* second_val) {
    if ((first_val->children == NULL && second_val->children != NULL) || (first_val->children != NULL && second_val->children == NULL)) {
        return 0;
    }

    if (first_val->uint_value != second_val->uint_value) {
        return 0;
    }
    for (size_t i = 0; i < layout->field_q; ++i) {
        int curr_res = CompareValues(layout->fields[i], &first_val->children[i], &second_val->children[i]);
        if (curr_res) {
            return 0;
        }
    }
    return 1;
}

static int TestUINT32VARINTConversion(uint64_t inp) {
    uint32_t out = 0;
    int exit_code = 0;

    size_t array_size = GetVarintSizeOfUINT(inp);
    char* varint_array = calloc(array_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint_array, inp, array_size);
    if (exit_code) {
        free(varint_array);
        return !exit_code;
    }
    exit_code = ConvertVARINTtoUINT32(varint_array,&out);
    free(varint_array);
    return (!exit_code && inp == out);
}

static int TestUINT64VARINTConversion(uint64_t inp) {
    uint64_t out = 0;
    int exit_code = 0;

    size_t array_size = GetVarintSizeOfUINT(inp);
    char* varint_array = calloc(array_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint_array, inp, array_size);
    if (exit_code) {
        free(varint_array);
        return !exit_code;
    }
    exit_code = ConvertVARINTtoUINT64(varint_array,&out);
    free(varint_array);
    return (!exit_code && inp == out);
}

static int TestVARINTNotEnoughSize() {
    size_t varint_size = 1;
    char* varint = calloc(varint_size, sizeof(char));
    int exit_code = ConvertUINTtoVARINT(varint, UINT32_MAX, varint_size);
    free(varint);
    return exit_code;
}

static int TestBARRAYConversion(const char* bool_array, size_t bool_array_size) {
    size_t barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    char* barray = calloc(barray_size, sizeof(char));
    int exit_code = ConvertBoolArrayToBarray(barray, bool_array, bool_array_size, barray_size);

    size_t out_barray_size = GetBoolArraySizeofBarray(barray);
    char* out_bool_array = calloc(out_barray_size, sizeof(char));

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
    for (size_t i = 0; i < bool_array_size; ++i) {
        if ((bool_array[i] && !out_bool_array[i]) || (!bool_array[i] && out_bool_array[i])) {
            return 0;
        }
    }
    free(barray);
    free(out_bool_array);
    return out_barray_size = barray_size;
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

static int TestUINT32(uint32_t val) {
    err_code = 0;

    Value inp = CreateValue(kUint32Layout, val);
    Value out = Deserialize(kUint32Layout, Serialize(kUint32Layout, inp));

    return (!err_code && CompareValues(kUint32Layout, &inp, &out));
}

static int TestUINT64(uint64_t val) {
    err_code = 0;

    Value inp = CreateValue(kUint64Layout, val);
    Value out = Deserialize(kUint64Layout, Serialize(kUint64Layout, inp));
    return (!err_code && CompareValues(kUint64Layout, &inp, &out));
}

static int TestINT32(int32_t val) {
    err_code = 0;

    Value inp = CreateValue(kInt32Layout, val);
    Value out = Deserialize(kInt32Layout, Serialize(kInt32Layout, inp));
    return (!err_code && CompareValues(kInt32Layout, &inp, &out));
}

static int TestINT64(int64_t val) {
    err_code = 0;

    Value inp = CreateValue(kInt64Layout, val);
    Value out = Deserialize(kInt64Layout, Serialize(kInt64Layout, inp));
    return (!err_code && CompareValues(kInt64Layout, &inp, &out));
}

static int TestFLOAT(float val) {
    err_code = 0;
    
    Value inp = CreateValue(kFloatLayout, val);
    Value out = Deserialize(kFloatLayout, Serialize(kFloatLayout, inp));
    return (!err_code && CompareValues(kFloatLayout, &inp, &out));
}

static int TestDOUBLE(double val) {
    err_code = 0;

    Value inp = CreateValue(kDoubleLayout, val);
    Value out = Deserialize(kDoubleLayout, Serialize(kDoubleLayout, inp));
    return (!err_code && CompareValues(kDoubleLayout, &inp, &out));
}

static int TestBOOL(char bool) {
    err_code = 0;

    Value inp = CreateBoolValue(bool);
    Value out = Deserialize(kBoolLayout, Serialize(kBoolLayout, inp));
    return (!err_code && CompareValues(kBoolLayout, &inp, &out));
}

static int TestSTRING(const char* val) {
    err_code = 0;
    Value inp = CreateValue(kStringLayout, val);
    Value out = Deserialize(kStringLayout, Serialize(kStringLayout, inp));
    return (!err_code && CompareValues(kStringLayout, &inp, &out));
}

static int TestVARINT(uint64_t val) {
    err_code = 0;
    int exit_code = 0;

    size_t varint_size = GetVarintSizeOfUINT(val);
    char* varint = calloc(varint_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint, val, varint_size);
    if (exit_code) {
        free(varint);
        return !exit_code;
    }
    Value inp = CreateValue(kVarintLayout, varint);
    Value out = Deserialize(kVarintLayout, Serialize(kVarintLayout, inp));
    if (err_code) {
        free(varint);
        free(out.varint_value);
        return !err_code;
    }
    uint64_t out_val = 0;
    exit_code = ConvertVARINTtoUINT64(out.varint_value, &out_val);

    free(varint);
    free(out.varint_value);
    return !exit_code && val == out_val;
}

static int TestBARRAY(const char* bool_array, size_t bool_array_size) {
    err_code = 0;
    int exit_code = 0;

    size_t barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    char* barray = calloc(barray_size, sizeof(char));
    exit_code = ConvertBoolArrayToBarray(barray, bool_array, bool_array_size, barray_size);
    if (exit_code) {
        free(barray);
        return !exit_code;
    }
    
    Value inp = CreateValue(kBarrayLayout, barray);
    Value out = Deserialize(kBarrayLayout, Serialize(kBarrayLayout, inp));
    if (err_code || bool_array_size != GetBoolArraySizeofBarray(out.barray_value)) {
        free(barray);
        free(out.barray_value);
        return 0;
    }

    char* out_bool_array = calloc(bool_array_size, sizeof(char));
    exit_code = ConvertBarrayToBoolArray(out.barray_value, out_bool_array, bool_array_size);
    int is_correct = 1;
    for (size_t i = 0; i < bool_array_size; ++i) {
        if ((out_bool_array[i] && !bool_array[i]) || (!out_bool_array[i] && bool_array[i])) {
            is_correct = 0;
            break;
        }
    }
    free(barray);
    free(out.barray_value);
    free(out_bool_array);

    return is_correct;
}

int main() {

    //TEST BASIC SER
    assert(TestUINT32(50));
    assert(TestUINT64(50));
    assert(TestINT32(-50));
    assert(TestINT64(-50));
    assert(TestINT32(50));
    assert(TestINT64(50));
    assert(TestFLOAT(0.5));
    assert(TestFLOAT(-0.5));
    assert(TestDOUBLE(0.5));
    assert(TestDOUBLE(-0.5));
    assert(TestBOOL(1));
    assert(TestBOOL(0));
    assert(TestSTRING("Hello, bebra!"));
    //TEST BASIC SER

    // TEST VARINT
    assert(TestUINT32VARINTConversion(50));
    assert(TestUINT64VARINTConversion(50));
    assert(TestUINT32VARINTConversion(0));
    assert(TestUINT64VARINTConversion(0));
    assert(!TestUINT32VARINTConversion(UINT64_MAX));
    assert(TestVARINTNotEnoughSize());
    assert(TestVARINT(50));
    // TEST VARINT

    //TEST BARRAY
    size_t bool_array_size = 50;
    char* bool_array = calloc(bool_array_size, sizeof(char));
    for (size_t i = 0; i < bool_array_size; ++i) {
        bool_array[i] = i % 2;
    }
    assert(TestBARRAYConversion(bool_array, bool_array_size));
    assert(!TestBARRAYConversion(bool_array, 0));
    assert(TestBARRAYNotEnoughSize());
    assert(TestBARRAYNotEnoughSizeReverse());
    assert(TestBARRAY(bool_array, bool_array_size));
    free(bool_array);
    //TEST BARRAY


}