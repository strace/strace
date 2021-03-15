/*
 * Check decoding of "old mmap" edition of mmap syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

/*
 * On s390x and m68k, this is the mmap syscall used by glibc, so,
 * from one side, it's already covered by another test, and, from another side,
 * it would require additional efforts to filter out mmap calls made by glibc.
 */

#if defined __NR_mmap \
	&& (defined __arm__ || defined __i386__ || defined __m68k__ \
		|| defined __s390__ || defined __s390x__) \
	&& (defined PATH_TRACING || !(defined __s390x__ || defined __m68k__))

# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <sys/mman.h>
# include <unistd.h>
# include "xmalloc.h"

# ifndef TEST_FD
#  define TEST_FD -2LU
# endif

int
main(void)
{
	long rc = syscall(__NR_mmap, 0);
	const bool implemented = rc != -1 || errno != ENOSYS;
# ifndef PATH_TRACING
	printf("mmap(NULL) = %s\n", sprintrc(rc));
# endif

	const unsigned long args1_c[6] = {
		(unsigned long) 0xbadc0deddeadbeefULL,	/* addr */
		(unsigned long) 0xdeefacedfacefeedULL,	/* len */
		PROT_READ|PROT_EXEC,	/* prot */
		MAP_FILE|MAP_FIXED,	/* flags */
		TEST_FD,		/* fd */
		(unsigned long) 0xdecaffedbadc0dedULL	/* offset */
	};
	const unsigned long page_size = get_page_size();
	const unsigned long args2_c[6] = {
		0,
		page_size,
		PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS,
		-1LU,
		(unsigned long) 0xda7a1057faced000ULL & -page_size
	};
	void *args = tail_memdup(args1_c, sizeof(args1_c));

	rc = syscall(__NR_mmap, args);
# if XLAT_RAW
	printf("mmap(%#lx, %lu, %#x, %#x, %d, %#lx) = %s\n",
	       args1_c[0], args1_c[1], PROT_READ|PROT_EXEC, MAP_FILE | MAP_FIXED,
	       (int) args1_c[4], args1_c[5], sprintrc(rc));
# elif XLAT_VERBOSE
	printf("mmap(%#lx, %lu, %#x /* PROT_READ|PROT_EXEC */"
	       ", %#x /* MAP_FILE|MAP_FIXED */, %d, %#lx) = %s\n",
	       args1_c[0], args1_c[1], PROT_READ|PROT_EXEC, MAP_FILE | MAP_FIXED,
	       (int) args1_c[4], args1_c[5], sprintrc(rc));
# else
	printf("mmap(%#lx, %lu, PROT_READ|PROT_EXEC, MAP_FILE|MAP_FIXED"
	       ", %d, %#lx) = %s\n",
	       args1_c[0], args1_c[1], (int) args1_c[4], args1_c[5],
	       sprintrc(rc));
# endif

	memcpy(args, args2_c, sizeof(args2_c));
	rc = syscall(__NR_mmap, args);
# ifndef PATH_TRACING
	const char *errstr;
	if (implemented) {
		char *str = xasprintf("%#lx", rc);
		errstr = str;
	} else {
		errstr = sprintrc(rc);
	}
#  if XLAT_RAW
	printf("mmap(NULL, %lu, %#x, %#x, %d, %#lx) = %s\n",
	       args2_c[1], PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
	       (int) args2_c[4], args2_c[5], errstr);
#  elif XLAT_VERBOSE
	printf("mmap(NULL, %lu, %#x /* PROT_READ|PROT_WRITE */"
	       ", %#x /* MAP_PRIVATE|MAP_ANONYMOUS */, %d, %#lx) = %s\n",
	       args2_c[1], PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
	       (int) args2_c[4], args2_c[5], errstr);
#  else
	printf("mmap(NULL, %lu, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS"
	       ", %d, %#lx) = %s\n",
	       args2_c[1], (int) args2_c[4], args2_c[5], errstr);
#  endif
# endif

	void *addr = (void *) rc;
	if (implemented && mprotect(addr, page_size, PROT_NONE))
		perror_msg_and_fail("mprotect(%p, %lu, PROT_NONE)",
				    addr, page_size);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("defined __NR_mmap "
	"&& (defined __arm__ || defined __i386__ || defined __m68k__ "
		"|| defined __s390__ || defined __s390x__) "
	"&& (defined PATH_TRACING || !(defined __s390x__ || defined __m68k__))")

#endif
