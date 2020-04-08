/*
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_FILTER_H
# define STRACE_FILTER_H

struct number_set;
typedef int (*string_to_uint_func)(const char *);

void qualify_tokens(const char *str, struct number_set *set,
		    string_to_uint_func func, const char *name);
void qualify_syscall_tokens(const char *str, struct number_set *set);

#endif /* !STRACE_FILTER_H */
