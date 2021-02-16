/*
 * Some simple implementation of lists similar to the one used in the kernel.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LIST_H
# define STRACE_LIST_H

/*
 * struct list_item and related macros and functions provide an interface
 * for manipulating and iterating linked lists.  In order to have list
 * associated with its payload, struct list_item has to be embedded into
 * a structure type representing payload, and (optionally) an additional
 * struct list_item should be added somewhere as a starting point for list
 * iteration (creating a list with a designated head). A situation where
 * no designated head exists, and each embedded struct list_head is considered
 * a head (i.e. starting point for list iteration), is also possible.
 *
 * List head has to be initialised with list_init() call. Statically allocated
 * list heads can also be defined with an EMPTY_LIST() macro.
 *
 * In order to get a pointer to list item from a struct list_item, list_elem
 * macro is used.
 *
 * When a designated head is used, list_head() and list_tail() can be used
 * for getting pointer to the first and the last list item, respectively.
 *
 * list_next() and list_prev() macros can be used for obtaining pointers
 * to the next and the previous items in the list, respectively.  Note that
 * they do not perform additional checks for the validity of these pointers,
 * so they have to be guarded with respective list_head/list_tail checks in case
 * of lists with designated heads (where the list's head is not embedded within
 * a list item.
 *
 * list_{insert,append,remove,remove_tail,remove_head,replace} provide some
 * basic means of list manipulation.
 *
 * list_foreach() and list_foreach_safe() are wrapper macros for simplifying
 * iteration over a list, with the latter having an additional argument
 * for storing temporary pointer, thus allowing list manipulations during
 * its iteration.
 *
 * A simple example:
 *
 *     struct my_struct {
 *             int a;
 *             struct list_item l1;
 *             struct list_item l2;
 *     };
 *
 *     EMPTY_LIST(list_1);              <--- Defining a designated head for list
 *
 *     struct my_struct *item =
 *             calloc(1, sizeof(*item));
 *     list_init(&item->l2);            <--- Initialising structure field that
 *                                           is used for lists without designated
 *                                           head.
 *     list_insert(&list_1, &item->l1); <--- Inserting an item into the list
 *
 *     item = calloc(1, sizeof(*item));
 *     list_init(&item->l2);
 *
 *     list_append(&(list_head(list_1, struct my_struct, l1)->l2), &item->l2);
 *
 *     struct my_struct *cur = item;    <--- Iteration over a headless list
 *     do {
 *             printf("%d\n", cur->a);
 *     } while ((cur = list_next(&cur, l2)) != item);
 *
 *     struct my_struct *i;
 *     list_foreach(i, list_1, l1) {    <--- Iteration over list_1 without list
 *             printf("%d\n", i->a);         modification
 *     }
 *
 *     struct my_struct *tmp;           <--- Iteration with modification
 *     list_foreach_safe(i, list_1, l1, tmp) {
 *             list_remove(&i->l1);
 *             free(i);
 *     }
 *
 * See also:
 *     "Linux kernel design patterns - part 2", section "Linked Lists"
 *     https://lwn.net/Articles/336255/
 */

# include "macros.h"

struct list_item {
	struct list_item *prev;
	struct list_item *next;
};

/**
 * Define an empty list head.
 *
 * @param l_ List head variable name.
 */
# define EMPTY_LIST(l_) struct list_item l_ = { &l_, &l_ }

/** Initialise an empty list. */
static inline void
list_init(struct list_item *l)
{
	l->prev = l;
	l->next = l;
}

/** Check whether list is empty. */
static inline bool
list_is_empty(const struct list_item *l)
{
	return ((l->next == l) && (l->prev == l))
		/*
		 * XXX This could be the case when struct list_item hasn't been
		 *     initialised at all; we should probably also call some
		 *     errror_func_msg() in that case, as it looks like sloppy
		 *     programming.
		 */
		|| (!l->next && !l->prev);
}

/**
 * Convert a pointer to a struct list_item to a pointer to a list item.
 *
 * @param var   Pointer to struct list_item.
 * @param type  Type of the list's item.
 * @param field Name of the field that holds the respective struct list_item.
 */
# define list_elem(var, type, field) containerof((var), type, field)

/**
 * Get the first element in a list.
 *
 * @param head  Pointer to the list's head.
 * @param type  Type of the list's item.
 * @param field Name of the field that holds the respective struct list_item.
 */
