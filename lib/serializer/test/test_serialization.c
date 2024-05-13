#include "serializer/error_handler/error_handler.h"
#include "serializer/layout/layout.h"
#include "serializer/serialization/serialization.h"
#include "serializer/conv/conv.h"
#include "test_includes.h"

static int CompareValues(const Layout* layout, const Value* first_val, const Value* second_val);
static int TestUINT32(uint32_t val);
static int TestUINT64(uint64_t val);
static int TestINT32(int32_t val);
static int TestINT64(int64_t val);
static int TestFLOAT(float val);
static int TestDOUBLE(double val);
static int TestBOOL(char bool);
static int TestSTRING(const char* val);
static int TestVARINT64(uint64_t val);
static int TestVARINT32(uint32_t val);
static int TestBARRAY(const char* bool_array, size_t bool_array_size);

static int CompareValues(const Layout* layout, const Value* first_val, const Value* second_val) {
    if ((first_val->children == NULL && second_val->children != NULL) || (first_val->children != NULL && second_val->children == NULL)) {
        return 0;
    }

    if (layout->kind == STRING) {
        return !strcmp(first_val->string_ptr, second_val->string_ptr);
    }
    if (layout->kind == BARRAY) {
        char* first_barray_iter = first_val->barray_value;
        char* second_barray_iter = second_val->barray_value;

        while (*first_barray_iter >> 7) {
            if (*first_barray_iter != *second_barray_iter) {
                return 0;
            }
            ++first_barray_iter;
            ++second_barray_iter;
        }
        return (*first_barray_iter == *second_barray_iter && *(first_barray_iter + 1) == *(second_barray_iter + 1));
    }
    if (layout->kind == VARINT) {
        char* first_varint_iter = first_val->varint_value;
        char* second_varint_iter = second_val->varint_value;

        while (*first_varint_iter >> 7) {
            if (*first_varint_iter != *second_varint_iter) {
                return 0;
            }
            ++first_varint_iter;
            ++second_varint_iter;
        }
        return (*first_varint_iter == *second_varint_iter);
    }

    if (first_val->uint_value != second_val->uint_value) {
        return 0;
    }
    for (size_t i = 0; i < layout->field_q; ++i) {
        int curr_res = CompareValues(layout->fields[i], &first_val->children[i], &second_val->children[i]);
        if (!curr_res) {
            return 0;
        }
    }
    return 1;
}

static int TestUINT32(uint32_t val) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kUint32Layout, val);
    Value out = Deserialize(kUint32Layout, Serialize(kUint32Layout, &inp));

    return (!err_code && CompareValues(kUint32Layout, &inp, &out));
}

static int TestUINT64(uint64_t val) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kUint64Layout, val);
    Value out = Deserialize(kUint64Layout, Serialize(kUint64Layout, &inp));
    return (!err_code && CompareValues(kUint64Layout, &inp, &out));
}

static int TestINT32(int32_t val) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kInt32Layout, val);
    Value out = Deserialize(kInt32Layout, Serialize(kInt32Layout, &inp));
    return (!err_code && CompareValues(kInt32Layout, &inp, &out));
}

static int TestINT64(int64_t val) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kInt64Layout, val);
    Value out = Deserialize(kInt64Layout, Serialize(kInt64Layout, &inp));
    return (!err_code && CompareValues(kInt64Layout, &inp, &out));
}

static int TestFLOAT(float val) {
    err_code = 0;
    
    Value inp = ConstructPrimitiveValue(kFloatLayout, val);
    Value out = Deserialize(kFloatLayout, Serialize(kFloatLayout, &inp));
    return (!err_code && CompareValues(kFloatLayout, &inp, &out));
}

static int TestDOUBLE(double val) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kDoubleLayout, val);
    Value out = Deserialize(kDoubleLayout, Serialize(kDoubleLayout, &inp));
    return (!err_code && CompareValues(kDoubleLayout, &inp, &out));
}

static int TestBOOL(char bool) {
    err_code = 0;

    Value inp = ConstructPrimitiveValue(kBoolLayout, bool);
    Value out = Deserialize(kBoolLayout, Serialize(kBoolLayout, &inp));
    return (!err_code && CompareValues(kBoolLayout, &inp, &out));
}

static int TestSTRING(const char* val) {
    err_code = 0;
    Value inp = ConstructPrimitiveValue(kStringLayout, val);
    Value out = Deserialize(kStringLayout, Serialize(kStringLayout, &inp));
    return (!err_code && CompareValues(kStringLayout, &inp, &out));
}

static int TestVARINT64(uint64_t val) {
    err_code = 0;
    int exit_code = 0;

    size_t varint_size = GetVarintSizeOfUINT(val);
    char* varint = calloc(varint_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint, val, varint_size);
    if (exit_code) {
        free(varint);
        return !exit_code;
    }
    Value inp = ConstructPrimitiveValue(kVarintLayout, varint);
    Value out = Deserialize(kVarintLayout, Serialize(kVarintLayout, &inp));
    if (err_code) {
        DestroyValue(kVarintLayout, &inp);
        DestroyValue(kVarintLayout, &out);
        free(varint);
        free(out.varint_value);
        return !err_code;
    }
    uint64_t out_val = 0;
    exit_code = ConvertVARINTtoUINT64(out.varint_value, &out_val);

    free(varint);
    DestroyValue(kVarintLayout, &inp);
    DestroyValue(kVarintLayout, &out);
    return !exit_code && val == out_val;
}

