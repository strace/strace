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
	hexdump_strdup.c \
	hexquote_strndup.c \
	inode_of_sockfd.c \
	libmmsg.c \
	libsocketcall.c \
	overflowuid.c \
	pipe_maxfd.c \
	print_quoted_string.c \
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

if USE_LIBUNWIND
LIBUNWIND_TESTS = strace-k.test
else
LIBUNWIND_TESTS =
endif

DECODER_TESTS = \
	_newselect.test \
	accept.test \
	accept4.test \
	access.test \
	acct.test \
	add_key.test \
	adjtimex.test \
	aio.test \
	alarm.test \
	bpf.test \
	brk.test \
	btrfs-v.test \
	btrfs-vw.test \
	btrfs-w.test \
	btrfs.test \
	caps-abbrev.test \
	caps.test \
	chmod.test \
	chown.test \
	chown32.test \
	chroot.test \
	clock_adjtime.test \
	clock_nanosleep.test \
	clock_xettime.test \
	copy_file_range.test \
	creat.test \
	delete_module.test \
	dup.test \
	dup2.test \
	dup3.test \
	epoll_create.test \
	epoll_create1.test \
	epoll_ctl.test \
	epoll_pwait.test \
	epoll_wait.test \
	erestartsys.test \
	eventfd.test \
	execve-v.test \
	execve.test \
	execveat-v.test \
	execveat.test \
	faccessat.test \
	fadvise64.test \
	fadvise64_64.test \
	fallocate.test \
	fanotify_init.test \
	fanotify_mark.test \
	fchdir.test \
	fchmod.test \
	fchmodat.test \
	fchown.test \
	fchown32.test \
	fchownat.test \
	fcntl.test \
	fcntl64.test \
	fdatasync.test \
	file_handle.test \
	file_ioctl.test \
	finit_module.test \
	flock.test \
	fstat.test \
	fstat64.test \
	fstatat64.test \
	fstatfs.test \
	fstatfs64.test \
	fsync.test \
	ftruncate.test \
	ftruncate64.test \
	futex.test \
	futimesat.test \
	get_mempolicy.test \
	getcpu.test \
	getcwd.test \
	getdents.test \
	getdents64.test \
	getegid.test \
	getegid32.test \
	geteuid.test \
	geteuid32.test \
	getgid.test \
	getgid32.test \
	getgroups.test \
	getgroups32.test \
	getpeername.test \
	getpgrp.test \
	getrandom.test \
	getresgid.test \
	getresgid32.test \
	getresuid.test \
	getresuid32.test \
	getrlimit.test \
	getrusage.test \
	getsid.test \
	getsockname.test \
	getuid.test \
	getuid32.test \
	getxxid.test \
	inet-cmsg.test \
	init_module.test \
	inotify.test \
	inotify_init1.test \
	ioctl.test \
	ioctl_block.test \
	ioctl_dm-v.test \
	ioctl_dm.test \
	ioctl_evdev-v.test \
	ioctl_evdev.test \
	ioctl_loop-nv.test \
	ioctl_loop-v.test \
	ioctl_loop.test \
	ioctl_mtd.test \
	ioctl_rtc-v.test \
	ioctl_rtc.test \
	ioctl_scsi.test \
	ioctl_sg_io_v3.test \
	ioctl_sg_io_v4.test \
	ioctl_sock_gifconf.test \
	ioctl_uffdio.test \
	ioctl_v4l2.test \
	ioperm.test \
	iopl.test \
	ioprio.test \
	ip_mreq.test \
	ipc.test \
	ipc_msg.test \
	ipc_msgbuf.test \
	ipc_sem.test \
	ipc_shm.test \
	kcmp.test \
	kexec_file_load.test \
	kexec_load.test \
	keyctl.test \
	kill.test \
	lchown.test \
	lchown32.test \
	link.test \
	linkat.test \
	llseek.test \
	lookup_dcookie.test \
	lseek.test \
	lstat.test \
	lstat64.test \
	mbind.test \
	membarrier.test \
	memfd_create.test \
	migrate_pages.test \
	mincore.test \
	mkdir.test \
	mkdirat.test \
	mknod.test \
	mknodat.test \
	mlock.test \
	mlock2.test \
	mlockall.test \
	mmap.test \
	mmap64.test \
	mmsg-silent.test \
	mmsg.test \
	mmsg_name-v.test \
	mmsg_name.test \
	mount.test \
	move_pages.test \
	mq.test \
	mq_sendrecv-read.test \
	mq_sendrecv-write.test \
	mq_sendrecv.test \
	msg_control-v.test \
	msg_control.test \
	msg_name.test \
	munlockall.test \
	nanosleep.test \
	net-icmp_filter.test \
	net-sockaddr.test \
	net-y-unix.test \
	net-yy-inet.test \
	net-yy-netlink.test \
	net-yy-unix.test \
	net.test \
	netlink_protocol.test \
	newfstatat.test \
	nsyscalls.test \
	old_mmap.test \
	oldfstat.test \
	oldlstat.test \
	oldselect.test \
	oldstat.test \
	open.test \
	openat.test \
	pause.test \
	perf_event_open.test \
	perf_event_open_nonverbose.test \
	perf_event_open_unabbrev.test \
	personality.test \
	pipe.test \
	pipe2.test \
	pkey_alloc.test \
	pkey_free.test \
	pkey_mprotect.test \
	poll.test \
	ppoll.test \
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
	preadv-pwritev.test \
	preadv.test \
	preadv2-pwritev2.test \
	printstr.test \
	prlimit64.test \
	process_vm_readv.test \
	process_vm_writev.test \
	pselect6.test \
	ptrace.test \
	pwritev.test \
	qual_fault-exit_group.test \
	quotactl-v.test \
	quotactl-xfs-v.test \
	quotactl-xfs.test \
	quotactl.test \
	read-write.test \
	readahead.test \
	readdir.test \
	readlink.test \
	readlinkat.test \
	readv.test \
	reboot.test \
	recvfrom.test \
	recvmmsg-timeout.test \
	recvmsg.test \
	remap_file_pages.test \
	rename.test \
	renameat.test \
	renameat2.test \
	request_key.test \
	rmdir.test \
	rt_sigpending.test \
	rt_sigprocmask.test \
	rt_sigqueueinfo.test \
	rt_sigsuspend.test \
	rt_sigtimedwait.test \
	rt_tgsigqueueinfo.test \
	sched_get_priority_mxx.test \
	sched_rr_get_interval.test \
	sched_xetaffinity.test \
	sched_xetattr.test \
	sched_xetparam.test \
	sched_xetscheduler.test \
	sched_yield.test \
	scm_rights-fd.test \
	seccomp-filter-v.test \
	seccomp-filter.test \
	seccomp-strict.test \
	select.test \
	semop.test \
	sendfile.test \
	sendfile64.test \
	set_mempolicy.test \
	setdomainname.test \
	setfsgid.test \
	setfsgid32.test \
	setfsuid.test \
	setfsuid32.test \
	setgid.test \
	setgid32.test \
	setgroups.test \
	setgroups32.test \
	sethostname.test \
	setns.test \
	setregid.test \
	setregid32.test \
	setresgid.test \
	setresgid32.test \
	setresuid.test \
	setresuid32.test \
	setreuid.test \
	setreuid32.test \
	setrlimit.test \
	setuid.test \
	setuid32.test \
	shmxt.test \
	shutdown.test \
	sigaction.test \
	sigaltstack.test \
	siginfo.test \
	signalfd4.test \
	sigreturn.test \
	socketcall.test \
	splice.test \
	stat.test \
	stat64.test \
	statfs.test \
	statfs64.test \
	sun_path.test \
	swap.test \
	symlink.test \
	symlinkat.test \
	sync.test \
	sync_file_range.test \
	sync_file_range2.test \
	sysinfo.test \
	syslog.test \
	tee.test \
	time.test \
	timer_create.test \
	timer_xettime.test \
	timerfd_xettime.test \
	times-fail.test \
	times.test \
	truncate.test \
	truncate64.test \
	ugetrlimit.test \
	uio.test \
	umask.test \
	umount.test \
	umount2.test \
	umoven-illptr.test \
	umovestr-illptr.test \
	umovestr.test \
	umovestr2.test \
	umovestr3.test \
	uname.test \
	unix-pair-send-recv.test \
	unix-pair-sendto-recvfrom.test \
	unlink.test \
	unlinkat.test \
	unshare.test \
	userfaultfd.test \
	ustat.test \
	utime.test \
	utimensat.test \
	utimes.test \
	vhangup.test \
	vmsplice.test \
	wait4-v.test \
	wait4.test \
	waitid-v.test \
	waitid.test \
	waitpid.test \
	xattr-strings.test \
	xattr.test \
	xet_robust_list.test \
	xetitimer.test \
	xetpgid.test \
	xetpriority.test \
	xettimeofday.test \
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
	fork-f.test \
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
	signal_receive.test \
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
	vfork-f.test \
	# end of MISC_TESTS

TESTS = $(DECODER_TESTS) $(MISC_TESTS) $(LIBUNWIND_TESTS)

XFAIL_TESTS_ =
XFAIL_TESTS_m32 = $(LIBUNWIND_TESTS)
XFAIL_TESTS_mx32 = $(LIBUNWIND_TESTS)
XFAIL_TESTS = $(XFAIL_TESTS_$(MPERS_NAME))

TEST_LOG_COMPILER = env
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) MIPS_ABI=$(MIPS_ABI) $(srcdir)/run.sh

@VALGRIND_CHECK_RULES@
VALGRIND_FLAGS = --quiet
VALGRIND_SUPPRESSIONS_FILES = $(srcdir)/strace.supp

EXTRA_DIST = init.sh run.sh match.awk \
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
	     ppoll-v.expected \
	     ppoll.expected \
	     process_vm_readv_writev.c \
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
	     statx.sh \
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

CLEANFILES = ksysent.h $(TESTS:=.tmp)

include ../scno.am
