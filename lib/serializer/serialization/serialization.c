#include "serializer/serialization/serialization.h"
#include "serializer/serialization/node_handler.h"

//Counts bytes requierd for serialization of value
static size_t SerializedDataSize(const Layout* layout, Value* value);

//Handle serialization of certain kinds from Kind enum.
static int SerializeVARINT(char* byte_array, size_t* byte_iter, char* varint, size_t data_size);
static int SerializeINT64(char* byte_array, size_t* byte_iter, uint64_t inp, size_t data_size);
static int SerializeINT32(char* byte_array, size_t* byte_iter, uint32_t inp, size_t data_size);
static int SerializeBOOL(char* byte_array, size_t* byte_iter, char inp, size_t data_size);
static int SerializeSTRING(char* byte_array, size_t* byte_iter, char* string_iter, size_t data_size);
static int SerializeFLOAT(char* byte_array, size_t* byte_iter, float inp, size_t data_size);
static int SerializeDOUBLE(char* byte_array, size_t* byte_iter, double inp, size_t data_size);
static int SerializeBARRAY(char* byte_array, size_t* byte_iter, char* barray_value, size_t data_size);

//Handle errors, mop up used heap memory in case of an error.
static int HandleSerializationOutOfSizeError();
static void HandleSerializationArrayAllocationError();
static void HandleMidSerializationError(List* list, char* byte_array);
static void HandleUknownKindError(List* list, char* byte_array);

static int HandleSerializationOutOfSizeError() {
    err_code = 1;
    return err_code;
}

static void HandleSerializationArrayAllocationError() {
    err_code = 1;
}

static void HandleMidSerializationError(List* list, char* byte_array) {
    Clear(list);
    free(byte_array);
}

static void HandleUknownKindError(List* list, char* byte_array) {
    Clear(list);
    free(byte_array);
    err_code = 1;
}

static int SerializeBARRAY(char *byte_array, size_t *byte_iter, char *barray_value, size_t data_size) {
    while (*barray_value >> 7) {
        if (*byte_iter >= data_size) {
            return HandleSerializationOutOfSizeError();
        }
        byte_array[*byte_iter] = *barray_value;
        *byte_iter += 1;
        ++barray_value;
    }
    if (*byte_iter >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    byte_array[*byte_iter] = *barray_value;
    *byte_iter += 1;
    return 0;
}

static int SerializeDOUBLE(char *byte_array, size_t *byte_iter, double inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < 8) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, 8);
    *byte_iter += 8;
    return 0;
}

static int SerializeFLOAT(char *byte_array, size_t *byte_iter, float inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < 8) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, 4);
    *byte_iter += 4;
    return 0;
}

static int SerializeVARINT(char* byte_array, size_t* byte_iter, char* varint, size_t data_size) {
    while (*varint >> 7) {
        if (*byte_iter >= data_size) {
            return HandleSerializationOutOfSizeError();
        }
        byte_array[*byte_iter] = *varint;
        *byte_iter += 1;
        ++varint;
    }
    if (*byte_iter >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    byte_array[*byte_iter] = *varint;
    *byte_iter += 1;
    return 0;
}

static int SerializeINT64(char* byte_array, size_t* byte_iter, uint64_t inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < 8) {
        return HandleSerializationOutOfSizeError();
    } 
    memcpy(&byte_array[*byte_iter], &inp, 8);
    *byte_iter += 8;
    return 0;
}

static int SerializeINT32(char *byte_array, size_t *byte_iter, uint32_t inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < 4) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, 4);
    *byte_iter += 4;
    return 0;
}

static int SerializeBOOL(char *byte_array, size_t *byte_iter, char inp, size_t data_size) {
    if (*byte_iter >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, 1);
    *byte_iter += 1;
    return 0;
}

