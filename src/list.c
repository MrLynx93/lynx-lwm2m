#include "list.h"
#include "lwm2m.h"


list *list_new() {
    list *list = malloc(sizeof(list));
    list->first = NULL;
    list->size = 0;
    return list;
}

void list_free(list *l) {
    list_elem *curr = l->first;
    list_elem *next = NULL;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(l);
}

bool lcontains(list *l, int key) {
    list_elem *curr = l->first;
    while (curr != NULL) {
        if (curr->key == key) return true;
        curr = curr->next;
    }
    return false;
}

void ladd(list *l, int key, void *value) {
    list_elem *elem = (list_elem*) malloc(sizeof(list_elem));
    elem->value = value;
    elem->key = key;

    if (l->first == NULL) {
        elem->next = NULL;
    } else {
        elem->next = l->first;
    }
    l->first = elem;
    l->size++;
}

void *lfind(list *l, int key) {
    list_elem *curr = l->first;
    while (curr != NULL && curr->key != key) {
        curr = curr->next;
    }
    return curr == NULL ? NULL : curr->value;
}

void lremove(list *l, int key) {
    if (l->first == NULL) {
        return;
    } else {
        if (l->first->key == key) {
            list_elem *to_remove = l->first;
            l->first = to_remove->next;
            free(to_remove);
            l->size--;
            return;
        }
    }

    list_elem *prev = l->first;
    list_elem *next = prev->next;
    while (next != NULL && next->key != key) {
        prev = prev->next;
        next = next->next;
    }

    if (next != NULL) {
        prev->next = next->next;
        free(next);
        l->size--;
    }
}