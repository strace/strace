/*
 * Check decoding of modify_ldt syscall.
 *
 * Copyright (c) 2018-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined __NR_modify_ldt && defined HAVE_STRUCT_USER_DESC

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

# include "print_user_desc.c"

void
printrc(long rc)
{
# ifdef __x86_64__
	/*
	 * Hopefully, we don't expect EPERM to be returned,
	 * otherwise we can't distinguish it on x32.
	 */
	if (rc != -1) {
		int err = -rc;

		/* Thanks, long return type of syscall(2) */
		printf("%lld", zero_extend_signed_to_ull(rc));

		if (err > 0 && err < 0x1000) {
			errno = err;
			printf(" %s (%m)", errno2name());
		}
	}
	else
# endif
	{
		printf("%s", sprintrc(rc));
	}

	puts("");
}

int
main(void)
{
	static const kernel_ulong_t bogus_func =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;
	static const kernel_ulong_t bogus_bytecount =
		(kernel_ulong_t) 0xdeadfacefa57beefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct user_desc, us);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, bogus_int);
	long rc;

	fill_memory(us, sizeof(*us));

	rc = syscall(__NR_modify_ldt, 0, 0, 0);
	printf("modify_ldt(0, NULL, 0) = ");
	printrc(rc);

	rc = syscall(__NR_modify_ldt, bogus_func, (kernel_long_t) -1,
		     bogus_bytecount);
	printf("modify_ldt(%d, %#llx, %llu) = ",
	       (int) bogus_func,
	       zero_extend_signed_to_ull((kernel_long_t) -1),
	       (unsigned long long) bogus_bytecount);
	printrc(rc);

	rc = syscall(__NR_modify_ldt, bogus_func, us + 1, 0);
	printf("modify_ldt(%d, %p, 0) = ", (int) bogus_func, us + 1);
	printrc(rc);

	rc = syscall(__NR_modify_ldt, bogus_func, us, 42);
	printf("modify_ldt(%d, %p, 42) = ", (int) bogus_func, us);
	printrc(rc);

	rc = syscall(__NR_modify_ldt, bogus_func, us + 1, sizeof(*us));
	printf("modify_ldt(%d, %p, %zu) = ",
	       (int) bogus_func, us + 1, sizeof(*us));
	printrc(rc);

	/*
	 * print_user_desc handles entry_number field in a special way for
	 * get_thread_area syscall, so let's also check here that we don't
	 * retrieve it accidentally.
	 */
	rc = syscall(__NR_modify_ldt, bogus_func, bogus_int, sizeof(*us));
	printf("modify_ldt(%d, %p, %zu) = ",
	       (int) bogus_func, bogus_int, sizeof(*us));
	printrc(rc);

	rc = syscall(__NR_modify_ldt, bogus_func, us, sizeof(*us));
	printf("modify_ldt(%d, ", (int) bogus_func);
	print_user_desc(us, NULL);
	printf(", %zu) = ", sizeof(*us));
	printrc(rc);

	fill_memory_ex(us, sizeof(*us), 0x55, 80);
	us->entry_number = -1;
	us->base_addr = 0;
	us->limit = 0;

	rc = syscall(__NR_modify_ldt, bogus_func, us, sizeof(*us));
	printf("modify_ldt(%d, ", (int) bogus_func);
	print_user_desc(us, "-1");
	printf(", %zu) = ", sizeof(*us));
	printrc(rc);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_modify_ldt && HAVE_STRUCT_USER_DESC");

#endif
