/*
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#ifdef __NR_futex

# include <errno.h>
# include <stdarg.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

# include <sys/time.h>

# ifndef FUTEX_PRIVATE_FLAG
#  define FUTEX_PRIVATE_FLAG 128
# endif
# ifndef FUTEX_CLOCK_REALTIME
#  define FUTEX_CLOCK_REALTIME 256
# endif
# ifndef FUTEX_CMD_MASK
#  define FUTEX_CMD_MASK ~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)
# endif

# include "xlat.h"
# include "xlat/futexops.h"
# include "xlat/futexwakeops.h"
# include "xlat/futexwakecmps.h"

static void
futex_error(int *uaddr, int op, unsigned long val, unsigned long timeout,
	    int *uaddr2, unsigned long val3, int rc, const char *func, int line)
{
	perror_msg_and_fail("%s:%d: futex(%p, %#x, %#x, %#lx, %p, %#x) = %d",
		func, line, uaddr, op, (unsigned) val, timeout, uaddr,
		(unsigned) val3, rc);
}

# define CHECK_FUTEX_GENERIC(uaddr, op, val, timeout, uaddr2, val3, check, \
	enosys) \
	do { \
		errno = 0; \
		rc = syscall(__NR_futex, (uaddr), (op), (val), (timeout), \
			(uaddr2), (val3)); \
		/* It is here due to EPERM on WAKE_OP on AArch64 */ \
		if ((rc == -1) && (errno == EPERM)) \
			break; \
		if (enosys && (rc == -1) && (errno == ENOSYS)) \
			break; \
		if (!(check)) \
			futex_error((uaddr), (op), (val), \
				(unsigned long) (timeout), (int *) (uaddr2), \
				(val3), rc, __func__, __LINE__); \
	} while (0)

# define CHECK_FUTEX_ENOSYS(uaddr, op, val, timeout, uaddr2, val3, check) \
	CHECK_FUTEX_GENERIC(uaddr, op, val, timeout, uaddr2, val3, check, 1)

# define CHECK_FUTEX(uaddr, op, val, timeout, uaddr2, val3, check) \
	CHECK_FUTEX_GENERIC(uaddr, op, val, timeout, uaddr2, val3, check, 0)

enum argmask {
	ARG3 = 1 << 0,
	ARG4 = 1 << 1,
	ARG5 = 1 << 2,
	ARG6 = 1 << 3,
};

static void
invalid_op(int *val, int op, uint32_t argmask, ...)
{
	static const unsigned long args[] = {
		(unsigned long) 0xface1e55deadbee1ULL,
		(unsigned long) 0xface1e56deadbee2ULL,
		(unsigned long) 0xface1e57deadbee3ULL,
		(unsigned long) 0xface1e58deadbee4ULL,
	};
	/* Since timeout value is copied before full op check, we should provide
	 * some valid timeout address or NULL */
	int cmd = op & FUTEX_CMD_MASK;
	bool valid_timeout = (cmd == FUTEX_WAIT) || (cmd == FUTEX_LOCK_PI) || \
		(cmd == FUTEX_LOCK_PI2) || (cmd == FUTEX_WAIT_BITSET) || \
		(cmd == FUTEX_WAIT_REQUEUE_PI);
	bool timeout_is_val2 = (cmd == FUTEX_REQUEUE) ||
		(cmd == FUTEX_CMP_REQUEUE) || (cmd == FUTEX_WAKE_OP) ||
		(cmd == FUTEX_CMP_REQUEUE_PI);
	const char *fmt;
	int saved_errno;
	int rc;
	va_list ap;


	CHECK_FUTEX(val, op, args[0], valid_timeout ? 0 : args[1], args[2],
		args[3], (rc == -1) && (errno == ENOSYS));
	saved_errno = errno;
	printf("futex(%p, %#x /* FUTEX_??? */", val, op);

	va_start(ap, argmask);

	for (int i = 0; i < 4; ++i) {
		if (argmask & (1 << i)) {
			fmt = va_arg(ap, const char *);

			printf(", ");

			if (((1 << i) == ARG3) || ((1 << i) == ARG6) ||
			    (((1 << i) == ARG4) && timeout_is_val2))
				printf(fmt, (unsigned) args[i]);
			else
				printf(fmt, args[i]);
		}
	}

	va_end(ap);

	errno = saved_errno;
	printf(")" RVAL_ENOSYS);
}

