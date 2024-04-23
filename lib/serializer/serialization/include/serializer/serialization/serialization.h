#pragma once

#include "serializer/linked_list/linked_list.h"

#include <string.h>

//Serializes values by layout. Returns bytes of serialized data.
char* Serialize(const Layout* layout, Value value);
static size_t SerializedDataSize(const Layout* layout, Value* value);

//Handle serialization of certain kinds from Kind enum.
static int SerializeVARINT(char* byte_array, size_t* byte_iter, char* varint, size_t data_size);
static int SerializeINT64(char* byte_array, size_t* byte_iter, uint64_t inp, size_t data_size);
static int SerializeINT32(char* byte_array, size_t* byte_iter, uint32_t inp, size_t data_size);
static int SerializeBOOL(char* byte_array, size_t* byte_iter, char inp, size_t data_size);
static int SerializeSTRING(char* byte_array, size_t* byte_iter, char* string_iter, size_t data_size);

//Handle errors, mop up used heap memory in case of an error.
static int HandleSerializationOutOfSizeError();
static void HandleSerializationArrayAllocationError();
static void HandleMidSerializationError(List* list, char* byte_array);
static void HandleUknownKindError(List* list, char* byte_array);