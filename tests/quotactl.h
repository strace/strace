/*
 * Common definitions for Linux and XFS quota tests.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_TESTS_QUOTACTL_H
#define STRACE_TESTS_QUOTACTL_H

# include <inttypes.h>
# include <stdarg.h>
# include <stdio.h>

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

# define PRINT_FIELD_D(prefix, where, field)	\
	printf("%s%s=%lld", (prefix), #field,	\
	       sign_extend_unsigned_to_ll((where)->field))

# define PRINT_FIELD_U(prefix, where, field)	\
	printf("%s%s=%llu", (prefix), #field,	\
	       zero_extend_signed_to_ull((where)->field))

# define PRINT_FIELD_X(prefix, where, field)	\
	printf("%s%s=%#llx", (prefix), #field,	\
	       zero_extend_signed_to_ull((where)->field))

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

	printf(") = %s\n", sprintrc(rc));
}


static const int bogus_cmd = 0xbadc0ded;
static const int bogus_id = 0xca7faced;

/* It is invalid anyway due to the slash in the end */
static const char *bogus_dev = "/dev/bogus/";
static const char *bogus_dev_str = "\"/dev/bogus/\"";

static const char unterminated_data[] = { '\1', '\2', '\3' };

#endif /* !STRACE_TESTS_QUOTACTL_H */
