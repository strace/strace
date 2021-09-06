/*
 * Copyright (c) 2015 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2016-2020 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2017 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2018 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

/* for __X32_SYSCALL_BIT */
#include "scno.h"

/* PERSONALITY*_AUDIT_ARCH definitions depend on AUDIT_ARCH_* constants.  */
#ifdef PERSONALITY0_AUDIT_ARCH
# include <linux/audit.h>
# define XLAT_MACROS_ONLY
#  include "xlat/elf_em.h"
#  include "xlat/audit_arch.h"
# undef XLAT_MACROS_ONLY
#endif

#include "nr_prefix.c"

const struct audit_arch_t audit_arch_vec[SUPPORTED_PERSONALITIES] = {
#ifdef PERSONALITY0_AUDIT_ARCH
	PERSONALITY0_AUDIT_ARCH,
# if SUPPORTED_PERSONALITIES > 1
	PERSONALITY1_AUDIT_ARCH,
#  if SUPPORTED_PERSONALITIES > 2
	PERSONALITY2_AUDIT_ARCH,
#  endif
# endif
#endif
};


const char *
syscall_name(kernel_ulong_t scno)
{
	return scno_is_valid(scno) ? sysent[scno].sys_name : NULL;
}

const char *
syscall_name_arch(kernel_ulong_t nr, unsigned int arch, const char **prefix)
{
#ifdef PERSONALITY0_AUDIT_ARCH
	for (size_t i = 0; i < SUPPORTED_PERSONALITIES; i++) {
		if (arch != audit_arch_vec[i].arch)
			continue;

		kernel_ulong_t scno = shuffle_scno_pers(nr, i);

		/*
		 * checks from scno_is_in_range/scno_is_valid,
		 * but for an arbitrary personality.
		 */
		if ((scno >= nsyscall_vec[i])
		    || !(sysent_vec[i][scno].sys_func)
		    || (sysent_vec[i][scno].sys_flags & TRACE_INDIRECT_SUBCALL))
			continue;

		if (prefix) {
			*prefix = (i == current_personality) ? nr_prefix(nr)
							     : NULL;
		}
		return sysent_vec[i][scno].sys_name;
	}
#endif

	if (prefix)
		*prefix = NULL;
	return NULL;
}
