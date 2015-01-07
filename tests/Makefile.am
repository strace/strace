# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = \
	inet-accept-connect-send-recv \
	caps \
	mmsg \
	net-accept-connect \
	netlink_inet_diag \
	netlink_unix_diag \
	scm_rights \
	set_ptracer_any \
	sigaction \
	stack-fcall \
	stat \
	stat32 \
	statfs \
	uid \
	uid16 \
	uid32 \
	uio

stat_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
uio_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

TESTS = \
	ptrace_setoptions.test \
	strace-f.test \
	qual_syscall.test \
	caps.test \
	getdents.test \
	scm_rights-fd.test \
	sigaction.test \
	stat.test \
	stat32-v.test \
	stat64-v.test \
	statfs.test \
	mmsg.test \
	net.test \
	net-fd.test \
	net-yy.test \
	unix-yy.test \
	uid.test \
	uid16.test \
	uid32.test \
	uio.test \
	count.test \
	detach-sleeping.test \
	detach-stopped.test \
	detach-running.test \
	strace-k.test

net-fd.log: net.log

TEST_LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh \
	     caps.awk \
	     getdents.awk \
	     mmsg.expected \
	     net-yy-accept.awk \
	     net-yy-connect.awk \
	     sigaction.awk \
	     uid.awk \
	     unix-yy-accept.awk \
	     unix-yy-connect.awk \
	     $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
