# Automake input for strace tests.
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2011-2024 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

OS = linux
CC = @CC@
ARCH = @arch@
KARCH = @karch@
NATIVE_ARCH = @arch_native@
STRACE_SIZEOF_KERNEL_LONG_T = @SIZEOF_KERNEL_LONG_T@
SIZEOF_KERNEL_LONG_T = @SIZEOF_KERNEL_LONG_T@
STRACE_SIZEOF_LONG = @SIZEOF_LONG@
SIZEOF_LONG = @SIZEOF_LONG@
MPERS_NAME =
MPERS_CC_FLAGS =
ARCH_MFLAGS =
AM_CFLAGS = $(WARN_CFLAGS)
bundled_CPPFLAGS =
if USE_BUNDLED_HEADERS
bundled_CPPFLAGS += -I$(top_srcdir)/bundled/$(OS)/arch/$(KARCH)/include/uapi \
		    -I$(top_srcdir)/bundled/$(OS)/include/uapi
endif
AM_CPPFLAGS = $(ARCH_MFLAGS) \
	      -I$(builddir) \
	      -I$(top_builddir)/src/$(OS)/$(ARCH) \
	      -I$(top_srcdir)/src/$(OS)/$(ARCH) \
	      -I$(top_builddir)/src/$(OS)/generic \
	      -I$(top_srcdir)/src/$(OS)/generic \
	      -I$(top_builddir)/src \
	      -I$(top_srcdir)/src \
	      $(bundled_CPPFLAGS) \
	      -DSTRACE_SIZEOF_KERNEL_LONG_T=$(STRACE_SIZEOF_KERNEL_LONG_T) \
	      -DTESTS_SIZEOF_KERNEL_LONG_T=$(SIZEOF_KERNEL_LONG_T) \
	      -DSTRACE_SIZEOF_LONG=$(STRACE_SIZEOF_LONG) \
	      -DTESTS_SIZEOF_LONG=$(SIZEOF_LONG) \
	      #
AM_LDFLAGS = $(ARCH_MFLAGS)

if HAVE_SELINUX_RUNTIME
libselinux_LDADD = $(libselinux_LIBS)
else
libselinux_LDADD =
endif

libtests_a_SOURCES = \
	clock_t_str.c \
	create_nl_socket.c \
	create_tmpfile.c \
	dirfd.c \
	errno2name.c \
	error_msg.c \
	fill_memory.c \
	futex2_flags.h \
	get_page_size.c \
	get_sigset_size.c \
	hexdump_strdup.c \
	hexquote_strndup.c \
	ifindex.c \
	inode_of_sockfd.c \
	libmmsg.c \
	libsocketcall.c \
	lock_file.c \
	overflowuid.c \
	pidns.c \
	pidns.h \
	pipe_maxfd.c \
	prctl_marker.c \
	print_quoted_string.c \
	print_time.c \
	printflags.c \
	printxval-Xabbrev.c \
	printxval-Xraw.c \
	printxval-Xverbose.c \
	secontext.c \
	secontext.h \
	signal2name.c \
	skip_unavailable.c \
	sprintrc.c \
	status.c \
	subdir.c \
	tail_alloc.c \
	test_netlink.h \
	test_nlattr.h \
	test_printpath.c \
	test_printstrn.c \
	test_ucopy.c \
	test_ucopy.h \
	tests.h \
	tprintf.c \
	xmalloc_for_tests.c \
	# end of libtests_a_SOURCES
libtests_a_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
check_LIBRARIES = libtests.a
LDADD = libtests.a ${m_LIBS}

include pure_executables.am

include secontext.am

