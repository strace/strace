/*
 * Check decoding of PIDFD_GET_INFO ioctl.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/pidfd.h>

#define PIDFD_INFO_MASK_all_known_u					\
	(PIDFD_INFO_PID |						\
	 PIDFD_INFO_CREDS |						\
	 PIDFD_INFO_CGROUPID |						\
	 PIDFD_INFO_EXIT |						\
	 PIDFD_INFO_COREDUMP |						\
	 PIDFD_INFO_SUPPORTED_MASK |					\
	 PIDFD_INFO_COREDUMP_SIGNAL |					\
	 PIDFD_INFO_COREDUMP_CODE)

#define PIDFD_INFO_MASK_all_known_str					\
	"PIDFD_INFO_PID|"						\
	"PIDFD_INFO_CREDS|"						\
	"PIDFD_INFO_CGROUPID|"						\
	"PIDFD_INFO_EXIT|"						\
	"PIDFD_INFO_COREDUMP|"						\
	"PIDFD_INFO_SUPPORTED_MASK|"					\
	"PIDFD_INFO_COREDUMP_SIGNAL|"					\
	"PIDFD_INFO_COREDUMP_CODE"

#define PIDFD_INFO_MASK_all_unknown_u	0xffffffffffffff00
#define PIDFD_INFO_MASK_all_unknown_str	STRINGIFY_VAL(PIDFD_INFO_MASK_all_unknown_u)

static_assert((PIDFD_INFO_MASK_all_known_u & PIDFD_INFO_MASK_all_unknown_u) == 0,
	      "PIDFD_INFO_MASK_all_known_u and PIDFD_INFO_MASK_all_unknown_u are complementary");
static_assert((PIDFD_INFO_MASK_all_known_u | PIDFD_INFO_MASK_all_unknown_u) == -1ULL,
	      "PIDFD_INFO_MASK_all_known_u and PIDFD_INFO_MASK_all_unknown_u are complementary");

#define PIDFD_INFO_MASK_all_u	0xffffffffffffffff
#define PIDFD_INFO_MASK_all_str	PIDFD_INFO_MASK_all_known_str "|" PIDFD_INFO_MASK_all_unknown_str

#define PIDFD_COREDUMP_MASK_all_known_u					\
	(PIDFD_COREDUMPED |						\
	 PIDFD_COREDUMP_SKIP |						\
	 PIDFD_COREDUMP_USER |						\
	 PIDFD_COREDUMP_ROOT)

#define PIDFD_COREDUMP_MASK_all_known_str				\
	"PIDFD_COREDUMPED|"						\
	"PIDFD_COREDUMP_SKIP|"						\
	"PIDFD_COREDUMP_USER|"						\
	"PIDFD_COREDUMP_ROOT"

#define PIDFD_COREDUMP_MASK_all_unknown_u	0xfffffff0
#define PIDFD_COREDUMP_MASK_all_unknown_str STRINGIFY_VAL(PIDFD_COREDUMP_MASK_all_unknown_u)

static_assert((PIDFD_COREDUMP_MASK_all_known_u & PIDFD_COREDUMP_MASK_all_unknown_u) == 0,
	      "PIDFD_COREDUMP_MASK_all_known_u and PIDFD_COREDUMP_MASK_all_unknown_u are complementary");
static_assert((PIDFD_COREDUMP_MASK_all_known_u | PIDFD_COREDUMP_MASK_all_unknown_u) == -1U,
	      "PIDFD_COREDUMP_MASK_all_known_u and PIDFD_COREDUMP_MASK_all_unknown_u are complementary");

#define PIDFD_COREDUMP_MASK_all_u	0xffffffff
#define PIDFD_COREDUMP_MASK_all_str	PIDFD_COREDUMP_MASK_all_known_str "|" PIDFD_COREDUMP_MASK_all_unknown_str

static const struct strval32 PIDFD_GET_INFO_cmd = {
	ARG_STR(PIDFD_GET_INFO)
};

static const struct strval32 PIDFD_GET_INFO_undersize_cmd = {
	ARG_STR(_IOC(_IOC_READ|_IOC_WRITE, 0xff, 0xb, 0x3f))
};

static const struct strval32 PIDFD_GET_INFO_oversize_cmd = {
	ARG_STR(_IOC(_IOC_READ|_IOC_WRITE, 0xff, 0xb, 0x59))
};

static const char *errstr;

static int
do_ioctl_fd(int fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(fd, cmd, arg);

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

#ifdef INJECT_RETVAL

static void
skip_ioctls(const char *const *argv)
{
	unsigned long num_skip;
	long inject_retval;

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);
	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, PIDFD_GET_INFO, 0);

		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s%s\n",
		       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
		       sprintrc(rc),
		       rc == inject_retval ? " (INJECTED)" : "");

		if (rc == inject_retval)
			return;
	}

	error_msg_and_fail("Issued %lu ioctl syscalls but failed"
			   " to detect an injected return code %ld",
			   num_skip, inject_retval);
}

/*
 * Exercise PIDFD_GET_INFO decoding under syscall injection: the kernel does not
 * overwrite the buffer, so each scenario uses an explicitly crafted struct and
 * an expected line built from XLAT_KNOWN/XLAT_UNKNOWN literals.
 */
