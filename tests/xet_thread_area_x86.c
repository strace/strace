/*
 * Check decoding of set_thread_area and get_thread_area syscalls on x86
 * architecture.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined __NR_get_thread_area && defined __NR_set_thread_area \
 && defined HAVE_STRUCT_USER_DESC

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>

# include "print_user_desc.c"

long errnum;

static void
printptr(kernel_ulong_t ptr, const char *ptr_str)
{
	if (ptr_str)
		printf("%s", ptr_str);
	else
		printf("%#llx", zero_extend_signed_to_ull(ptr));
}

/**
 * Perform set_thread_area call along with printing the expected output.
 *
 * @param ptr_val Pointer to thread area argument.
 * @param ptr_str Explicit string representation of the argument.
 * @param valid   Whether argument points to the valid memory and its contents
 *                should be decoded.
 * @param entry_number_str explicit decoding of the entry_number field.
 */
static long
set_thread_area(kernel_ulong_t ptr_val, const char *ptr_str, bool valid,
		const char *entry_number_str)
{
	struct user_desc *ptr = (struct user_desc *) (uintptr_t) ptr_val;
	long rc = -1;
	int saved_errno;

	rc = syscall(__NR_set_thread_area, ptr_val);
	saved_errno = errno;
	printf("set_thread_area(");

	if (valid)
		print_user_desc(ptr, entry_number_str);
	else
		printptr(ptr_val, ptr_str);

	errno = saved_errno;
	printf(") = %s", sprintrc(rc));
	if (!rc)
		printf(" (entry_number=%u)", ptr->entry_number);

	puts("");

	return rc;
}

/**
 * Perform get_thread_are call along with printing the expected output and
 * checking the result against the argument of the previous set_thread_area
 * call, if it had place.
 *
 * @param ptr_val  Pointer to thread area argument.
 * @param ptr_str  Explicit string representation of the argument.
 * @param valid    Whether argument points to the valid memory and its contents
 *                 should be decoded.
 * @param set_rc   Return code of the previous set_thread_area call.
 * @param expected The value of the argument passed to the previous
 *                 set_thread_area call.
 */
static void
get_thread_area(kernel_ulong_t ptr_val, const char *ptr_str, bool valid,
		long set_rc, kernel_ulong_t expected)
{
	struct user_desc *ptr = (struct user_desc *) (uintptr_t) ptr_val;
	struct user_desc *expected_ptr =
		(struct user_desc *) (uintptr_t) expected;
	int saved_errno;
	long rc;

	rc = syscall(__NR_get_thread_area, ptr_val);
	saved_errno = errno;

	printf("get_thread_area(");

	if (valid && !rc) {
		if (!set_rc) {
			assert(ptr->entry_number == expected_ptr->entry_number);
			assert(ptr->base_addr    == expected_ptr->base_addr);
			assert(ptr->limit        == expected_ptr->limit);
			assert(ptr->seg_32bit    == expected_ptr->seg_32bit);
			assert(ptr->contents     == expected_ptr->contents);
			assert(ptr->read_exec_only ==
			       expected_ptr->read_exec_only);
			assert(ptr->limit_in_pages ==
			       expected_ptr->limit_in_pages);
			assert(ptr->seg_not_present ==
			       expected_ptr->seg_not_present);
			assert(ptr->useable      == expected_ptr->useable);
			/*
			 * We do not check lm as 32-bit processes ignore it, and
			 * only 32-bit processes can successfully execute
			 * get_thread_area.
			 */
		}

		print_user_desc(ptr,
				(int) ptr->entry_number == -1 ? "-1" : NULL);
	} else {
		printptr(ptr_val, ptr_str);
	}

	errno = saved_errno;
	printf(") = %s\n", sprintrc(rc));
}

int main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct user_desc, ta1);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct user_desc, ta2);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, bogus_entry_number);

	long set_rc = -1;

	/*
	 * Let's do some weird syscall, it will mark the beginning of our
	 * expected output.
	 */
	syscall(__NR_reboot, 0, 0, 0, 0);

	set_rc = set_thread_area((uintptr_t) ARG_STR(NULL), false, NULL);
	get_thread_area((uintptr_t) ARG_STR(NULL), false, set_rc,
			(uintptr_t) NULL);

	set_rc = set_thread_area(-1, NULL, false, NULL);
	get_thread_area(-1, NULL, false, set_rc, -1);

	fill_memory(ta1, sizeof(*ta1));
	fill_memory_ex(ta2, sizeof(*ta2), 0xA5, 0x5A);

	set_thread_area((uintptr_t) (ta1 + 1), NULL, false, NULL);

	set_thread_area((uintptr_t) bogus_entry_number, NULL, false, NULL);

	set_thread_area((uintptr_t) ta1, NULL, true, NULL);

	ta1->entry_number = -1;
	ta1->base_addr = 0;
	ta1->limit = 0;
	ta1->contents = 1;
	ta1->seg_32bit = 1;
	ta1->seg_not_present = 0;

	set_rc = set_thread_area((uintptr_t) ta1, NULL, true, "-1");

	*bogus_entry_number = 2718281828U;
	get_thread_area((uintptr_t) bogus_entry_number,
			"{entry_number=2718281828, ...}",
			false, set_rc, (uintptr_t) ta1);

	/* That one should return -EFAULT on i386 */
	*bogus_entry_number = 12;
	get_thread_area((uintptr_t) bogus_entry_number,
			"{entry_number=12, ...}",
			false, set_rc, (uintptr_t) ta1);

	ta2->entry_number = 3141592653U;
	get_thread_area((uintptr_t) ta2, "{entry_number=3141592653, ...}",
			false, set_rc, (uintptr_t) ta1);

	ta2->entry_number = -1;
	get_thread_area((uintptr_t) ta2, "{entry_number=-1, ...}",
			false, set_rc, (uintptr_t) ta1);

	ta2->entry_number = ta1->entry_number;
	assert(set_rc == 0 || (int) ta2->entry_number == -1);
	get_thread_area((uintptr_t) ta2, "{entry_number=-1, ...}",
			true, set_rc, (uintptr_t) ta1);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_get_thread_area && __NR_set_thread_area"
		    " && HAVE_STRUCT_USER_DESC");

#endif
