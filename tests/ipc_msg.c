/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

#include "xlat.h"
#include "xlat/resource_flags.h"

#ifndef MSG_STAT_ANY
# define MSG_STAT_ANY 13
#endif

#undef TEST_MSGCTL_BOGUS_ADDR
#undef TEST_MSGCTL_BOGUS_CMD

/*
 * Starting with commit glibc-2.32~83, on every 32-bit architecture
 * where 32-bit time_t support is enabled, glibc tries to retrieve
 * the data provided in the third argument of msgctl call.
 */
#if GLIBC_PREREQ_GE(2, 32) && defined __TIMESIZE && __TIMESIZE != 64
# define TEST_MSGCTL_BOGUS_ADDR 0
#endif
/*
 * Starting with commit glibc-2.31~358, on every architecture where
 * __ASSUME_SYSVIPC_BROKEN_MODE_T is defined, glibc tries to modify
 * the data provided in the third argument of msgctl call.
 */
#if GLIBC_PREREQ_GE(2, 31) && \
 (defined __m68k__ || defined __s390__ || \
  (defined WORDS_BIGENDIAN && \
   (defined __arm__ || defined __microblaze__ || defined __sh__)))
# define TEST_MSGCTL_BOGUS_ADDR 0
#endif
/*
 * Before glibc-2.22-122-gbe48165, ppc64 code tried to retrieve data
 * provided in third argument of msgctl call (in case of IPC_SET cmd)
 * which led to segmentation fault.
 */
#if GLIBC_PREREQ_LT(2, 23) && (defined POWERPC64 || defined POWERPC64LE)
# define TEST_MSGCTL_BOGUS_ADDR 0
#endif

/*
 * Starting with commit glibc-2.32.9000-149-gbe9b0b9a012780a403a2,
 * glibc skips msgctl syscall invocations and returns EINVAL
 * for invalid msgctl commands.
 *
 * Apparently, this change was later backported to vendor packages, e.g.:
 * Thu Mar 18 2021 Carlos O'Donell <carlos@redhat.com> - 2.28-153
 * - Support SEM_STAT_ANY via semctl. Return EINVAL for unknown commands
 *   to semctl, msgctl, and shmctl. (#1912670)
 */
#if GLIBC_PREREQ_GE(2, 28)
# define TEST_MSGCTL_BOGUS_CMD 0
#endif

#ifndef TEST_MSGCTL_BOGUS_ADDR
# define TEST_MSGCTL_BOGUS_ADDR 1
#endif
#ifndef TEST_MSGCTL_BOGUS_CMD
# define TEST_MSGCTL_BOGUS_CMD 1
#endif

#if XLAT_RAW
# define str_ipc_excl_nowait "0xface1c00"
# define str_ipc_private "0"
# define str_ipc_rmid "0"
# define str_ipc_set "0x1"
# define str_ipc_stat "0x2"
# define str_ipc_info "0x3"
# define str_msg_stat "0xb"
# define str_msg_info "0xc"
# define str_msg_stat_any "0xd"
# define str_ipc_64 "0x100"
# define str_bogus_cmd "0xdeadbeef"
#elif XLAT_VERBOSE
# define str_ipc_excl_nowait \
	"0xface1c00 /\\* IPC_EXCL\\|IPC_NOWAIT\\|0xface1000 \\*/"
# define str_ipc_private "0 /\\* IPC_PRIVATE \\*/"
# define str_ipc_rmid "0 /\\* IPC_RMID \\*/"
# define str_ipc_set "0x1 /\\* IPC_SET \\*/"
# define str_ipc_stat "0x2 /\\* IPC_STAT \\*/"
# define str_ipc_info "0x3 /\\* IPC_INFO \\*/"
# define str_msg_stat "0xb /\\* MSG_STAT \\*/"
# define str_msg_info "0xc /\\* MSG_INFO \\*/"
# define str_msg_stat_any "0xd /\\* MSG_STAT_ANY \\*/"
# define str_ipc_64 "0x100 /\\* IPC_64 \\*/"
# define str_bogus_cmd "0xdeadbeef /\\* MSG_\\?\\?\\? \\*/"
#else
# define str_ipc_excl_nowait "IPC_EXCL\\|IPC_NOWAIT\\|0xface1000"
# define str_ipc_private "IPC_PRIVATE"
# define str_ipc_rmid "IPC_RMID"
# define str_ipc_set "IPC_SET"
# define str_ipc_stat "IPC_STAT"
# define str_ipc_info "IPC_INFO"
# define str_msg_stat "MSG_STAT"
# define str_msg_info "MSG_INFO"
# define str_msg_stat_any "MSG_STAT_ANY"
# define str_ipc_64 "IPC_64"
# define str_bogus_cmd "0xdeadbeef /\\* MSG_\\?\\?\\? \\*/"
#endif

static int id = -1;

static void
cleanup(void)
{
	msgctl(id, IPC_RMID, NULL);
	printf("msgctl\\(%d, (%s\\|)?%s, NULL\\) += 0\n",
	       id, str_ipc_64, str_ipc_rmid);
	id = -1;
}

