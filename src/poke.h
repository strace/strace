/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_POKE_H
# define STRACE_POKE_H

struct poke_payload {
	char *data;		/* data to be injected */
	uint16_t data_len;	/* length of the data, max 1024 */
	uint8_t arg_no;		/* number of the argument containing the pointer */
	uint8_t is_enter;	/* when to poke -- on entering or on exiting */
	struct list_item l;
};

uint16_t alloc_poke_data(void);
bool poke_add(uint16_t poke_idx, struct poke_payload *poke);
void poke_tcb(struct tcb *, uint16_t poke_idx, bool isenter);

#endif /* !STRACE_POKE_H */
