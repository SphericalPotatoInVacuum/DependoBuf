#include "serializer/layout/layout.h"
#include "serializer/error_handler/error_handler.h"
#include "serializer/layout/linked_list.h"
#include "serializer/layout/node_handler.h"

#include "stdarg.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "limits.h"

const Layout kInt64LayoutStatic = {.fields = NULL, .kind = INT64,.field_q = 0};
const Layout kUint64LayoutStatic = {.fields = NULL, .kind = UINT64, .field_q = 0};
const Layout kInt32LayoutStatic = {.fields = NULL, .kind = INT32, .field_q = 0};
const Layout kUint32LayoutStatic = {.fields = NULL, .kind = UINT32, .field_q = 0};
const Layout kDoubleLayoutStatic = {.fields = NULL, .kind = DOUBLE, .field_q = 0};
const Layout kVarintLayoutStatic = {.fields = NULL, .kind = VARINT, .field_q = 0};
const Layout kBoolLayoutStatic = {.fields = NULL, .kind = BOOL, .field_q = 0};
const Layout kStringLayoutStatic = {.fields = NULL, .kind = STRING, .field_q = 0};
const Layout kFloatLayoutStatic = {.fields = NULL, .kind = FLOAT, .field_q = 0};
const Layout kBarrayLayoutStatic = {.fields = NULL, .kind = BARRAY, .field_q = 0};

const Layout* kInt64Layout = &kInt64LayoutStatic;
const Layout* kUint64Layout = &kUint64LayoutStatic;
const Layout* kInt32Layout = &kInt32LayoutStatic;
const Layout* kUint32Layout = &kUint32LayoutStatic;
const Layout* kDoubleLayout = &kDoubleLayoutStatic;
const Layout* kVarintLayout = &kVarintLayoutStatic;
const Layout* kBoolLayout = &kBoolLayoutStatic;
const Layout* kStringLayout = &kStringLayoutStatic;
const Layout* kFloatLayout = &kFloatLayoutStatic;
const Layout* kBarrayLayout = &kBarrayLayoutStatic;

static LayoutNode* custom_layouts = NULL;

static void PushLayoutNode(LayoutNode* layout_node);

static char* CopyVARINT(char* varint);
static char* CopyBARRAY(char* barray);
static char* CopySTRING(char* string);

static void MopUpAfterError(Node* curr_node, List* list, const Layout* layout, Value* val);

static void MopUpAfterError(Node* curr_node, List* list, const Layout* layout, Value* val) {
    GiveNode(curr_node);
    Clear(list);
    Clear(&avalible_nodes);
    DestroyValue(layout, val);
}

static void PushLayoutNode(LayoutNode* layout_node) {
    if (custom_layouts == NULL) {
        custom_layouts = layout_node;
        return;
    }
    layout_node->next = custom_layouts;
    custom_layouts->prev = layout_node;
    custom_layouts = layout_node;
}

static char* CopyVARINT(char* varint) {
    char* varint_iter = varint;
    while (*varint_iter >> (CHAR_BIT - 1)) {
                ++varint_iter;
    }
    size_t varint_size = (varint_iter - varint) / sizeof(char) + 1; //NOLINT
    char* copied_varint = calloc(varint_size, sizeof(char));
    if (copied_varint == NULL) {
        err_code = ALLOCERR;
        return NULL;
    }
    for (size_t i =  0; i < varint_size; ++i) {
        copied_varint[i] = varint[i];
    }
    return copied_varint;
}

static char* CopySTRING(char* string) {
    char* str_ptr = string;
    char *str_copy = calloc(strlen(str_ptr) + 1, sizeof(*str_copy));
    if (str_copy == NULL) {
        err_code = ALLOCERR;
        return NULL;
    }
    strcpy(str_copy, str_ptr);
    return str_copy;
}

static char* CopyBARRAY(char* barray) {
    char* barray_iter = barray;
    while (*barray_iter >> (CHAR_BIT - 1)) {
        ++barray_iter;
    }
    size_t barray_size = (barray_iter - barray) / sizeof(char) + 2; //NOLINT
    char* copied_barray = calloc(barray_size, sizeof(char));
    if (copied_barray == NULL) {
        err_code = ALLOCERR;
        return NULL;
    }
    for (size_t i = 0; i < barray_size; ++i) {
        copied_barray[i] = barray[i];
    }
    return copied_barray;
}

void ClearLayouts() {
    LayoutNode* curr_node = custom_layouts;
    while (curr_node != NULL) {
        LayoutNode* next_node = curr_node->next;
        free(curr_node);
        curr_node = next_node;
    }
    custom_layouts = NULL;
}

