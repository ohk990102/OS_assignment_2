#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>

typedef struct LinkedListNode {
    void *data;
    struct LinkedListNode *next;
} LinkedListNode;

typedef struct LinkedList {
    LinkedListNode *head;
    LinkedListNode *tail;
} LinkedList;

#define GET_NEXTNODE(node)  \
    (((LinkedListNode *) node)->next)

LinkedListNode* make_node(void *data);
void delete_node(LinkedListNode* node);
void append_tail(LinkedList *list, LinkedListNode *node);

LinkedListNode* peek_head(LinkedList *list);
void pop_head(LinkedList *list);

#define FOR_ITER_LINKEDLIST(it, list)   for(typeof((list)->head) it = (list)->head; it != NULL; it = it->next)

#endif