static int TestVARINT32(uint32_t val) {
    err_code = 0;
    int exit_code = 0;

    size_t varint_size = GetVarintSizeOfUINT(val);
    char* varint = calloc(varint_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint, val, varint_size);
    if (exit_code) {
        free(varint);
        return !exit_code;
    }
    Value inp = ConstructPrimitiveValue(kVarintLayout, varint);
    Value out = Deserialize(kVarintLayout, Serialize(kVarintLayout, &inp));
    if (err_code) {
        DestroyValue(kVarintLayout, &inp);
        DestroyValue(kVarintLayout, &out);
        free(varint);
        return !err_code;
    }
    uint32_t out_val = 0;
    exit_code = ConvertVARINTtoUINT32(out.varint_value, &out_val);

    free(varint);
    DestroyValue(kVarintLayout, &inp);
    DestroyValue(kVarintLayout, &out);
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
    
    Value inp = ConstructPrimitiveValue(kBarrayLayout, barray);
    Value out = Deserialize(kBarrayLayout, Serialize(kBarrayLayout, &inp));
    if (err_code || bool_array_size != GetBoolArraySizeofBarray(out.barray_value)) {
        DestroyValue(kBarrayLayout, &inp);
        DestroyValue(kBarrayLayout, &out);
        free(barray);
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
    DestroyValue(kBarrayLayout, &inp);
    DestroyValue(kBarrayLayout, &out);
    free(out_bool_array);
    
    return is_correct;
}

static void PrintOutRes(int res){
    printf("%d\n", res);
}

typedef struct ValueLayoutPack {
    const Layout* layout;
    Value val;
} ValueLayoutPack;

static ValueLayoutPack CreateTreeValue(size_t n, const Layout* root_layout, Value* value) {
    if (n== 0) {
        ValueLayoutPack pack = {.layout = root_layout, .val = *value};
        return pack;
    }
    size_t varint_size = GetVarintSizeOfUINT(n);
    char* varint = calloc(varint_size, sizeof(varint_size));
    int exit_code = ConvertUINTtoVARINT(varint, n, varint_size);
    const Layout* new_layout = CreateLayout(2, root_layout, kVarintLayout);
    Value new_value = ConstructConstructedValue(new_layout, *value, ConstructPrimitiveValueNoCopy(kVarintLayout, varint));
    return CreateTreeValue(n - 1, new_layout, &new_value);
}

static int TestSimpleCONSTRUCTED() {
    err_code = 0;

    size_t bool_array_size = 50;
    char* bool_array = calloc(bool_array_size, sizeof(char));
    for (size_t i = 0; i < bool_array_size; ++i) {
        bool_array[i] = i % 2;
    }   

    const Layout* simple_layout = CreateLayout(6, kInt32Layout, kInt64Layout, kStringLayout, kVarintLayout, kBarrayLayout, kBoolLayout);
    size_t barray_size = GetBarraySizeOfBoolArray(bool_array_size);
    char* barray = calloc(barray_size, sizeof(char));
    int exit_code = ConvertBoolArrayToBarray(barray, bool_array, bool_array_size, barray_size);
    if (!exit_code) {
        return !exit_code;
    }

    int64_t encoded_uint = 150;
    size_t varint_size = GetVarintSizeOfUINT(encoded_uint);
    char* varint = calloc(varint_size, sizeof(char));
    exit_code = ConvertUINTtoVARINT(varint, encoded_uint, varint_size);
    if (!exit_code) {
        return !exit_code;
    }
    const char* string = "Hello, bebra!";

    Value val = ConstructConstructedValue(simple_layout, 
    ConstructPrimitiveValue(kInt32Layout, UINT32_MAX), 
    ConstructPrimitiveValue(kInt64Layout, UINT64_MAX),
    ConstructPrimitiveValue(kStringLayout, string), 
    ConstructPrimitiveValue(kVarintLayout, varint),
    ConstructPrimitiveValue(kBarrayLayout, barray),
    ConstructPrimitiveValue(kBoolLayout, 1));

    char* encoded = Serialize(simple_layout, &val);
    Value out_val = Deserialize(simple_layout, encoded);
    int compare_res = CompareValues(simple_layout, &val, &out_val);

    int res = err_code == NOERR && CompareValues(simple_layout, &val, &out_val);

    free(barray);
    free(varint);
    DestroyValue(simple_layout, &val);
    DestroyValue(simple_layout, &out_val);
    free(bool_array);
    free(encoded);
    ClearLayouts();

    return res;
}

static int TestComplexCONSTRUCTED(size_t n) {
    err_code = 0;

    char* varint_init = calloc(GetVarintSizeOfUINT(0), sizeof(char));
    uintptr_t final_layout_addr = 0;
    int exit_code = ConvertUINTtoVARINT(varint_init, 0, GetVarintSizeOfUINT(0));
    Value init_value = ConstructPrimitiveValueNoCopy(kVarintLayout, varint_init);
    ValueLayoutPack n_value_pack = CreateTreeValue(n, kVarintLayout, &init_value);

    char* encoded = Serialize(n_value_pack.layout, &n_value_pack.val);
    Value deserialized_value = Deserialize(n_value_pack.layout, encoded);
    int res = CompareValues(n_value_pack.layout, &n_value_pack.val, &deserialized_value);

    free(encoded);
    DestroyValue(n_value_pack.layout, &deserialized_value);
    DestroyValue(n_value_pack.layout, &n_value_pack.val);
    ClearLayouts();

    return err_code == NOERR && res;
}


int main() {
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

    assert(TestSimpleCONSTRUCTED());
    assert(TestComplexCONSTRUCTED(1));
    assert(TestComplexCONSTRUCTED(100));
    assert(TestComplexCONSTRUCTED(500));
    assert(TestComplexCONSTRUCTED(1000));
}