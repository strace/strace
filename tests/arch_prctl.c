/*
 * Check decoding of arch_prctl syscall.
 *
 * Copyright (c) 2021-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_arch_prctl

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <linux/prctl.h>

# define XLAT_MACROS_ONLY
#  include "xlat/archvals.h"
# undef XLAT_MACROS_ONLY

# include "xlat.h"
# include "xlat/x86_xfeature_bits.h"
# include "xlat/x86_xfeatures.h"

# ifdef INJECT_RETVAL
#  define INJ_STR " (INJECTED)\n"
# else
#  define INJ_STR "\n"
# endif

# define ARRAY_END(a_) ((a_) + ARRAY_SIZE(a_))

static long
sys_arch_prctl(unsigned int cmd, kernel_ulong_t arg)
{
	return syscall(__NR_arch_prctl, cmd, arg, (unsigned long) -3U,
						  (unsigned long) -4U,
						  (unsigned long) -5U);
}

static long
arch_prctl_marker(void)
{
	return sys_arch_prctl(-1U, (unsigned long) -2U);
}

int
main(int argc, char *argv[])
{
	const kernel_ulong_t dummy = (kernel_ulong_t) 0xbadfaceddeadbeefULL;
	const char *errstr;
	long rc;

	arch_prctl_marker();

# ifdef INJECT_RETVAL
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if (arch_prctl_marker() != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on arch_prctl(-1, -2)"
				   " returning %ld", inject_retval);
# endif /* INJECT_RETVAL */

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, u64_p);

	/* Unknown commands */
	static const uint32_t unk_cmds[] = {
		0, 0x1,
		0x1000, 0x1005,
		0x1010, 0x1013,
		0x1020, 0x1026,
		0x1030, 0x1031,
		0x1100, 0x1101,
		0x2000, 0x2004,
		0x2010, 0x2011,
		0x2100, 0x2101,
		0x3000, 0x3001,
		0xdeadc0de };

	for (size_t i = 0; i < ARRAY_SIZE(unk_cmds); i++) {
		rc = sys_arch_prctl(unk_cmds[i], 0);
		printf("arch_prctl(" XLAT_UNKNOWN_FMT("%#x", "ARCH_???")
		       ", 0) = %s" INJ_STR,
		       unk_cmds[i], sprintrc(rc));

		rc = sys_arch_prctl(unk_cmds[i], (kernel_ulong_t) dummy);
		printf("arch_prctl(" XLAT_UNKNOWN_FMT("%#x", "ARCH_???")
		       ", %#llx) = %s" INJ_STR,
		       unk_cmds[i], (unsigned long long) dummy, sprintrc(rc));

		rc = sys_arch_prctl(unk_cmds[i], (uintptr_t) u64_p);
		printf("arch_prctl(" XLAT_UNKNOWN_FMT("%#x", "ARCH_???")
		       ", %p) = %s" INJ_STR,
		       unk_cmds[i], u64_p, sprintrc(rc));
	}

	/* Default decoding */
	static const struct strval32 def_cmds[] = {
		{ ARG_XLAT_KNOWN(0x1001, "ARCH_SET_GS") },
# ifdef INJECT_RETVAL
		{ ARG_XLAT_KNOWN(0x1002, "ARCH_SET_FS") },
# endif
		{ ARG_XLAT_KNOWN(0x1012, "ARCH_SET_CPUID") },
		{ ARG_XLAT_KNOWN(0x2001, "ARCH_MAP_VDSO_X32") },
		{ ARG_XLAT_KNOWN(0x2002, "ARCH_MAP_VDSO_32") },
		{ ARG_XLAT_KNOWN(0x2003, "ARCH_MAP_VDSO_64") },
	};

	for (const struct strval32 *p = def_cmds; p < ARRAY_END(def_cmds); p++)
	{
		rc = sys_arch_prctl(p->val, (kernel_ulong_t) dummy);
		printf("arch_prctl(%s, %#llx) = %s" INJ_STR,
		       p->str, (unsigned long long) dummy, sprintrc(rc));

		rc = sys_arch_prctl(p->val, (uintptr_t) u64_p);
		printf("arch_prctl(%s, %p) = %s" INJ_STR,
		       p->str, u64_p, sprintrc(rc));

		rc = sys_arch_prctl(p->val, 0);
		printf("arch_prctl(%s, 0) = %s" INJ_STR, p->str, sprintrc(rc));
	}

	/* ARCH_GET_GS, ARCH_GET_FS */
	static const struct strval32 kptr_cmds[] = {
# ifdef INJECT_RETVAL
		{ ARG_XLAT_KNOWN(0x1003, "ARCH_GET_FS") },
# endif
		{ ARG_XLAT_KNOWN(0x1004, "ARCH_GET_GS") },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, kulong_p);
	const kernel_ulong_t ptrs[] = {
		(kernel_ulong_t) 0xdeadfacecafebeefULL,
		(uintptr_t) (kulong_p + 1),
		(uintptr_t) kulong_p ,
		0
	};

	for (const struct strval32 *p = kptr_cmds; p < ARRAY_END(kptr_cmds);
	     p++) {
		rc = sys_arch_prctl(p->val, 0);
		printf("arch_prctl(%s, NULL) = %s" INJ_STR,
		       p->str, sprintrc(rc));

		rc = sys_arch_prctl(p->val, (uintptr_t) (kulong_p + 1));
		printf("arch_prctl(%s, %p) = %s" INJ_STR,
		       p->str, kulong_p + 1, sprintrc(rc));

		for (size_t j = 0; j < ARRAY_SIZE(ptrs); j++) {
			*kulong_p = ptrs[j];
			uint32_t wr_cmd = p->val == ARCH_GET_GS ? ARCH_SET_GS
								: ARCH_SET_FS;
# if !XLAT_RAW
			const char *wr_str = p->val == ARCH_GET_GS
						? "ARCH_SET_GS" : "ARCH_SET_FS";
# endif
			rc = sys_arch_prctl(wr_cmd, *kulong_p);
			printf("arch_prctl(" XLAT_FMT ", %#llx) = %s" INJ_STR,
			       XLAT_SEL(wr_cmd, wr_str),
			       (unsigned long long) *kulong_p, sprintrc(rc));

			rc = sys_arch_prctl(p->val, (uintptr_t) kulong_p);
			errstr = sprintrc(rc);
			printf("arch_prctl(%s, ", p->str);
			if (rc >= 0) {
				if (*kulong_p) {
					printf("[%#llx]",
					       (unsigned long long) *kulong_p);
				} else {
					printf("[NULL]");
				}
			} else {
				printf("%p", kulong_p);
			}
			printf(") = %s" INJ_STR, errstr);
		}
	}

	/* ARCH_GET_CPUID */
	rc = sys_arch_prctl(ARCH_GET_CPUID, 0xdeadc0de);
	printf("arch_prctl(" XLAT_FMT ") = %s" INJ_STR,
	       XLAT_ARGS(ARCH_GET_CPUID), sprintrc(rc));

	/* xfeature mask get */
	static const struct strval32 xfget_cmds[] = {
		{ ARG_XLAT_KNOWN(0x1021, "ARCH_GET_XCOMP_SUPP") },
		{ ARG_XLAT_KNOWN(0x1022, "ARCH_GET_XCOMP_PERM") },
		{ ARG_XLAT_KNOWN(0x1024, "ARCH_GET_XCOMP_GUEST_PERM") },
	};
	static const struct strval64 xfget_vals[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_UNKNOWN(0x1, "XFEATURE_MASK_FP") },
		{ ARG_XLAT_UNKNOWN(0x2, "XFEATURE_MASK_SSE") },
		{ ARG_XLAT_UNKNOWN(0x3, "XFEATURE_MASK_FPSSE") },
		{ ARG_XLAT_UNKNOWN(0x20, "XFEATURE_MASK_OPMASK") },
		{ ARG_XLAT_UNKNOWN(0xc0, "XFEATURE_MASK_ZMM_Hi256"
					 "|XFEATURE_MASK_Hi16_ZMM") },
		{ ARG_XLAT_UNKNOWN(0xe0, "XFEATURE_MASK_AVX512") },
		{ ARG_XLAT_UNKNOWN(0x20000, "XFEATURE_MASK_XTILE_CFG") },
		{ ARG_XLAT_UNKNOWN(0x40000, "XFEATURE_MASK_XTILE_DATA") },
		{ ARG_XLAT_UNKNOWN(0x60000, "XFEATURE_MASK_XTILE") },
		{ ARG_XLAT_UNKNOWN(0xbadfaced,
				   "XFEATURE_MASK_FP|XFEATURE_MASK_YMM"
				   "|XFEATURE_MASK_BNDREGS|XFEATURE_MASK_AVX512"
				   "|XFEATURE_MASK_PASID|XFEATURE_MASK_LBR"
				   "|XFEATURE_MASK_XTILE|0xbad92800") },
		{ ARG_XLAT_UNKNOWN(0x687ff,
				   "XFEATURE_MASK_FPSSE|XFEATURE_MASK_YMM"
				   "|XFEATURE_MASK_BNDREGS|XFEATURE_MASK_BNDCSR"
				   "|XFEATURE_MASK_AVX512|XFEATURE_MASK_PT"
				   "|XFEATURE_MASK_PKRU|XFEATURE_MASK_PASID"
				   "|XFEATURE_MASK_LBR|XFEATURE_MASK_XTILE") },
		{ ARG_XLAT_UNKNOWN(0xfffffffffff97800, "XFEATURE_MASK_???") },
	};

	for (const struct strval32 *p = xfget_cmds; p < ARRAY_END(xfget_cmds);
	     p++) {
		rc = sys_arch_prctl(p->val, 0);
		printf("arch_prctl(%s, NULL) = %s" INJ_STR,
		       p->str, sprintrc(rc));

		rc = sys_arch_prctl(p->val, (uintptr_t) (u64_p + 1));
		printf("arch_prctl(%s, %p) = %s" INJ_STR,
		       p->str, u64_p + 1, sprintrc(rc));

		for (const struct strval64 *q = xfget_vals;
		     q < ARRAY_END(xfget_vals); q++) {
			*u64_p = q->val;
			rc = sys_arch_prctl(p->val, (uintptr_t) u64_p);
			errstr = sprintrc(rc);
			printf("arch_prctl(%s, ", p->str);
			if (rc >= 0) {
# ifdef INJECT_RETVAL
				printf("[%s]", q->str);
# else
				if (*u64_p) {
					printf("[%#llx" NRAW(" /* "),
					       (unsigned long long) *u64_p);
#  if !XLAT_RAW
					printflags(x86_xfeatures, *u64_p, NULL);
#  endif
					printf(NRAW(" */") "]");
				} else {
					printf("[0]");
				}
# endif
			} else {
				printf("%p", u64_p);
			}
			printf(") = %s" INJ_STR, errstr);
		}
	}

	/* xfeature in arg, xfeature mask in ret */
	static const struct strval32 xfreq_cmds[] = {
		{ ARG_XLAT_KNOWN(0x1023, "ARCH_REQ_XCOMP_PERM") },
		{ ARG_XLAT_KNOWN(0x1025, "ARCH_REQ_XCOMP_GUEST_PERM") },
	};
	static const struct strval32 xfreq_vals[] = {
		{ ARG_XLAT_UNKNOWN(0, "XFEATURE_FP") },
		{ ARG_XLAT_UNKNOWN(0x8, "XFEATURE_PT_UNIMPLEMENTED_SO_FAR") },
		{ ARG_XLAT_UNKNOWN(0xb, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0xc, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0xd, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0xe, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0xf, "XFEATURE_LBR") },
		{ ARG_XLAT_UNKNOWN(0x10, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0x11, "XFEATURE_XTILE_CFG") },
		{ ARG_XLAT_UNKNOWN(0x12, "XFEATURE_XTILE_DATA") },
		{ ARG_XLAT_UNKNOWN(0x13, "XFEATURE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadface, "XFEATURE_???") },
	};

	for (const struct strval32 *p = xfreq_cmds; p < ARRAY_END(xfreq_cmds);
	     p++) {
		for (const struct strval32 *q = xfreq_vals;
		     q < ARRAY_END(xfreq_vals); q++) {
			rc = sys_arch_prctl(p->val, q->val);
			errstr = sprintrc(rc);
			printf("arch_prctl(%s, %s) = ", p->str, q->str);
			if (rc > 0) {
				printf("%#lx", rc);
# if !XLAT_RAW
				if (rc & x86_xfeatures->flags_mask) {
					printf(" (");
					printflags(x86_xfeatures, rc, NULL);
					printf(")");
				}
# endif
				printf(INJ_STR);
			} else {
				printf("%s" INJ_STR, errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_arch_prctl")

#endif
