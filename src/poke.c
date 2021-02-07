/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "list.h"
#include "poke.h"

static struct list_item *poke_data_vec;
static size_t poke_data_vec_capacity; /* size of the arena */
static size_t poke_data_vec_size;     /* size of the used arena */

static void
expand_poke_data_vec(void)
{
	const size_t old_capacity = poke_data_vec_capacity;
	poke_data_vec = xgrowarray(poke_data_vec, &poke_data_vec_capacity,
				    sizeof(*poke_data_vec));
	memset(poke_data_vec + old_capacity, 0,
	       (poke_data_vec_capacity - old_capacity)
	       * sizeof(*poke_data_vec));
}

uint16_t
alloc_poke_data(void)
{
	const uint16_t rval = poke_data_vec_size;

	if (rval < poke_data_vec_size)
		error_func_msg_and_die("poke index overflow");

	if (poke_data_vec_size == poke_data_vec_capacity)
		expand_poke_data_vec();

	poke_data_vec_size++;

	list_init(&poke_data_vec[rval]);
	return rval;
}

bool
poke_add(uint16_t poke_idx, struct poke_payload *poke)
{
	struct poke_payload *i;
	list_foreach(i, &poke_data_vec[poke_idx], l)
		if ((i->is_enter == poke->is_enter) &&
		    (i->arg_no == poke->arg_no))
			/* duplicate */
			return 1;

	list_insert(&poke_data_vec[poke_idx], &poke->l);
	return 0;
}

void
poke_tcb(struct tcb *tcp, uint16_t poke_idx, bool is_enter)
{
	if (poke_idx >= poke_data_vec_size)
		error_func_msg_and_die("poke_idx >= poke_data_vec_size");

	debug_func_msg("poking pid %d on %s",
		       tcp->pid, is_enter ? "enter" : "exit");

	bool poked = false;
	struct poke_payload *i;
	list_foreach(i, &poke_data_vec[poke_idx], l) {
		if (i->is_enter != is_enter)
			continue;
		if (n_args(tcp) < i->arg_no) {
			error_func_msg("Failed to tamper with process %d:"
				       " requested to tamper with argument #%u,"
				       " but system call '%s' has only %u arguments",
				       tcp->pid, i->arg_no,
				       tcp_sysent(tcp)->sys_name,
				       n_args(tcp));
			continue;
		}
		unsigned int nwritten = upoken(tcp, tcp->u_arg[i->arg_no - 1],
					       i->data_len, i->data);
		if (!nwritten)
			error_func_msg("Failed to tamper with process %d:"
				       " couldn't poke",
				       tcp->pid);
		else
			poked = true;
	}

	if (poked)
		tcp->flags |= TCB_TAMPERED_POKED;
}
