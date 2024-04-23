#pragma once

#include "serializer/linked_list/linked_list.h"

//Deserializes bytes by layout. Returns constructed value.
Value Deserialize(const Layout *layout, char *buffer);