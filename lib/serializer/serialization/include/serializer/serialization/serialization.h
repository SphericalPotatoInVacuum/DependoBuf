#pragma once

#include "serializer/linked_list/linked_list.h"
#include <string.h>

//Defines how many nodes can be hanged simultaneously in serialization process.
#define MAX_NODES_IN_STORAGE 50

//Serializes values by layout. Returns bytes of serialized data.
char* Serialize(const Layout* layout, Value value);
//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer);