check_PROGRAMS = $(PURE_EXECUTABLES) \
	$(secontext_EXECUTABLES) \
	_newselect-P \
	answer \
	arch_prctl-success \
	arch_prctl-success-Xabbrev \
	arch_prctl-success-Xraw \
	arch_prctl-success-Xverbose \
	attach-f-p \
	attach-f-p-cmd \
	attach-p-cmd-cmd \
	attach-p-cmd-p \
	block_reset_raise_run \
	block_reset_run \
	bpf-obj_get_info_by_fd \
	bpf-obj_get_info_by_fd-prog \
	bpf-obj_get_info_by_fd-prog-v \
	bpf-obj_get_info_by_fd-v \
	bpf-success \
	bpf-success-long-y \
	bpf-success-v \
	caps-abbrev \
	check_sigblock \
	check_sigign \
	clone3-success \
	clone3-success-Xabbrev \
	clone3-success-Xraw \
	clone3-success-Xverbose \
	clone_parent \
	clone_parent--quiet-exit \
	clone_parent-q \
	clone_parent-qq \
	clone_ptrace \
	clone_ptrace--quiet-attach \
	clone_ptrace--quiet-exit \
	clone_ptrace-q \
	clone_ptrace-qq \
	close_range \
	count-f \
	delay \
	detach-vfork \
	execve-v \
	execveat-v \
	fcntl--pidns-translation \
	fcntl64--pidns-translation \
	filter-unavailable \
	filter_seccomp-flag \
	filter_seccomp-perf \
	fork--pidns-translation \
	fork-f \
	fsync-y \
	get_process_reaper \
	getpgrp--pidns-translation	\
	getpid--pidns-translation	\
	getsid--pidns-translation \
	gettid \
	gettid--pidns-translation \
	inject-nf \
	int_0x80 \
	io_uring_register-success \
	io_uring_register-success-Xabbrev \
	io_uring_register-success-Xraw \
	io_uring_register-success-Xverbose \
	ioctl_block--pidns-translation \
	ioctl_dm-v \
	ioctl_evdev-success \
	ioctl_evdev-success-Xabbrev \
	ioctl_evdev-success-Xraw \
	ioctl_evdev-success-Xverbose \
	ioctl_evdev-success-v \
	ioctl_evdev-success-v-Xabbrev \
	ioctl_evdev-success-v-Xraw \
	ioctl_evdev-success-v-Xverbose \
	ioctl_fiemap-success \
	ioctl_fiemap-success-Xabbrev \
	ioctl_fiemap-success-Xraw \
	ioctl_fiemap-success-Xverbose \
	ioctl_fiemap-success-v \
	ioctl_fiemap-success-v-Xabbrev \
	ioctl_fiemap-success-v-Xraw \
	ioctl_fiemap-success-v-Xverbose \
	ioctl_fs_x-success \
	ioctl_fs_x-success-Xabbrev \
	ioctl_fs_x-success-Xraw \
	ioctl_fs_x-success-Xverbose \
	ioctl_gpio-success \
	ioctl_gpio-success-Xabbrev \
	ioctl_gpio-success-Xraw \
	ioctl_gpio-success-Xverbose \
	ioctl_gpio-success-v \
	ioctl_gpio-success-v-Xabbrev \
	ioctl_gpio-success-v-Xraw \
	ioctl_gpio-success-v-Xverbose \
	ioctl_hdio-success \
	ioctl_hdio-success-Xabbrev \
	ioctl_hdio-success-Xraw \
	ioctl_hdio-success-Xverbose \
	ioctl_hdio-success-v \
	ioctl_hdio-success-v-Xabbrev \
	ioctl_hdio-success-v-Xraw \
	ioctl_hdio-success-v-Xverbose \
	ioctl_kd-success \
	ioctl_kd-success-Xabbrev \
	ioctl_kd-success-Xraw \
	ioctl_kd-success-Xverbose \
	ioctl_kd-success-s1024 \
	ioctl_kd-success-s1024-Xabbrev \
	ioctl_kd-success-s1024-Xraw \
	ioctl_kd-success-s1024-Xverbose \
	ioctl_lirc-success \
	ioctl_loop-nv \
	ioctl_loop-v \
	ioctl_mtd-success \
	ioctl_nsfs \
	ioctl_perf-success \
	ioctl_ptp-success \
	ioctl_ptp-success-Xabbrev \
	ioctl_ptp-success-Xraw \
	ioctl_ptp-success-Xverbose \
	ioctl_rtc-success \
	ioctl_rtc-v \
	ioctl_seccomp \
	ioctl_seccomp--pidns-translation \
	ioctl_seccomp-success \
	ioctl_seccomp-y \
	ioctl_seccomp-y-success \
	ioctl_seccomp-y-success--pidns-translation \
	ioctl_seccomp-y-success--pidns-translation-Xabbrev \
	ioctl_seccomp-y-success--pidns-translation-Xraw \
	ioctl_seccomp-y-success--pidns-translation-Xverbose \
	ioctl_seccomp-y-success-Xabbrev \
	ioctl_seccomp-y-success-Xraw \
	ioctl_seccomp-y-success-Xverbose \
	ioctl_ubi-success \
	ioctl_v4l2-success \
	ioctl_v4l2-success-Xabbrev \
	ioctl_v4l2-success-Xraw \
	ioctl_v4l2-success-Xverbose \
	ioctl_v4l2-success-v \
	ioctl_v4l2-success-v-Xabbrev \
	ioctl_v4l2-success-v-Xraw \
	ioctl_v4l2-success-v-Xverbose \
	ioprio--pidns-translation \
	ip_local_port_range-success \
	ip_local_port_range-success-Xabbrev \
	ip_local_port_range-success-Xraw \
	ip_local_port_range-success-Xverbose \
	is_linux_mips_n64 \
	kcmp-y--pidns-translation \
	kill--pidns-translation \
	kill_child \
	ksysent \
	landlock_create_ruleset-success \
	landlock_create_ruleset-success-y \
	link-P \
	list_sigaction_signum \
	localtime \
	looping_threads \
	memfd_secret-success \
	memfd_secret-success-y \
	migrate_pages--pidns-translation \
	mmsg-silent \
	mmsg_name-v \
	move_pages--pidns-translation \
	msg_control-v \
	net-accept-connect \
	net-sockaddr--pidns-translation \
	net-tpacket_stats-success \
	netlink_audit--pidns-translation \
	netlink_inet_diag \
	netlink_netlink_diag \
	netlink_unix_diag \
	nlattr_ifla_xdp-y \
	nsyscalls \
	nsyscalls-d \
	oldselect-P \
	oldselect-efault-P \
	orphaned_process_group \
	pc \
	perf_event_open_nonverbose \
	perf_event_open_unabbrev \
	pidfd_open--pidns-translation \
	pidfd_send_signal--pidns-translation \
	pidns-cache \
	poll-P \
	ppoll-P \
	ppoll-e-trace-fds-23 \
	ppoll-e-trace-fds-23-42 \
	ppoll-e-trace-fds-not-9-42-P \
	ppoll-v \
	prctl-fp-mode \
	prctl-mdwe \
	prctl-pac-enabled-keys-success \
	prctl-pac-enabled-keys-success-Xabbrev \
	prctl-pac-enabled-keys-success-Xraw \
	prctl-pac-enabled-keys-success-Xverbose \
	prctl-sched-core--pidns-translation \
	prctl-sched-core--pidns-translation-Xabbrev \
	prctl-sched-core--pidns-translation-Xraw \
	prctl-sched-core--pidns-translation-Xverbose \
	prctl-sched-core-success \
	prctl-sched-core-success--pidns-translation \
	prctl-sched-core-success--pidns-translation-Xabbrev \
	prctl-sched-core-success--pidns-translation-Xraw \
	prctl-sched-core-success--pidns-translation-Xverbose \
	prctl-sched-core-success-Xabbrev \
	prctl-sched-core-success-Xraw \
	prctl-sched-core-success-Xverbose \
	prctl-seccomp-filter-v \
	prctl-seccomp-strict \
	prctl-securebits-success \
	prctl-securebits-success-Xabbrev \
	prctl-securebits-success-Xraw \
	prctl-securebits-success-Xverbose \
	prctl-set-ptracer-success \
	prctl-set-ptracer-success--pidns-translation \
	prctl-set-ptracer-success-Xabbrev \
	prctl-set-ptracer-success-Xabbrev--pidns-translation \
	prctl-set-ptracer-success-Xraw \
	prctl-set-ptracer-success-Xraw--pidns-translation \
	prctl-set-ptracer-success-Xverbose \
	prctl-set-ptracer-success-Xverbose--pidns-translation \
	prctl-sme-success \
	prctl-sme-success-Xabbrev \
	prctl-sme-success-Xraw \
	prctl-sme-success-Xverbose \
	prctl-spec-inject \
	prctl-sve-success \
	prctl-sve-success-Xabbrev \
	prctl-sve-success-Xraw \
	prctl-sve-success-Xverbose \
	prctl-tagged-addr-success \
	prctl-tagged-addr-success-Xabbrev \
	prctl-tagged-addr-success-Xraw \
	prctl-tagged-addr-success-Xverbose \
	print_maxfd \
	print_ppid_tracerpid \
	prlimit64--pidns-translation \
	prlimit64-success \
	prlimit64-success--pidns-translation \
	process_vm_readv--pidns-translation \
	process_vm_writev--pidns-translation \
	qual_fault \
	qual_inject-error-signal \
	qual_inject-retval \
	qual_inject-signal \
	qual_signal \
	quotactl-success \
	quotactl-success-v \
	quotactl-v \
	quotactl-xfs-success \
	quotactl-xfs-success-v \
	quotactl-xfs-v \
	redirect-fds \
	restart_syscall \
	rt_sigqueueinfo--pidns-translation \
	rt_tgsigqueueinfo--pidns-translation \
	run_expect_termsig \
	sched_xetaffinity--pidns-translation \
	sched_xetattr--pidns-translation \
	sched_xetparam--pidns-translation \
	sched_xetscheduler--pidns-translation \
	scm_rights \
	seccomp-filter-v \
	seccomp-strict \
	seccomp_get_notif_sizes-success \
	segv_accerr \
	segv_pkuerr \
	select-P \
	select-trace-fd-7 \
	select-trace-fd-7-9 \
	select-trace-fd-7-P \
	set_ptracer_any \
	set_sigblock \
	set_sigign \
	setpgrp-exec \
	signal_receive \
	signal_receive--pidns-translation \
	sleep \
	so_peercred--pidns-translation \
	sockopt-sol_socket-success \
	sockopt-sol_socket-success-Xabbrev \
	sockopt-sol_socket-success-Xraw \
	sockopt-sol_socket-success-Xverbose \
	stack-fcall \
	stack-fcall-attach \
	stack-fcall-mangled \
	status-detached-threads \
	status-none-threads \
	status-successful-threads \
	status-unfinished-threads \
	strace--decode-pids-comm \
	strace--syscall-limit \
	strace--syscall-limit-c \
	strace--syscall-limit-path \
	strace--syscall-limit-status \
	strace--syscall-limit-status-c \
	strace--syscall-limit-status-summary \
	strace--syscall-limit-summary \
	strace-Y-0123456789 \
	strace-p-Y-p2 \
	strace-p1-Y-p \
	symlink-P \
	syslog-success \
	tgkill--pidns-translation \
	threads-execve \
	threads-execve--quiet-thread-execve \
	threads-execve-q \
	threads-execve-qq \
	threads-execve-qqq \
	tkill--pidns-translation \
	tracer_ppid_pgid_sid \
	trie_test \
	unblock_reset_raise \
	unix-pair-send-recv \
	unix-pair-sendto-recvfrom \
	vfork-f \
	wait4-v \
	waitid-Y \
	waitid-v \
	xet_robust_list--pidns-translation \
	xetpgid--pidns-translation \
	xetpriority--pidns-translation \
	zeroargc \
	# end of check_PROGRAMS

