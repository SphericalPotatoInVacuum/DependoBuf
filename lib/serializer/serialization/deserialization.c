#include "serializer/serialization/serialization.h"

static char *DeserializeVARINT(char **cur_buf_pos) {
    err_code = 0;
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
        perror("Could not allocate space\n");
        return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        varint[i] = tmp[i];
    }

    return varint;
}

//Deserializes built-in types.
static Value DeserializePrimitive(const Layout *layout, char **cur_buf_pos) {
    err_code = 0;
    Value value = {NULL, 0};
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
    } else {
        perror("Unknown type in layout\n");
        err_code = 1;
    }

    return value;
}

//Deserializes one element from layout. Returns constructed value.
static Value DeserializeUnit(const Layout *layout, char **cur_buf_pos) {
    err_code = 0;
    Value value = {NULL, 0};
    if (layout->kind != CONSTRUCTED) {
        value = DeserializePrimitive(layout, cur_buf_pos);
        if (err_code != 0) {
            perror("Failed to deserialize built-in type\n");
        }
    } else {
        Node *initial_node = calloc(1, sizeof(*initial_node));
        if (initial_node == NULL) {
            err_code = 1;
            perror("Could not allocate space\n");
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
                    perror("Failed to deserialize built-in type\n");
                }
            } else {
                Value *cur_value = cur_node->value_place;
                cur_value->children = calloc(cur_node->layout->field_q,
                                                         sizeof(*cur_value->children));
                if (cur_value->children == NULL) {
                    err_code = 1;
                    perror("Could not allocate space\n");
                    Clear(&nodes);
                    DestroyNode(cur_node);
                    return value;
                }
                for (ssize_t i = cur_node->layout->field_q - 1; i >= 0; --i) {
                    Node *node = calloc(1, sizeof(*node));
                    if (node == NULL) {
                        err_code = 1;
                        perror("Could not allocate space\n");
                        Clear(&nodes);
                        DestroyNode(cur_node);
                        return value;
                    }
                    node->layout = cur_node->layout->fields[i];
                    node->value_place = &cur_value->children[i];
                    PushFront(&nodes, node);
                }
            }
            DestroyNode(cur_node);
        }
    }

    return value;
}

//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer) {
    err_code = 0;
    Value value = DeserializeUnit(layout, &buffer);
    if (err_code != 0) {
        perror("Failed to deserialize\n");
        err_code = 1;
    }
    return value;
}