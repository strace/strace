/*
 * Check decoding of EPIOC* commands of ioctl syscall.
 *
 * Copyright (c) 2022-2024 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/ioctl.h>
#include <linux/eventpoll.h>


/* A hack for handling different types of _IOC() on various platforms */
#if XLAT_RAW
# define XLAT_ARGS_U(a_) (unsigned int) (a_)
#elif XLAT_VERBOSE
# define XLAT_ARGS_U(a_) (unsigned int) (a_), #a_
#else
# define XLAT_ARGS_U(a_) #a_
#endif

#ifdef INJECT_RETVAL
# define INJSTR " (INJECTED)"
#else
# define INJSTR ""
#endif

static const char *errstr;

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	const long rc = syscall(__NR_ioctl, fd, cmd, arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(int argc, char *argv[])
{
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = sys_ioctl(-1, EPIOCSPARAMS, 0);
		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(EPIOCSPARAMS), sprintrc(rc),
		       rc == 42 ? " (INJECTED)" : "");

		if (rc != 42)
			continue;

		locked = true;
		break;
	}

	if (!locked) {
		error_msg_and_fail("Have not locked on ioctl(-1"
				   ", EPIOCSPARAMS, NULL) returning 42");
	}
#endif /* INJECT_RETVAL */

	static const struct strval32 dirs[] = {
		{ ARG_STR(_IOC_NONE) },
		{ ARG_STR(_IOC_READ) },
		{ ARG_STR(_IOC_WRITE) },
		{ ARG_STR(_IOC_READ|_IOC_WRITE) },
	};
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;

	/* Unknown epoll ioctl */
	for (size_t i = 0; i < ARRAY_SIZE(dirs); i++) {
		for (unsigned int j = 0; j < 8; j += 1) {
			for (unsigned int k = 0; k < 32; k += 4) {
				if ((j == 1 || j == 2) && k == 8)
					continue;

				sys_ioctl(-1, _IOC(dirs[i].val, 0x8a, k, j),
					  magic);
				printf("ioctl(-1, "
				       XLAT_KNOWN(%#x,
						  "_IOC(%s, 0x8a, %#x, %#x)")
				       ", %#lx) = %s" INJSTR "\n",
#if XLAT_RAW || XLAT_VERBOSE
				       (unsigned int) _IOC(dirs[i].val, 0x8a,
							   k, j),
#endif
#if !XLAT_RAW
				       dirs[i].str, k, j,
#endif
				       (unsigned long) magic, errstr);
			}
		}
	}

	/* EPIOCSPARAMS/EPIOCGPARAMS */
	static struct strval32 epcmds[] = {
		{ ARG_STR(EPIOCSPARAMS) },
		{ ARG_STR(EPIOCGPARAMS) },
	};
	static const struct {
		struct epoll_params val;
		const char *str;
	} params[] = {
		{ { 0 },
		  "{busy_poll_usecs=0, busy_poll_budget=0, prefer_busy_poll=0}"
		  },
		{ { 1, 2, 3 },
		  "{busy_poll_usecs=1" NRAW(" /* 0.000001 s */")
		  ", busy_poll_budget=2, prefer_busy_poll=0x3}"
		  },
		{ { 123456, 789, 234, 56 },
		  "{busy_poll_usecs=123456" NRAW(" /* 0.123456 s */")
		  ", busy_poll_budget=789, prefer_busy_poll=0xea, __pad=0x38}"
		  },
		{ { 0xfffefdfc, 0xfbfa, 0xf9, 0xf8 },
		  "{busy_poll_usecs=4294901244" NRAW(" /* 4294.901244 s */")
		  ", busy_poll_budget=64506, prefer_busy_poll=0xf9, __pad=0xf8}"
		  },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct epoll_params, ep);

	for (size_t i = 0; i < ARRAY_SIZE(epcmds); i++) {
		sys_ioctl(-1, epcmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %s" INJSTR "\n",
		       sprintxlat(epcmds[i].str, epcmds[i].val, NULL), errstr);

		sys_ioctl(-1, epcmds[i].val, (uintptr_t) ep + 1);
		printf("ioctl(-1, %s, %p) = %s" INJSTR "\n",
		       sprintxlat(epcmds[i].str, epcmds[i].val, NULL),
		       (char *) ep + 1, errstr);

		for (size_t j = 0; j < ARRAY_SIZE(params); j++) {
			memcpy(ep, &params[j].val, sizeof(params[j].val));
			sys_ioctl(-1, epcmds[i].val, (uintptr_t) ep);
			printf("ioctl(-1, %s, ",
			       sprintxlat(epcmds[i].str, epcmds[i].val, NULL));

#ifndef INJECT_RETVAL
			if (epcmds[i].val == EPIOCGPARAMS) {
				printf("%p", ep);
			} else
#endif
			{
				fputs(params[j].str, stdout);
			}

			printf(") = %s" INJSTR "\n", errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