attach_f_p_LDADD = -lpthread $(LDADD)
bpf_obj_get_info_by_fd_LDADD = $(clock_LIBS) $(LDADD)
bpf_obj_get_info_by_fd_v_LDADD = $(clock_LIBS) $(LDADD)
bpf_obj_get_info_by_fd_prog_LDADD = $(clock_LIBS) $(LDADD)
bpf_obj_get_info_by_fd_prog_v_LDADD = $(clock_LIBS) $(LDADD)
count_f_LDADD = -lpthread $(LDADD)
delay_LDADD = $(clock_LIBS) $(LDADD)
filter_unavailable_LDADD = -lpthread $(LDADD)
fstat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
fstatat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
ftruncate64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
localtime_LDADD = $(clock_LIBS) $(LDADD)
looping_threads_LDADD = -lpthread $(LDADD)
lstat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
maybe_switch_current_tcp_LDADD = -lpthread $(LDADD)
maybe_switch_current_tcp__quiet_thread_execve_LDADD = -lpthread $(LDADD)
mmap64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mmap64_Xabbrev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mmap64_Xraw_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mmap64_Xverbose_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
mq_LDADD = $(mq_LIBS) $(LDADD)
newfstatat_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
nlattr_mdba_router_port_LDADD = ${m_LIBS} $(LDADD)
pc_LDADD = $(dl_LIBS) $(LDADD)
pread64_pwrite64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
stat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
status_detached_threads_LDADD = -lpthread $(LDADD)
status_none_threads_LDADD = -lpthread $(LDADD)
status_successful_threads_LDADD = -lpthread $(LDADD)
status_unfinished_threads_LDADD = -lpthread $(LDADD)
threads_execve_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve__quiet_thread_execve_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_q_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_qq_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_qqq_LDADD = -lpthread $(clock_LIBS) $(LDADD)
times_LDADD = $(clock_LIBS) $(LDADD)
times_Xabbrev_LDADD = $(clock_LIBS) $(LDADD)
times_Xraw_LDADD = $(clock_LIBS) $(LDADD)
times_Xverbose_LDADD = $(clock_LIBS) $(LDADD)
truncate64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
uio_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64

stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c
if USE_LIBDW
if ACCEPT_GDWARF_ARANGES
stack_fcall_CFLAGS = $(AM_CFLAGS) -gdwarf-aranges
endif
endif

stack_fcall_attach_SOURCES = stack-fcall-attach.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c
if USE_LIBDW
if ACCEPT_GDWARF_ARANGES
stack_fcall_attach_CFLAGS = $(AM_CFLAGS) -gdwarf-aranges
endif
endif

stack_fcall_mangled_SOURCES = stack-fcall-mangled.c \
	stack-fcall-mangled-0.c stack-fcall-mangled-1.c \
	stack-fcall-mangled-2.c stack-fcall-mangled-3.c

trie_test_SOURCES = trie_test.c trie_for_tests.c
trie_test_CPPFLAGS = $(AM_CPPFLAGS) $(CODE_COVERAGE_CPPFLAGS)
trie_test_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS)
trie_test_LDADD = $(LDADD) $(CODE_COVERAGE_LIBS)

include gen_tests.am

if ENABLE_STACKTRACE
STACKTRACE_TESTS = strace-k.test strace-k-p.test strace-k-with-depth-limit.test
if USE_LIBDW
STACKTRACE_TESTS += strace-kk.test strace-kk-p.test
endif
if USE_DEMANGLE
STACKTRACE_TESTS += strace-k-demangle.test
endif
if USE_LIBUNWIND
STACKTRACE_XFAIL_TESTS = $(STACKTRACE_TESTS)
else
STACKTRACE_XFAIL_TESTS =
endif
else
STACKTRACE_TESTS =
STACKTRACE_XFAIL_TESTS =
endif

