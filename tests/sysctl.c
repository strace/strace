/*
 * Check decoding of sysctl syscall.
 *
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR__sysctl

# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <linux/sysctl.h>

static const char *errstr;

static long
k_sysctl(const void *const args)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const long rc = syscall(__NR__sysctl, (uintptr_t) args,
				bad, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct __sysctl_args, args);
	void *const efault = args + 1;

	k_sysctl(efault);
	printf("_sysctl(%p) = %s\n", efault, errstr);

	memset(args, 0, sizeof(*args));
	k_sysctl(args);
	printf("_sysctl({name=NULL, nlen=0, oldval=NULL, oldlenp=NULL"
	       ", newval=NULL, newlen=0}) = %s\n", errstr);

	fill_memory_ex(args, sizeof(*args), 'a', 'z' - 'a' + 1);
	args->name = efault;

	k_sysctl(args);
	printf("_sysctl({name=%p, nlen=%d, oldval=%p, oldlenp=%p"
	       ", newval=%p, newlen=%llu}) = %s\n",
	       args->name, args->nlen, args->oldval, args->oldlenp,
	       args->newval, (unsigned long long) args->newlen, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR__sysctl")

#endif
