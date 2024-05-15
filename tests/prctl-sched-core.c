/*
 * Check decoding of prctl PR_SCHED_CORE operation.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>

#include "pidns.h"

struct op_str {
	unsigned int op;
	const char *str;
};

#ifdef INJECT_RETVAL
# define NUM_SKIP 256
# define INJ_STR " (INJECTED)"
#else
# define NUM_SKIP 1
# define INJ_STR ""
#endif

int
main(int argc, char *argv[])
{
	PIDNS_TEST_INIT;

	long rc;

	unsigned long num_skip = NUM_SKIP;

	if (argc >= 2)
		num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		rc = prctl_marker();
#ifdef PIDNS_TRANSLATION
		const char *errstr = sprintrc(rc);
		pidns_print_leader();
		printf("prctl(" XLAT_UNKNOWN(0xffffffff, "PR_???")
		       ", 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb) = ");

		if (rc < 0) {
			puts(errstr);
		} else {
			printf("%ld (INJECTED)\n", rc);
		}
#endif

		if (rc < 0)
			continue;

		break;
	}

	static const struct {
		unsigned int decode_ptr:1,
			     valid:1;
		unsigned int op;
		const char *str;
	} ops[] = {
		{ true,  true,  ARG_STR(PR_SCHED_CORE_GET) },
		{ false, true,  ARG_STR(PR_SCHED_CORE_CREATE) },
		{ false, true,  ARG_STR(PR_SCHED_CORE_SHARE_TO) },
		{ false, true,  ARG_STR(PR_SCHED_CORE_SHARE_FROM) },
		{ false, false, 4, "PR_SCHED_CORE_???" },
	};
	static const struct {
		unsigned int val;
		const char *str;
	} pidtypes[] = {
		{ 0, "PIDTYPE_PID" },
		{ 1, "PIDTYPE_TGID" },
		{ 2, "PIDTYPE_PGID" },
		{ 3, "PIDTYPE_SID" },
		{ 4, "PIDTYPE_???" },
		{ -1U, "PIDTYPE_???" },
	};

	int ids[5];
	ids[0] = syscall(__NR_gettid);
	ids[1] = getpid();
	ids[2] = getpgid(0);
	ids[3] = getsid(0);
	ids[4] = -1;

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, uptr);
	uint64_t *uptrs[] = { NULL, uptr + 1, uptr };

	for (size_t i = 0; i < ARRAY_SIZE(ops); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(pidtypes); j++) {
			for (size_t k = 0; k < ARRAY_SIZE(uptrs); k++) {
				*uptr = 0xdeadc0debadc0dedULL;
				rc = syscall(__NR_prctl, PR_SCHED_CORE,
						  ops[i].op | F8ILL_KULONG_MASK,
						  ids[MIN(pidtypes[j].val, 4)]
							| F8ILL_KULONG_MASK,
						  pidtypes[j].val
							| F8ILL_KULONG_MASK,
						  uptrs[k]);
				const char *errstr = sprintrc(rc);

				pidns_print_leader();
				printf("prctl("
				       XLAT_KNOWN(0x3e, "PR_SCHED_CORE") ", ");
				if (ops[i].valid) {
					printf(XLAT_FMT,
					       XLAT_SEL(ops[i].op, ops[i].str));
				} else {
					printf("%#x"
					       NRAW(" /* PR_SCHED_CORE_??? */"),
					       ops[i].op);
				}
				printf(", %d%s, %#x" NRAW(" /* %s */") ", ",
				       ids[MIN(pidtypes[j].val, 4)],
				       pidns_pid2str(pidtypes[j].val),
				       pidtypes[j].val
#if !XLAT_RAW
				       , pidtypes[j].str
#endif
				       );

				if (uptrs[k]) {
					if (uptrs[k] == uptr
					    && ops[i].decode_ptr && rc >= 0)
#ifdef INJECT_RETVAL
						printf("[0xdeadc0debadc0ded]");
#else
						printf("[%#" PRIx64 "]", *uptr);
#endif
					else
						printf("%p", uptrs[k]);
				} else {
					printf("NULL");
				}

				printf(") = %s" INJ_STR "\n", errstr);
			}
		}
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
