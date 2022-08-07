/*
 * Validate syscallent.h file.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "sysent.h"
#include <stdio.h>
#include <string.h>
#include "scno.h"

#include "sysent_shorthand_defs.h"

static const struct_sysent syscallent[] = {
#include "syscallent.h"
};

#include "sysent_shorthand_undefs.h"

DIAG_PUSH_IGNORE_OVERRIDE_INIT
typedef const char *pstr_t;
static const pstr_t ksyslist[] = {
#include "ksysent.h"
};
DIAG_POP_IGNORE_OVERRIDE_INIT

int
main(void)
{
	int rc = 0;

	for (unsigned int i = 0; i < ARRAY_SIZE(ksyslist); ++i) {
		if (!ksyslist[i])
			continue;
		if (i >= ARRAY_SIZE(syscallent) || !syscallent[i].sys_name) {
			fprintf(stderr, "warning: \"%s\" syscall #%u"
				" is missing in syscallent.h\n",
				ksyslist[i], i);
			continue;
		}
#ifdef SYS_socket_nsubcalls
		if (i >= SYS_socket_subcall &&
		    i < SYS_socket_subcall + SYS_socket_nsubcalls) {
			fprintf(stderr, "error: \"%s\" syscall #%u"
				" is a socket subcall in syscallent.h\n",
				ksyslist[i], i);
			rc = 1;
			continue;
		}
#endif
#ifdef SYS_ipc_nsubcalls
		if (i >= SYS_ipc_subcall &&
		    i < SYS_ipc_subcall + SYS_ipc_nsubcalls) {
			fprintf(stderr, "error: \"%s\" syscall #%u"
				" is an ipc subcall in syscallent.h\n",
				ksyslist[i], i);
			rc = 1;
			continue;
		}
#endif
		if (strcmp(ksyslist[i], syscallent[i].sys_name)) {
			fprintf(stderr, "error: \"%s\" syscall #%u"
				" is \"%s\" in syscallent.h\n",
				ksyslist[i], i, syscallent[i].sys_name);
			rc = 1;
			continue;
		}
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(syscallent); ++i) {
		if (!syscallent[i].sys_name
#ifdef SYS_socket_nsubcalls
		    || (i >= SYS_socket_subcall &&
			i < SYS_socket_subcall + SYS_socket_nsubcalls)
#endif
#ifdef SYS_ipc_nsubcalls
		    || (i >= SYS_ipc_subcall &&
			i < SYS_ipc_subcall + SYS_ipc_nsubcalls)
#endif
#ifdef ARM_FIRST_SHUFFLED_SYSCALL
		    || (i >= ARM_FIRST_SHUFFLED_SYSCALL &&
			i <= ARM_FIRST_SHUFFLED_SYSCALL +
			    ARM_LAST_SPECIAL_SYSCALL + 1)
#endif
		   )
			continue;
		if (i >= ARRAY_SIZE(ksyslist) || !ksyslist[i]) {
			fprintf(stderr, "note: unknown syscall #%u"
				" is \"%s\" in syscallent.h\n",
				i, syscallent[i].sys_name);
		}
	}

	return rc;
}
