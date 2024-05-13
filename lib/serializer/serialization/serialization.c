#include "serializer/serialization/serialization.h"
#include "serializer/error_handler/error_handler.h"
#include "serializer/layout/node_handler.h"
#include "serializer/layout/linked_list.h"
#include "serializer/snappy-c/snappy-c.h"

#include "stdlib.h"
#include "string.h"
#include <limits.h>

//Counts bytes requierd for serialization of value
static size_t SerializedDataSize(const Layout* layout, Value* value);

//Handle serialization of certain kinds from Kind enum.
static int SerializeVARINT(char* byte_array, size_t* byte_iter, const char* varint, size_t data_size);
static int SerializeINT64(char* byte_array, size_t* byte_iter, uint64_t inp, size_t data_size);
static int SerializeINT32(char* byte_array, size_t* byte_iter, uint32_t inp, size_t data_size);
static int SerializeBOOL(char* byte_array, size_t* byte_iter, char inp, size_t data_size);
static int SerializeSTRING(char* byte_array, size_t* byte_iter, const char* string_iter, size_t data_size);
static int SerializeFLOAT(char* byte_array, size_t* byte_iter, float inp, size_t data_size);
static int SerializeDOUBLE(char* byte_array, size_t* byte_iter, double inp, size_t data_size);
static int SerializeBARRAY(char* byte_array, size_t* byte_iter, const char* barray_value, size_t data_size);

//Handle errors, mop up used heap memory in case of an error.
static inline int HandleSerializationOutOfSizeError();
static inline void MopUpAfterError(List* list);
static inline void HandleAllocationError();
static inline void HandleUknownKindError();

static inline int HandleSerializationOutOfSizeError() {
    err_code = SERSIZEERR;
    return err_code;
}

static inline void HandleAllocationError() {
    err_code = ALLOCERR;
}

static inline void MopUpAfterError(List* list) {
    Clear(&avalible_nodes);
    Clear(list);
}

static inline void HandleUknownKindError() {
    err_code = UNKNKIND;
}

static int SerializeBARRAY(char *byte_array, size_t *byte_iter, const char *barray_value, size_t data_size) {
    while (*barray_value >> (CHAR_BIT - 1)) {
        if (*byte_iter >= data_size) {
            return HandleSerializationOutOfSizeError();
        }
        byte_array[*byte_iter] = *barray_value;
        *byte_iter += 1;
        ++barray_value;
    }
    if (*byte_iter + 1 >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    byte_array[*byte_iter] = *barray_value;
    *byte_iter += 1;
    byte_array[*byte_iter] = *(barray_value + 1);
    *byte_iter += 1;
    return 0;
}

static int SerializeDOUBLE(char *byte_array, size_t *byte_iter, double inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < sizeof(double)) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, sizeof(double));
    *byte_iter += sizeof(double);
    return 0;
}

static int SerializeFLOAT(char *byte_array, size_t *byte_iter, float inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < sizeof(float)) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, sizeof(float));
    *byte_iter += sizeof(float);
    return 0;
}

static int SerializeVARINT(char* byte_array, size_t* byte_iter, const char* varint, size_t data_size) {
    while (*varint >> (CHAR_BIT - 1)) {
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
    if (*byte_iter >= data_size || data_size - *byte_iter < sizeof(int64_t)) {
        return HandleSerializationOutOfSizeError();
    } 
    memcpy(&byte_array[*byte_iter], &inp, sizeof(int64_t));
    *byte_iter += sizeof(int64_t);
    return 0;
}

static int SerializeINT32(char *byte_array, size_t *byte_iter, uint32_t inp, size_t data_size) {
    if (*byte_iter >= data_size || data_size - *byte_iter < sizeof(int32_t)) {
        return HandleSerializationOutOfSizeError();
    }
    memcpy(&byte_array[*byte_iter], &inp, sizeof(int32_t));
    *byte_iter += sizeof(int32_t);
    return 0;
}

static int SerializeBOOL(char *byte_array, size_t *byte_iter, char inp, size_t data_size) {
    if (*byte_iter >= data_size) {
        return HandleSerializationOutOfSizeError();
    }
    byte_array[*byte_iter] = inp;
    *byte_iter += 1;
    return 0;
}

static int SerializeSTRING(char *byte_array, size_t *byte_iter, const char *string_iter, size_t data_size) {
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
    if (err_code != NOERR) {
        return 0;
    }

    List layer_list = {.head = NULL, .tail = NULL};
    size_t res = 0;

    Node* first_node = GetNode();
    if (first_node == NULL) {
        err_code = ALLOCERR;
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
                    err_code = ALLOCERR;
                    GiveNode(curr_node);
                    MopUpAfterError(&layer_list);
                    return 0;
                }
                new_node->layout = curr_node->layout->fields[i];
                new_node->value_place = &curr_node->value_place->children[i];
                PushBack(&layer_list, new_node);
            }
        } else if (curr_node->layout->kind == VARINT) {
            char* varint_byte = curr_node->value_place->varint_value;
            while (*varint_byte >> (CHAR_BIT - 1)) {
                ++res;
                ++varint_byte;
            }
            ++res;
        } else if (curr_node->layout->kind == INT64 || curr_node->layout->kind == UINT64 || curr_node->layout->kind == DOUBLE) {
            res += sizeof(int64_t);
        } else if (curr_node->layout->kind == UINT32 || curr_node->layout->kind == INT32 || curr_node->layout->kind == FLOAT) {
            res += sizeof(int32_t);
        } else if (curr_node->layout->kind == BOOL) {
            ++res;
        } else if (curr_node->layout->kind == STRING) {
            char* str_byte = curr_node->value_place->string_ptr;
            while (*str_byte) {
                ++res;
                ++str_byte;
            }
            ++res;
        } else if (curr_node->layout->kind == BARRAY) {
            char* barray_byte = curr_node->value_place->varint_value;
            while (*barray_byte >> (CHAR_BIT - 1)) {
                ++res;
                ++barray_byte;
            }
            res += 2;
        } else {
            HandleUknownKindError();
            GiveNode(curr_node);
            MopUpAfterError(&layer_list);
            return 0;
        }
        GiveNode(curr_node);
    }
    return res;
}

