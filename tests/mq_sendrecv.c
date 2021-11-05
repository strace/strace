/*
 * Check decoding of mq_open, mq_timedsend, mq_notify, mq_timedreceive and
 * mq_unlink syscalls.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_mq_timedsend && defined __NR_mq_timedreceive

# include <assert.h>
# include <errno.h>
# include <inttypes.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <unistd.h>

# include "xmalloc.h"
# include "kernel_fcntl.h"
# include "sigevent.h"

# ifndef DUMPIO_READ
#  define DUMPIO_READ 0
# endif

# ifndef DUMPIO_WRITE
#  define DUMPIO_WRITE 0
# endif

static char *mq_name;

enum {
	NUM_ATTRS = 8,
	MSG_CUT = 8,
	MSG_MAX_UNCUT = 32,
	MSG_SIZE = 64,
	MSG_START = 0x80,
};


static void
printstr(unsigned char start, unsigned int count)
{
	printf("\"");
	for (unsigned int i = 0; i < count; ++i) {
		printf("\\%hho", (unsigned char) (start + i));
	}
	printf("\"");
}

# if DUMPIO_READ || DUMPIO_WRITE
static void
dumpstr(unsigned char start, unsigned int count)
{
	for (unsigned int i = 0; i < count; ++i) {
		if (i < count) {
			if (!(i % 16))
				printf(" | %05x ", i);
			if (!(i % 8))
				printf(" ");

			printf("%02hhx ", (unsigned char) (start + i));
		}

		if ((i % 16 == 15) || (i == (count - 1))) {
			if (i % 16 != 15)
				printf("%*s", 3 * (15 - i % 16) +
				       ((i + 8) % 16) / 8, " ");

			printf(" ");

			for (unsigned int j = 0; j <= (i % 16); ++j)
				printf(".");
			for (unsigned int j = i % 16; j < 15; ++j)
				printf(" ");

			printf(" |\n");

		}
	}
}
# endif /* DUMPIO_READ || DUMPIO_WRITE */

static void
cleanup(void)
{
	long rc;

	rc = syscall(__NR_mq_unlink, mq_name);
	printf("mq_unlink(\"%s\") = %s\n", mq_name, sprintrc(rc));

	puts("+++ exited with 0 +++");
}

static void
do_send(int fd, char *msg, unsigned int msg_size, struct timespec *tmout,
	bool cropped)
{
	long rc;
	long saved_errno;

	do {
		rc = syscall(__NR_mq_timedsend, fd, msg, msg_size, 42,
			     tmout);
		saved_errno = errno;
		printf("mq_timedsend(%d, ", fd);
		printstr(MSG_START, msg_size > MSG_MAX_UNCUT ? MSG_MAX_UNCUT :
			 msg_size);
		if (cropped)
			printf("...");
		errno = saved_errno;
		printf(", %u, 42, {tv_sec=%lld, tv_nsec=%llu}) = %s\n", msg_size,
		       (long long) tmout->tv_sec,
		       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));
		errno = saved_errno;

		if (rc == -1) {
			if (errno == EINTR)
				continue;
			perror_msg_and_skip("mq_timedsend");
		}
# if DUMPIO_WRITE
		dumpstr(MSG_START, msg_size);
# endif
	} while (rc);
}

static void
do_recv(int fd, char *msg, unsigned int msg_size, struct timespec *tmout,
	bool cropped)
{
	long rc;
	long saved_errno;
	unsigned prio;

	do {
		rc = syscall(__NR_mq_timedreceive, fd, msg, MSG_SIZE, &prio,
			     tmout);
		saved_errno = errno;
		printf("mq_timedreceive(%d, ", fd);
		if (rc >= 0) {
			printstr(MSG_START, rc > MSG_MAX_UNCUT ? MSG_MAX_UNCUT :
				 rc);
			if (cropped)
				printf("...");
		} else {
			printf("%p", msg);
		}
		errno = saved_errno;
		printf(", %u, [42], {tv_sec=%lld, tv_nsec=%llu}) = %s\n", MSG_SIZE,
		       (long long) tmout->tv_sec,
		       zero_extend_signed_to_ull(tmout->tv_nsec), sprintrc(rc));
		errno = saved_errno;

		if (rc == -1) {
			if (errno == EINTR)
				continue;
			perror_msg_and_skip("mq_timedreceive");
		}
		if ((rc >= 0) && ((unsigned long) rc != msg_size))
			error_msg_and_skip("mq_timedreceive size mismatch"
					   ": expected %u, got %ld",
					   msg_size, rc);
# if DUMPIO_READ
		dumpstr(MSG_START, rc);
# endif
	} while (rc < 0);
}