DECODER_TESTS = \
	bpf-success-long-y.test \
	brk.test \
	btrfs-X.test \
	btrfs-v.test \
	btrfs-vX.test \
	btrfs-vw.test \
	btrfs-vwX.test \
	btrfs-w.test \
	btrfs-wX.test \
	caps-abbrev.test \
	caps.test \
	clone-flags.test \
	eventfd.test \
	execve-v.test \
	execve.test \
	faccessat.test \
	faccessat2.test \
	fadvise64.test \
	futex.test \
	getuid.test \
	inotify_init-y.test \
	int_0x80.test \
	ioctl.test \
	ioctl_block--pidns-translation.test \
	ioctl_evdev-success.test \
	ipc_msgbuf.test \
	kern_features-fault.test \
	llseek.test \
	lseek.test \
	mmap.test \
	net-y-unix.test \
	net-yy-inet.test \
	net-yy-netlink.test \
	net-yy-unix.test \
	net.test \
	netlink_sock_diag.test \
	nsyscalls-d.test \
	nsyscalls-nd.test \
	nsyscalls.test \
	personality.test \
	pipe.test \
	poll-P.test \
	poll.test \
	prctl-seccomp-strict.test \
	qual_fault-exit_group.test \
	readv.test \
	rt_sigaction.test \
	scm_rights-fd.test \
	seccomp-strict.test \
	sigaltstack.test \
	sun_path.test \
	syslog-success.test \
	uio.test \
	umount.test \
	umount2.test \
	umovestr.test \
	umovestr2.test \
	uname.test \
	unix-pair-send-recv.test \
	unix-pair-sendto-recvfrom.test \
	xet_thread_area_x86.test \
	# end of DECODER_TESTS

