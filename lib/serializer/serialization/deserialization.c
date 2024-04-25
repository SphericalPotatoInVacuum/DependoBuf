#include "serializer/error_handler/error_handler.h"
#include "serializer/serialization/serialization.h"
#include "serializer/serialization/node_handler.h"

static char *DeserializeVARINT(char **cur_buf_pos) {
    char tmp[8];
    size_t size = 0;
    while (**cur_buf_pos >> 7) {
        tmp[size] = **cur_buf_pos;
        ++size;
        ++*cur_buf_pos;
    }
    tmp[size] = **cur_buf_pos;
    ++size;
    ++*cur_buf_pos;
    char *varint = calloc(size, sizeof(*varint));
    if (varint == NULL) {
        err_code = 1;
        return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        varint[i] = tmp[i];
    }

    return varint;
}

static char* DeserializeBARRAY(char **cur_buf_pos) {
    char* start_point = *cur_buf_pos;
    while (**cur_buf_pos >> 7) {
        ++(*cur_buf_pos);
    }
    *cur_buf_pos += 2;
    size_t barray_size = *cur_buf_pos - start_point;
    char* barray = calloc(barray_size, sizeof(char));
    if (barray == NULL) {
        err_code = 1;
        return NULL;
    }
    
    for (size_t i = 0; i < barray_size; ++i) {
        barray[i] = *start_point;
        ++start_point;
    }
    return barray;
}  

static char* DeserializeSTRING(char **cur_buf_pos) {
    char* start_point = *cur_buf_pos;
    while (**cur_buf_pos) {
        ++(*cur_buf_pos);
    }
    ++(*cur_buf_pos);
    size_t str_size = *cur_buf_pos - start_point;
    char* str = calloc(str_size, sizeof(char));

    for (size_t i = 0; i < str_size; ++i) {
        str[i] = *start_point;
        ++start_point;
    }
    return str;
}

//Deserializes built-in types.
static Value DeserializePrimitive(const Layout *layout, char **cur_buf_pos) {
    Value value = {NULL, 0};
    if (err_code) {
        return value;
    }
    if (layout->kind == INT64) {
        int64_t val = *(int64_t *) (*cur_buf_pos);
        *cur_buf_pos += 8;
        value.int_value = val;
    } else if (layout->kind == UINT64) {
        uint64_t val = *(uint64_t *) (*cur_buf_pos);
        *cur_buf_pos += 8;
        value.uint_value = val;
    } else if (layout->kind == INT32) {
        int32_t val = *(int32_t *) (*cur_buf_pos);
        *cur_buf_pos += 4;
        value.int_value = val;
    } else if (layout->kind == UINT32) {
        uint32_t val = *(uint32_t *) (*cur_buf_pos);
        *cur_buf_pos += 4;
        value.uint_value = val;
    } else if (layout->kind == VARINT) {
        value.varint_value = DeserializeVARINT(cur_buf_pos);;
    } else if (layout->kind == BARRAY) {
        value.barray_value = DeserializeBARRAY(cur_buf_pos);
    } else if (layout->kind == DOUBLE) {
        double val = *(double *)(*cur_buf_pos);
        *cur_buf_pos += 8;
        value.double_value = val;
    } else if (layout->kind == FLOAT) {
        float val = *(float *)(*cur_buf_pos);
        *cur_buf_pos += 4;
        value.float_value = val;
    } else if (layout->kind == STRING) {
        value.string_ptr = DeserializeSTRING(cur_buf_pos);
    } else {
        err_code = 1;
    }

    return value;
}

//Deserializes one element from layout. Returns constructed value.
static Value DeserializeUnit(const Layout *layout, char **cur_buf_pos) {
    Value value = {NULL, 0};
    if (err_code) {
        return value;
    }
    if (layout->kind != CONSTRUCTED) {
        value = DeserializePrimitive(layout, cur_buf_pos);
        if (err_code != 0) {
        }
    } else {
        Node *initial_node = GetNode();
        if (initial_node == NULL) {
            err_code = 1;
            return value;
        }
        initial_node->next = NULL;
        initial_node->prev = NULL;
        initial_node->value_place = &value;
        initial_node->layout = layout;
        List nodes = {initial_node, initial_node};

        while (Empty(&nodes) == 0) {
            Node *cur_node = PopFront(&nodes);
            if (cur_node->layout->kind != CONSTRUCTED) {
                *cur_node->value_place = DeserializePrimitive(cur_node->layout, cur_buf_pos);
                if (err_code != 0) {
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return value;
                }
            } else {
                Value *cur_value = cur_node->value_place;
                cur_value->children = calloc(cur_node->layout->field_q,
                                                         sizeof(*cur_value->children));
                if (cur_value->children == NULL) {
                    err_code = 1;
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return value;
                }
                for (ssize_t i = cur_node->layout->field_q - 1; i >= 0; --i) {
                    Node *node = GetNode();
                    if (node == NULL) {
                        err_code = 1;
                        Clear(&nodes);
                        DestroyNode(cur_node);
                        return value;
                    }
                    node->layout = cur_node->layout->fields[i];
                    node->value_place = &cur_value->children[i];
                    PushFront(&nodes, node);
                }
            }
            GiveNode(cur_node);
        }
    }

    return value;
}

//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer) {
    if (err_code) {
        Value value = {.children = NULL, .uint_value = 0};
        return value;
    }
    Value value = DeserializeUnit(layout, &buffer);
    Clear(&avalible_nodes);

    return value;
}