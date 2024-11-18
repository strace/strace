/*
 * Check decoding of SECCOMP_IOCTL_* commands of ioctl syscall.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "pidns.h"
#include "scno.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kernel_fcntl.h"

#include <linux/ioctl.h>
#include <linux/seccomp.h>

#include "cur_audit_arch.h"

#include "xlat.h"
#define XLAT_MACROS_ONLY
# include "xlat/elf_em.h"
#undef XLAT_MACROS_ONLY
#include "xlat/audit_arch.h"

#ifndef INJECT_RETVAL
# define INJECT_RETVAL 0
#endif
#ifndef PRINT_PATHS
# define PRINT_PATHS 0
#endif

#if INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

static const char null_path[] = "/dev/null";
static const char zero_path[] = "/dev/zero";

#define NULL_FD 0
#define ZERO_FD 42

#if PRINT_PATHS
# define PATH_FMT "<%s>"
#else
# define PATH_FMT "%s"
#endif

/**
 * Generate an ioctl command with a different direction based
 * on the existing one.
 */
#define IOC_ANOTHER_DIR(nr_, dir_) \
	_IOC(dir_, _IOC_TYPE(nr_), _IOC_NR(nr_), _IOC_SIZE(nr_))

static const char *errstr;

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	const long rc = syscall(__NR_ioctl, fd, cmd, arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(int argc, char **argv)
{
	static const struct strval32 dirs[] = {
		{ ARG_STR(_IOC_NONE) },
		{ ARG_STR(_IOC_READ) },
		{ ARG_STR(_IOC_WRITE) },
		{ ARG_STR(_IOC_READ|_IOC_WRITE) },
	};
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc;

	PIDNS_TEST_INIT;

	/*
	 * Start of output marker.  printf is in front of ioctl() here because
	 * musl calls an ioctl before the first output to stdout, specifically,
	 * ioctl(TIOCGWINSZ) in src/stdio/__stdout_write.c:__stdout_write.
	 */
	pidns_print_leader();
	errno = EBADF;
	printf("ioctl(-1, " XLAT_FMT ", NULL)" RVAL_EBADF,
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	fflush(NULL);
	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, 0);


#if INJECT_RETVAL
	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);
	long inject_retval = strtol(argv[2], NULL, 0);
	bool locked = false;

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned long i = 0; i < num_skip; i++) {
		rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, 0);
		pidns_print_leader();
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s%s\n",
		       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV),
		       errstr, rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked) {
		error_msg_and_fail("Have not locked on ioctl(-1"
				   ", SECCOMP_IOCTL_NOTIF_RECV, NULL) "
				   "returning %lu", inject_retval);
	}
#endif /* INJECT_RETVAL */

	/* Unknown seccomp ioctl */
	for (size_t i = 0; i < ARRAY_SIZE(dirs); i++) {
		for (unsigned int j = 0; j < 32; j += 4) {
			sys_ioctl(-1, _IOC(dirs[i].val, '!', 5, j), magic);
			pidns_print_leader();
			printf("ioctl(-1, "
			       XLAT_KNOWN(%#x, "_IOC(%s, 0x21, 0x5, %#x)")
			       ", %#lx) = %s" INJ_STR "\n",
#if XLAT_RAW || XLAT_VERBOSE
			       (unsigned int) _IOC(dirs[i].val, '!', 5, j),
#endif
#if !XLAT_RAW
			       dirs[i].str, j,
#endif
			       (unsigned long) magic, errstr);
		}
	}


	/* SECCOMP_IOCTL_NOTIF_RECV */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct seccomp_notif, notif);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, 0);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV), errstr);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif + 1);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV),
	       (char *) notif + 1, errstr);

	memset(notif, 0, sizeof(*notif));
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	if (rc >= 0) {
		printf("{id=0, pid=0, flags=0, data={nr=0, arch="
		       XLAT_UNKNOWN(0, "AUDIT_ARCH_???")
		       ", instruction_pointer=NULL, args=[0, 0, 0, 0, 0, 0]}}");
	} else {
		printf("%p", notif);
	}
	printf(") = %s" INJ_STR "\n", errstr);

	notif->id = 0xdeadc0debadc0dedULL;
	notif->pid = getpid();
	notif->flags = 0xdeefaced;
	notif->data.nr = 0xbad5ca11;
	notif->data.arch = 0xfeedface;
	notif->data.instruction_pointer = (uintptr_t) sys_ioctl;
	for (size_t i = 0; i < ARRAY_SIZE(notif->data.args); i++)
		notif->data.args[i] = 0xdeadfacebadc0dedULL ^ i;
