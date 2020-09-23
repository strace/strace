/*
 * Check decoding of prlimit64 syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_prlimit64

# include <inttypes.h>
# include <stdio.h>
# include <stdint.h>
# include <sys/resource.h>
# include <unistd.h>

# include "pidns.h"
# include "xlat.h"
# include "xlat/resources.h"

const char *
sprint_rlim(uint64_t lim)
{
	if (lim == -1ULL)
		return "RLIM64_INFINITY";

	static char buf[2][sizeof(lim)*3 + sizeof("*1024")];
	static int i;
	i &= 1;
	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf[i], "%" PRIu64 "*1024", lim / 1024);
	else
		sprintf(buf[i], "%" PRIu64, lim);

	return buf[i++];
}

int
main(void)
{
	PIDNS_TEST_INIT;

	unsigned long pid =
		(unsigned long) 0xdefaced00000000ULL | (unsigned) getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
	uint64_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat_data *xlat;
	size_t i = 0;

	for (xlat = resources->data; i < resources->size; ++xlat, ++i) {
		if (!xlat->str)
			continue;

		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(__NR_prlimit64, pid, res, 0, rlimit);
		pidns_print_leader();
		if (rc)
			printf("prlimit64(%d%s, %s, NULL, %p) ="
				     " %ld %s (%m)\n",
			       (unsigned) pid, pid_str,
			       xlat->str, rlimit,
			       rc, errno2name());
		else
			printf("prlimit64(%d%s, %s, NULL"
			       ", {rlim_cur=%s, rlim_max=%s}) = 0\n",
			       (unsigned) pid, pid_str,
			       xlat->str,
			       sprint_rlim(rlimit[0]),
			       sprint_rlim(rlimit[1]));
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prlimit64")

#endif
