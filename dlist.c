/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */

#include "dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dlist_init(DList *list, void (*destroy)(void *data), int max_size) {
    list->size = 0;
    list->max_size = max_size;
    list->destroy = destroy;
    list->head = NULL;
    list->tail = NULL;

    return;
}

void dlist_clear(DList *list) {
    while (list->size > 0) {
        if (dlist_remove(list, dlist_tail(list)) != 0) {
            fprintf(stderr, " Error removing element at list size: %d\n",
                    list->size);
        }
    }

    memset(list, 0, sizeof(DList));

    return;
}

void dlist_resize(DList *list, unsigned int new_size) {
    list->max_size = new_size;
    if (new_size >= list->size || list->size == 0) return;

    unsigned int to_remove = list->size - new_size;

    while (list->size > 0 &&  to_remove > 0) {
        if (dlist_remove(list, dlist_tail(list)) != 0) {
            fprintf(stderr, " Error removing element at list size: %d\n",
                    list->size);
        }
        to_remove--;
    }
}

int dlist_insert(DList *list, DListElmt *element, const void *data) {
    DListElmt *new_element;

    if ((new_element = (DListElmt *)malloc(sizeof(DListElmt))) == NULL)
        return -1;

    new_element->data = (void *)data;

    if (element == NULL && dlist_size(list) != 0) {
        element = list->head;
    }

    if (list->size == 0) {
        list->head = new_element;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = new_element;
    } else {
        if (element == list->head) {
            new_element->prev = NULL;
            new_element->next = element;
            element->prev = new_element;
            list->head = new_element;
        } else {
            new_element->prev = element;
            new_element->next = element->next;
            if (element->next == NULL)
                list->tail = new_element;
            else
                element->next->prev = new_element;

            element->next = new_element;
        }
    }

    list->size++;

    if (list->max_size > 0 && list->size > list->max_size) {
        //fprintf(stdout, "List over specified size: %d\n", list->max_size);
        // New Element is tail remove head, otherwise remove tail
        if (new_element == list->tail) {
            dlist_remove(list, list->head);
        } else {
            dlist_remove(list, list->tail);
        }
    }

    return 0;
}

int dlist_remove(DList *list, DListElmt *element) {
    if (element == NULL || dlist_size(list) == 0) return -1;

    if (list->destroy) {
        list->destroy(element->data);
    }

    if (element == list->head) {
        list->head = element->next;

        if (list->head == NULL)
            list->tail = NULL;
        else
            element->next->prev = NULL;
    } else {
        element->prev->next = element->next;

        if (element->next == NULL)
            list->tail = element->prev;
        else
            element->next->prev = element->prev;
    }

    free(element);
    list->size--;

    return 0;
}

void dlist_print(const DList *list) {
    DListElmt *element;
    int *data;
    fprintf(stdout, "List size is %d\n", dlist_size(list));

    int i = 0;
    element = dlist_head(list);

    while (1) {
        data = dlist_data(element);
        fprintf(stdout, "list[%03d]=%03d\n", i, *data);

        i++;

        if (dlist_is_tail(element))
            break;
        else
            element = dlist_next(element);
    }

    return;
}