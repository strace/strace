/*
 * Check that syscall numbers do not conflict with seccomp filter flags.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2018-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "arch_defs.h"
#include "sysent.h"
#include "scno.h"

/* PERSONALITY*_AUDIT_ARCH definitions depend on AUDIT_ARCH_* constants.  */
#ifdef PERSONALITY0_AUDIT_ARCH
# include <linux/audit.h>
# define XLAT_MACROS_ONLY
#  include "xlat/elf_em.h"
#  include "xlat/audit_arch.h"
# undef XLAT_MACROS_ONLY
#endif

/* Define these shorthand notations to simplify the syscallent files. */
#include "sysent_shorthand_defs.h"

const struct_sysent sysent0[] = {
#include "syscallent.h"
};

#if SUPPORTED_PERSONALITIES > 1
const struct_sysent sysent1[] = {
# include "syscallent1.h"
};
#endif

#if SUPPORTED_PERSONALITIES > 2
const struct_sysent sysent2[] = {
# include "syscallent2.h"
};
#endif

const unsigned int nsyscall_vec[SUPPORTED_PERSONALITIES] = {
	ARRAY_SIZE(sysent0),
#if SUPPORTED_PERSONALITIES > 1
	ARRAY_SIZE(sysent1),
#endif
#if SUPPORTED_PERSONALITIES > 2
	ARRAY_SIZE(sysent2),
#endif
};

struct audit_arch_t {
	unsigned int arch;
	unsigned int flag;
};

static const struct audit_arch_t audit_arch_vec[SUPPORTED_PERSONALITIES] = {
#if SUPPORTED_PERSONALITIES > 1
	PERSONALITY0_AUDIT_ARCH,
	PERSONALITY1_AUDIT_ARCH,
# if SUPPORTED_PERSONALITIES > 2
	PERSONALITY2_AUDIT_ARCH,
# endif
#endif
};

int
main(void)
{
	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		if (!audit_arch_vec[p].flag)
			continue;
		for (unsigned int nr = 1; nr < nsyscall_vec[p]; ++nr) {
			if (!(audit_arch_vec[p].flag & nr))
				continue;
			error_msg_and_fail("system call number %u of"
					   " personality %u conflicts with"
					   " seccomp filter flag %#x",
					   nr, p, audit_arch_vec[p].flag);
		}
	}
	return 0;
}
