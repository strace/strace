/*
 * Check decoding of WDIOC* commands of ioctl syscall.
 *
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define XLAT_MACROS_ONLY
#include "xlat/watchdog_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static const char *errstr;

static int
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(-1, cmd, arg);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static int
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

int
main(int argc, const char *argv[])
{
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, WDIOC_GETSUPPORT, 0);

		printf("ioctl(-1, WDIOC_GETSUPPORT, NULL) = %s%s\n",
		       sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Issued %lu ioctl syscalls but failed"
				   " to detect an injected return code %d",
				   num_skip, INJECT_RETVAL);
#endif

	TAIL_ALLOC_OBJECT_CONST_PTR(struct watchdog_info, ident);
#define IDENT_OPTIONS 0x87ff
	ident->options = IDENT_OPTIONS;
	ident->firmware_version = 6;
	memset(ident->identity, 'A', sizeof(ident->identity));
	ident->identity[sizeof(ident->identity) - 1] = '\0';

	int rc = do_ioctl_ptr(WDIOC_GETSUPPORT, ident);
	if (rc < 0) {
		printf("ioctl(-1, WDIOC_GETSUPPORT, %p) = %s\n", ident, errstr);
	} else {
		printf("ioctl(-1, WDIOC_GETSUPPORT, {options=%s, ",
		       XLAT_KNOWN(IDENT_OPTIONS,
				  "WDIOF_OVERHEAT|WDIOF_FANFAULT|"
				  "WDIOF_EXTERN1|WDIOF_EXTERN2|"
				  "WDIOF_POWERUNDER|WDIOF_CARDRESET|"
				  "WDIOF_POWEROVER|WDIOF_SETTIMEOUT|"
				  "WDIOF_MAGICCLOSE|WDIOF_PRETIMEOUT|"
				  "WDIOF_ALARMONLY|WDIOF_KEEPALIVEPING"));
#if VERBOSE
		printf("firmware_version=0x%x, identity=\"%s\"",
		       ident->firmware_version,
			   ident->identity);
#else
		printf("...");
#endif
		printf("}) = %s\n", errstr);
	}

	static const struct {
		uint32_t cmd;
		const char *str;
	} simple_get_cmds[] = {
		{ ARG_STR(WDIOC_GETSTATUS) },
		{ ARG_STR(WDIOC_GETBOOTSTATUS) },
		{ ARG_STR(WDIOC_GETTEMP) },
		{ ARG_STR(WDIOC_GETTIMEOUT) },
		{ ARG_STR(WDIOC_GETPRETIMEOUT) },
		{ ARG_STR(WDIOC_GETTIMELEFT) },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(int, val);
	*val = 123;

	for (size_t i = 0; i < ARRAY_SIZE(simple_get_cmds); ++i) {
		int rc = do_ioctl_ptr(simple_get_cmds[i].cmd, val);
		printf("ioctl(-1, " XLAT_FMT ", ",
		       XLAT_SEL(simple_get_cmds[i].cmd, simple_get_cmds[i].str));
		if (rc < 0) {
			printf("%p) = %s\n", val, errstr);
		} else {
			printf("[%d]) = %s\n", *val, errstr);
		}
	}

	do_ioctl_ptr(WDIOC_SETTIMEOUT, val);
	printf("ioctl(-1, WDIOC_SETTIMEOUT, [123]) = %s\n", errstr);

	do_ioctl_ptr(WDIOC_SETPRETIMEOUT, val);
	printf("ioctl(-1, WDIOC_SETPRETIMEOUT, [123]) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, options);

	*options = 1;
	do_ioctl_ptr(WDIOC_SETOPTIONS, options);
	printf("ioctl(-1, WDIOC_SETOPTIONS, [%s]) = %s\n",
	       XLAT_KNOWN(0x1, "WDIOS_DISABLECARD"), errstr);

	*options = 6;
	do_ioctl_ptr(WDIOC_SETOPTIONS, options);
	printf("ioctl(-1, WDIOC_SETOPTIONS, [%s]) = %s\n",
	       XLAT_KNOWN(0x6, "WDIOS_ENABLECARD|WDIOS_TEMPPANIC"), errstr);

	*options = 0xfed8;
	do_ioctl_ptr(WDIOC_SETOPTIONS, options);
	printf("ioctl(-1, WDIOC_SETOPTIONS, [%s]) = %s\n",
	       XLAT_UNKNOWN(0xfed8, "WDIOS_???"), errstr);

	do_ioctl_ptr(WDIOC_KEEPALIVE, NULL);
	printf("ioctl(-1, WDIOC_KEEPALIVE) = %s\n", errstr);

	ioctl(-1, _IOC(_IOC_NONE, 'W', 0xff, 0), &val);
	printf("ioctl(-1, _IOC(_IOC_NONE, %#x, 0xff, 0), %p) = %s\n",
	       'W', &val, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
