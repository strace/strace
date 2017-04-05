# Automake input for strace tests.
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

OS = linux
ARCH = @arch@
MPERS_NAME =
ARCH_MFLAGS =
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = $(ARCH_MFLAGS) \
	      -I$(builddir) \
	      -I$(top_builddir)/$(OS)/$(ARCH) \
	      -I$(top_srcdir)/$(OS)/$(ARCH) \
	      -I$(top_builddir)/$(OS) \
	      -I$(top_srcdir)/$(OS) \
	      -I$(top_builddir) \
	      -I$(top_srcdir)
AM_LDFLAGS = $(ARCH_MFLAGS)

libtests_a_SOURCES = \
	errno2name.c \
	error_msg.c \
	fill_memory.c \
	get_page_size.c \
	get_sigset_size.c \
	hexdump_strdup.c \
	hexquote_strndup.c \
	inode_of_sockfd.c \
	libmmsg.c \
	libsocketcall.c \
	overflowuid.c \
	pipe_maxfd.c \
	print_quoted_string.c \
	print_time.c \
	printflags.c \
	printxval.c \
	signal2name.c \
	sprintrc.c \
	tail_alloc.c \
	tests.h \
	tprintf.c \
	# end of libtests_a_SOURCES
libtests_a_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
check_LIBRARIES = libtests.a
LDADD = libtests.a

