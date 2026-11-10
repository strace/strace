/*
 * Check poke_enter injection with various argument combinations.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef TEST_CASE
# error "TEST_CASE must be defined before including this file"
#endif

/* aNv = set arg N value, aNm = poke arg N memory, aNaMv = set args N..M */
#define TC_A1A6V_MAX32     1
#define TC_A1A6V_MAX64     2
#define TC_A1A6V_NEG       3
#define TC_A1V_BADFD       4
#define TC_A2A6V_MAX32     5
#define TC_A2A6V_MAX64     6
#define TC_A2A6V_NEG       7
#define TC_A2V_ZERO        8
#define TC_A3V_BADFD       9
#define TC_A4V_ZERO        10
#define TC_A5V_A2M         11
#define TC_A5V_ERROR       12
#define TC_A5V_LEN         13
#define TC_A5V_RETVAL      14
#define TC_A6V_ZERO        15

#if TEST_CASE == TC_A1A6V_MAX32
# define INJECT_SPEC "arg1=0x7fffffff,arg2=0x80000000,arg3=0xfffffffe,arg4=0xffffffff,arg5=0xdeadbeef,arg6=0xf"

#elif TEST_CASE == TC_A1A6V_MAX64
# define INJECT_SPEC "arg1=0x100000000,arg2=0x200000000,arg3=0x7fffffffffffffff,arg4=0x8000000000000000,arg5=0xffffffffffffffff,arg6=0xfffffffffffffffe"

#elif TEST_CASE == TC_A1A6V_NEG
# define INJECT_SPEC "arg1=-1,arg2=-2,arg3=-3,arg4=-4,arg5=-5,arg6=-6"

#elif TEST_CASE == TC_A1V_BADFD
# define INJECT_SPEC "arg1=2147483647"

#elif TEST_CASE == TC_A2A6V_MAX32
# define INJECT_SPEC "arg2=0x80000000,arg3=0xfffffffe,arg4=0xffffffff,arg5=0xdeadbeef,arg6=0xf"

#elif TEST_CASE == TC_A2A6V_MAX64
# define INJECT_SPEC "arg2=0x200000000,arg3=0x7fffffffffffffff,arg4=0x8000000000000000,arg5=0xffffffffffffffff,arg6=0xfffffffffffffffe"

#elif TEST_CASE == TC_A2A6V_NEG
# define INJECT_SPEC "arg2=-2,arg3=-3,arg4=-4,arg5=-5,arg6=-6"

#elif TEST_CASE == TC_A2V_ZERO
# define INJECT_SPEC "arg2=0"

#elif TEST_CASE == TC_A3V_BADFD
# define INJECT_SPEC "arg3=2147483647"

#elif TEST_CASE == TC_A4V_ZERO
# define INJECT_SPEC "arg4=0"

#elif TEST_CASE == TC_A5V_A2M
# define INJECT_SPEC "arg5=4,@arg2=0000000000000000"

#elif TEST_CASE == TC_A5V_ERROR
# define INJECT_SPEC "arg5=8:error=EISDIR"

#elif TEST_CASE == TC_A5V_LEN
# define INJECT_SPEC "arg5=010"

#elif TEST_CASE == TC_A5V_RETVAL
# define INJECT_SPEC "arg5=8:retval=42"

#elif TEST_CASE == TC_A6V_ZERO
# define INJECT_SPEC "arg6=0"

#else
# error "unknown TEST_CASE value"
#endif

#if (TEST_CASE == TC_A1A6V_MAX64 || TEST_CASE == TC_A2A6V_MAX64) && SIZEOF_KERNEL_LONG_T < 8

int main(void)
{
	error_msg_and_skip("SIZEOF_KERNEL_LONG_T < 8");
}

#else

static const char *errstr;
static long splice_rc;
static int splice_errno;

static long
k_splice(const unsigned int fd_in, const long long int *off_in,
	 const unsigned int fd_out, const long long int *off_out,
	 const unsigned long len, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = fill | fd_in;
	const kernel_ulong_t arg3 = fill | fd_out;
	const kernel_ulong_t arg6 = fill | flags;
	splice_rc = syscall(__NR_splice, arg1, off_in, arg3, off_out,
			    (kernel_ulong_t) len, arg6);
	splice_errno = errno;
	errstr = sprintrc(splice_rc);
	return splice_rc;
}

