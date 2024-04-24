#pragma once

#include "serializer/error_handler/error_handler.h"

#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"

//Enumerates all possible types that can be used in protocol (Type string is a zero-terminated string).
typedef enum Kind {
    INT64,
    UINT64,
    UINT32,
    INT32,
    VARINT,
    BOOL,
    STRING,
    CONSTRUCTED,
    DOUBLE,
    FLOAT,
    BARRAY
} Kind;

//Layout is used to decribe layout of type. Can be constructed by user.
typedef struct Layout {
    const struct Layout **fields;
    Kind kind;
    size_t field_q;
} Layout;

//clang-for

//pre-defined layouts for built-in types
extern const Layout* kInt64Layout; //NOLINT
extern const Layout* kUint64Layout; //NOLINT
extern const Layout* kInt32Layout; //NOLINT
extern const Layout* kUint32Layout; //NOLINT
extern const Layout* kDoubleLayout; //NOLINT
extern const Layout* kVarintLayout; //NOLINT
extern const Layout* kBoolLayout; //NOLINT
extern const Layout* kStringLayout; //NOLINT
extern const Layout* kFloatLayout; //NOLINT
extern const Layout* kBarrayLayout; //NOLINT

typedef struct LayoutNode {
    struct LayoutNode* next;
    struct LayoutNode* prev;
} LayoutNode;

//Clears all custom layouts from memory
void ClearLayouts();
//Creates custom layout and returns pointer to it
const Layout* CreateLayout(size_t field_q, ...);
//Deletes layout by its pointer and returns 0. If layout doesn't exists, returns 1.
int DeleteLayout(const Layout* layout);

//Value is used to fill fields of certain defined layout by values.
typedef struct Value {
    struct Value* children;
    union {
        uint64_t uint_value;
        int64_t int_value;
        char *varint_value;
        char *string_ptr;
        char bool_value;
        double double_value;
        float float_value;
        char* barray_value;
    };
} Value;

Value CreateValue(const Layout* layout, ...);
Value CreateBoolValue(char bool);
void DestroyValue(Value* value);