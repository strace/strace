/*
 * Some simple implementation of a list similar to one used in the kernel.
 *
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STRACE_LIST_H
#define STRACE_LIST_H

#include "macros.h"

struct list_item {
	struct list_item *prev;
	struct list_item *next;
};

#define EMPTY_LIST(l_) struct list_item l_ = { &l_, &l_ }

static inline void
list_init(struct list_item *l)
{
	l->prev = l;
	l->next = l;
}

static inline bool
list_is_empty(struct list_item *l)
{
	return (l->next == l) && (l->prev == l);
}

#define list_elem(var, type, field) containerof((var), type, field)

#define list_head(head, type, field) \
	(list_is_empty(head) ? NULL : list_elem((head)->next, type, field))
#define list_tail(head, type, field) \
	(list_is_empty(head) ? NULL : list_elem((head)->prev, type, field))

#define list_next(val, field) \
	list_elem((val)->field.next, typeof(*(val)), field)
#define list_prev(val, field) \
	list_elem((val)->field.prev, typeof(*(val)), field)

static inline void
list_insert(struct list_item *head, struct list_item *item)
{
	item->next = head->next;
	item->prev = head;
	head->next->prev = item;
	head->next = item;
}

static inline void
list_append(struct list_item *head, struct list_item *item)
{
	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
}

static inline void
list_remove(struct list_item *item)
{
	if (!item->next || !item->prev)
		return;

	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = item->prev = NULL;
}

static inline struct list_item *
list_remove_tail(struct list_item *head)
{
	struct list_item *t = list_is_empty(head) ? NULL : head->prev;

	if (t)
		list_remove(t);

	return t;
}

static inline struct list_item *
list_remove_head(struct list_item *head)
{
	struct list_item *h = list_is_empty(head) ? NULL : head->next;

	if (h)
		list_remove(h);

	return h;
}

static inline void
list_replace(struct list_item *old, struct list_item *new)
{
	new->next = old->next;
	new->prev = old->prev;
	old->prev->next = new;
	old->next->prev = new;
	old->next = old->prev = NULL;
}

#define list_foreach(var_, head_, field_) \
	for (var_ = list_elem((head_)->next, typeof(*var_), field_); \
	    &(var_->field_) != (head_); var_ = list_next(var_, field_))

#define list_foreach_safe(var_, head_, field_, _tmp) \
	for (var_ = list_elem((head_)->next, typeof(*var_), field_), \
	    _tmp = list_elem((var_)->field_.next, typeof(*var_), field_); \
	    &var_->field_ != head_; var_ = _tmp, _tmp = list_next(_tmp, field_))

#endif /* !STRACE_LIST_H */