check_PROGRAMS = \
	_newselect \
	accept \
	accept4 \
	access \
	acct \
	add_key \
	adjtimex \
	aio \
	alarm \
	answer \
	attach-f-p \
	attach-f-p-cmd \
	attach-p-cmd-cmd \
	attach-p-cmd-p \
	bpf \
	brk \
	btrfs \
	caps \
	caps-abbrev \
	chmod \
	chown \
	chown32 \
	chroot \
	clock_adjtime \
	clock_nanosleep \
	clock_xettime \
	copy_file_range \
	count-f \
	creat \
	delete_module \
	dup \
	dup2 \
	dup3 \
	epoll_create \
	epoll_create1 \
	epoll_ctl \
	epoll_pwait \
	epoll_wait \
	erestartsys \
	eventfd \
	execve \
	execve-v \
	execveat \
	execveat-v \
	faccessat \
	fadvise64 \
	fadvise64_64 \
	fallocate \
	fanotify_init \
	fanotify_mark \
	fchdir \
	fchmod \
	fchmodat \
	fchown \
	fchown32 \
	fchownat \
	fcntl \
	fcntl64 \
	fdatasync \
	file_handle \
	file_ioctl \
	filter-unavailable \
	finit_module \
	flock \
	fork-f \
	fstat \
	fstat64 \
	fstatat64 \
	fstatfs \
	fstatfs64 \
	fsync \
	ftruncate \
	ftruncate64 \
	futex \
	futimesat \
	get_mempolicy \
	getcpu \
	getcwd \
	getdents \
	getdents64 \
	getegid \
	getegid32 \
	geteuid \
	geteuid32 \
	getgid \
	getgid32 \
	getgroups \
	getgroups32 \
	getpeername \
	getpgrp \
	getrandom \
	getresgid \
	getresgid32 \
	getresuid \
	getresuid32 \
	getrlimit \
	getrusage \
	getsid \
	getsockname \
	getuid \
	getuid32 \
	getxxid \
	inet-cmsg \
	init_module \
	inotify \
	inotify_init1 \
	int_0x80 \
	ioctl \
	ioctl_block \
	ioctl_dm \
	ioctl_dm-v \
	ioctl_evdev \
	ioctl_evdev-v \
	ioctl_loop \
	ioctl_loop-nv \
	ioctl_loop-v \
	ioctl_mtd \
	ioctl_rtc \
	ioctl_rtc-v \
	ioctl_scsi \
	ioctl_sg_io_v3 \
	ioctl_sg_io_v4 \
	ioctl_sock_gifconf \
	ioctl_uffdio \
	ioctl_v4l2 \
	ioperm \
	iopl \
	ioprio \
	ip_mreq \
	ipc \
	ipc_msg \
	ipc_msgbuf \
	ipc_sem \
	ipc_shm \
	kcmp \
	kexec_file_load \
	kexec_load \
	keyctl \
	kill \
	ksysent \
	lchown \
	lchown32 \
	link \
	linkat \
	llseek \
	lookup_dcookie \
	lseek \
	lstat \
	lstat64 \
	mbind \
	membarrier \
	memfd_create \
	migrate_pages \
	mincore \
	mkdir \
	mkdirat \
	mknod \
	mknodat \
	mlock \
	mlock2 \
	mlockall \
	mmap \
	mmap64 \
	mmsg \
	mmsg-silent \
	mmsg_name \
	mmsg_name-v \
	mount \
	move_pages \
	mq \
	mq_sendrecv \
	mq_sendrecv-read \
	mq_sendrecv-write \
	msg_control \
	msg_control-v \
	msg_name \
	munlockall \
	nanosleep \
	net-accept-connect \
	net-icmp_filter \
	net-sockaddr \
	net-y-unix \
	net-yy-inet \
	net-yy-netlink \
	net-yy-unix \
	netlink_inet_diag \
	netlink_netlink_diag \
	netlink_protocol \
	netlink_unix_diag \
	newfstatat \
	nsyscalls \
	old_mmap \
	oldfstat \
	oldlstat \
	oldselect \
	oldstat \
	open \
	openat \
	pause \
	pc \
	perf_event_open \
	perf_event_open_nonverbose \
	perf_event_open_unabbrev \
	personality \
	pipe \
	pipe2 \
	pkey_alloc \
	pkey_free \
	pkey_mprotect \
	poll \
	ppoll \
	ppoll-v \
	prctl-arg2-intptr \
	prctl-dumpable \
	prctl-name \
	prctl-no-args \
	prctl-pdeathsig \
	prctl-seccomp-filter-v \
	prctl-seccomp-strict \
	prctl-securebits \
	prctl-tid_address \
	prctl-tsc \
	pread64-pwrite64 \
	preadv \
	preadv-pwritev \
	preadv2-pwritev2 \
	print_maxfd \
	printstr \
	prlimit64 \
	process_vm_readv \
	process_vm_writev \
	pselect6 \
	ptrace \
	pwritev \
	qual_fault \
	qual_inject-error-signal \
	qual_inject-retval \
	qual_inject-signal \
	qual_signal \
	quotactl \
	quotactl-v \
	quotactl-xfs \
	quotactl-xfs-v \
	read-write \
	readahead \
	readdir \
	readlink \
	readlinkat \
	readv \
	reboot \
	recvfrom \
	recvmmsg-timeout \
	recvmsg \
	redirect-fds \
	remap_file_pages \
	rename \
	renameat \
	renameat2 \
	request_key \
	restart_syscall \
	rmdir \
	rt_sigpending \
	rt_sigprocmask \
	rt_sigqueueinfo \
	rt_sigreturn \
	rt_sigsuspend \
	rt_sigtimedwait \
	rt_tgsigqueueinfo \
	sched_get_priority_mxx \
	sched_rr_get_interval \
	sched_xetaffinity \
	sched_xetattr \
	sched_xetparam \
	sched_xetscheduler \
	sched_yield \
	scm_rights \
	seccomp-filter \
	seccomp-filter-v \
	seccomp-strict \
	select \
	semop \
	sendfile \
	sendfile64 \
	set_mempolicy \
	set_ptracer_any \
	setdomainname \
	setfsgid \
	setfsgid32 \
	setfsuid \
	setfsuid32 \
	setgid \
	setgid32 \
	setgroups \
	setgroups32 \
	sethostname \
	setns \
	setregid \
	setregid32 \
	setresgid \
	setresgid32 \
	setresuid \
	setresuid32 \
	setreuid \
	setreuid32 \
	setrlimit \
	setuid \
	setuid32 \
	shmxt \
	shutdown \
	sigaction \
	sigaltstack \
	siginfo \
	signal_receive \
	signalfd4 \
	sigreturn \
	sleep \
	socketcall \
	splice \
	stack-fcall \
	stat \
	stat64 \
	statfs \
	statfs64 \
	statx \
	swap \
	symlink \
	symlinkat \
	sync \
	sync_file_range \
	sync_file_range2 \
	sysinfo \
	syslog \
	tee \
	threads-execve \
	time \
	timer_create \
	timer_xettime \
	timerfd_xettime \
	times \
	times-fail \
	truncate \
	truncate64 \
	ugetrlimit \
	uio \
	umask \
	umount \
	umount2 \
	umoven-illptr \
	umovestr \
	umovestr-illptr \
	umovestr2 \
	umovestr3 \
	uname \
	unix-pair-send-recv \
	unix-pair-sendto-recvfrom \
	unlink \
	unlinkat \
	unshare \
	userfaultfd \
	ustat \
	utime \
	utimensat \
	utimes \
	vfork-f \
	vhangup \
	vmsplice \
	wait4 \
	wait4-v \
	waitid \
	waitid-v \
	waitpid \
	xattr \
	xattr-strings \
	xet_robust_list \
	xetitimer \
	xetpgid \
	xetpriority \
	xettimeofday \
	# end of check_PROGRAMS

