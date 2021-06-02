/*
 * Check decoding of remap_file_pages syscall.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/mman.h>

static const char *errstr;

static long
k_remap_file_pages(const kernel_ulong_t addr,
		   const kernel_ulong_t size,
		   const kernel_ulong_t prot,
		   const kernel_ulong_t pgoff,
		   const kernel_ulong_t flags)
{
	const long rc = syscall(__NR_remap_file_pages,
				addr, size, prot, pgoff, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	kernel_ulong_t addr = (kernel_ulong_t) 0xfacefeeddeadbeefULL;
	kernel_ulong_t size = (kernel_ulong_t) 0xdefaced1bad2f00dULL;
	kernel_ulong_t prot = PROT_READ|PROT_WRITE|PROT_EXEC;
	kernel_ulong_t pgoff = (kernel_ulong_t) 0xcaf3babebad4deedULL;
	kernel_ulong_t flags = MAP_PRIVATE|MAP_ANONYMOUS;
#define prot1_str "PROT_READ|PROT_WRITE|PROT_EXEC"
#define flags1_str "MAP_PRIVATE|MAP_ANONYMOUS"

	k_remap_file_pages(addr, size, prot, pgoff, flags);
#if XLAT_RAW
	printf("remap_file_pages(%#jx, %ju, %#jx, %ju, %#jx) = %s\n",
	       (uintmax_t) addr, (uintmax_t) size, (uintmax_t) prot,
	       (uintmax_t) pgoff, (uintmax_t) flags, errstr);
#elif XLAT_VERBOSE
	printf("remap_file_pages(%#jx, %ju, %#jx /* %s */, %ju, %#jx /* %s */)"
	       " = %s\n",
	       (uintmax_t) addr, (uintmax_t) size, (uintmax_t) prot, prot1_str,
	       (uintmax_t) pgoff, (uintmax_t) flags,
	       flags1_str, errstr);
#else /* XLAT_ABBREV */
	printf("remap_file_pages(%#jx, %ju, %s, %ju, %s) = %s\n",
	       (uintmax_t) addr, (uintmax_t) size, prot1_str,
	       (uintmax_t) pgoff, flags1_str, errstr);
#endif

#ifdef MAP_HUGETLB
# ifndef MAP_HUGE_2MB
#  ifndef MAP_HUGE_SHIFT
#   define MAP_HUGE_SHIFT 26
#  endif
#  define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
# endif /* !MAP_HUGE_2MB */
	addr = (kernel_ulong_t) 0xfacefeeddeadf00dULL;
	size = (kernel_ulong_t) 0xdefaced1bad2beefULL;
	prot = (kernel_ulong_t) 0xdefaced00000000ULL | PROT_NONE;
	flags = MAP_TYPE | MAP_FIXED | MAP_NORESERVE | MAP_HUGETLB | MAP_HUGE_2MB;

	k_remap_file_pages(addr, size, prot, pgoff, flags);

/*
 * HP PA-RISC is the only architecture that has MAP_TYPE defined to something
 * different.  For example, before commit v4.17-rc1~146^2~9 it was defined to
 * 0x3 which is also used for MAP_SHARED_VALIDATE since Linux commit
 * v4.15-rc1~71^2^2~23.
 */
# ifdef __hppa__
#  if MAP_TYPE == 0x03
#   define MAP_TYPE_str "MAP_SHARED_VALIDATE"
#  else
#   define MAP_TYPE_str "0x2b /* MAP_??? */"
#  endif
# else
#  define MAP_TYPE_str "0xf /* MAP_??? */"
# endif
# define flags2_str \
	MAP_TYPE_str "|MAP_FIXED|MAP_NORESERVE|MAP_HUGETLB|21<<MAP_HUGE_SHIFT"

# if XLAT_RAW
	printf("remap_file_pages(%#jx, %ju, %#jx, %ju, %#jx) = %s\n",
	       (uintmax_t) addr, (uintmax_t) size, (uintmax_t) prot,
	       (uintmax_t) pgoff, (uintmax_t) flags, errstr);
# elif XLAT_VERBOSE
	printf("remap_file_pages(%#jx, %ju, %#jx /* %s */, %ju, %#jx /* %s */)"
	       " = %s\n",
	       (uintmax_t) addr, (uintmax_t) size, (uintmax_t) prot,
	       prot == PROT_NONE ? "PROT_NONE" : "PROT_???",
	       (uintmax_t) pgoff, (uintmax_t) flags, flags2_str, errstr);
# else /* XLAT_ABBREV */
	printf("remap_file_pages(%#jx, %ju, %s, %ju, %s) = %s\n",
	       (uintmax_t) addr, (uintmax_t) size,
	       prot == PROT_NONE ? "PROT_NONE" :
				   "0xdefaced00000000 /* PROT_??? */",
	       (uintmax_t) pgoff, flags2_str, errstr);
# endif
#endif /* MAP_HUGETLB */

	puts("+++ exited with 0 +++");
	return 0;
}
