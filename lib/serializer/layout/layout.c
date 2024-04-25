#include "serializer/layout/layout.h"
#include "serializer/layout/linked_list.h"

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

Value CreateBoolValue(char bool) {
    Value res = {.bool_value = bool, .children = NULL};
    return res;
}

void DestroyValue(Value* value) {
    free(value->children);
    value->children = NULL;
}