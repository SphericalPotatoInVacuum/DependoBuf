#include "serializer/layout/linked_list.h"

#include "stdlib.h"

void PushBack(List *list, Node *node) {
    if (Empty(list)) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    ++list->size;
}

void PushFront(List *list, Node *node) {
    if (Empty(list)) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    ++list->size;
}

Node *PopBack(List *list) {
    err_code = NOERR;
    if (Empty(list)) {
        err_code = EMPTYLIST;
        return NULL;
    } else { //NOLINT
        --list->size;
        if (list->head == list->tail) {
            Node *node = list->tail;
            list->head = NULL;
            list->tail = NULL;
            return node;
        } else { //NOLINT
            Node *node = list->tail;
            list->tail->prev->next = NULL;
            list->tail = list->tail->prev;
            return node;
        }
    }
}

Node *PopFront(List *list) {
    err_code = NOERR;
    if (Empty(list)) {
        err_code = EMPTYLIST;
        return NULL;
    } else { //NOLINT
        --list->size;
        if (list->head == list->tail) {
            Node *node = list->head;
            list->head = NULL;
            list->tail = NULL;
            return node;
        } else { //NOLINT
            Node *node = list->head;
            list->head->next->prev = NULL;
            list->head = list->head->next;
            return node;
        }
    }
}

int Empty(const List *list) {
    return list->head == NULL;
}

void DestroyNode(Node *node) {
    free(node);
}

void Clear(List *list) {
    while (Empty(list) != 0) {
        DestroyNode(PopFront(list));
    }
}