static void
injected_pidfd_get_info_decode_checks(struct pidfd_info *info)
{
	static const struct strval32 PIDFD_GET_INFO_ver0_cmd = {
		ARG_STR(_IOC(_IOC_READ|_IOC_WRITE, 0xff, 0xb, 0x40))
	};
	static const struct strval32 PIDFD_GET_INFO_ver1_cmd = {
		ARG_STR(_IOC(_IOC_READ|_IOC_WRITE, 0xff, 0xb, 0x48))
	};
	static const struct strval32 PIDFD_GET_INFO_ver2_cmd = {
		ARG_STR(_IOC(_IOC_READ|_IOC_WRITE, 0xff, 0xb, 0x50))
	};

	const kernel_ulong_t cmd = PIDFD_GET_INFO_cmd.val;

	fill_memory(info, sizeof(*info));
	info->mask = PIDFD_INFO_MASK_all_unknown_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_UNKNOWN(PIDFD_INFO_MASK_all_unknown_u, "PIDFD_INFO_???"),
	       errstr);

	info->mask = PIDFD_INFO_PID | 0x8000000000000000ULL;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT
	       ", {mask=%s, pid=%d, tgid=%d, ppid=%d}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x8000000000000001, "PIDFD_INFO_PID|0x8000000000000000"),
	       info->pid, info->tgid, info->ppid,
	       errstr);

	info->mask = PIDFD_INFO_PID;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT
	       ", {mask=%s, pid=%d, tgid=%d, ppid=%d}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"),
	       info->pid, info->tgid, info->ppid,
	       errstr);

	info->mask = PIDFD_INFO_CREDS;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, "
	       "ruid=%u, rgid=%u, euid=%u, egid=%u, "
	       "suid=%u, sgid=%u, fsuid=%u, fsgid=%u}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x2, "PIDFD_INFO_CREDS"),
	       info->ruid, info->rgid, info->euid, info->egid,
	       info->suid, info->sgid, info->fsuid, info->fsgid,
	       errstr);

	info->mask = PIDFD_INFO_CGROUPID;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, cgroupid=%llu}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x4, "PIDFD_INFO_CGROUPID"),
	       (unsigned long long) info->cgroupid,
	       errstr);

	info->mask = PIDFD_INFO_SUPPORTED_MASK;
	info->supported_mask = PIDFD_INFO_PID;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, supported_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x20, "PIDFD_INFO_SUPPORTED_MASK"),
	       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"),
	       errstr);

	info->mask = PIDFD_INFO_SUPPORTED_MASK;
	info->supported_mask = PIDFD_INFO_MASK_all_known_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, supported_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x20, "PIDFD_INFO_SUPPORTED_MASK"),
	       XLAT_KNOWN(0xff, PIDFD_INFO_MASK_all_known_str),
	       errstr);

	info->mask = PIDFD_INFO_SUPPORTED_MASK;
	info->supported_mask = PIDFD_INFO_MASK_all_unknown_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, supported_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x20, "PIDFD_INFO_SUPPORTED_MASK"),
	       XLAT_UNKNOWN(PIDFD_INFO_MASK_all_unknown_u, "PIDFD_INFO_???"),
	       errstr);

	info->mask = PIDFD_INFO_SUPPORTED_MASK;
	info->supported_mask = PIDFD_INFO_MASK_all_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, supported_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x20, "PIDFD_INFO_SUPPORTED_MASK"),
	       XLAT_KNOWN(PIDFD_INFO_MASK_all_u, PIDFD_INFO_MASK_all_str),
	       errstr);

	info->mask = PIDFD_INFO_EXIT;
	info->exit_code = 4 << 8; /* WIFEXITED with status 4 */
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, exit_code=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x8, "PIDFD_INFO_EXIT"),
	       "[{WIFEXITED(s) && WEXITSTATUS(s) == 4}]",
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_MASK_all_unknown_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_UNKNOWN(PIDFD_COREDUMP_MASK_all_unknown_u,
			    "PIDFD_COREDUMP_???"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMPED;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(0x1, "PIDFD_COREDUMPED"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_SKIP;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(0x2, "PIDFD_COREDUMP_SKIP"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_USER;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(0x4, "PIDFD_COREDUMP_USER"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_ROOT;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(0x8, "PIDFD_COREDUMP_ROOT"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_MASK_all_known_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(0xf, PIDFD_COREDUMP_MASK_all_known_str),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP;
	info->coredump_mask = PIDFD_COREDUMP_MASK_all_u;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x10, "PIDFD_INFO_COREDUMP"),
	       XLAT_KNOWN(PIDFD_COREDUMP_MASK_all_u,
			  PIDFD_COREDUMP_MASK_all_str),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP_SIGNAL;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_signal=%d}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x40, "PIDFD_INFO_COREDUMP_SIGNAL"),
	       info->coredump_signal,
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP_SIGNAL;
	info->coredump_signal = SIGSEGV;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_signal=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x40, "PIDFD_INFO_COREDUMP_SIGNAL"),
	       XLAT_KNOWN(11, "SIGSEGV"),
	       errstr);

	info->mask = PIDFD_INFO_COREDUMP_CODE;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s, coredump_code=%u}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x80, "PIDFD_INFO_COREDUMP_CODE"),
	       info->coredump_code,
	       errstr);

	info->mask = PIDFD_INFO_MASK_all_known_u;
	info->supported_mask = PIDFD_INFO_PID;
	info->exit_code = 2 << 8; /* WIFEXITED with status 2 */
	info->coredump_mask = PIDFD_COREDUMPED;
	info->coredump_signal = SIGABRT;
	do_ioctl_fd(-1, cmd, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s"
	       ", cgroupid=%llu, pid=%d, tgid=%d, ppid=%d"
	       ", ruid=%u, rgid=%u, euid=%u, egid=%u"
	       ", suid=%u, sgid=%u, fsuid=%u, fsgid=%u"
	       ", exit_code=%s, coredump_mask=%s"
	       ", coredump_signal=%s, coredump_code=%u"
	       ", supported_mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0xff, PIDFD_INFO_MASK_all_known_str),
	       (unsigned long long) info->cgroupid,
	       info->pid, info->tgid, info->ppid,
	       info->ruid, info->rgid, info->euid, info->egid,
	       info->suid, info->sgid, info->fsuid, info->fsgid,
	       "[{WIFEXITED(s) && WEXITSTATUS(s) == 2}]",
	       XLAT_KNOWN(0x1, "PIDFD_COREDUMPED"),
	       XLAT_KNOWN(6, "SIGABRT"),
	       info->coredump_code,
	       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"),
	       errstr);

	do_ioctl_fd(-1, PIDFD_GET_INFO_ver2_cmd.val, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s"
	       ", cgroupid=%llu, pid=%d, tgid=%d, ppid=%d"
	       ", ruid=%u, rgid=%u, euid=%u, egid=%u"
	       ", suid=%u, sgid=%u, fsuid=%u, fsgid=%u"
	       ", exit_code=%s, coredump_mask=%s"
	       ", coredump_signal=%s, coredump_code=%u}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_ver2_cmd.val,
		        PIDFD_GET_INFO_ver2_cmd.str),
	       XLAT_KNOWN(0xff, PIDFD_INFO_MASK_all_known_str),
	       (unsigned long long) info->cgroupid,
	       info->pid, info->tgid, info->ppid,
	       info->ruid, info->rgid, info->euid, info->egid,
	       info->suid, info->sgid, info->fsuid, info->fsgid,
	       "[{WIFEXITED(s) && WEXITSTATUS(s) == 2}]",
	       XLAT_KNOWN(0x1, "PIDFD_COREDUMPED"),
	       XLAT_KNOWN(6, "SIGABRT"),
	       info->coredump_code,
	       errstr);

	do_ioctl_fd(-1, PIDFD_GET_INFO_ver1_cmd.val, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s"
	       ", cgroupid=%llu, pid=%d, tgid=%d, ppid=%d"
	       ", ruid=%u, rgid=%u, euid=%u, egid=%u"
	       ", suid=%u, sgid=%u, fsuid=%u, fsgid=%u"
	       ", exit_code=%s, coredump_mask=%s"
	       ", coredump_signal=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_ver1_cmd.val,
		        PIDFD_GET_INFO_ver1_cmd.str),
	       XLAT_KNOWN(0xff, PIDFD_INFO_MASK_all_known_str),
	       (unsigned long long) info->cgroupid,
	       info->pid, info->tgid, info->ppid,
	       info->ruid, info->rgid, info->euid, info->egid,
	       info->suid, info->sgid, info->fsuid, info->fsgid,
	       "[{WIFEXITED(s) && WEXITSTATUS(s) == 2}]",
	       XLAT_KNOWN(0x1, "PIDFD_COREDUMPED"),
	       XLAT_KNOWN(6, "SIGABRT"),
	       errstr);

	do_ioctl_fd(-1, PIDFD_GET_INFO_ver0_cmd.val, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s"
	       ", cgroupid=%llu, pid=%d, tgid=%d, ppid=%d"
	       ", ruid=%u, rgid=%u, euid=%u, egid=%u"
	       ", suid=%u, sgid=%u, fsuid=%u, fsgid=%u"
	       ", exit_code=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_ver0_cmd.val,
		        PIDFD_GET_INFO_ver0_cmd.str),
	       XLAT_KNOWN(0xff, PIDFD_INFO_MASK_all_known_str),
	       (unsigned long long) info->cgroupid,
	       info->pid, info->tgid, info->ppid,
	       info->ruid, info->rgid, info->euid, info->egid,
	       info->suid, info->sgid, info->fsuid, info->fsgid,
	       "[{WIFEXITED(s) && WEXITSTATUS(s) == 2}]",
	       errstr);
}

