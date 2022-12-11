/*
 * Check decoding of COUNTER_* commands of ioctl syscall.
 *
 * Copyright (c) 2022 Eugene Syromyatnikov <evgsyr@gmail.com>.
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
#include <linux/counter.h>


/* A hack for handling different types of _IOC() on various platforms */
#if XLAT_RAW
# define XLAT_ARGS_U(a_) (unsigned int) (a_)
#elif XLAT_VERBOSE
# define XLAT_ARGS_U(a_) (unsigned int) (a_), #a_
#else
# define XLAT_ARGS_U(a_) #a_
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
main(void)
{
	static const struct {
		uint32_t val;
		const char *str;
	} dirs[] = {
		{ ARG_STR(_IOC_NONE) },
		{ ARG_STR(_IOC_READ) },
		{ ARG_STR(_IOC_WRITE) },
		{ ARG_STR(_IOC_READ|_IOC_WRITE) },
	};
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;

	/* Unknown counter ioctl */
	for (size_t i = 0; i < ARRAY_SIZE(dirs); i++) {
		for (unsigned int j = 0; j < 32; j += 1) {
			sys_ioctl(-1, _IOC(dirs[i].val, '>', 3, j), magic);
			printf("ioctl(-1, "
			       XLAT_KNOWN(%#x, "_IOC(%s, 0x3e, 0x3, %#x)")
			       ", %#lx) = %s\n",
#if XLAT_RAW || XLAT_VERBOSE
			       (unsigned int) _IOC(dirs[i].val, '>', 3, j),
#endif
#if !XLAT_RAW
			       dirs[i].str, j,
#endif
			       (unsigned long) magic, errstr);
		}
	}

	/* COUNTER_ADD_WATCH_IOCTL */
	static const struct {
		struct counter_watch val;;
		const char *str;
	} watches[] = {
		{ { { 0 } },
		  "{component={type=" XLAT_KNOWN(0, "COUNTER_COMPONENT_NONE")
		  ", scope=" XLAT_KNOWN(0, "COUNTER_SCOPE_DEVICE")
		  ", parent=0, id=0}"
		  ", event=" XLAT_KNOWN(0, "COUNTER_EVENT_OVERFLOW")
		  ", channel=0}" },
		{ { { COUNTER_COMPONENT_EXTENSION, COUNTER_SCOPE_COUNT,
		      23, 42 }, COUNTER_EVENT_CAPTURE, 69 },
		  "{component="
		  "{type=" XLAT_KNOWN(0x5, "COUNTER_COMPONENT_EXTENSION")
		  ", scope=" XLAT_KNOWN(0x2, "COUNTER_SCOPE_COUNT")
		  ", parent=23, id=42}"
		  ", event=" XLAT_KNOWN(0x6, "COUNTER_EVENT_CAPTURE")
		  ", channel=69}" },
		{ { { COUNTER_COMPONENT_EXTENSION + 1, COUNTER_SCOPE_COUNT + 1,
		      142, 160 }, COUNTER_EVENT_CAPTURE + 1, 173 },
		  "{component={type=" XLAT_UNKNOWN(0x6, "COUNTER_COMPONENT_???")
		  ", scope=" XLAT_UNKNOWN(0x3, "COUNTER_SCOPE_???")
		  ", parent=142, id=160}"
		  ", event=" XLAT_UNKNOWN(0x7, "COUNTER_EVENT_???")
		  ", channel=173}" },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct counter_watch, watch);

	sys_ioctl(-1, COUNTER_ADD_WATCH_IOCTL, 0);
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
	       XLAT_ARGS_U(COUNTER_ADD_WATCH_IOCTL), errstr);

	sys_ioctl(-1, COUNTER_ADD_WATCH_IOCTL, (uintptr_t) watch + 1);
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
	       XLAT_ARGS_U(COUNTER_ADD_WATCH_IOCTL),
	       (char *) watch + 1, errstr);

	for (size_t i = 0; i < ARRAY_SIZE(watches); i++) {
		memcpy(watch, &watches[i].val, sizeof(watches[i].val));
		sys_ioctl(-1, COUNTER_ADD_WATCH_IOCTL, (uintptr_t) watch);
		printf("ioctl(-1, " XLAT_FMT ", %s) = %s\n",
		       XLAT_ARGS_U(COUNTER_ADD_WATCH_IOCTL),
		       watches[i].str, errstr);
	}

	/* COUNTER_ENABLE_EVENTS_IOCTL */
	sys_ioctl(-1, COUNTER_ENABLE_EVENTS_IOCTL, magic);
	printf("ioctl(-1, " XLAT_FMT ") = %s\n",
	       XLAT_ARGS_U(COUNTER_ENABLE_EVENTS_IOCTL), errstr);

	/* COUNTER_DISABLE_EVENTS_IOCTL */
	sys_ioctl(-1, COUNTER_DISABLE_EVENTS_IOCTL, magic);
	printf("ioctl(-1, " XLAT_FMT ") = %s\n",
	       XLAT_ARGS_U(COUNTER_DISABLE_EVENTS_IOCTL), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