void SerializeInBuffer(const Layout *layout, Value* value, char *byte_array, size_t data_size) {
    if (err_code != NOERR) {
        return;
    }
    size_t byte_iter = 0;
    List layer_list = {.head = NULL, .tail = NULL};
    Node* first_node = GetNode();
    if (first_node == NULL) {
        HandleAllocationError();
        MopUpAfterError(&layer_list);
        return;
    }
    first_node->layout = layout;
    first_node->value_place = value;
    PushBack(&layer_list, first_node);

    while (!Empty(&layer_list)) {
        Node* curr_node = PopFront(&layer_list);
        if (curr_node->layout->kind == CONSTRUCTED) {
            size_t iter = curr_node->layout->field_q - 1;
            for (size_t i = 0; i < curr_node->layout->field_q; ++i) {
                Node* new_node = GetNode();
                if (new_node == NULL) {
                    HandleAllocationError();
                    GiveNode(curr_node);
                    MopUpAfterError(&layer_list);
                    return;
                }
                new_node->layout = curr_node->layout->fields[curr_node->layout->field_q - 1 - i];
                new_node->value_place = &curr_node->value_place->children[curr_node->layout->field_q - 1 - i];
                PushFront(&layer_list, new_node); 
            }
        } else if (curr_node->layout->kind == VARINT) {
            int exit_code = SerializeVARINT(byte_array, &byte_iter, curr_node->value_place->varint_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == INT32 || curr_node->layout->kind == UINT32) {
            int exit_code = SerializeINT32(byte_array, &byte_iter, curr_node->value_place->uint_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == INT64 || curr_node->layout->kind == UINT64) {
            int exit_code = SerializeINT64(byte_array, &byte_iter, curr_node->value_place->uint_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list); 
                return;
            }
        } else if (curr_node->layout->kind == BOOL) {
            int exit_code = SerializeBOOL(byte_array, &byte_iter, curr_node->value_place->bool_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == STRING) {
            int exit_code = SerializeSTRING(byte_array, &byte_iter, curr_node->value_place->string_ptr, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == BARRAY) {
            int exit_code = SerializeBARRAY(byte_array, &byte_iter, curr_node->value_place->barray_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == FLOAT) {
            int exit_code = SerializeFLOAT(byte_array, &byte_iter, curr_node->value_place->float_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else if (curr_node->layout->kind == DOUBLE) {
            int exit_code = SerializeDOUBLE(byte_array, &byte_iter, curr_node->value_place->double_value, data_size);
            if (exit_code) {
                GiveNode(curr_node);
                MopUpAfterError(&layer_list);
                return;
            }
        } else {
            HandleUknownKindError();
            GiveNode(curr_node);
            MopUpAfterError(&layer_list);
            return;
        }
        GiveNode(curr_node);
    }
    Clear(&layer_list);
}

char* Serialize(const Layout* layout, Value* value) {
    if (err_code != NOERR) {
        return NULL;
    }

    size_t data_size = SerializedDataSize(layout, value);

    if (err_code != NOERR) {
        return NULL;
    }

    char* byte_array = malloc(SerializedDataSize(layout, value));
    if (byte_array == NULL) {
        HandleAllocationError();
        return NULL;
    }
    SerializeInBuffer(layout, value, byte_array, data_size);
    Clear(&avalible_nodes);
    if (err_code != NOERR) {
        free(byte_array);
        return NULL;
    }
    return byte_array;
}