#define ARGS_STR "args=[0xdeadfacebadc0ded, 0xdeadfacebadc0dec" \
			", 0xdeadfacebadc0def, 0xdeadfacebadc0dee" \
			", 0xdeadfacebadc0de9, 0xdeadfacebadc0de8]" \
	/* End of ARGS_STR definition */
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	for (size_t i = 0; i < 2; i++) {
		if (i)
			printf(" => ");
		if (!i || (rc >= 0)) {
			printf("{id=0xdeadc0debadc0ded, pid=%d%s"
			       ", flags=0xdeefaced, data={nr=3134573073, arch="
			       XLAT_UNKNOWN(0xfeedface, "AUDIT_ARCH_???")
			       ", instruction_pointer=%p, " ARGS_STR "}}",
			       getpid(), pidns_pid2str(PT_TGID), sys_ioctl);
		} else {
			printf("%p", notif);
		}
	}
	printf(") = %s" INJ_STR "\n", errstr);

#ifdef CUR_AUDIT_ARCH
	notif->id = 0;
	notif->flags = 0;
	notif->data.nr = __NR_gettid;
	notif->data.arch = CUR_AUDIT_ARCH;
	notif->data.instruction_pointer = 0;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	for (size_t i = 0; i < 2; i++) {
		if (i)
			printf(" => ");
		if (!i || (rc >= 0)) {
			printf("{id=0, pid=%d%s, flags=0, data={nr=" XLAT_FMT_U
			       ", arch=%s, instruction_pointer=NULL, " ARGS_STR
			       "}}",
			       getpid(), pidns_pid2str(PT_TGID),
			       XLAT_ARGS(__NR_gettid),
			       sprintxval(audit_arch, CUR_AUDIT_ARCH,
					  "AUDIT_ARCH_???"));
		} else {
			printf("%p", notif);
		}
	}
	printf(") = %s" INJ_STR "\n", errstr);
# if defined(PERS0_AUDIT_ARCH)
	notif->data.nr = PERS0__NR_gettid;
	notif->data.arch = PERS0_AUDIT_ARCH;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	for (size_t i = 0; i < 2; i++) {
		if (i)
			printf(" => ");
		if (!i || (rc >= 0)) {
			printf("{id=0, pid=%d%s, flags=0, data={nr=%u"
			       NRAW(" /* gettid */") ", arch=%s"
			       ", instruction_pointer=NULL, " ARGS_STR "}}",
			       getpid(), pidns_pid2str(PT_TGID),
			       PERS0__NR_gettid,
			       sprintxval(audit_arch, PERS0_AUDIT_ARCH,
					  "AUDIT_ARCH_???"));
		} else {
			printf("%p", notif);
		}
	}
	printf(") = %s" INJ_STR "\n", errstr);
# endif
# if defined(M32_AUDIT_ARCH)
	notif->data.nr = M32__NR_gettid;
	notif->data.arch = M32_AUDIT_ARCH;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	for (size_t i = 0; i < 2; i++) {
		if (i)
			printf(" => ");
		if (!i || (rc >= 0)) {
			printf("{id=0, pid=%d%s, flags=0, data={nr=%u"
			       NRAW(" /* gettid */") ", arch=%s"
			       ", instruction_pointer=NULL, " ARGS_STR "}}",
			       getpid(), pidns_pid2str(PT_TGID), M32__NR_gettid,
			       sprintxval(audit_arch, M32_AUDIT_ARCH,
					  "AUDIT_ARCH_???"));
		} else {
			printf("%p", notif);
		}
	}
	printf(") = %s" INJ_STR "\n", errstr);
