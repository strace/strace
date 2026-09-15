/*
 * Check decoding of riscv_hwprobe syscall.
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"


#include "scno.h"

#ifdef __NR_riscv_hwprobe

# include <stdint.h>
# include <stdio.h>
# include <unistd.h>
# include <sched.h>
# include <sys/sysinfo.h>

struct riscv_hwprobe_case {
	const char *key_str;
	struct riscv_hwprobe_pair {
		kernel_long_t key;
		kernel_ulong_t value;
	} pair;
	kernel_ulong_t flags;
};

#define resume_prefix "<... riscv_hwprobe resumed>"

static void print_rc(long rc)
{
	printf(" = %s\n", sprintrc(rc));
}

static void run_case(struct riscv_hwprobe_case *tcase)
{
	struct riscv_hwprobe_pair pair = tcase->pair;

	/* test 1: query with an invalid flag */
	pair = tcase->pair;
	long rc = syscall(__NR_riscv_hwprobe,
			  &pair, 1,
			  0, NULL,
			  0x4b4b4b40);
	printf("riscv_hwprobe([{key=%s}], 1, 0, NULL, %s <unfinished ...>\n",
	       tcase->key_str, "0x4b4b4b40 /* RISCV_HWPROBE_??? */");
	printf(resume_prefix "%p, 1, 0, NULL, %s)",
	       &pair, "0x4b4b4b40 /* RISCV_HWPROBE_??? */");
	print_rc(rc);

	/* test 2: query a key on all CPU cores */
	rc = syscall(__NR_riscv_hwprobe,
		     &pair, 1,
		     0, NULL,
		     0);

	printf("riscv_hwprobe([{key=%s}], 1, 0, NULL, 0 <unfinished ...>\n",
	       tcase->key_str);

	/*
	 * kernel sets key to -1 when
	 *  - the key is invalid or unimplemented yet
	 *  - there's no unique result applying for all CPU cores
	 */
	if (pair.key == -1)
		printf(resume_prefix "[{key=-1}], 1, 0, NULL, 0)");
	else
		printf(resume_prefix "[{key=%s, value=0x%lx}], 1, 0, NULL, 0)",
		       tcase->key_str, pair.value);
	print_rc(rc);

	/*
	 * test 3: query CPUs satisifying the pair
	 * We only run it when query in test 2 succeeds, which ensures all CPUs
	 * satisfy the pair provided in tcase
	 */
	if (pair.key == -1)
		return;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);

	rc = syscall(__NR_riscv_hwprobe,
		     &pair, 1,
		     CPU_SETSIZE, &cpuset,
		     0x1);	// RISCV_HWPROBE_WHICH_CPUS

	printf("riscv_hwprobe([{key=%s, value=0x%lx}], 1, %d, [...], "
	       "RISCV_HWPROBE_WHICH_CPUS <unfinished ...>\n",
	       tcase->key_str, pair.value, CPU_SETSIZE);

	printf(resume_prefix "[{key=%s, value=0x%lx}], 1, %d, [",
	       tcase->key_str, pair.value, CPU_SETSIZE);

	int cpucount = CPU_COUNT(&cpuset);
	for (int i = 0; i < CPU_SETSIZE && cpucount; i++) {
		if (CPU_ISSET(i, &cpuset)) {
			cpucount--;
			printf("%d ", i);
		}
	}

	printf("...], RISCV_HWPROBE_WHICH_CPUS)");
	print_rc(rc);
}

int main(void)
{
	struct riscv_hwprobe_case cases[] = {
		{ "RISCV_HWPROBE_KEY_MVENDORID", { 0 } },
		{ "RISCV_HWPROBE_KEY_MARCHID", { 1 } },
		{ "RISCV_HWPROBE_KEY_MIMPID", { 2 } },
		{ "RISCV_HWPROBE_KEY_HIGHEST_VIRT_ADDRESS", { 7 } },
		{ "0x5a5a5a5a /* RISCV_HWPROBE_KEY_??? */", { 0x5a5a5a5a } },
	};

	for (size_t i = 0; i < ARRAY_SIZE(cases); i++)
		run_case(cases + i);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_riscv_hwprobe");

#endif
