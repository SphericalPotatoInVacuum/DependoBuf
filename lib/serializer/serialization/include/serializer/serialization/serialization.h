#pragma once

#include "serializer/layout/linked_list.h"

#include "limits.h"

//Defines how many nodes can be stored simultaneously during serialization process.
#define MAX_NODES_IN_STORAGE 50

#if CHAR_BIT <= 1
#error "Uncompatible platform to run"
#endif

//Serializes values by layout. Returns bytes of serialized data.
char* Serialize(const Layout* layout, Value* value);
//Serializes values by layout in ceratin buffer byte_array with size data_size.
void SerializeInBuffer(const Layout* layout, Value* value, char* byte_array, size_t data_size);
//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer);

