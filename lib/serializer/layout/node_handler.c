#include "serializer/layout/node_handler.h"

#include "stdlib.h"

List avalible_nodes = {.head = NULL, .tail = NULL, .size = 0};

Node* GetNode() {
    if (!Empty(&avalible_nodes)) {
        return PopBack(&avalible_nodes);
    }
    Node* new_node = calloc(1, sizeof(Node));
    return new_node;
}

void GiveNode(Node* node) {
    if (avalible_nodes.size >= MAX_NODES_IN_STORAGE) {
        DestroyNode(node);
        return;
    }
    PushBack(&avalible_nodes, node);
}