static void
print_msginfo(const char *const str_ipc_cmd,
	      const struct msginfo *const info,
	      const int rc)
{
	if (rc < 0) {
		printf("msgctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, info, sprintrc_grep(rc));
		return;
	}

	printf("msgctl\\(%d, (%s\\|)?%s, \\{msgpool=%d, msgmap=%d, msgmax=%d"
	       ", msgmnb=%d, msgmni=%d, msgssz=%d, msgtql=%d, msgseg=%u\\}\\)"
	       " = %d\n",
	       id,
	       str_ipc_64,
	       str_ipc_cmd,
	       info->msgpool,
	       info->msgmap,
	       info->msgmax,
	       info->msgmnb,
	       info->msgmni,
	       info->msgssz,
	       info->msgtql,
	       (unsigned) info->msgseg,
	       rc);
}

static void
print_msqid_ds(const char *const str_ipc_cmd,
	       const struct msqid_ds *const ds,
	       const int rc)
{
	if (rc < 0) {
		printf("msgctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, ds, sprintrc_grep(rc));
		return;
	}

	printf("msgctl\\(%d, (%s\\|)?%s, \\{msg_perm=\\{uid=%u"
	       ", gid=%u, mode=%#o, key=%u, cuid=%u, cgid=%u\\}"
	       ", msg_stime=%u, msg_rtime=%u, msg_ctime=%u, msg_qnum=%u"
	       ", msg_qbytes=%u, msg_lspid=%d, msg_lrpid=%d\\}\\) = %d\n",
	       id,
	       str_ipc_64,
	       str_ipc_cmd,
	       (unsigned) ds->msg_perm.uid,
	       (unsigned) ds->msg_perm.gid,
	       (unsigned) ds->msg_perm.mode,
	       (unsigned) ds->msg_perm.__key,
	       (unsigned) ds->msg_perm.cuid,
	       (unsigned) ds->msg_perm.cgid,
	       (unsigned) ds->msg_stime,
	       (unsigned) ds->msg_rtime,
	       (unsigned) ds->msg_ctime,
	       (unsigned) ds->msg_qnum,
	       (unsigned) ds->msg_qbytes,
	       (int) ds->msg_lspid,
	       (int) ds->msg_lrpid,
	       rc);
}

int
main(void)
{
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb9f531ULL;
	static const int bogus_flags = 0xface1e55 & ~IPC_CREAT;
#if TEST_MSGCTL_BOGUS_CMD || TEST_MSGCTL_BOGUS_ADDR
	static const int bogus_msgid = 0xfdb97531;
#endif
#if TEST_MSGCTL_BOGUS_CMD
	static const int bogus_cmd = 0xdeadbeef;
#endif
#if TEST_MSGCTL_BOGUS_ADDR
	static void * const bogus_addr = (void *) -1L;
#endif

	int rc;
	union {
		struct msqid_ds ds;
		struct msginfo info;
	} buf;

	rc = msgget(bogus_key, bogus_flags);
	printf("msgget\\(%#llx, %s\\|%#04o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key),
	       str_ipc_excl_nowait,
	       bogus_flags & 0777, sprintrc_grep(rc));

	id = msgget(private_key, 0600);
	if (id < 0)
		perror_msg_and_skip("msgget");
	printf("msgget\\(%s, 0600\\) = %d\n", str_ipc_private, id);
	atexit(cleanup);

#if TEST_MSGCTL_BOGUS_CMD
	rc = msgctl(bogus_msgid, bogus_cmd, NULL);
	printf("msgctl\\(%d, (%s\\|)?%s, NULL\\) = %s\n",
	       bogus_msgid, str_ipc_64, str_bogus_cmd, sprintrc_grep(rc));
#endif

#if TEST_MSGCTL_BOGUS_ADDR
	rc = msgctl(bogus_msgid, IPC_SET, bogus_addr);
	printf("msgctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
	       bogus_msgid, str_ipc_64, str_ipc_set, bogus_addr,
	       sprintrc_grep(rc));
#endif

	rc = msgctl(id, IPC_STAT, &buf.ds);
	if (rc < 0)
		perror_msg_and_skip("msgctl IPC_STAT");
	print_msqid_ds(str_ipc_stat, &buf.ds, rc);

	if (msgctl(id, IPC_SET, &buf.ds))
		perror_msg_and_skip("msgctl IPC_SET");
	printf("msgctl\\(%d, (%s\\|)?%s, \\{msg_perm=\\{uid=%u"
	       ", gid=%u, mode=%#o\\}, msg_qbytes=%u\\}\\) = 0\n",
	       id, str_ipc_64, str_ipc_set,
	       (unsigned) buf.ds.msg_perm.uid,
	       (unsigned) buf.ds.msg_perm.gid,
	       (unsigned) buf.ds.msg_perm.mode,
	       (unsigned) buf.ds.msg_qbytes);

	rc = msgctl(id, IPC_INFO, &buf.ds);
	print_msginfo(str_ipc_info, &buf.info, rc);

	rc = msgctl(id, MSG_INFO, &buf.ds);
	print_msginfo(str_msg_info, &buf.info, rc);

	rc = msgctl(id, MSG_STAT, &buf.ds);
	print_msqid_ds(str_msg_stat, &buf.ds, rc);

	rc = msgctl(id, MSG_STAT_ANY, &buf.ds);
	print_msqid_ds(str_msg_stat_any, &buf.ds, rc);

	return 0;
}