attach_f_p_LDADD = -lrt -lpthread $(LDADD)
clock_xettime_LDADD = -lrt $(LDADD)
count_f_LDADD = -lpthread $(LDADD)
filter_unavailable_LDADD = -lpthread $(LDADD)
fstat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
fstatat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
ftruncate64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
lstat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mmap64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mq_LDADD = -lrt $(LDADD)
mq_sendrecv_LDADD = -lrt $(LDADD)
mq_sendrecv_read_LDADD = -lrt $(LDADD)
mq_sendrecv_write_LDADD = -lrt $(LDADD)
newfstatat_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
pc_LDADD = $(dl_LIBS) $(LDADD)
pread64_pwrite64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
stat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
threads_execve_LDADD = -lrt -lpthread $(LDADD)
times_LDADD = -lrt $(LDADD)
truncate64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
uio_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64

stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

include gen_tests.am

if USE_LIBUNWIND
LIBUNWIND_TESTS = strace-k.test
else
LIBUNWIND_TESTS =
endif

DECODER_TESTS = \
	brk.test \
	btrfs-v.test \
	btrfs-vw.test \
	btrfs-w.test \
	caps-abbrev.test \
	caps.test \
	eventfd.test \
	execve-v.test \
	execve.test \
	fadvise64.test \
	fstat.test \
	futex.test \
	getuid.test \
	ioctl.test \
	ioctl_dm-v.test \
	ioctl_dm.test \
	ioctl_evdev-v.test \
	ioctl_loop-nv.test \
	ioctl_loop-v.test \
	ioctl_rtc-v.test \
	ioctl_sock_gifconf.test \
	ipc_msgbuf.test \
	llseek.test \
	lseek.test \
	mmap.test \
	net-y-unix.test \
	net-yy-inet.test \
	net-yy-netlink.test \
	net-yy-unix.test \
	net.test \
	netlink_protocol.test \
	nsyscalls.test \
	oldselect.test \
	personality.test \
	pipe.test \
	poll.test \
	prctl-arg2-intptr.test \
	prctl-dumpable.test \
	prctl-name.test \
	prctl-no-args.test \
	prctl-pdeathsig.test \
	prctl-seccomp-filter-v.test \
	prctl-seccomp-strict.test \
	prctl-securebits.test \
	prctl-tid_address.test \
	prctl-tsc.test \
	pread64-pwrite64.test \
	qual_fault-exit_group.test \
	read-write.test \
	readv.test \
	sched.test \
	scm_rights-fd.test \
	seccomp-strict.test \
	shmxt.test \
	sigaction.test \
	sigaltstack.test \
	statfs.test \
	sun_path.test \
	uio.test \
	umount.test \
	umount2.test \
	umovestr.test \
	umovestr2.test \
	uname.test \
	unix-pair-send-recv.test \
	unix-pair-sendto-recvfrom.test \
	utimensat.test \
	xattr.test \
	# end of DECODER_TESTS

