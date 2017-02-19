#include "list.h"
#include "lwm2m.h"
#include "semaphore.h"

list *list_new() {
    list *l = malloc(sizeof(list));
    l->greatest_key = -1;
    l->first = NULL;
    l->size = 0;
    sem_init(&l->guard, 0, 1);
    return l;
}

void list_free(list *l) {
    list_elem *curr = l->first;
    list_elem *next = NULL;
    while (curr != NULL) {
        next = curr->next;
        curr->value = NULL;
        free(curr);
        curr = next;
    }
    free(l);
}

bool lcontains(list *l, int key) {
    sem_wait(&l->guard);
    list_elem *curr = l->first;
    while (curr != NULL) {
        if (curr->key == key) {
            sem_post(&l->guard);
            return true;
        }
        curr = curr->next;
    }
    sem_post(&l->guard);
    return false;
}

void ladd(list *l, int key, void *value) {
    sem_wait(&l->guard);
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
    if (key > l->greatest_key) {
        l->greatest_key = key;
    }
    sem_post(&l->guard);
}

void *lfind(list *l, int key) {
    sem_wait(&l->guard);
    list_elem *curr = l->first;
    while (curr != NULL && curr->key != key) {
        curr = curr->next;
    }
    sem_post(&l->guard);
    return curr == NULL ? NULL : curr->value;
}

void lremove(list *l, int key) {
    sem_wait(&l->guard);
    if (l->first == NULL) {
        return;
    } else {
        if (l->first->key == key) {
            list_elem *to_remove = l->first;
            l->first = to_remove->next;
            free(to_remove);
            l->size--;

            if (key == l->greatest_key) {
                l->greatest_key = -1;
                for (list_elem *elem = l->first; elem != NULL; elem = elem->next) {
                    if (elem->key > l->greatest_key) {
                        l->greatest_key = elem->key;
                    }
                }
            }
            sem_post(&l->guard);
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
        next->value = NULL;
        free(next);
        l->size--;
    }

    if (key == l->greatest_key) {
        l->greatest_key = -1;
        for (list_elem *elem = l->first; elem != NULL; elem = elem->next) {
            if (elem->key > l->greatest_key) {
                l->greatest_key = elem->key;
            }
        }
    }
    sem_post(&l->guard);
}