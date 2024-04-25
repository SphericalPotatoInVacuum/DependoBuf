#include "serializer/layout/layout.h"
#include "serializer/layout/linked_list.h"
#include "serializer/layout/node_handler.h"

#include <stdarg.h>

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

static void PushLayoutNode(LayoutNode* layout_node) {
    if (custom_layouts == NULL) {
        custom_layouts = layout_node;
        return;
    }
    layout_node->next = custom_layouts;
    custom_layouts->prev = layout_node;
    custom_layouts = layout_node;
}

void ClearLayouts() {
    LayoutNode* curr_node = custom_layouts;
    while (curr_node != NULL) {
        LayoutNode* next_node = curr_node->next;
        free(curr_node);
        curr_node = next_node;
    }
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

Value CreateValue(const Layout* layout, ...) {
    va_list ap;
    va_start(ap, layout);

    Value res = {.children = NULL, .uint_value = 0};
    if (layout->kind == CONSTRUCTED) {
        Value* children = calloc(layout->field_q, sizeof(Value));
        if (children == NULL) {
            err_code = 1;
            return res;
        }
        for (size_t i = 0; i < layout->field_q; ++i) {
            Value curr_val = va_arg(ap, Value);
            children[i] = curr_val;
        }
        res.children = children;
    } else if (layout->kind == UINT32 || layout->kind == INT32) {
        uint32_t val = va_arg(ap, uint32_t);
        res.uint_value = val;
    } else if (layout->kind == UINT64 || layout->kind == INT64) {
        uint64_t val = va_arg(ap, uint64_t);
        res.uint_value = val;
    } else if (layout->kind == VARINT) {
        char* val = va_arg(ap, char*);
        res.varint_value = val;
    } else if (layout->kind == BARRAY) {
        char* val = va_arg(ap, char*);
        res.barray_value = val;
   } else if (layout->kind == STRING) {
        char* val = va_arg(ap, char*);
        res.string_ptr = val;
   } else if (layout->kind == FLOAT) {
        uint32_t float_bits = va_arg(ap, uint32_t);
        res.float_value = *(float*)(&float_bits);
   } else if (layout->kind == DOUBLE) {
        double val = va_arg(ap, double);
        res.double_value = val;
   } else {
        err_code = 1;
   }
   va_end(ap);
   return res;
}

Value ConstructValue(const Layout *layout, void **values) {
    err_code = NOERR;
    Value constructed_value = {NULL, 0};
    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) {
            constructed_value.int_value = *(int64_t *)values;
        } else if (layout->kind == UINT64) {
            constructed_value.uint_value = *(uint64_t *)values;
        } else if (layout->kind == INT32) {
            constructed_value.int_value = *(int32_t *)values;
        } else if (layout->kind == UINT32) {
            constructed_value.uint_value = *(uint32_t *)values;
        } else if (layout->kind == VARINT) {
            constructed_value.varint_value = *(char **)values;
        } else if (layout->kind == DOUBLE) {
            constructed_value.double_value = *(double *)values;
        } else if (layout->kind == FLOAT) {
            constructed_value.float_value = *(float *)values;
        } else if (layout->kind == BOOL) {
            constructed_value.bool_value = *(char *)values;
        } else if (layout->kind == STRING) {
            char *str_ptr = *(char **) values;
            char *str_copy = calloc(strlen(str_ptr), sizeof(*str_copy));
            strcpy(str_copy, str_ptr);
            constructed_value.string_ptr = str_copy;
        } else if (layout->kind == BARRAY) {
            char *barr_ptr = *(char **) values;
            char *barr_copy = calloc(strlen(barr_ptr), sizeof(*barr_copy));
            strcpy(barr_copy, barr_ptr);
            constructed_value.string_ptr = barr_copy;
        } else {
            err_code = UNKNKIND;
        }
    } else {
        Node *initial_node = GetNode();
        if (initial_node == NULL) {
            err_code = ALLOCERR;
            return constructed_value;
        }
        initial_node->next = NULL;
        initial_node->prev = NULL;
        initial_node->value_place = &constructed_value;
        initial_node->layout = layout;
        initial_node->data = values;
        List nodes = {initial_node, initial_node};

        while (Empty(&nodes) == 0) {
            Node *cur_node = PopFront(&nodes);
            if (cur_node->layout->kind != CONSTRUCTED) {
                *cur_node->value_place = ConstructValue(cur_node->layout, cur_node->data);
                if (err_code != NOERR) {
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return constructed_value;
                }
            } else {
                Value *cur_value = cur_node->value_place;
                cur_value->children = calloc(cur_node->layout->field_q,
                                             sizeof(*cur_value->children));
                if (cur_value->children == NULL) {
                    err_code = ALLOCERR;
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return constructed_value;
                }
                for (ssize_t i = cur_node->layout->field_q - 1; i >= 0; --i) {
                    Node *node = GetNode();
                    void **data_ptr = cur_node->data;
                    if (node == NULL) {
                        err_code = ALLOCERR;
                        Clear(&nodes);
                        DestroyNode(cur_node);
                        return constructed_value;
                    }
                    node->layout = cur_node->layout->fields[i];
                    node->value_place = &cur_value->children[i];
                    node->data = (void **)data_ptr[i];
                    PushFront(&nodes, node);
                }
            }
            GiveNode(cur_node);
        }
    }
    return constructed_value;
}