MISC_TESTS = \
	attach-f-p.test \
	attach-p-cmd.test \
	bexecve.test \
	count-f.test \
	count.test \
	detach-running.test \
	detach-sleeping.test \
	detach-stopped.test \
	filter-unavailable.test \
	ksysent.test \
	opipe.test \
	options-syntax.test \
	pc.test \
	qual_fault-syntax.test \
	qual_fault.test \
	qual_inject-error-signal.test \
	qual_inject-retval.test \
	qual_inject-signal.test \
	qual_inject-syntax.test \
	qual_signal.test \
	qual_syscall.test \
	redirect-fds.test \
	redirect.test \
	restart_syscall.test \
	strace-C.test \
	strace-E.test \
	strace-S.test \
	strace-T.test \
	strace-V.test \
	strace-ff.test \
	strace-r.test \
	strace-t.test \
	strace-tt.test \
	strace-ttt.test \
	threads-execve.test \
	# end of MISC_TESTS

TESTS = $(GEN_TESTS) $(DECODER_TESTS) $(MISC_TESTS) $(LIBUNWIND_TESTS)

XFAIL_TESTS_ =
XFAIL_TESTS_m32 = $(LIBUNWIND_TESTS)
XFAIL_TESTS_mx32 = $(LIBUNWIND_TESTS)
XFAIL_TESTS_x86_64 = int_0x80.gen.test
XFAIL_TESTS_x32 = int_0x80.gen.test
XFAIL_TESTS = $(XFAIL_TESTS_$(MPERS_NAME)) $(XFAIL_TESTS_$(ARCH))

TEST_LOG_COMPILER = env
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) MIPS_ABI=$(MIPS_ABI) $(srcdir)/run.sh

@VALGRIND_CHECK_RULES@
VALGRIND_FLAGS = --quiet
VALGRIND_SUPPRESSIONS_FILES = $(srcdir)/strace.supp

EXTRA_DIST = gen_tests.in gen_tests.sh init.sh run.sh match.awk \
	     caps-abbrev.awk \
	     caps.awk \
	     count-f.expected \
	     eventfd.expected \
	     fadvise.h \
	     filter-unavailable.expected \
	     fstatat.c \
	     fstatx.c \
	     getresugid.c \
	     init_delete_module.h \
	     ipc.sh \
	     ipc_msgbuf.expected \
	     ksysent.sed \
	     lstatx.c \
	     net.expected \
	     oldselect.expected \
	     pipe.expected \
	     process_vm_readv_writev.c \
	     pure_executables.list \
	     qual_fault-exit_group.expected \
	     qual_inject-error-signal.expected \
	     qual_inject-signal.expected \
	     quotactl.h \
	     scno_tampering.sh \
	     setfsugid.c \
	     setresugid.c \
	     setreugid.c \
	     setugid.c \
	     sigaction.awk \
	     sigaltstack.expected \
	     sockname.c \
	     statfs.expected \
	     strace-C.expected \
	     strace-E.expected \
	     strace-T.expected \
	     strace-ff.expected \
	     strace-k.test \
	     strace-r.expected \
	     strace.supp \
	     struct_flock.c \
	     sun_path.expected \
	     uio.expected \
	     umode_t.c \
	     umovestr.expected \
	     unix-pair-send-recv.expected \
	     unix-pair-sendto-recvfrom.expected \
	     xchownx.c \
	     xgetrlimit.c \
	     xselect.c \
	     xstatfs.c \
	     xstatfs64.c \
	     xstatfsx.c \
	     xstatx.c \
	     $(TESTS)

ksysent.h: $(srcdir)/ksysent.sed
	echo '#include <asm/unistd.h>' | \
		$(CPP) $(AM_CPPFLAGS) $(CPPFLAGS) -dM - > $@.t1
	LC_COLLATE=C sed -r -n -f $(srcdir)/ksysent.sed < $@.t1 > $@.t2
	mv -f $@.t2 $@
	rm -f $@.t1

ksysent.$(OBJEXT): ksysent.h

objects = $(filter %.$(OBJEXT),$(SOURCES:.c=.$(OBJEXT)))
$(objects): scno.h

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -rf -- $(TESTS:.test=.dir) $(GEN_TESTS:.gen.test=.dir)

CLEANFILES = ksysent.h

include ../scno.am
