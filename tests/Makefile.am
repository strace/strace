# Automake input for strace tests.
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2011-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

OS = linux
CC = @CC@
ARCH = @arch@
NATIVE_ARCH = @arch_native@
SIZEOF_KERNEL_LONG_T = @SIZEOF_KERNEL_LONG_T@
SIZEOF_LONG = @SIZEOF_LONG@
MPERS_NAME =
MPERS_CC_FLAGS =
ARCH_MFLAGS =
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = $(ARCH_MFLAGS) \
	      -I$(builddir) \
	      -I$(top_builddir)/$(OS)/$(ARCH) \
	      -I$(top_srcdir)/$(OS)/$(ARCH) \
	      -I$(top_builddir)/$(OS) \
	      -I$(top_srcdir)/$(OS) \
	      -I$(top_builddir) \
	      -I$(top_srcdir) \
	      -DTESTS_SIZEOF_KERNEL_LONG_T=$(SIZEOF_KERNEL_LONG_T) \
	      -DTESTS_SIZEOF_LONG=$(SIZEOF_LONG)
AM_LDFLAGS = $(ARCH_MFLAGS)

libtests_a_SOURCES = \
	create_nl_socket.c \
	errno2name.c \
	error_msg.c \
	fill_memory.c \
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
	pipe_maxfd.c \
	print_quoted_string.c \
	print_time.c \
	printflags.c \
	printxval-Xabbrev.c \
	printxval-Xraw.c \
	printxval-Xverbose.c \
	signal2name.c \
	skip_unavailable.c \
	sprintrc.c \
	status.c \
	tail_alloc.c \
	test_netlink.h \
	test_nlattr.h \
	test_printpath.c \
	test_printstrn.c \
	test_ucopy.c \
	test_ucopy.h \
	tests.h \
	tprintf.c \
	# end of libtests_a_SOURCES
libtests_a_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
check_LIBRARIES = libtests.a
LDADD = libtests.a

include pure_executables.am

check_PROGRAMS = $(PURE_EXECUTABLES) \
	_newselect-P \
	answer \
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
	clone_parent \
	clone_parent--quiet-exit \
	clone_parent-q \
	clone_parent-qq \
	clone_ptrace \
	clone_ptrace--quiet-attach \
	clone_ptrace--quiet-exit \
	clone_ptrace-q \
	clone_ptrace-qq \
	clone3-success \
	clone3-success-Xabbrev \
	clone3-success-Xraw \
	clone3-success-Xverbose \
	count-f \
	delay \
	execve-v \
	execveat-v \
	filter_seccomp-flag \
	filter_seccomp-perf \
	filter-unavailable \
	fork-f \
	fsync-y \
	get_process_reaper \
	getpid	\
	getppid	\
	gettid \
	inject-nf \
	int_0x80 \
	ioctl_dm-v \
	ioctl_evdev-success \
	ioctl_evdev-success-Xabbrev \
	ioctl_evdev-success-Xraw \
	ioctl_evdev-success-Xverbose \
	ioctl_evdev-success-v \
	ioctl_evdev-success-v-Xabbrev \
	ioctl_evdev-success-v-Xraw \
	ioctl_evdev-success-v-Xverbose \
	ioctl_loop-nv \
	ioctl_loop-v \
	ioctl_nsfs \
	ioctl_perf-success \
	ioctl_rtc-v \
	is_linux_mips_n64 \
	kill_child \
	ksysent \
	list_sigaction_signum \
	localtime \
	looping_threads \
	mmsg-silent \
	mmsg_name-v \
	msg_control-v \
	net-accept-connect \
	net-tpacket_stats-success \
	netlink_inet_diag \
	netlink_netlink_diag \
	netlink_unix_diag \
	nsyscalls \
	nsyscalls-d \
	oldselect-P \
	oldselect-efault-P \
	orphaned_process_group \
	pc \
	perf_event_open_nonverbose \
	perf_event_open_unabbrev \
	ppoll-v \
	prctl-seccomp-filter-v \
	prctl-seccomp-strict \
	prctl-spec-inject \
	print_maxfd \
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
	run_expect_termsig \
	scm_rights \
	seccomp-filter-v \
	seccomp-strict \
	select-P \
	set_ptracer_any \
	set_sigblock \
	set_sigign \
	signal_receive \
	sleep \
	stack-fcall \
	stack-fcall-attach \
	stack-fcall-mangled \
	status-none-threads \
	status-unfinished-threads \
	syslog-success \
	threads-execve \
	threads-execve--quiet-thread-execve \
	threads-execve-q \
	threads-execve-qq \
	threads-execve-qqq \
	tracer_ppid_pgid_sid \
	unblock_reset_raise \
	unix-pair-send-recv \
	unix-pair-sendto-recvfrom \
	vfork-f \
	wait4-v \
	waitid-v \
	zeroargc \
	# end of check_PROGRAMS

