# Automake input for strace tests.

OS = linux
ARCH = @arch@
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_builddir)/$(OS)/$(ARCH) \
	      -I$(top_srcdir)/$(OS)/$(ARCH) \
	      -I$(top_builddir)/$(OS) \
	      -I$(top_srcdir)/$(OS)

check_PROGRAMS = \
	caps \
	fanotify_mark \
	getrandom \
	inet-accept-connect-send-recv \
	ioctl \
	ipc_msg \
	ipc_sem \
	ipc_shm \
	mmap \
	mmap64 \
	mmsg \
	net-accept-connect \
	netlink_inet_diag \
	netlink_unix_diag \
	pc \
	pipe \
	scm_rights \
	seccomp \
	select \
	set_ptracer_any \
	sigaction \
	sigreturn \
	stack-fcall \
	stat \
	stat32 \
	statfs \
	uid \
	uid16 \
	uid32 \
	uio \
	umovestr \
	umovestr2 \
	unix-pair-send-recv

mmap64_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
pc_LDADD = $(dl_LIBS)
stat_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
uio_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

TESTS = \
	strace-f.test \
	qual_syscall.test \
	bexecve.test \
	caps.test \
	dumpio.test \
	fanotify_mark.test \
	getdents.test \
	getrandom.test \
	ioctl.test \
	ipc_msg.test \
	ipc_shm.test \
	ipc_sem.test \
	scm_rights-fd.test \
	seccomp.test \
	select.test \
	sigaction.test \
	sigreturn.test \
	stat.test \
	stat32-v.test \
	stat64-v.test \
	statfs.test \
	mmap.test \
	mmap64.test \
	mmsg.test \
	net.test \
	net-fd.test \
	net-yy.test \
	pipe.test \
	pc.test \
	sun_path.test \
	umovestr.test \
	umovestr2.test \
	unix-yy.test \
	uid.test \
	uid16.test \
	uid32.test \
	uio.test \
	count.test \
	detach-sleeping.test \
	detach-stopped.test \
	detach-running.test \
	restart_syscall.test \
	strace-k.test

net-fd.log: net.log

TEST_LOG_COMPILER = env
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh match.awk \
	     caps.awk \
	     dumpio.expected \
	     fanotify_mark.expected \
	     getdents.awk \
	     getdents.out \
	     getrandom.awk \
	     ioctl.expected \
	     ipc.sh \
	     mmsg.expected \
	     net.expected \
	     net-fd.expected \
	     net-yy-accept.awk \
	     net-yy-connect.awk \
	     pipe.expected \
	     restart_syscall.expected \
	     restart_syscall_unknown.expected \
	     select.awk \
	     sigaction.awk \
	     statfs.expected \
	     sun_path.expected \
	     uid.awk \
	     uio.expected \
	     umovestr.expected \
	     unix-yy-accept.awk \
	     unix-yy-connect.awk \
	     $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