MISC_TESTS = \
	attach-f-p.test \
	attach-p-cmd.test \
	bexecve.test \
	clone_ptrace.test \
	count-f.test \
	count.test \
	delay.test \
	detach-running.test \
	detach-sleeping.test \
	detach-stopped.test \
	detach-vfork.test \
	exec-PATH.test \
	fflush.test \
	filter-unavailable.test \
	filter_seccomp-exitkill.test \
	filter_seccomp-perf.test \
	filtering_fd-syntax.test \
	filtering_syscall-syntax.test \
	first_exec_failure.test \
	fork--pidns-translation.test \
	get_regs.test \
	gettid--pidns-translation.test \
	inject-nf.test \
	interactive_block.test \
	kill-on-exit.test \
	kill_child.test \
	legacy_syscall_info.test \
	localtime.test \
	looping_threads.test \
	netlink_audit--pidns-translation.test \
	opipe.test \
	options-syntax.test \
	pc.test \
	pidns-cache.test \
	poke-ptrace.test \
	poke-range.test \
	poke-unaligned.test \
	poke.test \
	printpath-umovestr-legacy.test \
	printstrn-umoven-legacy.test \
	qual_fault-syntax.test \
	qual_fault-syscall.test \
	qual_fault.test \
	qual_inject-error-signal-syscall.test \
	qual_inject-error-signal.test \
	qual_inject-retval-syscall.test \
	qual_inject-retval.test \
	qual_inject-signal-syscall.test \
	qual_inject-signal.test \
	qual_inject-syntax.test \
	qual_signal.test \
	qual_syscall.test \
	redirect-fds.test \
	redirect.test \
	restart_syscall.test \
	sigblock.test \
	sigign.test \
	status-detached-threads.test \
	status-detached.test \
	status-none-threads.test \
	status-successful-threads.test \
	status-unfinished-threads.test \
	strace--argv0.test \
	strace--syscall-limit.test \
	strace--syscall-limit--seccomp-bpf.test \
	strace--tips-full.test \
	strace--tips.test \
	strace-C.test \
	strace-D.test \
	strace-DD.test \
	strace-DDD.test \
	strace-E-override.test \
	strace-E-unset.test \
	strace-E.test \
	strace-S.test \
	strace-T_upper.test \
	strace-V.test \
	strace-ff.test \
	strace-log-merge-error.test \
	strace-log-merge-suffix.test \
	strace-p-Y-p.test \
	strace-r.test \
	strace-self.test \
	strace-t.test \
	strace-tt.test \
	strace-ttt.test \
	tampering-notes.test \
	termsig.test \
	threads-execve.test \
	umovestr_cached.test \
	# end of MISC_TESTS

