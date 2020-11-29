/*
 * Test PID namespace translation
 *
 * Copyright (c) 2020 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef STRACE_PIDNS_H
# define STRACE_PIDNS_H

# ifdef PIDNS_TRANSLATION
#  define PIDNS_TEST_INIT pidns_test_init()
# else
#  define PIDNS_TEST_INIT
# endif

# include <sys/types.h>

enum pid_type {
	PT_TID,
	PT_TGID,
	PT_PGID,
	PT_SID,

	PT_COUNT,
	PT_NONE = -1
};

/* Prints leader (process tid) if pidns_test_init was called */
void pidns_print_leader(void);

/*
 * Returns a static buffer containing the translation string of our PID.
 */
const char *pidns_pid2str(enum pid_type type);

/**
 * Skips the test if NS_* ioctl commands are not supported by the kernel.
 */
void check_ns_ioctl(void);

/**
 * Init pidns testing.
 *
 * Should be called at the beginning of the test's main function
 *
 * This function calls fork a couple of times, and returns in the child
 * processes. These child processes are in a new PID namespace with different
 * PID configurations (group leader, session leader, ...). If any child
 * terminates with nonzero exit status the test is failed. Otherwise the test is
 * successful, and the parent process exits with 0.
 */
void pidns_test_init(void);

#endif
