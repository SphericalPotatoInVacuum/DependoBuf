#include "serializer/error_handler/error_handler.h"
#include "serializer/layout/layout.h"
#include "serializer/serialization/serialization.h"
#include "serializer/layout/node_handler.h"
#include "serializer/layout/linked_list.h"


#include "stdlib.h"
#include "limits.h"
#include "memory.h"
#include "stdint.h"
#include "stdio.h"

static Value DeserializeUnit(const Layout* layout, char* encoded);

static char* DeserializeVARINT(char** encoded_ptr);
static char* DeserializeBARRAY(char** encoded_ptr);

static void MopUpAfterError(Node* curr_node, List* list, const Layout* layout, Value* vall);
static inline void HandleUknownKindError(Node* curr_node, List* list, const Layout* layout, Value* val);
static void HandleAllocationError(Node* curr_node, List* list, const Layout* layout, Value* val);

static void HandleAllocationError(Node* curr_node, List* list, const Layout* layout, Value* val) {
    err_code = ALLOCERR;
    MopUpAfterError(curr_node, list, layout, val);
}

static inline void HandleUknownKindError(Node* curr_node, List* list, const Layout* layout, Value* val) {
    err_code = UNKNKIND;
     MopUpAfterError(curr_node, list, layout, val);
}

static void MopUpAfterError(Node* curr_node, List* list, const Layout* layout, Value* val) {
    GiveNode(curr_node);
    Clear(list);
    Clear(&avalible_nodes);
    DestroyValue(layout, val);
}

static char* DeserializeVARINT(char** encoded_ptr) {
    char* encoded_iter = *encoded_ptr;
    while (*encoded_iter >> (CHAR_BIT - 1)) {
        ++encoded_iter;
    }
    size_t varint_size = (encoded_iter - *encoded_ptr)/sizeof(char) + 1; //NOLINT
    char* varint = calloc(varint_size, sizeof(char));
    if (varint == NULL) {
        err_code = ALLOCERR;
        return NULL;
    }
    for (size_t i = 0; i < varint_size; ++i) {
        varint[i] = **encoded_ptr;
        *encoded_ptr += 1;
    }
    return varint;
}

static char* DeserializeBARRAY(char** encoded_ptr) {
    char* encoded_iter = *encoded_ptr;
    while (*encoded_iter >> (CHAR_BIT - 1)) {
        ++encoded_iter;
    }
    size_t barray_size = (encoded_iter - *encoded_ptr)/sizeof(char) + 2; //NOLINT
    char* barray = calloc(barray_size, sizeof(char));
    if (barray == NULL) {
        err_code = ALLOCERR;
        return NULL;
    }
    for (size_t i = 0; i < barray_size; ++i) {
        barray[i] = **encoded_ptr;
        *encoded_ptr += 1;
    }
    return barray;
}

static Value DeserializeUnit(const Layout* layout, char* encoded) {
    Value value = {NULL, 0};
    if (err_code != NOERR) {
        return value;
    }

    List layer_list = {.head = NULL, .tail = NULL};
    Node* initial_node = GetNode();
    if (initial_node == NULL) {
        HandleAllocationError(initial_node, &layer_list, layout, &value);
        return value;
    }
    initial_node->value_place = &value;
    initial_node->layout = layout;
    PushFront(&layer_list, initial_node);

    while (!Empty(&layer_list)) {
        Node* curr_node = PopFront(&layer_list);
        if (curr_node->layout->kind == CONSTRUCTED) {
            Value* children = calloc(curr_node->layout->field_q, sizeof(Value));
            if (children == NULL) {
                HandleAllocationError(curr_node, &layer_list, layout, &value);
                return value;
            }
            curr_node->value_place->children = children;
            for (size_t i = 0; i < curr_node->layout->field_q; ++i) {
                Node* new_node = GetNode();
                if (new_node == NULL) {
                    HandleAllocationError(curr_node, &layer_list, layout, &value);
                    return value;
                }   
                new_node->value_place = &children[curr_node->layout->field_q - 1 - i];
                new_node->layout = curr_node->layout->fields[curr_node->layout->field_q - 1 - i];
                PushFront(&layer_list, new_node);
            }
    } else if (curr_node->layout->kind == INT32 || curr_node->layout->kind == UINT32 || curr_node->layout->kind == FLOAT) {
        memcpy(&curr_node->value_place->uint_value, encoded, sizeof(uint32_t));
        encoded += sizeof(uint32_t);
    } else if (curr_node->layout->kind == INT64 || curr_node->layout->kind == UINT64 || curr_node->layout->kind == DOUBLE) {
        memcpy(&curr_node->value_place->uint_value, encoded, sizeof(uint64_t));
        encoded += sizeof(uint64_t);
    } else if (curr_node->layout->kind == STRING) {
        size_t string_size = strlen(encoded) + 1;
        char* string = calloc(string_size, sizeof(char));
        if (string == NULL) {
            HandleAllocationError(curr_node, &layer_list, layout, &value);
            return value;
        }
        strcpy(string, encoded);
        curr_node->value_place->string_ptr = string;
        encoded += string_size;
    } else if (curr_node->layout->kind == VARINT) {
        char* varint = DeserializeVARINT(&encoded);
        if (varint == NULL) {
            HandleAllocationError(curr_node, &layer_list, layout, &value);
            return value;
        }
        curr_node->value_place->varint_value = varint;
    } else if (curr_node->layout->kind == BARRAY) {
        char* barray = DeserializeBARRAY(&encoded);
        if (barray == NULL) {
            HandleAllocationError(curr_node, &layer_list, layout, &value);
            return value;
        }
        curr_node->value_place->barray_value = barray;
    } else if (curr_node->layout->kind == BOOL) {
        memcpy(&curr_node->value_place->uint_value, encoded, sizeof(char));
        encoded += sizeof(char);
    } else {
        err_code = UNKNKIND;
        HandleUknownKindError(curr_node, &layer_list, layout, &value);
    }
    GiveNode(curr_node); 
}  
    Clear(&layer_list);
    return value;
}

Value Deserialize(const Layout *layout, char *buffer) {
    if (err_code != NOERR) {
        Value value = {.children = NULL, .string_ptr = NULL};
        return value;
    }
    Value value = DeserializeUnit(layout, buffer);
    Clear(&avalible_nodes);

    return value;
}