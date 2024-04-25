#pragma once

#include "serializer/layout/linked_list.h"
#include <string.h>

//Defines how many nodes can be hanged simultaneously in serialization process.
#define MAX_NODES_IN_STORAGE 50

//Serializes values by layout. Returns bytes of serialized data.
char* Serialize(const Layout* layout, Value value);
void SerializeInBuffer(const Layout* layout, Value value, char* byte_array, size_t data_size);
//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer);

