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
	skip_unavailable.c \
	sprintrc.c \
	tail_alloc.c \
	tests.h \
	tprintf.c \
	# end of libtests_a_SOURCES
libtests_a_CPPFLAGS = $(AM_CPPFLAGS) -D_FILE_OFFSET_BITS=64
check_LIBRARIES = libtests.a
LDADD = libtests.a

include pure_executables.am

check_PROGRAMS = $(PURE_EXECUTABLES) \
	answer \
	attach-f-p \
	attach-f-p-cmd \
	attach-p-cmd-cmd \
	attach-p-cmd-p \
	caps-abbrev \
	count-f \
	execve-v \
	execveat-v \
	filter-unavailable \
	fork-f \
	getpid	\
	getppid	\
	gettid \
	int_0x80 \
	ioctl_dm-v \
	ioctl_evdev-v \
	ioctl_loop-nv \
	ioctl_loop-v \
	ioctl_nsfs \
	ioctl_rtc-v \
	ksysent \
	mmsg-silent \
	mmsg_name-v \
	msg_control-v \
	net-accept-connect \
	netlink_inet_diag \
	netlink_netlink_diag \
	netlink_unix_diag \
	nsyscalls \
	pc \
	perf_event_open_nonverbose \
	perf_event_open_unabbrev \
	ppoll-v \
	prctl-seccomp-filter-v \
	prctl-seccomp-strict \
	print_maxfd \
	qual_fault \
	qual_inject-error-signal \
	qual_inject-retval \
	qual_inject-signal \
	qual_signal \
	quotactl-v \
	quotactl-xfs-v \
	redirect-fds \
	restart_syscall \
	scm_rights \
	seccomp-filter-v \
	seccomp-strict \
	set_ptracer_any \
	signal_receive \
	sleep \
	stack-fcall \
	threads-execve \
	unix-pair-send-recv \
	unix-pair-sendto-recvfrom \
	vfork-f \
	wait4-v \
	waitid-v \
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
	futex.test \
	getuid.test \
	ioctl.test \
	ioctl_dm-v.test \
	ioctl_dm.test \
	ioctl_loop-nv.test \
	ioctl_nsfs.test \
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
	qual_fault-exit_group.test \
	readv.test \
	scm_rights-fd.test \
	seccomp-strict.test \
	sigaction.test \
	sigaltstack.test \
	sun_path.test \
	uio.test \
	umount.test \
	umount2.test \
	umovestr.test \
	umovestr2.test \
	uname.test \
	unix-pair-send-recv.test \
	unix-pair-sendto-recvfrom.test \
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
	get_regs.test \
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

EXTRA_DIST = \
	caps-abbrev.awk \
	caps.awk \
	clock.in \
	count-f.expected \
	eventfd.expected \
	fadvise.h \
	filter-unavailable.expected \
	fstatat.c \
	fstatx.c \
	gen_pure_executables.sh \
	gen_tests.in \
	gen_tests.sh \
	getresugid.c \
	init.sh \
	init_delete_module.h \
	ioctl-v.sh \
	ipc.sh \
	ipc_msgbuf.expected \
	ksysent.sed \
	lstatx.c \
	match.awk \
	net.expected \
	oldselect.expected \
	pipe.expected \
	process_vm_readv_writev.c \
	pure_executables.list \
	qual_fault-exit_group.expected \
	qual_inject-error-signal.expected \
	qual_inject-signal.expected \
	quotactl.h \
	regex.in \
	run.sh \
	sched.in \
	scno_tampering.sh \
	setfsugid.c \
	setresugid.c \
	setreugid.c \
	setugid.c \
	sigaction.awk \
	sigaltstack.expected \
	sockname.c \
	strace-C.expected \
	strace-E.expected \
	strace-T.expected \
	strace-ff.expected \
	strace-k.test \
	strace-r.expected \
	strace.supp \
	struct_flock.c \
	sun_path.expected \
	trace_fstat.in \
	trace_fstatfs.in \
	trace_lstat.in \
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

CLEANFILES = ksysent.h

include ../scno.am
