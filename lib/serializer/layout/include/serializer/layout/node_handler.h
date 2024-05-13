#pragma once

#include "serializer/layout/linked_list.h"

#define MAX_NODES_IN_STORAGE 50

Node* GetNode();
void GiveNode(Node* node);
extern List avalible_nodes;
