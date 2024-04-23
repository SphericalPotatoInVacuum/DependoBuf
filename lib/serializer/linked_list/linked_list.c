#include "serializer/linked_list/linked_list.h"
#include "serializer/error_handler/error_handler.h"

void PushBack(List *list, Node *node) {
    if (Empty(list)) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
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
}

Node *PopBack(List *list) {
    err_code = 0;
    if (Empty(list)) {
        err_code = 1;
        perror("Could not pop from empty list\n");
        return NULL;
    } else {
        if (list->head == list->tail) {
            Node *node = list->tail;
            list->head = NULL;
            list->tail = NULL;
            return node;
        } else {
            Node *node = list->tail;
            list->tail->prev->next = NULL;
            list->tail = list->tail->prev;
            return node;
        }
    }
}

Node *PopFront(List *list) {
    err_code = 0;
    if (Empty(list)) {
        err_code = 1;
        perror("Could not pop from empty list\n");
        return NULL;
    } else {
        if (list->head == list->tail) {
            Node *node = list->head;
            list->head = NULL;
            list->tail = NULL;
            return node;
        } else {
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

void HandleNodeAllocationError(List *list) {
    Clear(list);
    perror("Unable to allocate memory on heap.\n");
    err_code = 1;
}