int
main(void)
{
	static const kernel_ulong_t bogus_zero =
		(kernel_ulong_t) 0x8765432100000000ULL;
	static const kernel_ulong_t bogus_oflags =
		(kernel_ulong_t) 0xdefaced100000003ULL;
	static const kernel_ulong_t bogus_mode =
		(kernel_ulong_t) 0xdec0deadfacefeedULL;
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xfeedfacedeadba5eULL;
	static const kernel_ulong_t bogus_zero_size =
		(sizeof(kernel_ulong_t) > sizeof(int)) ? (kernel_ulong_t) 0 :
			(kernel_ulong_t) 0xface1e5500000000ULL;
	static const kernel_ulong_t bogus_size =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;
	static const kernel_ulong_t bogus_prio =
		(kernel_ulong_t) 0xdec0ded1defaced3ULL;
	static const struct timespec bogus_tmout_data = {
		.tv_sec = (time_t) 0xdeadfacebeeff00dLL,
		.tv_nsec = (long) 0xfacefee1deadfeedLL,
	};
	static const struct timespec future_tmout_data = {
		.tv_sec = (time_t) 0x7ea1fade7e57faceLL,
		.tv_nsec = 999999999,
	};
	struct_sigevent bogus_sev_data = {
		.sigev_notify = 0xdefaced,
		.sigev_signo = 0xfacefeed,
		.sigev_value.sival_ptr =
			(void *) (unsigned long) 0xdeadbeefbadc0dedULL
	};

	const char *errstr;
	long rc;
	kernel_long_t *bogus_attrs = tail_alloc(sizeof(*bogus_attrs) *
		NUM_ATTRS);
	char *msg = tail_alloc(MSG_SIZE);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned, bogus_prio_ptr);
	struct timespec *bogus_tmout = tail_memdup(&bogus_tmout_data,
		sizeof(*bogus_tmout));
	struct timespec *future_tmout = tail_memdup(&future_tmout_data,
		sizeof(*future_tmout));
	struct_sigevent *bogus_sev = tail_memdup(&bogus_sev_data,
		sizeof(*bogus_sev));
	int fd = -1;


	fill_memory_ex(msg, MSG_SIZE, MSG_START, MSG_SIZE);
	fill_memory_ex(bogus_attrs, sizeof(*bogus_attrs) * NUM_ATTRS,
		       0xbb, 0x70);


	/* mq_open */

	/* Zero values, non-O_CREAT mode */
	rc = syscall(__NR_mq_open, NULL, bogus_zero, bogus_mode, NULL);
	printf("mq_open(NULL, O_RDONLY) = %s\n", sprintrc(rc));

	/* O_CREAT parsing, other flags, bogs values */
	rc = syscall(__NR_mq_open, msg, O_CREAT | bogus_oflags, bogus_mode,
		     NULL);
	printf("mq_open(%p, O_ACCMODE|O_CREAT, %#o, NULL) = %s\n",
	       msg, (unsigned short) bogus_mode, sprintrc(rc));

	/* Partially invalid attributes structure */
	rc = syscall(__NR_mq_open, msg, O_CREAT | bogus_oflags, bogus_mode,
		     bogus_attrs + 1);
	printf("mq_open(%p, O_ACCMODE|O_CREAT, %#o, %p) = %s\n",
	       msg, (unsigned short) bogus_mode, bogus_attrs + 1, sprintrc(rc));

	/* Valid attributes structure */
	rc = syscall(__NR_mq_open, msg, O_CREAT | bogus_oflags, bogus_mode,
		     bogus_attrs);
	printf("mq_open(%p, O_ACCMODE|O_CREAT, %#o, {mq_flags=%#llx"
	       ", mq_maxmsg=%lld, mq_msgsize=%lld, mq_curmsgs=%lld}) = %s\n",
	       msg, (unsigned short) bogus_mode,
	       (unsigned long long) (kernel_ulong_t) bogus_attrs[0],
	       (long long) bogus_attrs[1],
	       (long long) bogus_attrs[2],
	       (long long) bogus_attrs[3], sprintrc(rc));


	/* mq_timedsend */

	/* Zero values*/
	rc = syscall(__NR_mq_timedsend, bogus_zero, NULL, bogus_zero_size,
		     bogus_zero, NULL);
	printf("mq_timedsend(0, NULL, 0, 0, NULL) = %s\n", sprintrc(rc));

	/* Invalid pointers */
	rc = syscall(__NR_mq_timedsend, bogus_fd, msg + MSG_SIZE, bogus_size,
		     bogus_prio, bogus_tmout + 1);
	printf("mq_timedsend(%d, %p, %llu, %u, %p) = %s\n",
	       (int) bogus_fd, msg + MSG_SIZE, (unsigned long long) bogus_size,
	       (unsigned) bogus_prio, bogus_tmout + 1, sprintrc(rc));

	/* Partially invalid message (memory only partially available) */
	rc = syscall(__NR_mq_timedsend, bogus_fd, msg + MSG_SIZE - MSG_CUT,
		     MSG_SIZE, bogus_prio, bogus_tmout);
	printf("mq_timedsend(%d, %p, %llu, %u, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       (int) bogus_fd, msg + MSG_SIZE - MSG_CUT,
	       (unsigned long long) MSG_SIZE, (unsigned) bogus_prio,
	       (long long) bogus_tmout->tv_sec,
	       zero_extend_signed_to_ull(bogus_tmout->tv_nsec), sprintrc(rc));

	/* Fully valid message, uncut */
	rc = syscall(__NR_mq_timedsend, bogus_fd, msg + MSG_SIZE - MSG_CUT,
		     MSG_CUT, bogus_prio, bogus_tmout);
	errstr = sprintrc(rc);
	printf("mq_timedsend(%d, ", (int) bogus_fd);
	printstr(MSG_START + MSG_SIZE - MSG_CUT, MSG_CUT);
	printf(", %llu, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       (unsigned long long) MSG_CUT, (unsigned) bogus_prio,
	       (long long) bogus_tmout->tv_sec,
	       zero_extend_signed_to_ull(bogus_tmout->tv_nsec), errstr);

	/* Partially invalid message, cut at maxstrlen */
	rc = syscall(__NR_mq_timedsend, bogus_fd, msg + MSG_CUT, MSG_SIZE,
		     bogus_prio, bogus_tmout);
	errstr = sprintrc(rc);
	printf("mq_timedsend(%d, ", (int) bogus_fd);
	printstr(MSG_START + MSG_CUT, MSG_MAX_UNCUT);
	printf("..., %llu, %u, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       (unsigned long long) MSG_SIZE, (unsigned) bogus_prio,
	       (long long) bogus_tmout->tv_sec,
	       zero_extend_signed_to_ull(bogus_tmout->tv_nsec), errstr);


	/* mq_timedreceive */

	/* Zero values */
	rc = syscall(__NR_mq_timedreceive, bogus_zero, NULL, bogus_zero_size,
		     NULL, NULL);
	printf("mq_timedreceive(0, NULL, 0, NULL, NULL) = %s\n", sprintrc(rc));

	/* Invalid addresses */
	rc = syscall(__NR_mq_timedreceive, bogus_fd, msg + MSG_SIZE, bogus_size,
		     bogus_prio_ptr + 1, bogus_tmout + 1);
	printf("mq_timedreceive(%d, %p, %llu, %p, %p) = %s\n",
	       (int) bogus_fd, msg + MSG_SIZE, (unsigned long long) bogus_size,
	       bogus_prio_ptr + 1, bogus_tmout + 1, sprintrc(rc));

	/* Invalid fd, valid msg pointer */
	rc = syscall(__NR_mq_timedreceive, bogus_fd, msg, bogus_size,
		     bogus_prio_ptr, bogus_tmout);
	printf("mq_timedreceive(%d, %p, %llu, %p, {tv_sec=%lld, tv_nsec=%llu}) "
	       "= %s\n",
	       (int) bogus_fd, msg, (unsigned long long) bogus_size,
	       bogus_prio_ptr, (long long) bogus_tmout->tv_sec,
	       zero_extend_signed_to_ull(bogus_tmout->tv_nsec), sprintrc(rc));


	/* mq_notify */

	/* Zero values */
	rc = syscall(__NR_mq_notify, bogus_zero, NULL);
	printf("mq_notify(0, NULL) = %s\n", sprintrc(rc));

	/* Invalid pointer */
	rc = syscall(__NR_mq_notify, bogus_fd, bogus_sev + 1);
	printf("mq_notify(%d, %p) = %s\n",
	       (int) bogus_fd, bogus_sev + 1, sprintrc(rc));

	/* Invalid SIGEV_* */
	rc = syscall(__NR_mq_notify, bogus_fd, bogus_sev);
	printf("mq_notify(%d, {sigev_value={sival_int=%d, sival_ptr=%p}"
	       ", sigev_signo=%u, sigev_notify=%#x /* SIGEV_??? */}) = %s\n",
	       (int) bogus_fd, bogus_sev->sigev_value.sival_int,
	       bogus_sev->sigev_value.sival_ptr,
	       bogus_sev->sigev_signo, bogus_sev->sigev_notify,
	       sprintrc(rc));

	/* SIGEV_NONE */
	bogus_sev->sigev_notify = SIGEV_NONE;
	rc = syscall(__NR_mq_notify, bogus_fd, bogus_sev);
	printf("mq_notify(%d, {sigev_value={sival_int=%d, sival_ptr=%p}"
	       ", sigev_signo=%u, sigev_notify=SIGEV_NONE}) = %s\n",
	       (int) bogus_fd, bogus_sev->sigev_value.sival_int,
	       bogus_sev->sigev_value.sival_ptr,
	       bogus_sev->sigev_signo, sprintrc(rc));

	/* SIGEV_SIGNAL */
	bogus_sev->sigev_notify = SIGEV_SIGNAL;
	bogus_sev->sigev_signo = SIGALRM;
	rc = syscall(__NR_mq_notify, bogus_fd, bogus_sev);
	printf("mq_notify(%d, {sigev_value={sival_int=%d, sival_ptr=%p}"
	       ", sigev_signo=SIGALRM, sigev_notify=SIGEV_SIGNAL}) = %s\n",
	       (int) bogus_fd, bogus_sev->sigev_value.sival_int,
	       bogus_sev->sigev_value.sival_ptr, sprintrc(rc));

	/* SIGEV_THREAD */
	bogus_sev->sigev_notify = SIGEV_THREAD;
	bogus_sev->sigev_un.sigev_thread.function =
		(void *) (unsigned long) 0xdeadbeefbadc0dedULL;
	bogus_sev->sigev_un.sigev_thread.attribute =
		(void *) (unsigned long) 0xcafef00dfacefeedULL;
	rc = syscall(__NR_mq_notify, bogus_fd, bogus_sev);
	printf("mq_notify(%d, {sigev_value={sival_int=%d, sival_ptr=%p}"
	       ", sigev_signo=SIGALRM, sigev_notify=SIGEV_THREAD"
	       ", sigev_notify_function=%p, sigev_notify_attributes=%p})"
	       " = %s\n",
	       (int) bogus_fd, bogus_sev->sigev_value.sival_int,
	       bogus_sev->sigev_value.sival_ptr,
	       bogus_sev->sigev_un.sigev_thread.function,
	       bogus_sev->sigev_un.sigev_thread.attribute, sprintrc(rc));

	/* mq_unlink */

	/* Zero values */
	rc = syscall(__NR_mq_unlink, NULL);
	printf("mq_unlink(NULL) = %s\n", sprintrc(rc));

	/* Invalid ptr */
	rc = syscall(__NR_mq_unlink, msg + MSG_SIZE);
	printf("mq_unlink(%p) = %s\n", msg + MSG_SIZE, sprintrc(rc));

	/* Long unterminated string */
	rc = syscall(__NR_mq_unlink, msg);
	errstr = sprintrc(rc);
	printf("mq_unlink(%p) = %s\n", msg, errstr);


	/* Sending and receiving test */

	mq_name = xasprintf("strace-mq_sendrecv-%u.sample", getpid());

