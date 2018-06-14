/*
 * Check decoding of kcmp syscall.
 *
 * Copyright (c) 2016-2017 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#include <asm/unistd.h>
#include "scno.h"

#ifdef __NR_kcmp

# include <fcntl.h>
# include <stdarg.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# ifndef VERBOSE_FD
#  define VERBOSE_FD 0
# endif

/*
 * We prefer to use system headers in order to catch some possible deviations in
 * system's headers from our perception of reality, but happy to include our own
 * definitions as well.
 */
# ifdef HAVE_LINUX_KCMP_H
#  include <linux/kcmp.h>
# else
#  define KCMP_FILE	0
#  define KCMP_VM	1
#  define KCMP_FILES	2
#  define KCMP_FS	3
#  define KCMP_SIGHAND	4
#  define KCMP_IO	5
#  define KCMP_SYSVSEM	6
# endif

/* All other kcmp types have been added atomically */
# define KCMP_EPOLL_TFD	7

# ifndef HAVE_STRUCT_KCMP_EPOLL_SLOT
struct kcmp_epoll_slot {
	uint32_t efd;
	uint32_t tfd;
	uint32_t toff;
};
# endif

static const kernel_ulong_t kcmp_max_type = KCMP_EPOLL_TFD;

static const char null_path[] = "/dev/null";
static const char zero_path[] = "/dev/zero";

# define NULL_FD 23
# define ZERO_FD 42

static void
printpidfd(const char *prefix, pid_t pid, unsigned fd)
{
	printf("%s%d", prefix, fd);
}

/*
 * Last argument is optional and is used as follows:
 *  * When type is KCMP_EPOLL_TFD, it signalises whether idx2 is a valid
 *    pointer.
 */
static void
do_kcmp(kernel_ulong_t pid1, kernel_ulong_t pid2, kernel_ulong_t type,
	const char *type_str, kernel_ulong_t idx1, kernel_ulong_t idx2, ...)
{
	long rc;
	const char *errstr;

	rc = syscall(__NR_kcmp, pid1, pid2, type, idx1, idx2);
	errstr = sprintrc(rc);

	printf("kcmp(%d, %d, ", (int) pid1, (int) pid2);

	if (type_str)
		printf("%s", type_str);
	else
		printf("%#x /* KCMP_??? */", (int) type);

	if (type == KCMP_FILE) {
		printpidfd(", ", pid1, idx1);
		printpidfd(", ", pid2, idx2);
	} else if (type == KCMP_EPOLL_TFD) {
		va_list ap;
		int valid_ptr;

		va_start(ap, idx2);
		valid_ptr = va_arg(ap, int);
		va_end(ap);

		printpidfd(", ", pid1, idx1);
		printf(", ");

		if (valid_ptr) {
			struct kcmp_epoll_slot *slot =
				(struct kcmp_epoll_slot *) (uintptr_t) idx2;

			printpidfd("{efd=", pid2, slot->efd);
			printpidfd(", tfd=", pid2, slot->tfd);
			printf(", toff=%llu}", (unsigned long long) slot->toff);
		} else {
			if (idx2)
				printf("%#llx", (unsigned long long) idx2);
			else
				printf("NULL");
		}
	} else if (type > kcmp_max_type) {
		printf(", %#llx, %#llx",
		       (unsigned long long) idx1, (unsigned long long) idx2);
	}

	printf(") = %s\n", errstr);
}

int
main(void)
{
	static const kernel_ulong_t bogus_pid1 =
		(kernel_ulong_t) 0xdeadca75face1057ULL;
	static const kernel_ulong_t bogus_pid2 =
		(kernel_ulong_t) 0xdefaced1defaced2ULL;
	static const kernel_ulong_t bogus_type =
		(kernel_ulong_t) 0xbadc0dedda7adeadULL;
	static const kernel_ulong_t bogus_idx1 =
		(kernel_ulong_t) 0xdec0ded3dec0ded4ULL;
	static const kernel_ulong_t bogus_idx2 =
		(kernel_ulong_t) 0xba5e1e55deadc0deULL;
	static const struct kcmp_epoll_slot slot_data[] = {
		{ 0xdeadc0de, 0xfacef157, 0xbadc0ded },
		{ NULL_FD, ZERO_FD, 0 },
		{ 0, 0, 0 },
	};
	static kernel_ulong_t ptr_check =
		F8ILL_KULONG_SUPPORTED ? F8ILL_KULONG_MASK : 0;

	int fd;
	unsigned i;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct kcmp_epoll_slot, slot);

	/* Open some files to test printpidfd */
	fd = open(null_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", null_path);
	if (fd != NULL_FD) {
		if (dup2(fd, NULL_FD) < 0)
			perror_msg_and_fail("dup2(fd, NULL_FD)");
		close(fd);
	}

	fd = open(zero_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", zero_path);
	if (fd != ZERO_FD) {
		if (dup2(fd, ZERO_FD) < 0)
			perror_msg_and_fail("dup2(fd, ZERO_FD)");
		close(fd);
	}

	close(0);

	/* Invalid values */
	do_kcmp(bogus_pid1, bogus_pid2, bogus_type, NULL, bogus_idx1,
		bogus_idx2);
	do_kcmp(F8ILL_KULONG_MASK, F8ILL_KULONG_MASK, kcmp_max_type + 1, NULL,
		0, 0);

	/* KCMP_FILE is the only type which has additional args */
	do_kcmp(3141592653U, 2718281828U, ARG_STR(KCMP_FILE), bogus_idx1,
		bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_FILE), NULL_FD, ZERO_FD);

	/* Types without additional args */
	do_kcmp(-1, -1, ARG_STR(KCMP_VM), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_FILES), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_FS), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_SIGHAND), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_IO), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_SYSVSEM), bogus_idx1, bogus_idx2);

	/* KCMP_EPOLL_TFD checks */
	do_kcmp(-1, -1, ARG_STR(KCMP_EPOLL_TFD),
		F8ILL_KULONG_MASK | 2718281828U, ptr_check, 0);
	do_kcmp(-1, -1, ARG_STR(KCMP_EPOLL_TFD),
		3141592653U, (uintptr_t) slot + 1, 0);

	for (i = 0; i < ARRAY_SIZE(slot_data); i++) {
		memcpy(slot, slot_data + i, sizeof(*slot));

		do_kcmp(getpid(), getppid(), ARG_STR(KCMP_EPOLL_TFD), NULL_FD,
			(uintptr_t) slot, 1);
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_kcmp");

#endif
