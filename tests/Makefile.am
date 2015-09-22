# Automake input for strace tests.

OS = linux
ARCH = @arch@
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_builddir)/$(OS)/$(ARCH) \
	      -I$(top_srcdir)/$(OS)/$(ARCH) \
	      -I$(top_builddir)/$(OS) \
	      -I$(top_srcdir)/$(OS) \
	      -I$(top_builddir) \
	      -I$(top_srcdir)

check_PROGRAMS = \
	adjtimex \
	aio \
	bpf \
	caps \
	clock_nanosleep \
	clock_xettime \
	epoll_create1 \
	eventfd \
	execve \
	execveat \
	fanotify_mark \
	filter-unavailable \
	getrandom \
	inet-accept-connect-send-recv \
	ioctl \
	ip_mreq \
	ipc_msg \
	ipc_msgbuf \
	ipc_sem \
	ipc_shm \
	memfd_create \
	mmap \
	mmap64 \
	mmsg \
	mq \
	nanosleep \
	net-accept-connect \
	netlink_inet_diag \
	netlink_unix_diag \
	oldselect \
	pc \
	pipe \
	ppoll \
	pselect6 \
	readlink \
	readlinkat \
	restart_syscall \
	rt_sigqueueinfo \
	sched_xetattr \
	scm_rights \
	seccomp \
	select \
	sendfile \
	sendfile64 \
	set_ptracer_any \
	sigaction \
	sigaltstack \
	signalfd \
	sigreturn \
	stack-fcall \
	stat \
	stat32 \
	statfs \
	sysinfo \
	time \
	timer_create \
	timer_xettime \
	timerfd_xettime \
	times \
	times-fail \
	uid \
	uid16 \
	uid32 \
	uio \
	umovestr \
	umovestr2 \
	unix-pair-send-recv \
	utime \
	utimensat \
	wait \
	xattr \
	xet_robust_list \
	xetitimer \
	xettimeofday \
	# end of check_PROGRAMS

clock_xettime_LDADD = -lrt
filter_unavailable_LDADD = -lpthread
mmap64_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
mq_LDADD = -lrt
pc_LDADD = $(dl_LIBS)
stat_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
times_LDADD = -lrt
uio_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64
stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

TESTS = \
	strace-f.test \
	qual_syscall.test \
	bexecve.test \
	adjtimex.test \
	aio.test \
	bpf.test \
	caps.test \
	clock_nanosleep.test \
	clock_xettime.test \
	dumpio.test \
	epoll_create1.test \
	eventfd.test \
	execve.test \
	execveat.test \
	fanotify_mark.test \
	filter-unavailable.test \
	getdents.test \
	getrandom.test \
	ioctl.test \
	ip_mreq.test \
	ipc_msg.test \
	ipc_msgbuf.test \
	ipc_shm.test \
	ipc_sem.test \
	mq.test \
	pselect6.test \
	readlink.test \
	readlinkat.test \
	rt_sigqueueinfo.test \
	sched_xetattr.test \
	scm_rights-fd.test \
	seccomp.test \
	select.test \
	sendfile.test \
	sendfile64.test \
	sigaction.test \
	sigaltstack.test \
	signalfd.test \
	sigreturn.test \
	stat.test \
	stat32-v.test \
	stat64-v.test \
	statfs.test \
	sysinfo.test \
	memfd_create.test \
	mmap.test \
	mmap64.test \
	mmsg.test \
	nanosleep.test \
	net.test \
	net-fd.test \
	net-yy.test \
	oldselect.test \
	pipe.test \
	pc.test \
	ppoll.test \
	sun_path.test \
	time.test \
	timer_create.test \
	timer_xettime.test \
	timerfd_xettime.test \
	times.test \
	times-fail.test \
	umovestr.test \
	umovestr2.test \
	unix-yy.test \
	uid.test \
	uid16.test \
	uid32.test \
	uio.test \
	utime.test \
	utimensat.test \
	wait.test \
	xattr.test \
	xet_robust_list.test \
	xetitimer.test \
	xettimeofday.test \
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
	     epoll_create1.expected \
	     eventfd.expected \
	     execve.expected \
	     execve-v.expected \
	     execveat.expected \
	     execveat-v.expected \
	     fanotify_mark.expected \
	     filter-unavailable.expected \
	     getdents.awk \
	     getdents.out \
	     ip_mreq.expected \
	     ipc.sh \
	     ipc_msgbuf.expected \
	     memfd_create.expected \
	     mmsg.expected \
	     mq.expected \
	     net.expected \
	     net-fd.expected \
	     net-yy-accept.awk \
	     net-yy-connect.awk \
	     oldselect.expected \
	     pipe.expected \
	     ppoll.expected \
	     ppoll-v.expected \
	     sigaction.awk \
	     sigaltstack.expected \
	     signalfd.expected \
	     statfs.expected \
	     sun_path.expected \
	     uid.awk \
	     uio.expected \
	     umovestr.expected \
	     unix-yy-accept.awk \
	     unix-yy-connect.awk \
	     wait.expected \
	     xattr.expected \
	     $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
