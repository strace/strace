/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_DELAY_H
# define STRACE_DELAY_H

uint16_t alloc_delay_data(void);
void fill_delay_data(uint16_t delay_idx, struct timespec *val, bool isenter);
bool is_delay_timer_armed(void);
void delay_timer_expired(void);
void arm_delay_timer(const struct tcb *);
void delay_tcb(struct tcb *, uint16_t delay_idx, bool isenter);

#endif /* !STRACE_DELAY_H */
