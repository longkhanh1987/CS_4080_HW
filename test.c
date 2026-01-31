#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char* value;
    struct Node* prev;
    struct Node* next;
} Node;

static char* xstrdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (!p) exit(1);
    memcpy(p, s, n);
    return p;
}

// Insert new node AFTER `after`.
// If after == NULL, insert at head.
void insert(Node** list, Node* after, const char* value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) exit(1);

    node->value = xstrdup(value);
    node->prev = NULL;
    node->next = NULL;

    if (*list == NULL) {                 // empty list
        *list = node;
        return;
    }

    if (after == NULL) {                 // insert at head
        node->next = *list;
        (*list)->prev = node;
        *list = node;
        return;
    }

    node->next = after->next;
    node->prev = after;

    if (after->next) after->next->prev = node;
    after->next = node;
}

Node* find(Node* list, const char* key) {
    for (Node* n = list; n; n = n->next) {
        if (strcmp(n->value, key) == 0) return n;
    }
    return NULL;
}

void delete(Node** list, Node* node) {
    if (!node) return;

    if (node->prev) node->prev->next = node->next;
    else *list = node->next;               // deleting head

    if (node->next) node->next->prev = node->prev;

    free(node->value);
    free(node);
}

void dump(Node* list) {
    printf("List: ");
    for (Node* n = list; n; n = n->next) {
        printf("%s", n->value);
        if (n->next) printf(" <-> ");
    }
    printf("\n");
}

// optional cleanup
void free_all(Node** list) {
    Node* cur = *list;
    while (cur) {
        Node* next = cur->next;
        free(cur->value);
        free(cur);
        cur = next;
    }
    *list = NULL;
}

int main(void) {
    printf("Hello, world!\n");

    Node* list = NULL;

    insert(&list, NULL, "four");
    insert(&list, NULL, "one");
    insert(&list, find(list, "one"), "two");
    insert(&list, find(list, "two"), "three");

    dump(list);

    printf("-- delete three --\n");
    delete(&list, find(list, "three"));
    dump(list);

    printf("-- delete one --\n");
    delete(&list, find(list, "one"));
    dump(list);

    free_all(&list);
    return 0;
}