attach_f_p_LDADD = -lpthread $(LDADD)
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
pc_LDADD = $(dl_LIBS) $(LDADD)
pread64_pwrite64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
preadv_pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
pwritev_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
stat64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
statfs_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
status_none_threads_LDADD = -lpthread $(LDADD)
status_unfinished_threads_LDADD = -lpthread $(LDADD)
threads_execve_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve__quiet_thread_execve_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_q_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_qq_LDADD = -lpthread $(clock_LIBS) $(LDADD)
threads_execve_qqq_LDADD = -lpthread $(clock_LIBS) $(LDADD)
times_LDADD = $(clock_LIBS) $(LDADD)
truncate64_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
uio_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64

stack_fcall_SOURCES = stack-fcall.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

stack_fcall_attach_SOURCES = stack-fcall-attach.c \
	stack-fcall-0.c stack-fcall-1.c stack-fcall-2.c stack-fcall-3.c

stack_fcall_mangled_SOURCES = stack-fcall-mangled.c \
	stack-fcall-mangled-0.c stack-fcall-mangled-1.c \
	stack-fcall-mangled-2.c stack-fcall-mangled-3.c

include gen_tests.am

if ENABLE_STACKTRACE
STACKTRACE_TESTS = strace-k.test strace-k-p.test
if USE_DEMANGLE
STACKTRACE_TESTS += strace-k-demangle.test
endif
else
STACKTRACE_TESTS =
endif

DECODER_TESTS = \
	bpf-success-long-y.test \
	bpf-success-v.test \
	bpf-success.test \
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
	clone3-success.test \
	eventfd.test \
	execve-v.test \
	execve.test \
	fadvise64.test \
	futex.test \
	getuid.test \
	int_0x80.test \
	inotify_init-y.test \
	ioctl.test \
	ioctl_evdev-success.test \
	ipc_msgbuf.test \
	kern_features-fault.test \
	llseek.test \
	lseek.test \
	mmap.test \
	net-tpacket_stats-success.test \
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
	prctl-arg2-intptr.test \
	prctl-dumpable.test \
	prctl-name.test \
	prctl-no-args.test \
	prctl-pdeathsig.test \
	prctl-seccomp-filter-v.test \
	prctl-seccomp-strict.test \
	prctl-securebits.test \
	prctl-spec-inject.test \
	prctl-tid_address.test \
	prctl-tsc.test \
	qual_fault-exit_group.test \
	quotactl-success-v.test \
	quotactl-success.test \
	quotactl-xfs-success-v.test \
	quotactl-xfs-success.test \
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
	fflush.test \
	filter_seccomp-perf.test \
	filter-unavailable.test \
	filtering_fd-syntax.test \
	filtering_syscall-syntax.test \
	first_exec_failure.test \
	get_regs.test \
	inject-nf.test \
	interactive_block.test \
	kill_child.test \
	localtime.test \
	looping_threads.test \
	opipe.test \
	options-syntax.test \
	pc.test \
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
	status-detached.test \
	status-none-threads.test \
	status-unfinished-threads.test \
	strace-C.test \
	strace-D.test \
	strace-DD.test \
	strace-DDD.test \
	strace-E.test \
	strace-S.test \
	strace-T.test \
	strace-V.test \
	strace-ff.test \
	strace-log-merge-error.test \
	strace-log-merge-suffix.test \
	strace-r.test \
	strace-t.test \
	strace-tt.test \
	strace-ttt.test \
	termsig.test \
	threads-execve.test \
	umovestr_cached.test \
	# end of MISC_TESTS

TESTS = $(GEN_TESTS) $(DECODER_TESTS) $(MISC_TESTS) $(STACKTRACE_TESTS)

