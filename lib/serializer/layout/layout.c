#include "serializer/layout/layout.h"

#include <stdarg.h>

const Layout kInt64Layout = {.fields = NULL, .kind = INT64,.field_q = 0};
const Layout kUint64Layout = {.fields = NULL, .kind = UINT64, .field_q = 0};
const Layout kInt32Layout = {.fields = NULL, .kind = INT32, .field_q = 0};
const Layout kUint32Layout = {.fields = NULL, .kind = UINT32, .field_q = 0};
const Layout kDoubleLayout = {.fields = NULL, .kind = DOUBLE, .field_q = 0};
const Layout kVarintLayout = {.fields = NULL, .kind = VARINT, .field_q = 0};
const Layout kBoolLayout = {.fields = NULL, .kind = BOOL, .field_q = 0};
const Layout kStringLayout = {.fields = NULL, .kind = STRING, .field_q = 0};
const Layout kFloatLayout = {.fields = NULL, .kind = FLOAT, .field_q = 0};
const Layout kBarrayLayout = {.fields = NULL, .kind = BARRAY, .field_q = 0};

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