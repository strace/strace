/*
 * Common definitions for Linux and XFS quota tests.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_QUOTACTL_H
# define STRACE_TESTS_QUOTACTL_H

# include <inttypes.h>
# include <stdarg.h>
# include <stdio.h>
# include "print_fields.h"

# ifdef HAVE_LINUX_QUOTA_H
/* Broken in CentOS 5: has extern spinlock_t dq_data_lock; declaration */
#  include <linux/quota.h>
# else
#  include <linux/types.h>
/* Broken in some new glibc versions: have Q_GETNEXTQUOTA definition but no
 * struct nextdqblk defined. Fixed in glibc-2.24-106-g4d72808. */
#  include <sys/quota.h>
# endif

# ifndef QCMD_CMD
#  define QCMD_CMD(_val) ((unsigned) (_val) >> SUBCMDSHIFT)
# endif /* !QCMD_CMD */

# ifndef QCMD_TYPE
#  define QCMD_TYPE(_val) ((unsigned) (_val) & SUBCMDMASK)
# endif /* !QCMD_TYPE */

# ifndef PRJQUOTA
#  define PRJQUOTA 2
# endif

typedef void (*print_cb)(long rc, void *addr, void *arg);

enum check_quotactl_flag_bits {
	CQF_ID_SKIP_BIT,
	CQF_ID_STR_BIT,
	CQF_ADDR_SKIP_BIT,
	CQF_ADDR_STR_BIT,
	CQF_ADDR_CB_BIT,
};

enum check_quotactl_flags {
	CQF_NONE,
	CQF_ID_SKIP   = 1 << CQF_ID_SKIP_BIT,
	CQF_ID_STR    = 1 << CQF_ID_STR_BIT,
	CQF_ADDR_SKIP = 1 << CQF_ADDR_SKIP_BIT,
	CQF_ADDR_STR  = 1 << CQF_ADDR_STR_BIT,
	CQF_ADDR_CB   = 1 << CQF_ADDR_CB_BIT,
};

static const char *errstr;

/**
 * Generic quotactl syscall checker function.  Call convention:
 *
 *     check_quota(flags, cmd, cmd_str, special, special_str
 *		   [, id [, id_str]]
 *		   [, addr [, { addr_cb, addr_cb_arg | addr_str }]])
 *
 * check_quota performs a syscall invocation and prints the expected output
 * for it.
 *
 * It might be useful to employ ARG_STR macro for passing cmd/cmd_str,
 * special_special_str, id/id_str, and addr/addr_str argument pairs.
 *
 * @param flags Check flags:
 *               - CQF_ID_SKIP: the "id" syscall argument is ignored
 *                 in the syscall invocation.  No id and id_str arguments
 *                 should be provided if this flag is set.
 *                 This flag has priority over the CQF_ID_STR flag.
 *               - CQF_ID_STR: the "id" syscall argument has a special string
 *                 representation.  id_str argument should be provided if this
 *                 flag is set; no id_str argument should be provided and id
 *                 argument is printed as unsigned integer (with an exception
 *                 for -1, which is printed as signed) if this flag is not set.
 *               - CQF_ADDR_SKIP: the "addr" syscall argument is ignored
 *                 in the syscall invocation.  None of the addr, addr_cb,
 *                 addr_cb_arg, and/or addr_str arguments should be provided
 *                 if this flag is set.  This flag has priority
 *                 over the CQF_ADDR_STR and CQF_ADDR_CB flags.
 *               - CQF_ADDR_CB: the "addr" syscall argument printing is handled
 *                 via a callback function.  addr_cb (that points to a callback
 *                 function of type print_cb) and addr_cb_arg (an opaque pointer
 *                 that is passed to addr_cb in the third argument) should
 *                 be provided if this flag is set.
 *                 This flag has priority over the CQF_ADDR_STR flag.
 *               - CQF_ADDR_STR: addr syscall argument has a special string
 *                 representation.  addr_str argument should be provided if this
 *                 flag is set.  If both CQF_ADDR_CB and CQF_ADDR_STR flags
 *                 are not set, addr syscall argument is printed using "%p"
 *                 printf format.
 * @param cmd Value of the "cmd" syscall argument that should be passed
 *            in the syscall invocation.
 * @param cmd_str String representation of the "cmd" syscall argument.
 * @param special Value of the "special" syscall argument that should be passed
 *                in the syscall invocation.
 * @param special_str String representation of the "special" syscall argument.
 * @param ... Additional arguments depend on the flags being set:
 *             - id: Value of the "id" syscall argument.  Provided
 *               if CQF_ID_SKIP is not set, otherwise -1 is passed
 *               in the syscall invocation and the argument is not printed
 *               in the expected output.
 *             - id_str: String representation of the "id" syscall argument.
 *               Provided if CQF_ID_SKIP is not set and CQF_ID_STR is set.
 *             - addr: Value of the "addr" syscall argument.  Provided
 *               if CQF_ADDR_SKIP is not set, otherwise NULL is passed
 *               in the syscall invocation and the argument is not printed
 *               in the expected output.
 *             - addr_cb: Callback function that is called for the "addr"
 *               syscall argument printing. Should be of print_cb type.
 *               Provided if CQF_ADDR_SKIP is not set and CQF_ADDR_CB is set.
 *             - addr_cb_arg: Opaque pointer that is passed to addr_cb,
 *               Provided if CQF_ADDR_SKIP is not set and CQF_ADDR_CB is set.
 *             - addr_str: String representation of the "addr" syscall argument.
 *               Provided if CQF_ADDR_SKIP is not set, CQF_ADDR_CB is not set,
 *               and CQF_ADDR_STR is set.
 */