# define list_head(head, type, field) \
	(list_is_empty(head) ? NULL : list_elem((head)->next, type, field))
/**
 * Get the last element in a list.
 *
 * @param head  Pointer to the list's head.
 * @param type  Type of the list's item.
 * @param field Name of the field that holds the respective struct list_item.
 */
# define list_tail(head, type, field) \
	(list_is_empty(head) ? NULL : list_elem((head)->prev, type, field))

/**
 * Get the next element in a list.
 *
 * @param var   Pointer to a list item.
 * @param field Name of the field that holds the respective struct list_item.
 */
# define list_next(var, field) \
	list_elem((var)->field.next, typeof(*(var)), field)
/**
 * Get the previous element in a list.
 *
 * @param var   Pointer to a list item.
 * @param field Name of the field that holds the respective struct list_item.
 */
# define list_prev(var, field) \
	list_elem((var)->field.prev, typeof(*(var)), field)

/**
 * Insert an item into a list. The item is placed as the next list item
 * to the head.
 */
static inline void
list_insert(struct list_item *head, struct list_item *item)
{
	item->next = head->next;
	item->prev = head;
	head->next->prev = item;
	head->next = item;
}

/**
 * Insert an item into a list. The item is placed as the previous list item
 * to the head.
 */
static inline void
list_append(struct list_item *head, struct list_item *item)
{
	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
}

/**
 * Remove an item from a list.
 *
 * @param item Pointer to struct list_item of the item to be removed.
 * @return     Whether the action has been performed.
 */
static inline bool
list_remove(struct list_item *item)
{
	if (!item->next || !item->prev || list_is_empty(item))
		return false;

	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = item->prev = item;

	return true;
}

/**
 * Remove the last element of a list.
 *
 * @param head Pointer to the list's head.
 * @return     Pointer to struct list_item removed from the list;
 *             or NULL, if the list is empty.
 */
static inline struct list_item *
list_remove_tail(struct list_item *head)
{
	struct list_item *t = list_is_empty(head) ? NULL : head->prev;

	if (t)
		list_remove(t);

	return t;
}

/**
 * Remove the first element of a list.
 *
 * @param head Pointer to the list's head.
 * @return     Pointer to struct list_item removed from the list;
 *             or NULL, if the list is empty.
 */
static inline struct list_item *
list_remove_head(struct list_item *head)
{
	struct list_item *h = list_is_empty(head) ? NULL : head->next;

	if (h)
		list_remove(h);

	return h;
}

/**
 * Replace an old struct list_item in a list with the new one.
 *
 * @param old Pointer to struct list_item of the item to be replaced.
 * @param new Pointer to struct list_item of the item to be replaced with.
 * @return    Whether the replacement has been performed.
 */
static inline bool
list_replace(struct list_item *old, struct list_item *new)
{
	if (!old->next || !old->prev || list_is_empty(old))
		return false;

	new->next = old->next;
	new->prev = old->prev;
	old->prev->next = new;
	old->next->prev = new;
	old->next = old->prev = old;

	return true;
}

/**
 * List iteration wrapper for non-destructive operations.
 *
 * @param var_   Variable holding pointer to a current list item.
 * @param head_  Pointer to the list's head.
 * @param field_ Name of the field containing the respective struct list_item
 *               inside list items.
 */
# define list_foreach(var_, head_, field_) \
	for (var_ = list_elem((head_)->next, typeof(*var_), field_); \
	    &(var_->field_) != (head_); var_ = list_next(var_, field_))

/**
 * List iteration wrapper for destructive operations.
 *
 * @param var_   Variable holding pointer to a current list item.
 * @param head_  Pointer to the list's head.
 * @param field_ Name of the field containing the respective struct list_item
 *               inside list items.
 * @param _tmp   Temporary variable for storing pointer to the next item.
 */
# define list_foreach_safe(var_, head_, field_, _tmp) \
	for (var_ = list_elem((head_)->next, typeof(*var_), field_), \
	    _tmp = list_elem((var_)->field_.next, typeof(*var_), field_); \
	    &var_->field_ != head_; var_ = _tmp, _tmp = list_next(_tmp, field_))

#endif /* !STRACE_LIST_H */