int
main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "--print-inject-args") == 0) {
		puts("poke_enter=" INJECT_SPEC);
		return 0;
	}

	bool do_assert = argc > 1 && strcmp(argv[1], "--assert") == 0;

	int file_fd = create_tmpfile(O_RDWR);
	int pfd[2];
	if (pipe(pfd))
		perror_msg_and_fail("pipe");

#if TEST_CASE == TC_A2V_ZERO || \
      TEST_CASE == TC_A4V_ZERO || \
      TEST_CASE == TC_A6V_ZERO
	static const char data[] = "01234567";
	if (write(file_fd, data, sizeof(data) - 1) != sizeof(data) - 1)
		perror_msg_and_fail("write");
	if (lseek(file_fd, 0, SEEK_SET) != 0)
		perror_msg_and_fail("lseek");

	const long long int *off_in = NULL;
	const long long int *off_out = NULL;
	unsigned int flags = 0;
# if TEST_CASE == TC_A2V_ZERO
	off_in = (const long long int *) (uintptr_t) -1;
# elif TEST_CASE == TC_A4V_ZERO
	off_out = (const long long int *) (uintptr_t) -1;
# else
	flags = 0x80000000U;
# endif

	k_splice(file_fd, off_in, pfd[1], off_out, 3, flags);
	printf("splice(%d, NULL, %d, NULL, 3, 0)"
	       " = %s (INJECTED: args)\n", file_fd, pfd[1], errstr);

	if (do_assert) {
		if (splice_rc != 3)
			error_msg_and_fail("expected 3 bytes, got %s", errstr);
		close(pfd[1]);
		char rbuf[sizeof(data)];
		ssize_t n = read(pfd[0], rbuf, sizeof(rbuf));
		if (n != 3 || memcmp(rbuf, data, 3) != 0)
			error_msg_and_fail("injected argument did not copy"
					   " the expected 3 bytes");
	}

#elif TEST_CASE == TC_A5V_A2M
	static const char file_data[] = "POKEAAAA";
	if (write(file_fd, file_data, sizeof(file_data) - 1) !=
	    sizeof(file_data) - 1)
		perror_msg_and_fail("write");

	/*
	 * off_in starts at 4: without injection, splice reads "AAAA".
	 * The poke writes 0 to off_in: splice reads "POKE" instead.
	 */
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_in);
	*off_in = 4;

	if (k_splice(file_fd, off_in, pfd[1], NULL, 8, 0) < 0)
		perror_msg_and_fail("splice");

	printf("splice(%d, [0], %d, NULL, 4, 0) = 4 (INJECTED: args)\n",
	       file_fd, pfd[1]);

	if (do_assert) {
		close(pfd[1]);
		char rbuf[8];
		ssize_t n = read(pfd[0], rbuf, sizeof(rbuf));
		if (n != 4)
			error_msg_and_fail("expected 4 bytes, got %zd", n);
		if (memcmp(rbuf, "POKE", 4) != 0)
			error_msg_and_fail("expected \"POKE\", got \"%.*s\"",
					   (int) n, rbuf);
	}