const Layout* CreateLayout(size_t field_q, ...) {
    va_list ap;
    va_start(ap, field_q);

    LayoutNode* new_layout_node = malloc(sizeof(LayoutNode) + sizeof(Layout) + field_q * sizeof(Layout*));
    if (new_layout_node == NULL) {
        return NULL;
    }

    new_layout_node->next = NULL;
    new_layout_node->prev = NULL;
    Layout* new_layout = (Layout*)(new_layout_node + 1);
    new_layout->field_q = field_q;
    new_layout->fields = (const Layout**)(new_layout + 1);
    new_layout->kind = CONSTRUCTED;

    for (size_t i = 0; i < field_q; ++i) {
        const Layout* curr_layout = va_arg(ap, const Layout*);
        new_layout->fields[i] = curr_layout;
    }
    va_end(ap);

    PushLayoutNode(new_layout_node);
    return new_layout;
}

int DeleteLayout(const Layout* layout) {
    LayoutNode* curr_node = custom_layouts;
    while (curr_node != NULL) {
        Layout* curr_layout = (Layout*)(curr_node + 1);
        if (curr_layout == layout) {
            LayoutNode* prev_layout_node = curr_node->prev;
            LayoutNode* next_layout_node = curr_node->next;
            if (prev_layout_node != NULL) {
                prev_layout_node->next = next_layout_node;
            }
            if (next_layout_node != NULL) {
                next_layout_node->prev = prev_layout_node;
            }
            free(curr_node);
            return 0;
        }
        curr_node = curr_node->next;
    }
    return 1;
}

Value ConstructPrimitiveValue(const Layout* layout, ...) {
    Value constructed_value = {NULL, 0};
    if (err_code != NOERR) {
        return constructed_value;
    }

    va_list ap;
    va_start(ap, layout);

    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) {
            int64_t val = va_arg(ap, int64_t);
            memcpy(&constructed_value.int_value, &val, sizeof(int64_t));
        } else if (layout->kind == UINT64) {
            uint64_t val = va_arg(ap, uint64_t);
            memcpy(&constructed_value.uint_value, &val, sizeof(uint64_t));
        } else if (layout->kind == INT32) {
            int32_t val = va_arg(ap, int32_t);
            memcpy(&constructed_value.int_value, &val, sizeof(int32_t));
        } else if (layout->kind == UINT32) {
            uint32_t val = va_arg(ap, uint32_t);
            memcpy(&constructed_value.uint_value, &val, sizeof(uint32_t));
        } else if (layout->kind == VARINT) {
            char* varint = va_arg(ap, char*);
            constructed_value.varint_value = CopyVARINT(varint);
        } else if (layout->kind == DOUBLE) {
            constructed_value.double_value = va_arg(ap, double);
        } else if (layout->kind == FLOAT) {
            constructed_value.float_value = va_arg(ap, double);
        } else if (layout->kind == BOOL) {
            if (va_arg(ap, int) != 0) {
                constructed_value.bool_value = 1;
            }
        } else if (layout->kind == STRING) {
            char *str_ptr = va_arg(ap, char *);
            char *str_copy = calloc(strlen(str_ptr), sizeof(*str_copy));
            strcpy(str_copy, str_ptr);
            constructed_value.string_ptr = str_copy;
        } else if (layout->kind == BARRAY) {
            char* barray = va_arg(ap, char *);
            constructed_value.barray_value = CopyBARRAY(barray);
        } else {
            err_code = UNKNKIND;
        }
    } else {
        err_code = CONSTRERR;
    }
    va_end(ap);
    return constructed_value;
}

Value ConstructPrimitiveValueNoCopy(const Layout* layout, ...) {
        Value constructed_value = {NULL, 0};
    if (err_code != NOERR) {
        return constructed_value;
    }

    va_list ap;
    va_start(ap, layout);

    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) {
            int64_t val = va_arg(ap, int64_t);
            memcpy(&constructed_value.int_value, &val, sizeof(int64_t));
        } else if (layout->kind == UINT64) {
            uint64_t val = va_arg(ap, uint64_t);
            memcpy(&constructed_value.uint_value, &val, sizeof(uint64_t));
        } else if (layout->kind == INT32) {
            int32_t val = va_arg(ap, int32_t);
            memcpy(&constructed_value.int_value, &val, sizeof(int32_t));
        } else if (layout->kind == UINT32) {
            uint32_t val = va_arg(ap, uint32_t);
            memcpy(&constructed_value.uint_value, &val, sizeof(uint32_t));
        } else if (layout->kind == VARINT) {
            char* varint = va_arg(ap, char*);
            constructed_value.varint_value = varint;
        } else if (layout->kind == DOUBLE) {
            constructed_value.double_value = va_arg(ap, double);
        } else if (layout->kind == FLOAT) {
            constructed_value.float_value = va_arg(ap, double);
        } else if (layout->kind == BOOL) {
            constructed_value.bool_value = va_arg(ap, int);
        } else if (layout->kind == STRING) {
            char *str_ptr = va_arg(ap, char *);
            constructed_value.string_ptr = str_ptr;
        } else if (layout->kind == BARRAY) {
            char* barray = va_arg(ap, char *);
            constructed_value.barray_value = barray;
        } else {
            va_end(ap);
            err_code = UNKNKIND;
        }
    } else {
        err_code = CONSTRERR;
    }
    va_end(ap);
    return constructed_value;
}

