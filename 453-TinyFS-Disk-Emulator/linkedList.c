#include <stdlib.h>

#include "linkedList.h"
#include "tinyFS.h"

int init(linkedList **li) {
    *li = malloc(sizeof(linkedList));
    if (!*li)
        return -1;

    (*li)->head = NULL;
    (*li)->tail = NULL;
    (*li)->count = 0;
    (*li)->cmp = cmp;

    return 0;
}

int deallocate(linkedList **li) {
    node *current;
    while ((*li)->head) {
        current = (*li)->head;
        (*li)->head = (*li)->head->next;
        free(current);
        current = NULL;
    }

    free(*li);
    *li = NULL;

    return 0;
}

int removeAllEntries(linkedList *li) {
    node *current;

    while (li->head) {
        current = li->head;
        li->head = li->head->next;
        free(current);
        current = NULL;
    }

    li->head = NULL;
    li->tail = NULL;
    li->count = 0;

    return 0;
}

void *getDataFromIdx(linkedList *li, int idx) {
    int i = 0;
    node *current;
    void *ret = NULL;

    current = li->head;

    while (current && i < idx) {
        current = current->next;
        i++;
    }

    if (current) {
        ret = current->data;
    }
    return ret;
}

int insert(linkedList *li, void *data) {
    node *new = NULL;

    new = malloc(sizeof(node));
    if (!new)
        return -1;

    new->data = data;
    new->next = NULL;
    if (li->tail)
        li->tail->next = new;
    if (li->count == 0)
        li->head = new;
    li->tail = new;

    li->count++;

    return 0;
}

//should return file descriptor?
int getIndexOfData(linkedList *li, int data) {
    int i = 0;
    node *current;

    current = li->head;
    while (current) {
        if (li->cmp(current->data, data) == 0)
            return i;
        current = current->next;
        i++;
    }

    return -1;
}

int isEmpty(linkedList *li) {
    return li->count == 0;
}

void *removeEntry(linkedList *li, int idx) {
    int i = 0;
    void *ret = NULL;
    node *current, *prev = NULL;

    if (idx == 0)
        ret = li->head->data;
        current = li->head;
        li->head = li->head->next;
        li->count--;
        free(current);
        current = NULL;
        return ret;
    if (idx == li->count)
        current = li->head;
        while (current && current->next) {
            prev = current;
            current = current->next;
        }
        if (li->tail) {
            ret = li->tail->data;
            current = li->tail;
            li->tail = prev;
            li->tail->next = NULL;
            li->count--;
            free(current);
            current = NULL;
        }
    if (idx > li->count)
        return ret;

    current = li->head;
    while (current && i <= (idx - 1)) {
        prev = current;
        current = current->next;
        i++;
    }

    ret = current->data;
    prev->next = current->next;
    free(current);
    current = NULL;
    li->count--;

    return ret;
}

int size(LinkedList *l) {
    return l->count;
}
