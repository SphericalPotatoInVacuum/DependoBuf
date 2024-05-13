//Custom two-linked list
#pragma once

#include "serializer/layout/layout.h"

typedef struct Node {
    struct Node *prev;
    struct Node *next;
    Value *value_place;
    const Layout *layout;
    union {
        void **data;
        const Value *value;
    };
} Node;

typedef struct List {
    Node *head;
    Node *tail;
    size_t size;
} List;

void PushBack(List *list, Node *node);
void PushFront(List *list, Node *node);
Node *PopBack(List *list);
Node *PopFront(List *list);
int Empty(const List *list);
void DestroyNode(Node *node);
void Clear(List *list);