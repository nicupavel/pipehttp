/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#ifndef __PH_DLIST_H
#define __PH_DLIST_H

#include <stdlib.h>

typedef struct DListElmt_ {

void               *data;
struct DListElmt_  *prev;
struct DListElmt_  *next;

} DListElmt;

typedef struct DList_ {
int                size;
int                max_size;
void               (*destroy)(void *data);
DListElmt          *head;
DListElmt          *tail;

} DList;

void dlist_init(DList *list, void (*destroy)(void *data), int max_size);
void dlist_clear(DList *list);
void dlist_resize(DList *list, unsigned int new_size);
int dlist_insert(DList *list, DListElmt *element, const void *data);
int dlist_remove(DList *list, DListElmt *element);
void dlist_print(const DList *list);

#define dlist_size(list) ((list)->size)
#define dlist_head(list) ((list)->head)
#define dlist_tail(list) ((list)->tail)
#define dlist_is_head(element) ((element)->prev == NULL ? 1 : 0)
#define dlist_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define dlist_data(element) ((element)->data)
#define dlist_next(element) ((element)->next)
#define dlist_prev(element) ((element)->prev)

#endif
