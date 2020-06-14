#include <stdlib.h>
#include "linkedlist.h"

LinkedListNode* make_node(void *data) {
    LinkedListNode* node = (LinkedListNode *) malloc(sizeof(LinkedListNode));
    node->data = data;
    node->next = NULL;
    return node;
}

void delete_node(LinkedListNode* node) {
    free(node);
}

void append_tail(LinkedList *list, LinkedListNode *node) {
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    }
    else {
        list->tail->next = node;
        list->tail = node;
    }
}

LinkedListNode* peek_head(LinkedList *list) {
    return list->head;
}

void pop_head(LinkedList *list) {
    if (list->head == NULL)
        return;
    
    list->head = list->head->next;
    if (list->head == NULL) {
        list->tail = NULL;
    }
}