static inline void
check_quota(uint32_t flags, int cmd, const char *cmd_str,
	const char *special, const char *special_str, ...)
{
	long rc;
	const char *addr_str = NULL;
	const char *id_str = NULL;
	void *addr = NULL;
	print_cb addr_cb = NULL;
	void *addr_cb_arg = NULL;
	uint32_t id = -1;

	va_list ap;

	va_start(ap, special_str);

	if (!(flags & CQF_ID_SKIP)) {
		id = va_arg(ap, uint32_t);

		if (flags & CQF_ID_STR)
			id_str = va_arg(ap, const char *);
	}

	if (!(flags & CQF_ADDR_SKIP)) {
		addr = va_arg(ap, void *);

		if (flags & CQF_ADDR_CB) {
			addr_cb = va_arg(ap, print_cb);
			addr_cb_arg = va_arg(ap, void *);
		} else if (flags & CQF_ADDR_STR) {
			addr_str = va_arg(ap, const char *);
		}
	}

	va_end(ap);

	rc = syscall(__NR_quotactl, cmd, special, id, addr);

	errstr = sprintrc(rc);

# ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %d",
				   rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
# endif

	printf("quotactl(%s, %s", cmd_str, special_str);

	if (!(flags & CQF_ID_SKIP)) {
		if (flags & CQF_ID_STR) {
			printf(", %s", id_str);
		} else {
			if (id == (uint32_t)-1)
				printf(", -1");
			else
				printf(", %u", id);
		}
	}

	if (!(flags & CQF_ADDR_SKIP)) {
		if (flags & CQF_ADDR_CB) {
			printf(", ");
			addr_cb(rc, addr, addr_cb_arg);
		} else if (flags & CQF_ADDR_STR) {
			printf(", %s", addr_str);
		} else {
			printf(", %p", addr);
		}
	}

	printf(") = %s\n", errstr);
}


static const int bogus_cmd = 0xbadc0ded;
static const int bogus_id = 0xca7faced;

/* It is invalid anyway due to the slash in the end */
static const char *bogus_dev = "/dev/bogus/";
static const char *bogus_dev_str = "\"/dev/bogus/\"";

static const char unterminated_data[] = { '\1', '\2', '\3' };

#endif /* !STRACE_TESTS_QUOTACTL_H */