#elif TEST_CASE == TC_A5V_ERROR || TEST_CASE == TC_A5V_RETVAL
	static const char data[] = "0123456789abcdef";
	const unsigned int data_size = sizeof(data) - 1;
	if (write(file_fd, data, data_size) != data_size)
		perror_msg_and_fail("write");
	if (lseek(file_fd, 0, SEEK_SET) != 0)
		perror_msg_and_fail("lseek");

	k_splice(file_fd, NULL, pfd[1], NULL, data_size, 0);
	printf("splice(%d, NULL, %d, NULL, 8, 0)"
	       " = %s (INJECTED: args, retval)\n", file_fd, pfd[1], errstr);

	if (do_assert) {
		/* Check for execution even if its result was overridden. */
		if (close(pfd[1]))
			perror_msg_and_fail("close");
		char rbuf[sizeof(data)];
		ssize_t n = read(pfd[0], rbuf, sizeof(rbuf));
		if (n < 0)
			perror_msg_and_fail("read");
		if (n != 0)
			error_msg_and_fail("suppressed splice transferred %zd"
					   " bytes", n);
		off_t offset = lseek(file_fd, 0, SEEK_CUR);
		if (offset < 0)
			perror_msg_and_fail("lseek");
		if (offset != 0)
			error_msg_and_fail("suppressed splice changed the file"
					   " offset");
# if TEST_CASE == TC_A5V_ERROR
		if (splice_rc != -1 || splice_errno != EISDIR)
			error_msg_and_fail("expected EISDIR, got %s", errstr);
# else
		if (splice_rc != 42)
			error_msg_and_fail("expected injected return value 42"
					   ", got %s", errstr);
# endif
	} else if (splice_rc != data_size) {
		error_msg_and_fail("expected %u bytes, got %s", data_size, errstr);
	}

#elif TEST_CASE == TC_A5V_LEN
	static const char data[] = "0123456789abcdef";
	enum {
		data_size = sizeof(data) - 1,
		calls = 2,
		length = 8
	};
	static_assert(calls * length == data_size, "data size mismatch");

	if (write(file_fd, data, data_size) != data_size)
		perror_msg_and_fail("write");
	if (lseek(file_fd, 0, SEEK_SET) != 0)
		perror_msg_and_fail("lseek");

	for (unsigned int i = 0; i < calls; ++i) {
		if (k_splice(file_fd, NULL, pfd[1], NULL, data_size, 0) < 0)
			perror_msg_and_fail("splice");
		printf("splice(%d, NULL, %d, NULL, %d, 0)"
		       " = %s (INJECTED: args)\n",
		       file_fd, pfd[1], length, errstr);
		if (do_assert && splice_rc != length)
			error_msg_and_fail("expected %d bytes, got %ld",
					   length, splice_rc);
	}

	if (do_assert) {
		close(pfd[1]);
		char rbuf[sizeof(data)];
		ssize_t n = read(pfd[0], rbuf, sizeof(rbuf));
		if (n != length * calls)
			error_msg_and_fail("expected %u bytes, got %zd",
					   length * calls, n);
		if (memcmp(rbuf, data, length * calls) != 0)
			error_msg_and_fail("data mismatch");
	}

#else /* TC_A1A6V .. TC_A3V */
	static const char data[] = "0123456789abcdef";
	if (write(file_fd, data, sizeof(data) - 1) != sizeof(data) - 1)
		perror_msg_and_fail("write");
	if (lseek(file_fd, 0, SEEK_SET) != 0)
		perror_msg_and_fail("lseek");

	k_splice(file_fd, NULL, pfd[1], NULL, sizeof(data) - 1, 0);

# if TEST_CASE == TC_A1A6V_MAX32
	/*
	 * All 6 args set to 32-bit boundary values:
	 * arg1=0x7fffffff, arg2=0x80000000,
	 * arg3=0xfffffffe, arg4=0xffffffff,
	 * arg5=0xdeadbeef, arg6=0xf
	 *
	 * printfd truncates to int;
	 * printnum_int64_d with invalid address prints printaddr;
	 * PRINT_VAL_U zero-extends to unsigned long long;
	 * printflags truncates to unsigned int.
	 */
	printf("splice(2147483647, 0x80000000, -2, 0xffffffff, 3735928559"
	       ", SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT)"
	       " = %s (INJECTED: args)\n",
	       errstr);
	if (do_assert && (splice_rc != -1 || splice_errno != EBADF))
		error_msg_and_fail("expected EBADF for injected fds, got %s",
				   errstr);