# endif
# if defined(MX32_AUDIT_ARCH)
	notif->data.nr = MX32__NR_gettid;
	notif->data.arch = MX32_AUDIT_ARCH;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_RECV, (uintptr_t) notif);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", ",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_RECV));
	for (size_t i = 0; i < 2; i++) {
		if (i)
			printf(" => ");
		if (!i || (rc >= 0)) {
			printf("{id=0, pid=%d%s, flags=0, data={nr=%u"
			       NRAW(" /* gettid */") ", arch=%s"
			       ", instruction_pointer=NULL, " ARGS_STR "}}",
			       getpid(), pidns_pid2str(PT_TGID),
			       MX32__NR_gettid,
			       sprintxval(audit_arch, MX32_AUDIT_ARCH,
					  "AUDIT_ARCH_???"));
		} else {
			printf("%p", notif);
		}
	}
	printf(") = %s" INJ_STR "\n", errstr);
# endif
#endif /* CUR_AUDIT_ARCH */


	/* SECCOMP_IOCTL_NOTIF_SEND */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct seccomp_notif_resp, resp);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SEND, 0);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SEND), errstr);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SEND, (uintptr_t) resp + 1);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SEND),
	       (char *) resp + 1, errstr);

	memset(resp, 0, sizeof(*resp));
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SEND, (uintptr_t) resp);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0, val=0, error=0, flags=0}) = %s"
	       INJ_STR "\n", XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SEND), errstr);

	resp->id = 0xdeadc0debadc0dedULL;
	resp->val = 0xdadfacedbeefdeedULL;
	resp->error = 0xbadc0ded;
	resp->flags = 0xfacecafe;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SEND, (uintptr_t) resp);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0xdeadc0debadc0ded"
	       ", val=-2675229516524167443, error=-1159983635, flags=0xfacecafe"
	       NRAW(" /* SECCOMP_USER_NOTIF_FLAG_??? */") "}) = %s"
	       INJ_STR "\n", XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SEND), errstr);

	resp->error = -ENOSR;
	resp->flags = 1;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SEND, (uintptr_t) resp);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0xdeadc0debadc0ded"
	       ", val=-2675229516524167443, error=" XLAT_FMT_D
	       ", flags=" XLAT_KNOWN(0x1, "SECCOMP_USER_NOTIF_FLAG_CONTINUE")
	       "}) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SEND),
	       XLAT_SEL(-ENOSR, "-ENOSR"), errstr);


	/* SECCOMP_IOCTL_NOTIF_ID_VALID */
	static const struct {
		uint32_t id;
		const char *str;
	} id_valid_cmds[] = {
		{ ARG_STR(SECCOMP_IOCTL_NOTIF_ID_VALID) },
		{ IOC_ANOTHER_DIR(SECCOMP_IOCTL_NOTIF_ID_VALID, _IOC_READ),
		  "SECCOMP_IOCTL_NOTIF_ID_VALID_WRONG_DIR" },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, id);

	for (size_t i = 0; i < ARRAY_SIZE(id_valid_cmds); i++) {

		sys_ioctl(-1, id_valid_cmds[i].id, 0);
		pidns_print_leader();
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
		       XLAT_SEL(id_valid_cmds[i].id, id_valid_cmds[i].str),
		       errstr);

		sys_ioctl(-1, id_valid_cmds[i].id, (uintptr_t) id + 1);
		pidns_print_leader();
		printf("ioctl(-1, " XLAT_FMT ", %p) = %s" INJ_STR "\n",
		       XLAT_SEL(id_valid_cmds[i].id, id_valid_cmds[i].str),
		       (char *) id + 1, errstr);

		memset(id, 0, sizeof(*id));
		rc = sys_ioctl(-1, id_valid_cmds[i].id, (uintptr_t) id);
		pidns_print_leader();
		printf("ioctl(-1, " XLAT_FMT ", [0]) = %s" INJ_STR "\n",
		       XLAT_SEL(id_valid_cmds[i].id, id_valid_cmds[i].str),
		       errstr);

		*id = 0xdecaffedfacefeedULL;
		rc = sys_ioctl(-1, id_valid_cmds[i].id, (uintptr_t) id);
		pidns_print_leader();
		printf("ioctl(-1, " XLAT_FMT ", [0xdecaffedfacefeed]) = %s"
		       INJ_STR "\n",
		       XLAT_SEL(id_valid_cmds[i].id, id_valid_cmds[i].str),
		       errstr);
	}


	/* SECCOMP_IOCTL_NOTIF_ADDFD */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct seccomp_notif_addfd, addfd);

	close(0);
	int fd = open(null_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", null_path);
	if (fd != NULL_FD) {
		if (dup2(fd, NULL_FD) < 0)
			perror_msg_and_fail("dup2(fd, NULL_FD)");
		close(fd);
	}

	fd = open(zero_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", zero_path);
	if (fd != ZERO_FD) {
		if (dup2(fd, ZERO_FD) < 0)
			perror_msg_and_fail("dup2(fd, ZERO_FD)");
		close(fd);
	}

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_ADDFD, 0);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_ADDFD), errstr);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_ADDFD, (uintptr_t) addfd + 1);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_ADDFD),
	       (char *) addfd + 1, errstr);

	memset(addfd, 0, sizeof(*addfd));
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_ADDFD, (uintptr_t) addfd);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0, flags=0, srcfd=0" PATH_FMT
	       ", newfd=0, newfd_flags=0}) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_ADDFD),
	       PRINT_PATHS ? null_path : "", errstr);

	addfd->id = 0xdeadc0debadfacedULL;
	addfd->flags = 0xbadc0dec;
	addfd->srcfd = 0xdeadface;
	addfd->newfd = 0xbeeffeed;
	addfd->newfd_flags = O_CLOEXEC|O_DSYNC;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_ADDFD, (uintptr_t) addfd);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0xdeadc0debadfaced"
	       ", flags=0xbadc0dec" NRAW(" /* SECCOMP_ADDFD_FLAG_??? */")
	       ", srcfd=-559023410, newfd=-1091567891, newfd_flags=" XLAT_FMT
	       "}) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_ADDFD),
	       XLAT_ARGS(O_DSYNC|O_CLOEXEC), errstr);

	addfd->flags = 3;
	addfd->srcfd = ZERO_FD;
	addfd->newfd = 0xbeeffeed;
	addfd->newfd_flags = O_DIRECT;
	rc = sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_ADDFD, (uintptr_t) addfd);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", {id=0xdeadc0debadfaced"
	       ", flags=" XLAT_KNOWN(0x3, "SECCOMP_ADDFD_FLAG_SETFD"
					  "|SECCOMP_ADDFD_FLAG_SEND")
	       ", srcfd=%d" PATH_FMT ", newfd=-1091567891, newfd_flags="
	       XLAT_FMT "}) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_ADDFD),
	       ZERO_FD, PRINT_PATHS ? zero_path : "",
	       XLAT_ARGS(O_DIRECT), errstr);


	/* SECCOMP_IOCTL_NOTIF_SET_FLAGS */
	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SET_FLAGS, 0);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", 0) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SET_FLAGS), errstr);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SET_FLAGS, 1);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", " XLAT_FMT_L ") = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SET_FLAGS),
	       XLAT_ARGS(SECCOMP_USER_NOTIF_FD_SYNC_WAKE_UP), errstr);

	sys_ioctl(-1, SECCOMP_IOCTL_NOTIF_SET_FLAGS, 0xfffffffeU);
	pidns_print_leader();
	printf("ioctl(-1, " XLAT_FMT ", "
	       XLAT_UNKNOWN(0xfffffffe, "SECCOMP_USER_NOTIF_???")
	       ") = %s" INJ_STR "\n",
	       XLAT_ARGS_U(SECCOMP_IOCTL_NOTIF_SET_FLAGS), errstr);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
