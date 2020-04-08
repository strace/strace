/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_MQUEUE_H

# include <fcntl.h>
# include <mqueue.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>

int
main(void)
{
	struct mq_attr attr;
	(void) close(0);

	char *name;
	if (asprintf(&name, "/strace-mq-%u.sample", getpid()) < 0)
		perror_msg_and_fail("asprintf");

	if (mq_open(name, O_CREAT, 0700, NULL))
		perror_msg_and_skip("mq_open");
	printf("mq_open(\"%s\", O_RDONLY|O_CREAT, 0700, NULL) = 0\n", name + 1);

	if (mq_getattr(0, &attr))
		perror_msg_and_skip("mq_getattr");
	printf("mq_getsetattr(0, NULL, {mq_flags=0, mq_maxmsg=%lld"
	       ", mq_msgsize=%lld, mq_curmsgs=0}) = 0\n",
	       (long long) attr.mq_maxmsg,
	       (long long) attr.mq_msgsize);

	if (mq_setattr(0, &attr, NULL))
		perror_msg_and_skip("mq_setattr");
	printf("mq_getsetattr(0, {mq_flags=0, mq_maxmsg=%lld"
	       ", mq_msgsize=%lld, mq_curmsgs=0}, NULL) = 0\n",
	       (long long) attr.mq_maxmsg,
	       (long long) attr.mq_msgsize);

	if (mq_unlink(name))
		perror_msg_and_skip("mq_unlink");
	printf("mq_unlink(\"%s\") = 0\n", name + 1);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_MQUEUE_H")

#endif