static int SerializeSTRING(char *byte_array, size_t *byte_iter, char *string_iter, size_t data_size) {
    while (*string_iter) {
        if (*byte_iter >= data_size) {
            return HandleSerializationOutOfSizeError();
        }
        byte_array[*byte_iter] = *string_iter;
        *byte_iter += 1;
        ++string_iter;
    }
    if (*byte_iter >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    byte_array[*byte_iter] = 0;
    *byte_iter += 1;
    return 0;
}

static size_t SerializedDataSize(const Layout* layout, Value* value) {
    if (err_code) {
        return 0;
    }
    List layer_list = {.head = NULL, .tail = NULL};
    size_t res = 0;

    Node* first_node = GetNode();
    if (first_node == NULL) {
        HandleNodeAllocationError(&layer_list);
        return 0;
    }
    first_node->layout = layout;
    first_node->value_place = value;
    PushBack(&layer_list, first_node);

    while(!Empty(&layer_list)) {
        Node* curr_node = PopFront(&layer_list);
        if (curr_node->layout->kind == CONSTRUCTED) {
            for (size_t i = 0; i < curr_node->layout->field_q; ++i) {
                Node* new_node = GetNode();
                if (new_node == NULL) {
                    HandleNodeAllocationError(&layer_list);
                    return 0;
                }
                new_node->layout = curr_node->layout->fields[i];
                new_node->value_place = &curr_node->value_place->children[i];
                PushBack(&layer_list, new_node);
            }
        } else if (curr_node->layout->kind == VARINT) {
            char* varint_byte = curr_node->value_place->varint_value;
            while (*varint_byte >> 7) {
                ++res;
                ++varint_byte;
            }
            ++varint_byte;
        } else if (curr_node->layout->kind == INT64 || curr_node->layout->kind == UINT64) {
            res += 8;
        } else if (curr_node->layout->kind == UINT32 || curr_node->layout->kind == INT32) {
            res += 4;
        } else if (curr_node->layout->kind == BOOL) {
            ++res;
        } else if (curr_node->layout->kind == STRING) {
            char* str_byte = curr_node->value_place->string_ptr;
            while (*str_byte) {
                ++res;
                ++str_byte;
            }
        } else {
            HandleUknownKindError(&layer_list, NULL);
            return 0;
        }
        GiveNode(curr_node);
    }
    return res;
}

char* Serialize(const Layout* layout, Value value) {
    if (err_code) {
        return NULL;
    }

    size_t data_size = SerializedDataSize(layout, &value);
    if (err_code) {
        return NULL;
    }

    char* byte_array = malloc(SerializedDataSize(layout, &value));
    if (byte_array == NULL) {
        HandleSerializationArrayAllocationError();
        return NULL;
    }

    size_t byte_iter = 0;
    List layer_list = {.head = NULL, .tail = NULL};
    Node* first_node = GetNode();
    if (first_node == NULL) {
        HandleNodeAllocationError(&layer_list);
        return NULL;
    }
    first_node->layout = layout;
    first_node->value_place = &value;
    PushBack(&layer_list, first_node);

    while (!Empty(&layer_list)) {
        Node* curr_node = PopFront(&layer_list);
        if (curr_node->layout->kind == CONSTRUCTED) {
            size_t iter = curr_node->layout->field_q - 1;
            for (size_t i = 0; i < curr_node->layout->field_q; ++i) {
                Node* new_node = GetNode();
                if (new_node == NULL) {
                    HandleNodeAllocationError(&layer_list);
                    return NULL;
                }
                new_node->layout = curr_node->layout->fields[curr_node->layout->field_q - 1 - i];
                new_node->value_place = &curr_node->value_place->children[curr_node->layout->field_q - 1 - i];
                PushFront(&layer_list, new_node); 
            }
        } else if (curr_node->layout->kind == VARINT) {
            int exit_code = SerializeVARINT(byte_array, &byte_iter, curr_node->value_place->varint_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == INT32 || curr_node->layout->kind == UINT32) {
            int exit_code = SerializeINT32(byte_array, &byte_iter, curr_node->value_place->uint_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == INT64 || curr_node->layout->kind == UINT64) {
            int exit_code = SerializeINT64(byte_array, &byte_iter, curr_node->value_place->uint_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array); 
                return NULL;
            }
        } else if (curr_node->layout->kind == BOOL) {
            int exit_code = SerializeBOOL(byte_array, &byte_iter, curr_node->value_place->bool_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == STRING) {
            int exit_code = SerializeSTRING(byte_array, &byte_iter, curr_node->value_place->string_ptr, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == BARRAY) {
            int exit_code = SerializeBARRAY(byte_array, &byte_iter, curr_node->value_place->barray_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == FLOAT) {
            int exit_code = SerializeFLOAT(byte_array, &byte_iter, curr_node->value_place->float_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else if (curr_node->layout->kind == DOUBLE) {
            int exit_code = SerializeDOUBLE(byte_array, &byte_iter, curr_node->value_place->double_value, data_size);
            if (exit_code) {
                HandleMidSerializationError(&layer_list, byte_array);
                return NULL;
            }
        } else {
            HandleUknownKindError(&layer_list, byte_array);
            return NULL;
        }
        GiveNode(curr_node);
    }
    Clear(&avalible_nodes);
    return byte_array;
}