# if DUMPIO_READ || DUMPIO_WRITE
	close(0);
# endif
	bogus_attrs[1] = 2;
	bogus_attrs[2] = MSG_SIZE;
	fd = rc = syscall(__NR_mq_open, mq_name,
			  O_CREAT|O_RDWR|O_NONBLOCK, 0700, bogus_attrs);
	errstr = sprintrc(rc);
	if (rc < 0)
		perror_msg_and_skip("mq_open");
	else
		atexit(cleanup);
# if DUMPIO_READ || DUMPIO_WRITE
	if (fd != 0)
		error_msg_and_skip("mq_open returned fd other than 0");
# endif
	fill_memory_ex(bogus_attrs, sizeof(*bogus_attrs) * NUM_ATTRS,
		       0xbb, 0x70);
	printf("mq_open(\"%s\", O_RDWR|O_CREAT|O_NONBLOCK, 0700"
	       ", {mq_flags=%#llx, mq_maxmsg=2, mq_msgsize=%u"
	       ", mq_curmsgs=%lld}) = %s\n",
	       mq_name, (unsigned long long) (kernel_ulong_t) bogus_attrs[0],
	       MSG_SIZE, (long long) bogus_attrs[3], errstr);

	rc = syscall(__NR_mq_getsetattr, fd, NULL, bogus_attrs);
	if (rc < 0)
		perror_msg_and_skip("mq_getsetattr");
	if ((bogus_attrs[1] < 2) || (bogus_attrs[2] < MSG_SIZE))
		error_msg_and_skip("mq too small");

	do_send(fd, msg, MSG_CUT, future_tmout, false);
	do_send(fd, msg, MSG_SIZE, future_tmout, true);

	memset(msg, '\0', MSG_SIZE);
	do_recv(fd, msg, MSG_CUT, future_tmout, false);

	memset(msg, '\0', MSG_SIZE);
	do_recv(fd, msg, MSG_SIZE, future_tmout, true);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mq_timedsend && __NR_mq_timedreceive")

#endif