# define CHECK_INVALID_CLOCKRT(op, ...) \
	do { \
		invalid_op(uaddr, FUTEX_CLOCK_REALTIME | (op), __VA_ARGS__); \
		invalid_op(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_PRIVATE_FLAG | \
			(op), __VA_ARGS__); \
	} while (0)

/* Value which differs from one stored in int *val */
# define VAL      ((unsigned long) 0xbadda7a0facefeedLLU)
# define VAL_PR   ((unsigned) VAL)

# define VALP     ((unsigned long) 0xbadda7a01acefeedLLU)
# define VALP_PR  ((unsigned) VALP)

# define VAL2     ((unsigned long) 0xbadda7a0ca7b100dLLU)
# define VAL2_PR  ((unsigned) VAL2)

# define VAL2P    ((unsigned long) 0xbadda7a07a7b100dLLU)
# define VAL2P_PR ((unsigned) VAL2P)

# define VAL3     ((unsigned long) 0xbadda7a09caffee1LLU)
# define VAL3_PR  ((unsigned) VAL3)

# define VAL3A    ((unsigned long) 0xbadda7a0ffffffffLLU)
# define VAL3A_PR "FUTEX_BITSET_MATCH_ANY"

int
main(int argc, char *argv[])
{
	TAIL_ALLOC_OBJECT_CONST_PTR(int, uaddr);
	TAIL_ALLOC_OBJECT_CONST_PTR(int, uaddr2);
	int rc;

	uaddr[0] = 0x1deadead;
	uaddr2[0] = 0xbadf00d;

	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_old_timespec_t, tmout);
	tmout->tv_sec = 123;
	tmout->tv_nsec = 0xbadc0de;

	/* FUTEX_WAIT - check whether uaddr == val and sleep
	 * Possible flags: PRIVATE, CLOCK_RT (since 4.5)
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_WAIT
	 * 3. val     - expected value
	 * 4. timeout - address to timespec with timeout
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	/* uaddr is NULL */
	CHECK_FUTEX(NULL, FUTEX_WAIT, VAL, tmout, uaddr2, VAL3,
		(rc == -1) && (errno == EFAULT));
	printf("futex(NULL, FUTEX_WAIT, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* uaddr is faulty */
	CHECK_FUTEX(uaddr + 1, FUTEX_WAIT, VAL, tmout, uaddr2, VAL3,
		(rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_WAIT, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr + 1, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* timeout is faulty */
	CHECK_FUTEX(uaddr, FUTEX_WAIT, VAL, tmout + 1, uaddr2, VAL3,
		(rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_WAIT, %u, %p) = %s\n",
	       uaddr, 0xfacefeed, tmout + 1, sprintrc(rc));

	/* timeout is invalid */
	tmout->tv_sec = 0xdeadbeefU;
	tmout->tv_nsec = 0xfacefeedU;

	CHECK_FUTEX(uaddr, FUTEX_WAIT, VAL, tmout, uaddr2, VAL3,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_WAIT, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	tmout->tv_sec = (typeof(tmout->tv_sec)) 0xcafef00ddeadbeefLL;
	tmout->tv_nsec = (long) 0xbadc0dedfacefeedLL;

	CHECK_FUTEX(uaddr, FUTEX_WAIT, VAL, tmout, uaddr2, VAL3,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_WAIT, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	tmout->tv_sec = 123;
	tmout->tv_nsec = 0xbadc0de;

	/* uaddr is not as provided; uaddr2 is faulty but ignored */
	CHECK_FUTEX(uaddr, FUTEX_WAIT, VAL, tmout, uaddr2 + 1, VAL3,
		(rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* uaddr is not as provided; uaddr2 is faulty but ignored */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_WAIT, VAL, tmout,
		uaddr2 + 1, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_PRIVATE, %u, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* Next 2 tests are with CLOCKRT bit set */

	/* Valid after v4.4-rc2-27-g337f130 */
	CHECK_FUTEX_ENOSYS(uaddr,
		FUTEX_CLOCK_REALTIME | FUTEX_WAIT,
		VAL, tmout, uaddr2, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr,
		FUTEX_CLOCK_REALTIME | FUTEX_PRIVATE_FLAG | FUTEX_WAIT,
		VAL, tmout, uaddr2, 0, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_PRIVATE|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* FUTEX_WAIT_BITSET - FUTEX_WAIT which provides additional bitmask
	 *                     which should be matched at least in one bit with
	 *                     wake mask in order to wake.
	 * Possible flags: PRIVATE, CLOCKRT
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_TRYLOCK_PI
	 * 3. val     - expected value stored in uaddr
	 * 4. timeout - timeout
	 * 5. uaddr2  - not used
	 * 6. val3    - bitmask
	 */

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAIT_BITSET, VAL, tmout, uaddr2 + 1,
		VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_BITSET, %u, {tv_sec=%lld, tv_nsec=%llu}"
	       ", %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), VAL3_PR,
	       sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAIT_BITSET, VAL, tmout, uaddr2 + 1,
		VAL3A, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_BITSET, %u, {tv_sec=%lld, tv_nsec=%llu}"
	       ", %s) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), VAL3A_PR,
	       sprintrc(rc));

	/* val3 of 0 is invalid  */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAIT_BITSET, VAL, tmout, uaddr2 + 1, 0,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_WAIT_BITSET, %u, {tv_sec=%lld, tv_nsec=%llu}"
	       ", %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), 0, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_WAIT_BITSET, VAL,
		tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_BITSET_PRIVATE, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), VAL3_PR,
	       sprintrc(rc));

	/* Next 3 tests are with CLOCKRT bit set */

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_WAIT_BITSET, VAL,
		tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), VAL3_PR,
	       sprintrc(rc));

	/* val3 of 0 is invalid  */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_WAIT_BITSET, VAL,
		tmout, uaddr2 + 1, 0, (rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), 0, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_PRIVATE_FLAG |
		FUTEX_WAIT_BITSET, VAL, tmout, uaddr2 + 1, VAL3,
		(rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_BITSET_PRIVATE|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %#x) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), VAL3_PR,
	       sprintrc(rc));

	/* FUTEX_WAKE - wake val processes waiting for uaddr
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_WAKE
	 * 3. val     - how many processes to wake
	 * 4. timeout - not used
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	/* Zero processes to wake is not a good idea, but it should return 0 */
	CHECK_FUTEX(uaddr, FUTEX_WAKE, 0, NULL, NULL, 0, (rc == 0));
	printf("futex(%p, FUTEX_WAKE, %u) = %s\n", uaddr, 0, sprintrc(rc));

	/* Trying to wake some processes, but there's nothing to wake */
	CHECK_FUTEX(uaddr, FUTEX_WAKE, 10, NULL, NULL, 0, (rc == 0));
	printf("futex(%p, FUTEX_WAKE, %u) = %s\n", uaddr, 10, sprintrc(rc));

	/* Trying to wake some processes, but there's nothing to wake */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_WAKE, 10, NULL,
		NULL, 0, (rc == 0));
	printf("futex(%p, FUTEX_WAKE_PRIVATE, %u) = %s\n", uaddr, 10,
		sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_WAKE, ARG3, "%u");

	/* FUTEX_WAKE_BITSET - wake val processes waiting for uaddr which has at
	 *                     least one common bit with bitset provided in
	 *                     val3.
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_WAKE
	 * 3. val     - how many processes to wake
	 * 4. timeout - not used
	 * 5. uaddr2  - not used
	 * 6. val3    - bitmask
	 */

	/* Trying to wake some processes, but there's nothing to wake */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAKE_BITSET, 10, NULL, NULL,
		VAL3, (rc == 0));
	printf("futex(%p, FUTEX_WAKE_BITSET, %u, %#x) = %s\n", uaddr, 10,
		VAL3_PR, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAKE_BITSET, 10, NULL, NULL,
		VAL3A, (rc == 0));
	printf("futex(%p, FUTEX_WAKE_BITSET, %u, %s) = %s\n", uaddr, 10,
		VAL3A_PR, sprintrc(rc));

	/* bitset 0 is invalid */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAKE_BITSET, 10, NULL, NULL, 0,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_WAKE_BITSET, %u, %#x) = %s\n", uaddr, 10, 0,
		sprintrc(rc));

	/* Trying to wake some processes, but there's nothing to wake */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_WAKE_BITSET, 10,
		NULL, NULL, VAL3, (rc == 0));
	printf("futex(%p, FUTEX_WAKE_BITSET_PRIVATE, %u, %#x) = %s\n", uaddr,
		10, VAL3_PR, sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_WAKE_BITSET, ARG3 | ARG6, "%u", "%#x");

	/* FUTEX_FD - deprecated
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_FD
	 * 3. val     - signal number
	 * 4. timeout - not used
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	/* FUTEX_FD is not implemented since 2.6.26 */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_FD, VAL, NULL, NULL, VAL3,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_FD, %u) = %s\n", uaddr, VAL_PR, sprintrc(rc));

	/* FUTEX_FD is not implemented since 2.6.26 */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_FD, VAL, NULL,
		NULL, VAL3, (rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_FD|FUTEX_PRIVATE_FLAG, %u) = %s\n", uaddr,
		VAL_PR, sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_FD, ARG3, "%u");

	/* FUTEX_REQUEUE - wake val processes and re-queue rest on uaddr2
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_REQUEUE
	 * 3. val     - how many processes to wake
	 * 4. val2    - amount of processes to re-queue on uadr2
	 * 5. uaddr2  - another futex address, to re-queue waiting processes on
	 * 6. val3    - not used
	 */

	/* Trying to re-queue some processes but there's nothing to re-queue */
	CHECK_FUTEX(uaddr, FUTEX_REQUEUE, VAL, VAL2, uaddr2, VAL3,
		(rc == 0) || ((rc == -1) && (errno == EINVAL)));
	printf("futex(%p, FUTEX_REQUEUE, %u, %u, %p) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, sprintrc(rc));

	CHECK_FUTEX(uaddr, FUTEX_REQUEUE, VALP, VAL2P, uaddr2, VAL3,
		(rc == 0));
	printf("futex(%p, FUTEX_REQUEUE, %u, %u, %p) = %s\n",
		uaddr, VALP_PR, VAL2P_PR, uaddr2, sprintrc(rc));

	/* Trying to re-queue some processes but there's nothing to re-queue */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_REQUEUE, VAL, VAL2,
		uaddr2, VAL3, (rc == 0) || ((rc == -1) && (errno == EINVAL)));
	printf("futex(%p, FUTEX_REQUEUE_PRIVATE, %u, %u, %p) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_REQUEUE, VALP,
		VAL2P, uaddr2, VAL3, (rc == 0));
	printf("futex(%p, FUTEX_REQUEUE_PRIVATE, %u, %u, %p) = %s\n",
		uaddr, VALP_PR, VAL2P_PR, uaddr2, sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_REQUEUE, ARG3 | ARG4 | ARG5, "%u", "%u",
		"%#lx");

	/* FUTEX_CMP_REQUEUE - wake val processes and re-queue rest on uaddr2
	 *                     if uaddr has value val3
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_CMP_REQUEUE
	 * 3. val     - how many processes to wake
	 * 4. val2    - amount of processes to re-queue on uadr2
	 * 5. uaddr2  - another futex address, to re-queue waiting processes on
	 * 6. val3    - expected value stored in uaddr
	 */

	/* Comparison re-queue with wrong val value */
	CHECK_FUTEX(uaddr, FUTEX_CMP_REQUEUE, VAL, VAL2, uaddr2, VAL3,
		(rc == -1) && (errno == EAGAIN || errno == EINVAL));
	printf("futex(%p, FUTEX_CMP_REQUEUE, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, VAL3_PR, sprintrc(rc));

	CHECK_FUTEX(uaddr, FUTEX_CMP_REQUEUE, VALP, VAL2P, uaddr2, VAL3,
		(rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_CMP_REQUEUE, %u, %u, %p, %u) = %s\n",
		uaddr, VALP_PR, VAL2P_PR, uaddr2, VAL3_PR, sprintrc(rc));

	/* Successful comparison re-queue */
	CHECK_FUTEX(uaddr, FUTEX_CMP_REQUEUE, VAL, VAL2, uaddr2, *uaddr,
		(rc == 0) || ((rc == -1) && (errno == EINVAL)));
	printf("futex(%p, FUTEX_CMP_REQUEUE, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, *uaddr, sprintrc(rc));

	CHECK_FUTEX(uaddr, FUTEX_CMP_REQUEUE, VALP, VAL2P, uaddr2, *uaddr,
		(rc == 0));
	printf("futex(%p, FUTEX_CMP_REQUEUE, %u, %u, %p, %u) = %s\n",
		uaddr, VALP_PR, VAL2P_PR, uaddr2, *uaddr, sprintrc(rc));

	/* Successful comparison re-queue */
	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_CMP_REQUEUE, VAL,
		VAL2, uaddr2, *uaddr,
		(rc == 0) || ((rc == -1) && (errno == EINVAL)));
	printf("futex(%p, FUTEX_CMP_REQUEUE_PRIVATE, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, *uaddr, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_CMP_REQUEUE, VALP,
		VAL2P, uaddr2, *uaddr, (rc == 0));
	printf("futex(%p, FUTEX_CMP_REQUEUE_PRIVATE, %u, %u, %p, %u) = %s\n",
		uaddr, VALP_PR, VAL2P_PR, uaddr2, *uaddr, sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_CMP_REQUEUE, ARG3 | ARG4 | ARG5 | ARG6,
		"%u", "%u", "%#lx", "%u");

	/* FUTEX_WAKE_OP - wake val processes waiting for uaddr, additionally
	 *                 wake val2 processes waiting for uaddr2 in case
	 *                 operation encoded in val3 (change of value at uaddr2
	 *                 and comparison of previous value against provided
	 *                 constant) succeeds with value at uaddr2. Operation
	 *                 result is written to value of uaddr2 (in any case).
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_WAKE_OP
	 * 3. val     - how many processes to wake
	 * 4. val2    - amount of processes to wake in case operation encoded in
	 *              val3 returns true
	 * 5. uaddr2  - another futex address, for conditional wake of
	 *              additional processes
	 * 6. val3    - encoded operation:
	 *                1. bit 31 - if 1 then value stored in field field 4
	 *                            should be interpreted as power of 2.
	 *                2. 28..30 - arithmetic operation which should be
	 *                            applied to previous value stored in
	 *                            uaddr2. Values available (from 2005 up to
	 *                            2016): SET. ADD, OR, ANDN, XOR.
	 *                3. 24..29 - comparison operation which should be
	 *                            applied to the old value stored in uaddr2
	 *                            (before arithmetic operation is applied).
	 *                            Possible values: EQ, NE, LT, LE, GT, GE.
	 *                4. 12..23 - Second operand for arithmetic operation.
	 *                            If bit 31 is set, it is interpreted as
	 *                            power of 2.
	 *                5. 00..11 - Value against which old value stored in
	 *                            uaddr2 is compared.
	 */

	static const struct {
		uint32_t val;
		const char *str;

		/*
		 * Peculiar semantics:
		 *  * err == 0 and err2 != 0 => expect both either the absence
		 *    of error or presence of err2
		 *  * err != 0 and err2 == 0 => expect err only, no success
		 *    expected.
		 */
		int err;
		int err2;
	} wake_ops[] = {
		{ 0x00000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_EQ<<24|0" },
		{ 0x00fff000, "FUTEX_OP_SET<<28|0xfff<<12|FUTEX_OP_CMP_EQ<<24|"
			"0" },
		{ 0x00000fff, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_EQ<<24|"
			"0xfff" },
		{ 0x00ffffff, "FUTEX_OP_SET<<28|0xfff<<12|FUTEX_OP_CMP_EQ<<24|"
			"0xfff" },
		{ 0x10000000, "FUTEX_OP_ADD<<28|0<<12|FUTEX_OP_CMP_EQ<<24|0" },
		{ 0x20000000, "FUTEX_OP_OR<<28|0<<12|FUTEX_OP_CMP_EQ<<24|0" },
		{ 0x30000000, "FUTEX_OP_ANDN<<28|0<<12|FUTEX_OP_CMP_EQ<<24|0" },
		{ 0x40000000, "FUTEX_OP_XOR<<28|0<<12|FUTEX_OP_CMP_EQ<<24|0" },
		{ 0x50000000, "0x5<<28 /* FUTEX_OP_??? */|0<<12|"
			"FUTEX_OP_CMP_EQ<<24|0", ENOSYS },
		{ 0x70000000, "0x7<<28 /* FUTEX_OP_??? */|0<<12|"
			"FUTEX_OP_CMP_EQ<<24|0", ENOSYS },
		{ 0x80000000, "FUTEX_OP_OPARG_SHIFT<<28|FUTEX_OP_SET<<28|0<<12|"
			"FUTEX_OP_CMP_EQ<<24|0" },
		{ 0xa0caffee, "FUTEX_OP_OPARG_SHIFT<<28|FUTEX_OP_OR<<28|"
			"0xcaf<<12|FUTEX_OP_CMP_EQ<<24|0xfee", 0, EINVAL },
		{ 0x01000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_NE<<24|0" },
		{ 0x01234567, "FUTEX_OP_SET<<28|0x234<<12|FUTEX_OP_CMP_NE<<24|"
			"0x567" },
		{ 0x02000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_LT<<24|0" },
		{ 0x03000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_LE<<24|0" },
		{ 0x04000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_GT<<24|0" },
		{ 0x05000000, "FUTEX_OP_SET<<28|0<<12|FUTEX_OP_CMP_GE<<24|0" },
		{ 0x06000000, "FUTEX_OP_SET<<28|0<<12|"
			"0x6<<24 /* FUTEX_OP_CMP_??? */|0", ENOSYS },
		{ 0x07000000, "FUTEX_OP_SET<<28|0<<12|"
			"0x7<<24 /* FUTEX_OP_CMP_??? */|0", ENOSYS },
		{ 0x08000000, "FUTEX_OP_SET<<28|0<<12|"
			"0x8<<24 /* FUTEX_OP_CMP_??? */|0", ENOSYS },
		{ 0x0f000000, "FUTEX_OP_SET<<28|0<<12|"
			"0xf<<24 /* FUTEX_OP_CMP_??? */|0", ENOSYS },
		{ 0xbadfaced, "FUTEX_OP_OPARG_SHIFT<<28|FUTEX_OP_ANDN<<28|"
			"0xdfa<<12|0xa<<24 /* FUTEX_OP_CMP_??? */|0xced",
			ENOSYS, EINVAL },
		{ 0xffffffff, "FUTEX_OP_OPARG_SHIFT<<28|"
			"0x7<<28 /* FUTEX_OP_??? */|0xfff<<12|"
			"0xf<<24 /* FUTEX_OP_CMP_??? */|0xfff",
			ENOSYS, EINVAL },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(wake_ops); ++i) {
		for (unsigned int j = 0; j < 2; ++j) {
			CHECK_FUTEX_ENOSYS(uaddr,
				j ? FUTEX_WAKE_OP_PRIVATE : FUTEX_WAKE_OP,
				VAL, i, uaddr2, wake_ops[i].val,
				/*
				 * Either one of errs is 0 or rc == 0 is not
				 * allowed.
				 */
				((!wake_ops[i].err || !wake_ops[i].err2 ||
					(rc != 0)) &&
				((!wake_ops[i].err && (rc == 0)) ||
				(wake_ops[i].err  && (rc == -1) &&
					(errno == wake_ops[i].err)) ||
				(wake_ops[i].err2 && (rc == -1) &&
					(errno == wake_ops[i].err2)))));
			printf("futex(%p, FUTEX_WAKE_OP%s, %u, %u, %p, %s)"
			       " = %s\n", uaddr, j ? "_PRIVATE" : "", VAL_PR,
			       i, uaddr2, wake_ops[i].str, sprintrc(rc));
		}
	}

	CHECK_INVALID_CLOCKRT(FUTEX_WAKE_OP, ARG3 | ARG4 | ARG5 | ARG6,
		"%u", "%u", "%#lx",
		/* Decoding of the 0xdeadbee4 value */
		"FUTEX_OP_OPARG_SHIFT<<28|0x5<<28 /* FUTEX_OP_??? */|0xadb<<12|"
		"0xe<<24 /* FUTEX_OP_CMP_??? */|0xee4");

	/* FUTEX_LOCK_PI - slow path for mutex lock with process inheritance
	 *                 support. Expect that futex has 0 in unlocked case and
	 *                 TID of owning process in locked case. Value can also
	 *                 contain FUTEX_WAITERS bit signalling the presence of
	 *                 waiters queue.
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_LOCK_PI
	 * 3. val     - not used
	 * 4. timeout - timeout
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	*uaddr = getpid();

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_LOCK_PI, VAL, tmout, uaddr2 + 1,
		VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_LOCK_PI, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr + 1, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_PRIVATE_FLAG | FUTEX_LOCK_PI, VAL,
		tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_LOCK_PI_PRIVATE, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       uaddr + 1, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	/* NULL is passed by invalid_op() in cases valid timeout address is
	 * needed */
	CHECK_INVALID_CLOCKRT(FUTEX_LOCK_PI, ARG4, "NULL");

	/* FUTEX_UNLOCK_PI - slow path for mutex unlock with process inheritance
	 *                   support. Expected to be called by process in case
	 *                   it failed to execute fast path (it usually means
	 *                   that FUTEX_WAITERS flag had been set while the lock
	 *                   has been held).
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_UNLOCK_PI
	 * 3. val     - not used
	 * 4. timeout - not used
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_UNLOCK_PI, VAL, tmout, uaddr2 + 1,
		VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_UNLOCK_PI) = %s\n", uaddr + 1, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_PRIVATE_FLAG | FUTEX_UNLOCK_PI, VAL,
		tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_UNLOCK_PI_PRIVATE) = %s\n", uaddr + 1,
		sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_UNLOCK_PI, 0);

	/* FUTEX_TRYLOCK_PI - slow path for mutex trylock with process
	 *                 inheritance support.
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_TRYLOCK_PI
	 * 3. val     - not used
	 * 4. timeout - not used
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_TRYLOCK_PI, VAL, tmout, uaddr2 + 1,
		VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_TRYLOCK_PI) = %s\n", uaddr + 1, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_PRIVATE_FLAG | FUTEX_TRYLOCK_PI,
		VAL, tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_TRYLOCK_PI_PRIVATE) = %s\n", uaddr + 1,
		sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_TRYLOCK_PI, 0);

	/* FUTEX_WAIT_REQUEUE_PI - kernel-side handling of special case when
	 *                         processes should be re-queued on PI-aware
	 *                         futexes. This is so special since PI futexes
	 *                         utilize rt_mutex and it should be at no time
	 *                         left free with a wait queue, so this should
	 *                         be performed atomically in-kernel.
	 * Possible flags: PRIVATE, CLOCKRT
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_WAIT_REQUEUE_PI
	 * 3. val     - expected value stored in uaddr
	 * 4. timeout - timeout
	 * 5. uaddr2  - (PI-aware) futex address to requeue process on
	 * 6. val3    - not used (in kernel, it always initialized to
	 *              FUTEX_BITSET_MATCH_ANY and passed to
	 *              futex_wait_requeue_pi())
	 */

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_WAIT_REQUEUE_PI, VAL, tmout, uaddr2,
		VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_REQUEUE_PI, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), uaddr2, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_WAIT_REQUEUE_PI,
		VAL, tmout, uaddr2, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_REQUEUE_PI_PRIVATE, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), uaddr2, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_WAIT_REQUEUE_PI,
		VAL, tmout, uaddr2, VAL3, (rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_REQUEUE_PI|FUTEX_CLOCK_REALTIME, %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), uaddr2, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CLOCK_REALTIME | FUTEX_PRIVATE_FLAG |
		FUTEX_WAIT_REQUEUE_PI, VAL, tmout, uaddr2, VAL3,
		(rc == -1) && (errno == EAGAIN));
	printf("futex(%p, FUTEX_WAIT_REQUEUE_PI_PRIVATE|FUTEX_CLOCK_REALTIME"
	       ", %u, {tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       uaddr, VAL_PR, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), uaddr2, sprintrc(rc));

	/* FUTEX_CMP_REQUEUE_PI - version of FUTEX_CMP_REQUEUE which re-queues
	 *                        on PI-aware futex.
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_CMP_REQUEUE
	 * 3. val     - how many processes to wake
	 * 4. val2    - amount of processes to re-queue on uadr2
	 * 5. uaddr2  - (PI-aware) futex address, to re-queue waiting processes
	 *              on
	 * 6. val3    - expected value stored in uaddr
	 */

	/* All these should fail with EINVAL since we try to re-queue to  non-PI
	 * futex.
	 */

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CMP_REQUEUE_PI, VAL, VAL2, uaddr2, VAL3,
		(rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_CMP_REQUEUE_PI, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, VAL3_PR, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_CMP_REQUEUE_PI, VAL, VAL2, uaddr2,
		*uaddr, (rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_CMP_REQUEUE_PI, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, *uaddr, sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr, FUTEX_PRIVATE_FLAG | FUTEX_CMP_REQUEUE_PI,
		VAL, VAL2, uaddr2, *uaddr, (rc == -1) && (errno == EINVAL));
	printf("futex(%p, FUTEX_CMP_REQUEUE_PI_PRIVATE, %u, %u, %p, %u) = %s\n",
		uaddr, VAL_PR, VAL2_PR, uaddr2, *uaddr, sprintrc(rc));

	CHECK_INVALID_CLOCKRT(FUTEX_CMP_REQUEUE_PI, ARG3 | ARG4 | ARG5 | ARG6,
		"%u", "%u", "%#lx", "%u");

	/* FUTEX_LOCK_PI2 - same as FUTEX_LOCK_PI, but with CLOCK_MONOTONIC
	 *                  instead of CLOCK_REALTIME.
	 * Possible flags: PRIVATE
	 * 1. uaddr   - futex address
	 * 2. op      - FUTEX_LOCK_PI2
	 * 3. val     - not used
	 * 4. timeout - timeout
	 * 5. uaddr2  - not used
	 * 6. val3    - not used
	 */

	*uaddr = getpid();

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_LOCK_PI2, VAL, tmout, uaddr2 + 1,
		VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_LOCK_PI2, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       uaddr + 1, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));

	CHECK_FUTEX_ENOSYS(uaddr + 1, FUTEX_PRIVATE_FLAG | FUTEX_LOCK_PI2, VAL,
		tmout, uaddr2 + 1, VAL3, (rc == -1) && (errno == EFAULT));
	printf("futex(%p, FUTEX_LOCK_PI2_PRIVATE, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       uaddr + 1, (long long) tmout->tv_sec,
	       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));


	/*
	 * Unknown commands
	 */

	CHECK_FUTEX(uaddr, 0xe, VAL, tmout + 1, uaddr2 + 1, VAL3,
		(rc == -1) && (errno == ENOSYS));
	printf("futex(%p, 0xe /* FUTEX_??? */, %u, %p, %p, %#x) = %s\n",
		uaddr, VAL_PR, tmout + 1, uaddr2 + 1, VAL3_PR, sprintrc(rc));

	CHECK_FUTEX(uaddr, 0xbefeeded, VAL, tmout + 1, uaddr2, VAL3,
		(rc == -1) && (errno == ENOSYS));
	printf("futex(%p, 0xbefeeded /* FUTEX_??? */, %u, %p, %p, %#x) = %s\n",
		uaddr, VAL_PR, tmout + 1, uaddr2, VAL3_PR, sprintrc(rc));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_futex")

#endif
