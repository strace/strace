/*
 * Check decoding of ptrace PTRACE_SET_SYSCALL_INFO request.
 *
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "ptrace.h"
#include "scno.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/audit.h>

#include "cur_audit_arch.h"

#include "xlat.h"
#define XLAT_MACROS_ONLY
# include "xlat/elf_em.h"
# include "xlat/audit_arch.h"
#undef XLAT_MACROS_ONLY

static const char *errstr;
static void *end_of_page;
static pid_t pid;

static long
ptrace_set_syscall_info(void *addr, size_t size)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	long rc = syscall(__NR_ptrace, PTRACE_SET_SYSCALL_INFO,
			  pid, size, (uintptr_t) addr, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static const struct str_val_8 {
	uint8_t val;
	const char *str;
	bool valid;
} ops[] = {
	{ ARG_STR(PTRACE_SYSCALL_INFO_NONE), true },
	{ ARG_XLAT_UNKNOWN(0x4, "PTRACE_SYSCALL_INFO_???"), false },
};

static const struct str_val_32 {
	uint32_t val;
	const char *str;
	bool valid;
} arches[] = {
#ifdef CUR_AUDIT_ARCH
	{ CUR_AUDIT_ARCH, CUR_AUDIT_ARCH_STR, true },
#endif
	{ ARG_STR(AUDIT_ARCH_CRIS), true },
	{ ARG_XLAT_UNKNOWN(0x102, "AUDIT_ARCH_???"), false },
};

static const uint64_t nrs[] = {
	__NR_gettid,
	0xbadc0ded00000000 | __NR_gettid,
	0xdeadbeef,
	0xbadc0dedfacefeed,
};

static const struct str_exit {
	uint8_t is_error;
	int64_t rval;
	const char *str;
} exits[] = {
	{ -3, ARG_STR(-ENOENT) },
	{ -2, 0xfacefeeddeadbeef, NULL },
	{ 0, -ENOENT, NULL },
	{ 0, 0xfacefeeddeadbeef, NULL },
};

#define OOB(m) offsetof(struct_ptrace_syscall_info, m)
#define OOE(m) offsetofend(struct_ptrace_syscall_info, m)

static void
test_none(const unsigned int size,
	  const struct str_val_8 *op,
	  const struct str_val_32 *arch)
{
	struct_ptrace_syscall_info psi;
	fill_memory(&psi, sizeof(psi));
	psi.op = op->val;
	psi.arch = arch->val;

	void *p = end_of_page - size;
	memcpy(p, &psi, size);
	memset(&psi, 0, sizeof(psi));
	memcpy(&psi, p, size);

	ptrace_set_syscall_info(p, size);

	printf("ptrace(" XLAT_FMT ", %d, %u, ",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, size);
	printf("{op=");
	if (op->valid)
		printf(XLAT_FMT, XLAT_SEL(op->val, op->str));
	else
		printf("%s", op->str);
	if (size > OOB(reserved) && psi.reserved)
		printf(", reserved=%#hhx", psi.reserved);
	if (size > OOB(flags) && psi.flags)
		printf(", flags=%#hx", psi.flags);
	if (size >= OOE(arch)) {
		printf(", arch=");
		if (arch->valid)
			printf(XLAT_FMT, XLAT_SEL(arch->val, arch->str));
		else
			printf("%s", arch->str);
	}
	if (size >= OOE(instruction_pointer))
		printf(", instruction_pointer=%#jx",
		       (uintmax_t) psi.instruction_pointer);
	if (size >= OOE(stack_pointer))
		printf(", stack_pointer=%#jx",
		       (uintmax_t) psi.stack_pointer);
	printf("}) = %s\n", errstr);
}

static void
test_entry(const unsigned int size,
	   const struct str_val_32 *arch,
	   const uint64_t nr,
	   const uint8_t op_val,
	   const char *op_str)
{
	struct_ptrace_syscall_info psi;
	fill_memory(&psi, sizeof(psi));
	psi.op = op_val;
	psi.arch = arch->val;
	psi.entry.nr = nr;

	void *p = end_of_page - size;
	memcpy(p, &psi, size);
	memset(&psi, 0, sizeof(psi));
	memcpy(&psi, p, size);

	ptrace_set_syscall_info(p, size);

	printf("ptrace(" XLAT_FMT ", %d, %u, ",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, size);
	printf("{op=" XLAT_FMT, XLAT_SEL(op_val, op_str));
	if (size > OOB(reserved) && psi.reserved)
		printf(", reserved=%#hhx", psi.reserved);
	if (size > OOB(flags) && psi.flags)
		printf(", flags=%#hx", psi.flags);
	if (size >= OOE(arch)) {
		printf(", arch=");
		if (arch->valid)
			printf(XLAT_FMT, XLAT_SEL(arch->val, arch->str));
		else
			printf("%s", arch->str);
	}
	if (size >= OOE(instruction_pointer))
		printf(", instruction_pointer=%#jx",
		       (uintmax_t) psi.instruction_pointer);
	if (size >= OOE(stack_pointer))
		printf(", stack_pointer=%#jx",
		       (uintmax_t) psi.stack_pointer);
	if (size >= OOE(entry.nr)) {
		bool is_seccomp = op_val == PTRACE_SYSCALL_INFO_SECCOMP;
		printf(", %s={nr=", is_seccomp ? "seccomp" : "entry");
#ifdef CUR_AUDIT_ARCH
		if (arch->val == CUR_AUDIT_ARCH && nr == __NR_gettid)
			printf(XLAT_FMT_U, XLAT_ARGS(__NR_gettid));
		else
#endif
			printf("%ju", (uintmax_t) nr);
		unsigned int nargs =
			(size - OOB(entry.args)) / sizeof(psi.entry.args[0]);
		if (nargs) {
			nargs = MIN(ARRAY_SIZE(psi.entry.args), nargs);
			for (unsigned int i = 0; i < nargs; ++i)
				printf("%s%#jx", i ? ", " : ", args=[",
				       (uintmax_t) psi.entry.args[i]);
			printf("]");
		}
		if (is_seccomp && size >= OOE(seccomp.ret_data))
			printf(", ret_data=%u", psi.seccomp.ret_data);
		printf("}");
	}
	printf("}) = %s\n", errstr);
}

static void
test_exit(const unsigned int size,
	  const struct str_val_32 *arch,
	  const struct str_exit *ex)
{
	struct_ptrace_syscall_info psi;
	fill_memory(&psi, sizeof(psi));
	psi.op = PTRACE_SYSCALL_INFO_EXIT;
	psi.arch = arch->val;
	psi.exit.rval = ex->rval;
	psi.exit.is_error = ex->is_error;

	void *p = end_of_page - size;
	memcpy(p, &psi, size);
	memset(&psi, 0, sizeof(psi));
	memcpy(&psi, p, size);

	ptrace_set_syscall_info(p, size);

	printf("ptrace(" XLAT_FMT ", %d, %u, ",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, size);
	printf("{op=" XLAT_FMT, XLAT_ARGS(PTRACE_SYSCALL_INFO_EXIT));
	if (size > OOB(reserved) && psi.reserved)
		printf(", reserved=%#hhx", psi.reserved);
	if (size > OOB(flags) && psi.flags)
		printf(", flags=%#hx", psi.flags);
	if (size >= OOE(arch)) {
		printf(", arch=");
		if (arch->valid)
			printf(XLAT_FMT, XLAT_SEL(arch->val, arch->str));
		else
			printf("%s", arch->str);
	}
	if (size >= OOE(instruction_pointer))
		printf(", instruction_pointer=%#jx",
		       (uintmax_t) psi.instruction_pointer);
	if (size >= OOE(stack_pointer))
		printf(", stack_pointer=%#jx",
		       (uintmax_t) psi.stack_pointer);
	if (size >= OOE(exit.rval)) {
		printf(", exit={rval=");
		if (ex->str && psi.exit.is_error)
			printf(XLAT_FMT_JD,
			       XLAT_SEL((intmax_t) ex->rval, ex->str));
		else
			printf("%jd", (intmax_t) ex->rval);
		if (size >= OOE(exit.is_error))
			printf(", is_error=%u", psi.exit.is_error);
		printf("}");
	}
	printf("}) = %s\n", errstr);
}

int
main(void)
{
	end_of_page = tail_alloc(sizeof(struct_ptrace_syscall_info)) +
				 sizeof(struct_ptrace_syscall_info);
	pid = getpid();

	ptrace_set_syscall_info(NULL, 1);
	printf("ptrace(" XLAT_FMT ", %d, 1, NULL) = %s\n",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, errstr);

	ptrace_set_syscall_info(end_of_page, 1);
	printf("ptrace(" XLAT_FMT ", %d, 1, %p) = %s\n",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, end_of_page, errstr);

	ptrace_set_syscall_info(end_of_page - 1, 0);
	printf("ptrace(" XLAT_FMT ", %d, 0, %p) = %s\n",
	       XLAT_ARGS(PTRACE_SET_SYSCALL_INFO), pid, end_of_page - 1, errstr);

	for (unsigned int size = 1;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		for (unsigned int ops_i = 0;
		     ops_i < ARRAY_SIZE(ops); ++ops_i) {
			for (unsigned int arch_i = 0;
			     arch_i < ARRAY_SIZE(arches); ++arch_i) {
				if (arch_i && size < OOB(arch))
					break;
				test_none(size, &ops[ops_i], &arches[arch_i]);
			}
		}
	}

	for (unsigned int size = 1;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		for (unsigned int arch_i = 0;
		     arch_i < ARRAY_SIZE(arches); ++arch_i) {
			if (arch_i && size < OOB(arch))
				break;
			for (unsigned int nr_i = 0;
			     nr_i < ARRAY_SIZE(nrs); ++nr_i) {
				if (nr_i && size < OOB(entry))
					break;
				test_entry(size, &arches[arch_i], nrs[nr_i],
					   ARG_STR(PTRACE_SYSCALL_INFO_ENTRY));
				test_entry(size, &arches[arch_i], nrs[nr_i],
					   ARG_STR(PTRACE_SYSCALL_INFO_SECCOMP));
			}
		}
	}

	for (unsigned int size = 1;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		for (unsigned int arch_i = 0;
		     arch_i < ARRAY_SIZE(arches); ++arch_i) {
			if (arch_i && size < OOB(arch))
				break;
			for (unsigned int exit_i = 0;
			     exit_i < ARRAY_SIZE(exits); ++exit_i) {
				if (exit_i && size < OOB(exit))
					break;
				test_exit(size, &arches[arch_i], &exits[exit_i]);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