XFAIL_TESTS_ =
XFAIL_TESTS_m32 = $(STACKTRACE_TESTS)
XFAIL_TESTS_mx32 = $(STACKTRACE_TESTS)
XFAIL_TESTS = $(XFAIL_TESTS_$(MPERS_NAME)) $(XFAIL_TESTS_$(ARCH))

TEST_LOG_COMPILER = env
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) STRACE_NATIVE_ARCH=$(NATIVE_ARCH) \
			SIZEOF_KERNEL_LONG_T=$(SIZEOF_KERNEL_LONG_T) \
			SIZEOF_LONG=$(SIZEOF_LONG) \
			MIPS_ABI=$(MIPS_ABI) $(srcdir)/run.sh

@VALGRIND_CHECK_RULES@
VALGRIND_FLAGS = --quiet
VALGRIND_SUPPRESSIONS_FILES = $(abs_srcdir)/strace.supp

EXTRA_DIST = \
	COPYING \
	GPL-2.0-or-later \
	PTRACE_SEIZE.sh \
	accept_compat.h \
	attach-p-cmd.h \
	caps-abbrev.awk \
	caps.awk \
	clock.in \
	count-f.expected \
	eventfd.expected \
	fadvise.h \
	fcntl-common.c \
	filter_seccomp.in \
	filter_seccomp.sh \
	filter-unavailable.expected \
	fstatat.c \
	fstatx.c \
	gen_pure_executables.sh \
	gen_tests.in \
	gen_tests.sh \
	getresugid.c \
	init.sh \
	init_delete_module.h \
	ioctl-success.sh \
	ioctl_kvm_run_common.c \
	ipc.sh \
	kernel_old_timespec.h \
	kernel_old_timex.h \
	ksysent.sed \
	lstatx.c \
	match.awk \
	net.expected \
	netlink_sock_diag-v.sh \
	nlattr_ifla.h \
	pipe.expected \
	print_user_desc.c \
	printsignal.c \
	printxval.c \
	process_vm_readv_writev.c \
	pure_executables.list \
	qual_fault-exit_group.expected \
	qual_inject-error-signal.expected \
	qual_inject-signal.expected \
	qualify_personality.sh \
	qualify_personality_empty.in \
	quotactl.h \
	regex.in \
	rt_sigaction.awk \
	run.sh \
	sched.in \
	scno_tampering.sh \
	semop-common.c \
	setfsugid.c \
	setresugid.c \
	setreugid.c \
	setugid.c \
	sigaltstack.expected \
	sockaddr_xlat.c \
	sockname.c \
	stack-fcall.h \
	status-detached.expected \
	strace--follow-forks-output-separately.expected \
	strace--relative-timestamps.expected \
	strace--relative-timestamps-s.expected \
	strace--relative-timestamps-ms.expected \
	strace--relative-timestamps-us.expected \
	strace--relative-timestamps-ns.expected \
	strace--syscall-times.expected \
	strace--syscall-times-s.expected \
	strace--syscall-times-ms.expected \
	strace--syscall-times-us.expected \
	strace--syscall-times-ns.expected \
	strace-C.expected \
	strace-D.expected \
	strace-DD.expected \
	strace-DDD.expected \
	strace-E.expected \
	strace-T.expected \
	strace-ff.expected \
	strace-k-demangle.expected \
	strace-k-demangle.test \
	strace-k-p.expected \
	strace-k-p.test \
	strace-k.expected \
	strace-k.test \
	strace-r.expected \
	strace.supp \
	sun_path.expected \
	syntax.sh \
	trace_creds.in \
	trace_fstat.in \
	trace_fstatfs.in \
	trace_lstat.in \
	trace_personality_32.in \
	trace_personality_64.in \
	trace_personality_regex_32.in \
	trace_personality_regex_64.in \
	trace_personality_regex_x32.in \
	trace_personality_x32.in \
	trace_question.in \
	trace_stat.in \
	trace_stat_like.in \
	trace_statfs.in \
	trace_statfs_like.in \
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
	xutimes.c \
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

.PHONY: check-valgrind-local
check-valgrind-local: $(check_LIBRARIES) $(check_PROGRAMS)

BUILT_SOURCES = ksysent.h
CLEANFILES = ksysent.h

include ../scno.am
