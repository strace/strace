/*
 * This file contains error printing functions.
 * These functions can be used by various binaries included in the strace
 * package.  Variable 'program_invocation_name' and function 'die()'
 * have to be defined globally.
 *
 * Copyright (c) 2001-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ERROR_PRINTS_H
# define STRACE_ERROR_PRINTS_H

# include <stdbool.h>

# include "gcc_compat.h"

extern bool debug_flag;

void die(void) ATTRIBUTE_NORETURN;

void error_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void error_msg_and_help(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void error_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;

/* Wrappers for if (debug_flag) error_msg(...) */
# define debug_msg(...) \
	do { \
		if (debug_flag) \
			error_msg(__VA_ARGS__); \
	} while (0)
# define debug_perror_msg(...) \
	do { \
		if (debug_flag) \
			perror_msg(__VA_ARGS__); \
	} while (0)

/* Simple wrappers for providing function name in error messages */
# define error_func_msg(fmt_, ...) \
	error_msg("%s: " fmt_,  __func__, ##__VA_ARGS__)
# define perror_func_msg(fmt_, ...) \
	perror_msg("%s: " fmt_, __func__, ##__VA_ARGS__)
# define debug_func_msg(fmt_, ...) \
	debug_msg("%s: " fmt_, __func__, ##__VA_ARGS__)
# define debug_func_perror_msg(fmt_, ...) \
	debug_perror_msg("%s: " fmt_, __func__, ##__VA_ARGS__)
# define error_func_msg_and_die(fmt_, ...) \
	error_msg_and_die("%s: " fmt_, __func__, ##__VA_ARGS__)
# define perror_func_msg_and_die(fmt_, ...) \
	perror_msg_and_die("%s: " fmt_, __func__, ##__VA_ARGS__)

#endif /* !STRACE_ERROR_PRINTS_H */