# elif TEST_CASE == TC_A1A6V_MAX64
	/*
	 * All 6 args set to 64-bit values (64-bit only):
	 * arg1=0x100000000, arg2=0x200000000,
	 * arg3=0x7fffffffffffffff, arg4=0x8000000000000000,
	 * arg5=0xffffffffffffffff, arg6=0xfffffffffffffffe
	 *
	 * printfd truncates to int: 0x100000000->0, 0x7f..f->-1;
	 * printflags truncates to unsigned int: 0xf..e->0xfffffffe.
	 */
	printf("splice(0, 0x200000000, -1, 0x8000000000000000"
	       ", 18446744073709551615"
	       ", SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT|0xfffffff0)"
	       " = %s (INJECTED: args)\n",
	       errstr);
	if (do_assert && splice_rc != -1)
		error_msg_and_fail("expected failure for injected args, got %s",
				   errstr);

# elif TEST_CASE == TC_A1A6V_NEG
	/*
	 * All 6 args set to distinct negative values:
	 * arg1=-1, arg2=-2, arg3=-3, arg4=-4, arg5=-5, arg6=-6
	 *
	 * printfd truncates to int: -1, -3;
	 * printnum_int64_d with invalid address: printaddr of
	 * kernel_ulong_t value;
	 * PRINT_VAL_U zero-extends kernel_ulong_t to ull;
	 * printflags truncates to unsigned int.
	 */
	printf("splice(-1, %#" PRI_klx ", -3, %#" PRI_klx ", %" PRI_klu
	       ", SPLICE_F_NONBLOCK|SPLICE_F_GIFT|%#x) = %s (INJECTED: args)\n",
	       (kernel_ulong_t) -2,
	       (kernel_ulong_t) -4,
	       (kernel_ulong_t) -5,
	       (unsigned int) -6 & ~0xfU,
	       errstr);
	if (do_assert && splice_rc != -1)
		error_msg_and_fail("expected failure for injected args, got %s",
				   errstr);

# elif TEST_CASE == TC_A1V_BADFD
	printf("splice(%d, NULL, %d, NULL, 16, 0) = %s (INJECTED: args)\n",
	       2147483647, pfd[1], errstr);

	if (do_assert && (splice_rc != -1 || splice_errno != EBADF))
		error_msg_and_fail("expected EBADF for injected fd, got %s",
				   errstr);

# elif TEST_CASE == TC_A2A6V_MAX32
	printf("splice(%d, 0x80000000, -2, 0xffffffff, 3735928559"
	       ", SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT)"
	       " = %s (INJECTED: args)\n",
	       file_fd, errstr);
	if (do_assert && (splice_rc != -1 || splice_errno != EBADF))
		error_msg_and_fail("expected EBADF for injected fd, got %s",
				   errstr);

# elif TEST_CASE == TC_A2A6V_MAX64
	printf("splice(%d, 0x200000000, -1, 0x8000000000000000"
	       ", 18446744073709551615"
	       ", SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT|0xfffffff0)"
	       " = %s (INJECTED: args)\n",
	       file_fd, errstr);
	if (do_assert && splice_rc != -1)
		error_msg_and_fail("expected failure for injected args, got %s",
				   errstr);

# elif TEST_CASE == TC_A2A6V_NEG
	printf("splice(%d, %#" PRI_klx ", -3, %#" PRI_klx ", %" PRI_klu
	       ", SPLICE_F_NONBLOCK|SPLICE_F_GIFT|%#x)"
	       " = %s (INJECTED: args)\n",
	       file_fd,
	       (kernel_ulong_t) -2,
	       (kernel_ulong_t) -4,
	       (kernel_ulong_t) -5,
	       (unsigned int) -6 & ~0xfU,
	       errstr);
	if (do_assert && splice_rc != -1)
		error_msg_and_fail("expected failure for injected args, got %s",
				   errstr);

# elif TEST_CASE == TC_A3V_BADFD
	printf("splice(%d, NULL, %d, NULL, 16, 0) = %s (INJECTED: args)\n",
	       file_fd, 2147483647, errstr);

	if (do_assert && (splice_rc != -1 || splice_errno != EBADF))
		error_msg_and_fail("expected EBADF for injected fd, got %s",
				   errstr);

# endif /* TC_A1A6V .. TC_A3V */

#endif /* TC_A2V .. TC_A6V / TC_A5V_A2M / TC_A5V vs. the rest */

	puts("+++ exited with 0 +++");

	return 0;
}

#endif /* !(TC_A1A6V_MAX64 && 32-bit) */
