#ifndef PROJECT_LIST_H
#define PROJECT_LIST_H

#include <stdbool.h>

typedef struct list_elem {
    int key;
    void *value;
    void *next;
} list_elem;

typedef struct list {
    list_elem *first;
    int size;
    int greatest_key;
} list;

list *list_new();

void list_free(list *l);

/**
 * - ladd
 * - lfind
 * - lremove
 *
 *
 */
bool lcontains(list *l, int key);

void ladd(list *l, int key, void *value);

void *lfind(list *l, int key);

void lremove(list *l, int key);


#endif //PROJECT_LIST_H
