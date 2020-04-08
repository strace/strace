/*
 * Check decoding of s390_guarded_storage syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_s390_guarded_storage && defined HAVE_ASM_GUARDED_STORAGE_H

# include <inttypes.h>
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>

# include <asm/guarded_storage.h>

# ifndef VERBOSE
#  define VERBOSE 0
# endif

static void
gs_no_arg(kernel_ulong_t val, const char *val_str)
{
	static const kernel_ulong_t bogus_addr =
		(kernel_ulong_t) 0xcaffeedadeadbed5ULL;
	static const kernel_ulong_t bogus_cmd_mask =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	long rc;

	rc = syscall(__NR_s390_guarded_storage, val | bogus_cmd_mask,
		     bogus_addr);
	printf("s390_guarded_storage(%s) = %s\n", val_str, sprintrc(rc));
}

static void
gs_print_epl(uint64_t addr, bool valid, const char *str)
{
	if (!valid) {
		if (str)
			printf("%s", str);
		else
			printf("%#" PRIx64, addr);

		return;
	}

	struct gs_epl *gsepl = (struct gs_epl *) (uintptr_t) addr;

	printf("[{");

# if VERBOSE
	if (gsepl->pad1)
		printf("pad1=%#02x, ", gsepl->pad1);

	printf("gs_eam=%#02x /* extended addressing mode: %u, "
	       "basic addressing mode: %u */"
	       ", gs_eci=%#02x /* CPU in TX: %u, CPU in CX: %u, "
	       "instruction: %s */"
	       ", gs_eai=%#02x /* DAT: %u, address space indication: %u, "
	       "AR number: %u */, ",
	       gsepl->gs_eam, gsepl->e, gsepl->b,
	       gsepl->gs_eci, gsepl->tx, gsepl->cx,
	       gsepl->in ? "LLGFGS": "LGG",
	       gsepl->gs_eai, gsepl->t, gsepl->as, gsepl->ar);

	if (gsepl->pad2)
		printf("pad2=%#08x, ", gsepl->pad2);
# endif /* VERBOSE */

	printf("gs_eha=%#llx, ", (unsigned long long) gsepl->gs_eha);

# if VERBOSE
	printf("gs_eia=%#llx, gs_eoa=%#llx, gs_eir=%#llx, gs_era=%#llx",
	       (unsigned long long) gsepl->gs_eia,
	       (unsigned long long) gsepl->gs_eoa,
	       (unsigned long long) gsepl->gs_eir,
	       (unsigned long long) gsepl->gs_era);
# else /* !VERBOSE */
	printf("...");
# endif /* VERBOSE */

	printf("}]");
}

static void
gs_set_cb(kernel_ulong_t addr, bool valid, bool epl_valid,
	  const char *bc_str, const char *epl_str)
{
	static const kernel_ulong_t bogus_cmd_mask =
		(kernel_ulong_t) 0xda7a105700000000ULL;

	long rc;

	printf("s390_guarded_storage(GS_SET_BC_CB, ");

	if (valid) {
		struct gs_cb *gscb = (struct gs_cb *) (uintptr_t) addr;

		printf("{");

		if (gscb->reserved)
			printf("reserved=%#016llx, ",
			       (unsigned long long) gscb->reserved);

		printf("gsd=%#16llx",
		       (unsigned long long) gscb->gsd);
# if VERBOSE
		printf(" /* GS origin: ");

		unsigned int gsc = gscb->gsd & 0x3F;
		unsigned int gls = (gscb->gsd >> 8) & 7;
		bool gsc_valid = gsc >= 25 && gsc <= 56;

		if (gsc_valid) {
			uint64_t gls = gscb->gsd >> gsc;
			int field_size = 2 + (67 - gsc) / 4;

			printf("%#0*" PRIx64, field_size, gls);
		} else {
			printf("[invalid]");
		}

		printf(", guard load shift: %u, GS characteristic: %u */",
		       gls, gsc);
# endif /* VERBOSE */

		printf(", gssm=%#016llx, gs_epl_a=",
		       (unsigned long long) gscb->gssm);

		gs_print_epl(gscb->gs_epl_a, epl_valid, epl_str);

		printf("}");
	} else {
		if (bc_str)
			printf("%s", bc_str);
		else
			printf("%#llx", (unsigned long long) addr);
	}

	rc = syscall(__NR_s390_guarded_storage,
		     GS_SET_BC_CB | bogus_cmd_mask, addr);
	printf(") = %s\n", sprintrc(rc));
}

int
main(void)
{
	static const kernel_ulong_t bogus_cmd =
		(kernel_ulong_t) 0xdeafbeefdeadc0deULL;
	static const kernel_ulong_t bogus_addr =
		(kernel_ulong_t) 0xfacefeedac0ffeedULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct gs_cb, gscb);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct gs_epl, gsepl);

	long rc;

	rc = syscall(__NR_s390_guarded_storage, 5, 0);
	printf("s390_guarded_storage(0x5 /* GS_??? */, NULL) = %s\n",
	       sprintrc(rc));

	rc = syscall(__NR_s390_guarded_storage, bogus_cmd, bogus_addr);
	printf("s390_guarded_storage(%#x /* GS_??? */, %#lx) = %s\n",
	       (unsigned) bogus_cmd, (unsigned long) bogus_addr, sprintrc(rc));

	gs_no_arg(ARG_STR(GS_BROADCAST));
	gs_no_arg(ARG_STR(GS_CLEAR_BC_CB));
	gs_no_arg(ARG_STR(GS_DISABLE));
	gs_no_arg(ARG_STR(GS_ENABLE));

	fill_memory(gscb, sizeof(*gscb));
	fill_memory_ex(gsepl, sizeof(*gsepl), 0xA5, 0x5A);

	gs_set_cb(0, false, false, "NULL", NULL);
	gs_set_cb((uintptr_t) (gscb + 1), false, false, NULL, NULL);

	gscb->gs_epl_a = 0;
	gs_set_cb((uintptr_t) gscb, true, false, NULL, "NULL");

	fill_memory_ex(gscb, sizeof(*gscb), 0x5A, 0xA5);
	gscb->gs_epl_a = (uintptr_t) (gsepl + 1) |
		 (sizeof(kernel_ulong_t) < sizeof(uint64_t) ?
			0xc0debad000000000ULL : 0);
	gs_set_cb((uintptr_t) gscb, true, false, NULL, NULL);

	fill_memory_ex(gscb, sizeof(*gscb), 0xA7, 0xA5);
	gscb->gs_epl_a = (uintptr_t) gsepl;
	gs_set_cb((uintptr_t) gscb, true, true, NULL, NULL);

	fill_memory_ex(gscb, sizeof(*gscb), 0x55, 0xAA);
	fill_memory_ex(gsepl, sizeof(*gsepl), 0x5A, 0xA5);
	gscb->gs_epl_a = (uintptr_t) gsepl;
	gs_set_cb((uintptr_t) gscb, true, true, NULL, NULL);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_s390_guarded_storage && HAVE_ASM_GUARDED_STORAGE_H")

#endif
