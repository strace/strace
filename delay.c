/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

struct inject_delay_data {
	struct timespec ts_enter;
	struct timespec ts_exit;
};

static struct inject_delay_data *delay_data_vec;
static size_t delay_data_vec_capacity; /* size of the arena */
static size_t delay_data_vec_size;     /* size of the used arena */

static timer_t delay_timer = (timer_t) -1;
static bool delay_timer_is_armed;

static void
expand_delay_data_vec(void)
{
	const size_t old_capacity = delay_data_vec_capacity;
	delay_data_vec = xgrowarray(delay_data_vec, &delay_data_vec_capacity,
				    sizeof(*delay_data_vec));
	memset(delay_data_vec + old_capacity, 0,
	       (delay_data_vec_capacity - old_capacity)
	       * sizeof(*delay_data_vec));
}

uint16_t
alloc_delay_data(void)
{
	const uint16_t rval = delay_data_vec_size;

	if (rval < delay_data_vec_size)
		error_func_msg_and_die("delay index overflow");

	if (delay_data_vec_size == delay_data_vec_capacity)
		expand_delay_data_vec();

	++delay_data_vec_size;
	return rval;
}

void
fill_delay_data(uint16_t delay_idx, struct timespec *val, bool isenter)
{
	if (delay_idx >= delay_data_vec_size)
		error_func_msg_and_die("delay_idx >= delay_data_vec_size");

	struct timespec *ts;
	if (isenter)
		ts = &(delay_data_vec[delay_idx].ts_enter);
	else
		ts = &(delay_data_vec[delay_idx].ts_exit);

	*ts = *val;
}

static bool
is_delay_timer_created(void)
{
	return delay_timer != (timer_t) -1;
}

bool
is_delay_timer_armed(void)
{
	return delay_timer_is_armed;
}

void
delay_timer_expired(void)
{
	delay_timer_is_armed = false;
}

void
arm_delay_timer(const struct tcb *const tcp)
{
	const struct itimerspec its = {
		.it_value = tcp->delay_expiration_time
	};

	if (timer_settime(delay_timer, TIMER_ABSTIME, &its, NULL))
		perror_msg_and_die("timer_settime");

	delay_timer_is_armed = true;

	debug_func_msg("timer set to %lld.%09ld for pid %d",
		       (long long) tcp->delay_expiration_time.tv_sec,
		       (long) tcp->delay_expiration_time.tv_nsec,
		       tcp->pid);
}

void
delay_tcb(struct tcb *tcp, uint16_t delay_idx, bool isenter)
{
	if (delay_idx >= delay_data_vec_size)
		error_func_msg_and_die("delay_idx >= delay_data_vec_size");

	debug_func_msg("delaying pid %d on %s",
		       tcp->pid, isenter ? "enter" : "exit");
	tcp->flags |= TCB_DELAYED;

	struct timespec *ts_diff;
	if (isenter)
		ts_diff = &(delay_data_vec[delay_idx].ts_enter);
	else
		ts_diff = &(delay_data_vec[delay_idx].ts_exit);

	struct timespec ts_now;
	clock_gettime(CLOCK_MONOTONIC, &ts_now);
	ts_add(&tcp->delay_expiration_time, &ts_now, ts_diff);

	if (is_delay_timer_created()) {
		struct itimerspec its;
		if (timer_gettime(delay_timer, &its))
			perror_msg_and_die("timer_gettime");

		const struct timespec *const ts_old = &its.it_value;
		if (ts_nz(ts_old) && ts_cmp(ts_diff, ts_old) > 0)
			return;
	} else {
		if (timer_create(CLOCK_MONOTONIC, NULL, &delay_timer))
			perror_msg_and_die("timer_create");
	}

	arm_delay_timer(tcp);
}