Value CopyValue(const Layout *layout, const Value *value) {
    err_code = NOERR;
    Value copy = {NULL, 0};
    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) { //NOLINT
            copy.int_value = value->int_value;
        } else if (layout->kind == UINT64) { //NOLINT
            copy.uint_value = value->uint_value;
        } else if (layout->kind == INT32) {
            copy.int_value = value->int_value;
        } else if (layout->kind == UINT32) {
            copy.uint_value = value->uint_value;
        } else if (layout->kind == VARINT) {
            copy.varint_value = CopyVARINT(value->varint_value);
            if (err_code != NOERR) {
                return copy;
            }
        } else if (layout->kind == DOUBLE) {
            copy.double_value = value->double_value;
        } else if (layout->kind == FLOAT) {
            copy.float_value = value->float_value;
        } else if (layout->kind == BOOL) {
            copy.bool_value = value->bool_value;
        } else if (layout->kind == STRING) {
            copy.string_ptr = CopySTRING(value->string_ptr);
            if (err_code != NOERR) {
                return copy;
            }
        } else if (layout->kind == BARRAY) {
            copy.barray_value = CopyBARRAY(value->barray_value);
            if (err_code != NOERR) {
                return copy;
            }
        } else {
            err_code = UNKNKIND;
        }
    } else {
        Node *initial_node = GetNode();
        if (initial_node == NULL) {
            err_code = ALLOCERR;
            return copy;
        }
        initial_node->next = NULL;
        initial_node->prev = NULL;
        initial_node->value_place = &copy;
        initial_node->layout = layout;
        initial_node->value = value;
        List nodes = {initial_node, initial_node};

        while (Empty(&nodes) == 0) {
            Node *cur_node = PopFront(&nodes);
            if (cur_node->layout->kind != CONSTRUCTED) {
                *cur_node->value_place = CopyValue(cur_node->layout, cur_node->value);
                if (err_code != NOERR) {
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return copy;
                }
            } else {
                Value *cur_value = cur_node->value_place;
                cur_value->children = calloc(cur_node->layout->field_q,
                                             sizeof(*cur_value->children));
                if (cur_value->children == NULL) {
                    err_code = ALLOCERR;
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return copy;
                }
                for (ssize_t i = cur_node->layout->field_q - 1; i >= 0; --i) {
                    Node *node = GetNode();
                    const Value *value_ptr = cur_node->value;
                    if (node == NULL) {
                        err_code = ALLOCERR;
                        Clear(&nodes);
                        DestroyNode(cur_node);
                        return copy;
                    }
                    node->layout = cur_node->layout->fields[i];
                    node->value_place = &cur_value->children[i];
                    node->value = &value_ptr->children[i];
                    PushFront(&nodes, node);
                }
            }
            GiveNode(cur_node);
        }
    }
    Clear(&avalible_nodes);
    return copy;
}

Value ConstructConstructedValue(const Layout* layout, ...) {
    Value out = {.children = NULL};

    if (layout->field_q == 0) {
        err_code = CONSTRERR;
        return out;        
    }
    va_list ap;
    va_start(ap, layout);

    Value* children = calloc(layout->field_q, sizeof(Value));
    if (children == NULL) {
        err_code = ALLOCERR;
        va_end(ap);
        return out;
    }
    for (size_t i  = 0; i < layout->field_q; ++i) {
        Value curr_val = va_arg(ap, Value);
        children[i] = curr_val;
    }
    va_end(ap);
    out.children = children;
    return out;
}

void DestroyValue(const Layout *layout, Value* value) {
    err_code = NOERR;
    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == VARINT) {
            free(value->varint_value);
        } else if (layout->kind == STRING) {
            free(value->string_ptr);

        } else if (layout->kind == BARRAY) {
            free(value->barray_value);
        } else {
            return;
        }
    } else {
        Node *initial_node = GetNode();
        if (initial_node == NULL) {
            err_code = ALLOCERR;
            return;
        }
        initial_node->next = NULL;
        initial_node->prev = NULL;
        initial_node->layout = layout;
        initial_node->value_place = value;
        List nodes = {initial_node, initial_node};

        while (!Empty(&nodes)) {
            Node *cur_node = PopFront(&nodes);
            if (cur_node->layout->kind != CONSTRUCTED) {
                DestroyValue(cur_node->layout, cur_node->value_place);
                if (err_code != NOERR) {
                    GiveNode(cur_node);
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return;
                }
            } else {
                Value *cur_value = cur_node->value_place;
                if (cur_value->children == NULL) {
                    GiveNode(cur_node);
                    continue;
                }
                for (ssize_t i = cur_node->layout->field_q - 1; i >= 0; --i) {
                    Node *node = GetNode();
                    node->layout = cur_node->layout->fields[i];
                    node->value_place = &cur_value->children[i];
                    PushFront(&nodes, node);
                }
            }
            GiveNode(cur_node);
        }
    }
    Clear(&avalible_nodes);
}