#else /* !INJECT_RETVAL */

static void
print_pidfd_info(struct pidfd_info *info)
{
	printf("{mask=%s", XLAT_KNOWN(0x1, "PIDFD_INFO_PID"));
	if (info->mask != PIDFD_INFO_PID) {
		fputs(" => ", stdout);
		switch (info->mask) {
		case PIDFD_INFO_PID | PIDFD_INFO_CREDS | PIDFD_INFO_CGROUPID:
			fputs(XLAT_KNOWN(0x7,
					 "PIDFD_INFO_PID|"
					 "PIDFD_INFO_CREDS|"
					 "PIDFD_INFO_CGROUPID"),
			      stdout);
			break;
		case PIDFD_INFO_PID | PIDFD_INFO_CREDS:
			fputs(XLAT_KNOWN(0x3,
					 "PIDFD_INFO_PID|"
					 "PIDFD_INFO_CREDS"),
			      stdout);
			break;
		default:
			error_msg_and_fail("unexpected PIDFD_GET_INFO result mask %#jx",
					   (uintmax_t) info->mask);
		}
	}

	if ((info->mask & PIDFD_INFO_CGROUPID))
		printf(", cgroupid=%llu",
		       (unsigned long long) info->cgroupid);

	if ((info->mask & PIDFD_INFO_PID))
		printf(", pid=%d, tgid=%d, ppid=%d",
		       info->pid, info->tgid, info->ppid);

	if ((info->mask & PIDFD_INFO_CREDS))
		printf(", ruid=%u, rgid=%u, euid=%u, egid=%u"
		       ", suid=%u, sgid=%u, fsuid=%u, fsgid=%u",
		       info->ruid, info->rgid, info->euid, info->egid,
		       info->suid, info->sgid, info->fsuid, info->fsgid);

	putchar('}');
}

