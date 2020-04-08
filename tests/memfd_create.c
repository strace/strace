/*
 * Check decoding of memfd_create syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_memfd_create

# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

# ifdef HAVE_LINUX_MEMFD_H
#  include <linux/memfd.h>
# endif

# ifndef MFD_HUGE_SHIFT
#  define MFD_HUGE_SHIFT 26
# endif

# ifndef MFD_HUGE_MASK
#  define MFD_HUGE_MASK 0x3f
# endif

static const char *errstr;

static long
k_memfd_create(const kernel_ulong_t name, const kernel_ulong_t flags)
{
	const long rc = syscall(__NR_memfd_create, name, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	const size_t size = 255 - (sizeof("memfd:") - 1) + 1;
	char *pattern = tail_alloc(size);
	fill_memory_ex(pattern, size, '0', 10);

	k_memfd_create((uintptr_t) pattern, 0);
	printf("memfd_create(\"%.*s\"..., 0) = %s\n",
	       (int) size - 1, pattern, errstr);

	kernel_ulong_t flags = (kernel_ulong_t) 0xfacefeed00000007ULL;
# define flags1_str "MFD_CLOEXEC|MFD_ALLOW_SEALING|MFD_HUGETLB"

	k_memfd_create((uintptr_t) pattern, flags);
# if XLAT_VERBOSE
	printf("memfd_create(\"%.*s\"..., %s /* %s */) = %s\n",
	       (int) size - 1, pattern,
	       "0x7", flags1_str, errstr);
# else
	printf("memfd_create(\"%.*s\"..., %s) = %s\n",
	       (int) size - 1, pattern,
#  if XLAT_RAW
	       "0x7",
#  else
	       flags1_str,
#  endif
	       errstr);
# endif

	pattern[size - 1] = '\0';
	flags = 30 << MFD_HUGE_SHIFT;
	k_memfd_create((uintptr_t) pattern, flags);
# if XLAT_RAW
	printf("memfd_create(\"%s\", %#x) = %s\n",
	       pattern, (unsigned int) flags, errstr);
# elif XLAT_VERBOSE
	printf("memfd_create(\"%s\", %#x /* %s */) = %s\n",
	       pattern, (unsigned int) flags, "30<<MFD_HUGE_SHIFT", errstr);
# else /* XLAT_ABBREV */
	printf("memfd_create(\"%s\", 30<<MFD_HUGE_SHIFT) = %s\n",
	       pattern, errstr);
# endif

	pattern += size - 1;
	flags = (kernel_ulong_t) -1ULL;
	k_memfd_create(0, flags);
	flags = -1U & ~(7 | (MFD_HUGE_MASK << MFD_HUGE_SHIFT));

# define memfd_create_fmt "%s|%#x|%u<<MFD_HUGE_SHIFT"

	printf("memfd_create(NULL, "
# if XLAT_RAW
	       "0xffffffff) = %s\n",
# else
#  if XLAT_VERBOSE
	       "0xffffffff /* " memfd_create_fmt " */"
#  else /* XLAT_ABBREV */
	       memfd_create_fmt
#  endif
	       ") = %s\n",
	       flags1_str, (unsigned int) flags, MFD_HUGE_MASK,
# endif
	       errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_memfd_create")

#endif