TESTS = $(GEN_TESTS) $(DECODER_TESTS) $(MISC_TESTS) $(STACKTRACE_TESTS)

XFAIL_TESTS_ =
XFAIL_TESTS_m32 = $(STACKTRACE_XFAIL_TESTS)
XFAIL_TESTS_mx32 = $(STACKTRACE_XFAIL_TESTS)
XFAIL_TESTS = $(XFAIL_TESTS_$(MPERS_NAME)) $(XFAIL_TESTS_$(ARCH))

TEST_LOG_COMPILER = env
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) STRACE_NATIVE_ARCH=$(NATIVE_ARCH) \
			SIZEOF_KERNEL_LONG_T=$(SIZEOF_KERNEL_LONG_T) \
			SIZEOF_LONG=$(SIZEOF_LONG) \
			MIPS_ABI=$(MIPS_ABI) $(srcdir)/run.sh

@VALGRIND_CHECK_RULES@
VALGRIND_FLAGS = --quiet
VALGRIND_SUPPRESSIONS_FILES = $(abs_srcdir)/strace.supp

check_SCRIPTS = \
	PTRACE_SEIZE.sh \
	arch_prctl.sh \
	filter_seccomp.sh \
	init.sh \
	ioctl-success.sh \
	ipc.sh \
	prctl-success.sh \
	prctl.sh \
	print_scno_getcwd.sh \
	qualify_personality.sh \
	qualify_personality_all.sh \
	run.sh \
	scno_tampering.sh \
	strace-k-demangle.test \
	strace-k-p.test \
	strace-k-with-depth-limit.test \
	strace-k.test \
	strace-kk-p.test \
	strace-kk.test \
	syntax.sh \
	# end of check_SCRIPTS

check_DATA = \
	COPYING \
	GPL-2.0-or-later \
	caps-abbrev.awk \
	caps.awk \
	clock.in \
	count-f.expected \
	eventfd.expected \
	filter-unavailable.expected \
	filter_seccomp.in \
	fork--pidns-translation.awk \
	match.awk \
	net.expected \
	pipe.expected \
	poke-range.expected \
	poke-unaligned.expected \
	pure_executables.list \
	qual_fault-exit_group.expected \
	qual_inject-error-signal.expected \
	qual_inject-signal.expected \
	qualify_personality_empty.in \
	regex.in \
	rt_sigaction.awk \
	sched.in \
	sigaltstack.expected \
	status-detached.expected \
	strace--argv0.expected \
	strace--follow-forks-output-separately.expected \
	strace--relative-timestamps-ms.expected \
	strace--relative-timestamps-ns.expected \
	strace--relative-timestamps-s.expected \
	strace--relative-timestamps-us.expected \
	strace--relative-timestamps.expected \
	strace--syscall-limit--seccomp-bpf-1.expected \
	strace--syscall-limit--seccomp-bpf-2.expected \
	strace--syscall-limit--seccomp-bpf-3.expected \
	strace--syscall-times-ms.expected \
	strace--syscall-times-ns.expected \
	strace--syscall-times-s.expected \
	strace--syscall-times-us.expected \
	strace--syscall-times.expected \
	strace--tips-0.exp \
	strace--tips.exp \
	strace-C.expected \
	strace-D.expected \
	strace-DDD.expected \
	strace-E-unset.expected \
	strace-E.expected \
	strace-T_upper.expected \
	strace-ff.expected \
	strace-k-demangle.expected \
	strace-k-p.expected \
	strace-k-with-depth-limit.expected \
	strace-k.expected \
	strace-kk-p.expected \
	strace-kk.expected \
	strace-r.expected \
	strace.supp \
	strauss_body.exp \
	strauss_head.exp \
	sun_path.expected \
	trace_clock.in \
	trace_creds.in \
	trace_fstat.in \
	trace_fstatfs.in \
	trace_lstat.in \
	trace_personality_32.in \
	trace_personality_64.in \
	trace_personality_number_32.in \
	trace_personality_number_64.in \
	trace_personality_number_x32.in \
	trace_personality_regex_32.in \
	trace_personality_regex_64.in \
	trace_personality_regex_x32.in \
	trace_personality_statfs_32.in \
	trace_personality_statfs_64.in \
	trace_personality_statfs_x32.in \
	trace_personality_x32.in \
	trace_question.in \
	trace_stat.in \
	trace_stat_like.in \
	trace_statfs.in \
	trace_statfs_like.in \
	uio.expected \
	umovestr.expected \
	unix-pair-send-recv.expected \
	unix-pair-sendto-recvfrom.expected \
	# end of check_DATA