#endif /* !INJECT_RETVAL */

int
main(int argc, const char *argv[])
{
	skip_if_unavailable("/proc/self/exe");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct pidfd_info, info);

#ifdef INJECT_RETVAL
	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	skip_ioctls(argv);

	injected_pidfd_get_info_decode_checks(info);
#else
	void *const efault = info + 1;
	const int pidfd =
		syscall(__NR_pidfd_open, (kernel_ulong_t) getpid(), 0L);
	int rc;

	if (pidfd < 0)
		perror_msg_and_skip("pidfd_open");

	memset(info, 0, sizeof(*info));
	info->mask = PIDFD_INFO_PID;

	do_ioctl_fd(-1, PIDFD_GET_INFO_cmd.val, (uintptr_t) info);
	printf("ioctl(-1, " XLAT_FMT ", {mask=%s}) = %s\n",
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"), errstr);

	do_ioctl_fd(pidfd, PIDFD_GET_INFO_cmd.val, (uintptr_t) efault);
	printf("ioctl(%d, " XLAT_FMT ", %p) = %s\n",
	       pidfd,
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str),
	       efault, errstr);

	memset(info, 0, sizeof(*info));
	info->mask = PIDFD_INFO_PID;

	do_ioctl_fd(pidfd, PIDFD_GET_INFO_undersize_cmd.val, (uintptr_t) info);
	printf("ioctl(%d, " XLAT_FMT ", %p) = %s\n",
	       pidfd,
	       XLAT_SEL(PIDFD_GET_INFO_undersize_cmd.val,
		        PIDFD_GET_INFO_undersize_cmd.str),
	       info, errstr);

	memset(info, 0, sizeof(*info));
	info->mask = PIDFD_INFO_PID;

	rc = do_ioctl_fd(pidfd, PIDFD_GET_INFO_oversize_cmd.val,
			 (uintptr_t) info);
	printf("ioctl(%d, " XLAT_FMT ", ",
	       pidfd,
	       XLAT_SEL(PIDFD_GET_INFO_oversize_cmd.val,
		        PIDFD_GET_INFO_oversize_cmd.str));
	if (rc < 0)
		printf("{mask=%s}",
		       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"));
	else
		print_pidfd_info(info);
	printf(") = %s\n", errstr);

	memset(info, 0, sizeof(*info));
	info->mask = PIDFD_INFO_PID;

	rc = do_ioctl_fd(pidfd, PIDFD_GET_INFO_cmd.val, (uintptr_t) info);

	printf("ioctl(%d, " XLAT_FMT ", ",
	       pidfd,
	       XLAT_SEL(PIDFD_GET_INFO_cmd.val, PIDFD_GET_INFO_cmd.str));
	if (rc < 0)
		printf("{mask=%s}",
		       XLAT_KNOWN(0x1, "PIDFD_INFO_PID"));
	else
		print_pidfd_info(info);
	printf(") = %s\n", errstr);

	close(pidfd);
#endif /* !INJECT_RETVAL */

	puts("+++ exited with 0 +++");
	return 0;
}