Value ConstructPrimitiveValue(const Layout* layout, ...) {
    err_code = NOERR;

    va_list ap;
    va_start(ap, layout);

    Value constructed_value = {NULL, 0};
    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) {
            constructed_value.int_value = va_arg(ap, int64_t);
        } else if (layout->kind == UINT64) {
            constructed_value.uint_value = va_arg(ap, uint64_t);
        } else if (layout->kind == INT32) {
            constructed_value.int_value = va_arg(ap, int32_t);
        } else if (layout->kind == UINT32) {
            constructed_value.uint_value = va_arg(ap, uint32_t);
        } else if (layout->kind == VARINT) {
            constructed_value.varint_value = va_arg(ap, char *);
        } else if (layout->kind == DOUBLE) {
            constructed_value.double_value = va_arg(ap, double);
        } else if (layout->kind == FLOAT) {
            constructed_value.float_value = va_arg(ap, double);
        } else if (layout->kind == BOOL) {
            constructed_value.bool_value = va_arg(ap, int);
        } else if (layout->kind == STRING) {
            char *str_ptr = va_arg(ap, char *);
            char *str_copy = calloc(strlen(str_ptr), sizeof(*str_copy));
            strcpy(str_copy, str_ptr);
            constructed_value.string_ptr = str_copy;
        } else if (layout->kind == BARRAY) {
            constructed_value.barray_value = va_arg(ap, char *);
        } else {
            err_code = UNKNKIND;
        }
    } else {
        err_code = CONSTRERR;
    }

    return constructed_value;
}

Value CopyValue(const Layout *layout, const Value *value) {
    err_code = NOERR;
    Value copy = {NULL, 0};
    if (layout->kind != CONSTRUCTED) {
        if (layout->kind == INT64) {
            copy.int_value = value->int_value;
        } else if (layout->kind == UINT64) {
            copy.uint_value = value->uint_value;
        } else if (layout->kind == INT32) {
            copy.int_value = value->int_value;
        } else if (layout->kind == UINT32) {
            copy.uint_value = value->uint_value;
        } else if (layout->kind == VARINT) {
            copy.varint_value = value->varint_value;
        } else if (layout->kind == DOUBLE) {
            copy.double_value = value->double_value;
        } else if (layout->kind == FLOAT) {
            copy.float_value = value->float_value;
        } else if (layout->kind == BOOL) {
            copy.bool_value = value->bool_value;
        } else if (layout->kind == STRING) {
            char *str_ptr = value->string_ptr;
            char *str_copy = calloc(strlen(str_ptr), sizeof(*str_copy));
            strcpy(str_copy, str_ptr);
            copy.string_ptr = str_copy;
        } else if (layout->kind == BARRAY) {
            char *barr_ptr = value->barray_value;
            char *barr_copy = calloc(strlen(barr_ptr), sizeof(*barr_copy));
            strcpy(barr_copy, barr_ptr);
            copy.string_ptr = barr_copy;
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
                *cur_node->value_place = ConstructValue(cur_node->layout, &cur_node->value->uint_value);
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
    return copy;
}

Value CreateBoolValue(char bool) {
    Value res = {.bool_value = bool, .children = NULL};
    return res;
}

void DestroyValue(Value* value) {
    free(value->children);
    value->children = NULL;
}