EXTRA_DIST = \
	attach-p-cmd.h \
	clock_adjtime-common.c \
	clock_xettime-common.c \
	cur_audit_arch.h \
	fadvise.h \
	fcntl-common.c \
	fstatat.c \
	fstatx.c \
	gen_pure_executables.sh \
	gen_tests.in \
	gen_tests.sh \
	getresugid.c \
	init_delete_module.h \
	ioctl_kvm_run_common.c \
	kernel_old_timespec.h \
	kernel_old_timex.h \
	kill-on-exit.sh \
	ksysent.sed \
	lstatx.c \
	nlattr_ifla.h \
	nlattr_ifla_af_inet6.h \
	print_user_desc.c \
	printsignal.c \
	printxval.c \
	process_vm_readv_writev.c \
	pselect6-common.c \
	quotactl.h \
	semop-common.c \
	semtimedop-common.c \
	semtimedop-syscall.c \
	setfsugid.c \
	setresugid.c \
	setreugid.c \
	setugid.c \
	sockaddr_xlat.c \
	sockname.c \
	stack-fcall.h \
	time_enjoyment.h \
	umode_t.c \
	xchownx.c \
	xgetdents.c \
	xgetrlimit.c \
	xselect.c \
	xstatfs.c \
	xstatfs64.c \
	xstatfsx.c \
	xstatx.c \
	xutimes.c \
	$(check_DATA) \
	$(check_SCRIPTS) \
	$(TESTS) \
	# end of EXTRA_DIST

if ENABLE_INSTALL_TESTS
testslibexecdir = $(libexecdir)/strace/tests$(MPERS_NAME)
testslibexec_PROGRAMS = $(check_PROGRAMS)
testslibexec_DATA = $(check_DATA)
testslibexec_SCRIPTS = $(check_SCRIPTS) $(TESTS)
endif

ksysent.h: $(srcdir)/ksysent.sed
	echo '#include <asm/unistd.h>' | \
		$(CPP) $(AM_CPPFLAGS) $(CPPFLAGS) -dM - > $@.t1
	LC_COLLATE=C sed -E -n -f $(srcdir)/ksysent.sed < $@.t1 > $@.t2
	mv -f $@.t2 $@
	rm -f $@.t1

ksysent.$(OBJEXT): ksysent.h

objects = $(filter %.$(OBJEXT),$(SOURCES:.c=.$(OBJEXT)))
$(objects): scno.h

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -rf -- $(TESTS:.test=.dir) $(GEN_TESTS:.gen.test=.dir)

.PHONY: check-valgrind-local
check-valgrind-local: $(check_LIBRARIES) $(check_PROGRAMS)

BUILT_SOURCES = ksysent.h
CLEANFILES = ksysent.h

include ../src/scno.am
