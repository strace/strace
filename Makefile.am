# Automake input for strace.
#
# Copyright (c) 2002-2009 Roland McGrath <roland@redhat.com>
# Copyright (c) 2006-2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2008-2015 Mike Frysinger <vapier@gentoo.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
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

if HAVE_M32_RUNTIME
TESTS_M32 = tests-m32
endif
if HAVE_MX32_RUNTIME
TESTS_MX32 = tests-mx32
endif
SUBDIRS = tests $(TESTS_M32) $(TESTS_MX32)

bin_PROGRAMS = strace
man_MANS = strace.1
bin_SCRIPTS = strace-graph strace-log-merge

OS		= linux
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir)/$(OS)/$(ARCH) \
	      -I$(srcdir)/$(OS)/$(ARCH) \
	      -I$(builddir)/$(OS) \
	      -I$(srcdir)/$(OS) \
	      -I$(builddir) \
	      -I$(srcdir)

AM_CFLAGS_FOR_BUILD = $(WARN_CFLAGS_FOR_BUILD)
AM_CPPFLAGS_FOR_BUILD = $(AM_CPPFLAGS)

include xlat/Makemodule.am

strace_CPPFLAGS = $(AM_CPPFLAGS)
strace_CFLAGS = $(AM_CFLAGS)
strace_LDFLAGS =
strace_LDADD = libstrace.a
noinst_LIBRARIES = libstrace.a

libstrace_a_CPPFLAGS = $(strace_CPPFLAGS)
libstrace_a_CFLAGS = $(strace_CFLAGS)
libstrace_a_SOURCES =	\
	fstatfs.c \
	fstatfs64.c \
	ipc.c \
	sigreturn.c	\
	socketcall.c \
	statfs.c \
	statfs64.c \
	sync_file_range.c \
	sync_file_range2.c \
	upeek.c		\
	upoke.c		\
	# end of libstrace_a_SOURCES

strace_SOURCES =	\
	access.c	\
	affinity.c	\
	aio.c		\
	alpha.c		\
	bjm.c		\
	block.c		\
	bpf.c		\
	btrfs.c		\
	cacheflush.c	\
	capability.c	\
	caps0.h		\
	caps1.h		\
	chdir.c		\
	chmod.c		\
	clone.c		\
	copy_file_range.c \
	count.c		\
	defs.h		\
	desc.c		\
	dirent.c	\
	dirent64.c	\
	dm.c		\
	empty.h		\
	epoll.c		\
	evdev.c		\
	eventfd.c	\
	execve.c	\
	fadvise.c	\
	fallocate.c	\
	fanotify.c	\
	fchownat.c	\
	fcntl.c		\
	fetch_seccomp_fprog.c \
	fetch_struct_flock.c \
	fetch_struct_mmsghdr.c \
	fetch_struct_msghdr.c \
	fetch_struct_stat.c \
	fetch_struct_stat64.c \
	fetch_struct_statfs.c \
	file_handle.c	\
	file_ioctl.c	\
	fs_x_ioctl.c	\
	flock.c		\
	flock.h		\
	futex.c		\
	gcc_compat.h	\
	get_robust_list.c \
	getcpu.c	\
	getcwd.c	\
	getrandom.c	\
	hdio.c		\
	hostname.c	\
	inotify.c	\
	io.c		\
	ioctl.c		\
	ioperm.c	\
	iopl.c		\
	ioprio.c	\
	ipc_defs.h	\
	ipc_msg.c	\
	ipc_msgctl.c	\
	ipc_sem.c	\
	ipc_shm.c	\
	ipc_shmctl.c	\
	kcmp.c		\
	kernel_types.h	\
	kexec.c		\
	keyctl.c	\
	ldt.c		\
	link.c		\
	linux/asm_stat.h \
	linux/x32/asm_stat.h \
	linux/x86_64/asm_stat.h \
	lookup_dcookie.c \
	loop.c		\
	lseek.c		\
	mem.c		\
	membarrier.c	\
	memfd_create.c	\
	mknod.c		\
	mmsghdr.c	\
	mount.c		\
	mpers_type.h	\
	mq.c		\
	msghdr.c	\
	msghdr.h	\
	mtd.c		\
	native_defs.h	\
	net.c		\
	netlink.c       \
	nsfs.c          \
	nsfs.h          \
	nsig.h		\
	numa.c		\
	oldstat.c	\
	open.c		\
	or1k_atomic.c	\
	pathtrace.c	\
	perf.c		\
	perf_event_struct.h \
	personality.c	\
	pkeys.c		\
	poll.c		\
	prctl.c		\
	print_dev_t.c	\
	print_mq_attr.c	\
	print_msgbuf.c	\
	print_sg_req_info.c \
	print_sigevent.c \
	print_statfs.c	\
	print_struct_stat.c \
	print_time.c	\
	print_timespec.c \
	print_timeval.c	\
	print_timex.c	\
	printmode.c	\
	printrusage.c	\
	printsiginfo.c	\
	printsiginfo.h	\
	process.c	\
	process_vm.c	\
	ptp.c		\
	ptrace.h	\
	qualify.c	\
	quota.c		\
	readahead.c	\
	readlink.c	\
	reboot.c	\
	regs.h		\
	renameat.c	\
	resource.c	\
	rt_sigframe.c	\
	rt_sigreturn.c	\
	rtc.c		\
	sched.c		\
	sched_attr.h	\
	scsi.c		\
	seccomp.c	\
	seccomp_fprog.h \
	sendfile.c	\
	sg_io_v3.c	\
	sg_io_v4.c	\
	sigaltstack.c	\
	sigevent.h	\
	signal.c	\
	signalfd.c	\
	sock.c		\
	sockaddr.c	\
	socketutils.c	\
	sram_alloc.c	\
	stat.c		\
	stat.h		\
	stat64.c	\
	statfs.c	\
	statfs.h	\
	statx.c		\
	statx.h		\
	strace.c	\
	swapon.c	\
	syscall.c	\
	sysctl.c	\
	sysent.h	\
	sysinfo.c	\
	syslog.c	\
	sysmips.c	\
	term.c		\
	time.c		\
	times.c		\
	truncate.c	\
	ubi.c		\
	uid.c		\
	uid16.c		\
	umask.c		\
	umount.c	\
	uname.c		\
	userfaultfd.c	\
	ustat.c		\
	util.c		\
	utime.c		\
	utimes.c	\
	v4l2.c		\
	wait.c		\
	xattr.c		\
	xlat.h		\
	xmalloc.c	\
	# end of strace_SOURCES

if USE_LIBUNWIND
strace_SOURCES += unwind.c
strace_CPPFLAGS += $(libunwind_CPPFLAGS)
strace_LDFLAGS += $(libunwind_LDFLAGS)
strace_LDADD += $(libunwind_LIBS)
endif

@CODE_COVERAGE_RULES@
CODE_COVERAGE_BRANCH_COVERAGE = 1
CODE_COVERAGE_GENHTML_OPTIONS = $(CODE_COVERAGE_GENHTML_OPTIONS_DEFAULT) \
	--prefix $(shell cd $(abs_top_srcdir)/.. && pwd || echo .)
CODE_COVERAGE_IGNORE_PATTERN = '/usr/include/*'
strace_CPPFLAGS += $(CODE_COVERAGE_CPPFLAGS)
strace_CFLAGS += $(CODE_COVERAGE_CFLAGS)
strace_LDADD += $(CODE_COVERAGE_LDFLAGS)

# Enable this to get link map generated
#strace_LDFLAGS += -Wl,-Map=strace.mapfile

EXTRA_DIST =				\
	$(man_MANS)			\
	.version			\
	COPYING				\
	CREDITS				\
	ChangeLog			\
	ChangeLog-CVS			\
	README-linux-ptrace		\
	debian/changelog		\
	debian/compat			\
	debian/control			\
	debian/copyright		\
	debian/rules			\
	debian/source/format		\
	debian/strace-udeb.install	\
	debian/strace.docs		\
	debian/strace.examples		\
	debian/strace.install		\
	debian/strace.manpages		\
	debian/strace64.install		\
	debian/strace64.manpages	\
	debian/watch			\
	errnoent.sh			\
	generate_sen.sh			\
	ioctl_iocdef.c			\
	ioctlsort.c			\
	linux/32/ioctls_inc.h		\
	linux/32/ioctls_inc_align16.h	\
	linux/32/ioctls_inc_align32.h	\
	linux/32/ioctls_inc_align64.h	\
	linux/32/syscallent.h		\
	linux/64/ioctls_inc.h		\
	linux/64/syscallent.h		\
	linux/aarch64/arch_regs.c	\
	linux/aarch64/arch_regs.h	\
	linux/aarch64/arch_rt_sigframe.c\
	linux/aarch64/arch_sigreturn.c	\
	linux/aarch64/errnoent1.h	\
	linux/aarch64/get_error.c	\
	linux/aarch64/get_scno.c	\
	linux/aarch64/get_syscall_args.c\
	linux/aarch64/ioctls_arch0.h	\
	linux/aarch64/ioctls_arch1.h	\
	linux/aarch64/ioctls_inc0.h	\
	linux/aarch64/ioctls_inc1.h	\
	linux/aarch64/set_error.c	\
	linux/aarch64/set_scno.c	\
	linux/aarch64/signalent1.h	\
	linux/aarch64/syscallent.h	\
	linux/aarch64/syscallent1.h	\
	linux/alpha/arch_getrval2.c	\
	linux/alpha/arch_regs.c		\
	linux/alpha/arch_regs.h		\
	linux/alpha/arch_rt_sigframe.c	\
	linux/alpha/arch_sigreturn.c	\
	linux/alpha/errnoent.h		\
	linux/alpha/get_error.c		\
	linux/alpha/get_scno.c		\
	linux/alpha/get_syscall_args.c	\
	linux/alpha/get_syscall_result.c\
	linux/alpha/ioctls_arch0.h	\
	linux/alpha/ioctls_inc0.h	\
	linux/alpha/set_error.c		\
	linux/alpha/set_scno.c		\
	linux/alpha/signalent.h		\
	linux/alpha/syscallent.h	\
	linux/alpha/userent.h		\
	linux/arc/arch_regs.c		\
	linux/arc/arch_regs.h		\
	linux/arc/arch_rt_sigframe.c	\
	linux/arc/get_error.c		\
	linux/arc/get_scno.c		\
	linux/arc/get_syscall_args.c	\
	linux/arc/ioctls_arch0.h	\
	linux/arc/ioctls_inc0.h		\
	linux/arc/set_error.c		\
	linux/arc/set_scno.c		\
	linux/arc/syscallent.h		\
	linux/arch_regs.h		\
	linux/arch_sigreturn.c		\
	linux/arm/arch_regs.c		\
	linux/arm/arch_regs.h		\
	linux/arm/arch_rt_sigframe.c	\
	linux/arm/arch_sigreturn.c	\
	linux/arm/get_error.c		\
	linux/arm/get_scno.c		\
	linux/arm/get_syscall_args.c	\
	linux/arm/ioctls_arch0.h	\
	linux/arm/ioctls_inc0.h		\
	linux/arm/set_error.c		\
	linux/arm/set_scno.c		\
	linux/arm/syscallent.h		\
	linux/arm/userent.h		\
	linux/avr32/arch_regs.c		\
	linux/avr32/arch_regs.h		\
	linux/avr32/arch_rt_sigframe.c	\
	linux/avr32/get_error.c		\
	linux/avr32/get_scno.c		\
	linux/avr32/get_syscall_args.c	\
	linux/avr32/ioctls_arch0.h	\
	linux/avr32/ioctls_inc0.h	\
	linux/avr32/set_error.c		\
	linux/avr32/set_scno.c		\
	linux/avr32/syscallent.h	\
	linux/avr32/userent.h		\
	linux/bfin/arch_regs.c		\
	linux/bfin/arch_rt_sigframe.c	\
	linux/bfin/get_error.c		\
	linux/bfin/get_scno.c		\
	linux/bfin/get_syscall_args.c	\
	linux/bfin/get_syscall_result.c	\
	linux/bfin/ioctls_arch0.h	\
	linux/bfin/ioctls_inc0.h	\
	linux/bfin/rt_sigframe.h	\
	linux/bfin/set_error.c		\
	linux/bfin/set_scno.c		\
	linux/bfin/syscallent.h		\
	linux/bfin/userent.h		\
	linux/crisv10/arch_regs.c	\
	linux/crisv10/arch_rt_sigframe.c\
	linux/crisv10/arch_sigreturn.c	\
	linux/crisv10/get_error.c	\
	linux/crisv10/get_scno.c	\
	linux/crisv10/get_syscall_args.c	\
	linux/crisv10/get_syscall_result.c	\
	linux/crisv10/rt_sigframe.h	\
	linux/crisv10/set_error.c	\
	linux/crisv10/set_scno.c	\
	linux/crisv10/syscallent.h	\
	linux/crisv10/userent.h		\
	linux/crisv32/arch_regs.c	\
	linux/crisv32/arch_rt_sigframe.c\
	linux/crisv32/arch_sigreturn.c	\
	linux/crisv32/get_error.c	\
	linux/crisv32/get_scno.c	\
	linux/crisv32/get_syscall_args.c	\
	linux/crisv32/get_syscall_result.c	\
	linux/crisv32/rt_sigframe.h	\
	linux/crisv32/set_error.c	\
	linux/crisv32/set_scno.c	\
	linux/crisv32/syscallent.h	\
	linux/crisv32/userent.h		\
	linux/dummy.h			\
	linux/errnoent.h		\
	linux/getregs_old.h		\
	linux/hppa/arch_regs.c		\
	linux/hppa/arch_regs.h		\
	linux/hppa/arch_rt_sigframe.c	\
	linux/hppa/errnoent.h		\
	linux/hppa/get_error.c		\
	linux/hppa/get_scno.c		\
	linux/hppa/get_syscall_args.c	\
	linux/hppa/get_syscall_result.c	\
	linux/hppa/ioctls_arch0.h	\
	linux/hppa/ioctls_inc0.h	\
	linux/hppa/rt_sigframe.h	\
	linux/hppa/set_error.c		\
	linux/hppa/set_scno.c		\
	linux/hppa/signalent.h		\
	linux/hppa/syscallent.h		\
	linux/i386/arch_regs.c		\
	linux/i386/arch_regs.h		\
	linux/i386/arch_rt_sigframe.c	\
	linux/i386/arch_sigreturn.c	\
	linux/i386/get_error.c		\
	linux/i386/get_scno.c		\
	linux/i386/get_syscall_args.c	\
	linux/i386/ioctls_arch0.h	\
	linux/i386/ioctls_inc0.h	\
	linux/i386/rt_sigframe.h	\
	linux/i386/set_error.c		\
	linux/i386/set_scno.c		\
	linux/i386/syscallent.h		\
	linux/i386/userent.h		\
	linux/i386/userent0.h		\
	linux/ia64/arch_getrval2.c	\
	linux/ia64/arch_regs.c		\
	linux/ia64/arch_regs.h		\
	linux/ia64/arch_rt_sigframe.c	\
	linux/ia64/get_error.c		\
	linux/ia64/get_scno.c		\
	linux/ia64/get_syscall_args.c	\
	linux/ia64/ioctls_arch0.h	\
	linux/ia64/ioctls_inc0.h	\
	linux/ia64/rt_sigframe.h	\
	linux/ia64/set_error.c		\
	linux/ia64/set_scno.c		\
	linux/ia64/syscallent.h		\
	linux/ia64/userent.h		\
	linux/inet_diag.h		\
	linux/m68k/arch_regs.c		\
	linux/m68k/arch_regs.h		\
	linux/m68k/arch_rt_sigframe.c	\
	linux/m68k/arch_sigreturn.c	\
	linux/m68k/get_error.c		\
	linux/m68k/get_scno.c		\
	linux/m68k/get_syscall_args.c	\
	linux/m68k/ioctls_arch0.h	\
	linux/m68k/ioctls_inc0.h	\
	linux/m68k/rt_sigframe.h	\
	linux/m68k/set_error.c		\
	linux/m68k/set_scno.c		\
	linux/m68k/syscallent.h		\
	linux/m68k/userent.h		\
	linux/metag/arch_regs.c		\
	linux/metag/arch_regs.h		\
	linux/metag/arch_rt_sigframe.c	\
	linux/metag/get_error.c		\
	linux/metag/get_scno.c		\
	linux/metag/get_syscall_args.c	\
	linux/metag/ioctls_arch0.h	\
	linux/metag/ioctls_inc0.h	\
	linux/metag/set_error.c		\
	linux/metag/set_scno.c		\
	linux/metag/syscallent.h	\
	linux/microblaze/arch_regs.c	\
	linux/microblaze/arch_rt_sigframe.c\
	linux/microblaze/arch_sigreturn.c	\
	linux/microblaze/get_error.c	\
	linux/microblaze/get_scno.c	\
	linux/microblaze/get_syscall_args.c	\
	linux/microblaze/get_syscall_result.c	\
	linux/microblaze/ioctls_arch0.h	\
	linux/microblaze/ioctls_inc0.h	\
	linux/microblaze/set_error.c	\
	linux/microblaze/set_scno.c	\
	linux/microblaze/syscallent.h	\
	linux/microblaze/userent.h	\
	linux/mips/arch_getrval2.c	\
	linux/mips/arch_regs.c		\
	linux/mips/arch_regs.h		\
	linux/mips/arch_rt_sigframe.c	\
	linux/mips/arch_sigreturn.c	\
	linux/mips/errnoent.h		\
	linux/mips/genstub.sh		\
	linux/mips/get_error.c		\
	linux/mips/get_scno.c		\
	linux/mips/get_syscall_args.c	\
	linux/mips/ioctls_arch0.h	\
	linux/mips/ioctls_inc0.h	\
	linux/mips/rt_sigframe.h	\
	linux/mips/set_error.c		\
	linux/mips/set_scno.c		\
	linux/mips/signalent.h		\
	linux/mips/syscallent-compat.h	\
	linux/mips/syscallent-n32.h	\
	linux/mips/syscallent-n64.h	\
	linux/mips/syscallent-o32.h	\
	linux/mips/syscallent.h		\
	linux/mips/userent.h		\
	linux/mtd-abi.h			\
	linux/netlink_diag.h		\
	linux/nios2/arch_regs.c		\
	linux/nios2/arch_regs.h		\
	linux/nios2/arch_rt_sigframe.c	\
	linux/nios2/get_error.c		\
	linux/nios2/get_scno.c		\
	linux/nios2/get_syscall_args.c	\
	linux/nios2/ioctls_arch0.h	\
	linux/nios2/ioctls_inc0.h	\
	linux/nios2/set_error.c		\
	linux/nios2/set_scno.c		\
	linux/nios2/syscallent.h	\
	linux/or1k/arch_regs.c		\
	linux/or1k/arch_regs.h		\
	linux/or1k/arch_rt_sigframe.c	\
	linux/or1k/get_error.c		\
	linux/or1k/get_scno.c		\
	linux/or1k/get_syscall_args.c	\
	linux/or1k/ioctls_arch0.h	\
	linux/or1k/ioctls_inc0.h	\
	linux/or1k/set_error.c		\
	linux/or1k/set_scno.c		\
	linux/or1k/syscallent.h		\
	linux/or1k/userent.h		\
	linux/personality.h		\
	linux/powerpc/arch_regs.c	\
	linux/powerpc/arch_regs.h	\
	linux/powerpc/arch_rt_sigframe.c\
	linux/powerpc/arch_sigreturn.c	\
	linux/powerpc/errnoent.h	\
	linux/powerpc/get_error.c	\
	linux/powerpc/get_scno.c	\
	linux/powerpc/get_syscall_args.c	\
	linux/powerpc/getregs_old.c	\
	linux/powerpc/getregs_old.h	\
	linux/powerpc/ioctls_arch0.h	\
	linux/powerpc/ioctls_inc0.h	\
	linux/powerpc/set_error.c	\
	linux/powerpc/set_scno.c	\
	linux/powerpc/syscallent.h	\
	linux/powerpc/userent.h		\
	linux/powerpc64/arch_regs.c	\
	linux/powerpc64/arch_regs.h	\
	linux/powerpc64/arch_rt_sigframe.c\
	linux/powerpc64/arch_sigreturn.c	\
	linux/powerpc64/errnoent.h	\
	linux/powerpc64/errnoent1.h	\
	linux/powerpc64/get_error.c	\
	linux/powerpc64/get_scno.c	\
	linux/powerpc64/get_syscall_args.c	\
	linux/powerpc64/getregs_old.c	\
	linux/powerpc64/getregs_old.h	\
	linux/powerpc64/ioctls_arch0.h	\
	linux/powerpc64/ioctls_arch1.h	\
	linux/powerpc64/ioctls_inc0.h	\
	linux/powerpc64/ioctls_inc1.h	\
	linux/powerpc64/rt_sigframe.h	\
	linux/powerpc64/set_error.c	\
	linux/powerpc64/set_scno.c	\
	linux/powerpc64/signalent1.h	\
	linux/powerpc64/syscallent.h	\
	linux/powerpc64/syscallent1.h	\
	linux/powerpc64/userent.h	\
	linux/ptp_clock.h		\
	linux/riscv/arch_regs.c		\
	linux/riscv/arch_regs.h		\
	linux/riscv/arch_rt_sigframe.c	\
	linux/riscv/errnoent1.h		\
	linux/riscv/get_error.c		\
	linux/riscv/get_scno.c		\
	linux/riscv/get_syscall_args.c	\
	linux/riscv/ioctls_arch0.h	\
	linux/riscv/ioctls_arch1.h	\
	linux/riscv/ioctls_inc0.h	\
	linux/riscv/ioctls_inc1.h	\
	linux/riscv/set_error.c		\
	linux/riscv/set_scno.c		\
	linux/riscv/signalent1.h	\
	linux/riscv/syscallent.h	\
	linux/riscv/syscallent1.h	\
	linux/rt_sigframe.h		\
	linux/s390/arch_regs.c		\
	linux/s390/arch_regs.h		\
	linux/s390/arch_rt_sigframe.c	\
	linux/s390/arch_sigreturn.c	\
	linux/s390/get_error.c		\
	linux/s390/get_scno.c		\
	linux/s390/get_syscall_args.c	\
	linux/s390/ioctls_arch0.h	\
	linux/s390/ioctls_inc0.h	\
	linux/s390/rt_sigframe.h	\
	linux/s390/set_error.c		\
	linux/s390/set_scno.c		\
	linux/s390/syscallent.h		\
	linux/s390/userent.h		\
	linux/s390/userent0.h		\
	linux/s390/userent1.h		\
	linux/s390x/arch_regs.c		\
	linux/s390x/arch_regs.h		\
	linux/s390x/arch_rt_sigframe.c	\
	linux/s390x/arch_sigreturn.c	\
	linux/s390x/get_error.c		\
	linux/s390x/get_scno.c		\
	linux/s390x/get_syscall_args.c	\
	linux/s390x/ioctls_arch0.h	\
	linux/s390x/ioctls_inc0.h	\
	linux/s390x/rt_sigframe.h	\
	linux/s390x/set_error.c		\
	linux/s390x/set_scno.c		\
	linux/s390x/syscallent.h	\
	linux/s390x/userent.h		\
	linux/sh/arch_getrval2.c	\
	linux/sh/arch_regs.c		\
	linux/sh/arch_rt_sigframe.c	\
	linux/sh/get_error.c		\
	linux/sh/get_scno.c		\
	linux/sh/get_syscall_args.c	\
	linux/sh/get_syscall_result.c	\
	linux/sh/ioctls_arch0.h		\
	linux/sh/ioctls_inc0.h		\
	linux/sh/set_error.c		\
	linux/sh/set_scno.c		\
	linux/sh/syscallent.h		\
	linux/sh/userent.h		\
	linux/sh/userent0.h		\
	linux/sh64/arch_regs.c		\
	linux/sh64/arch_regs.h		\
	linux/sh64/arch_rt_sigframe.c	\
	linux/sh64/get_error.c		\
	linux/sh64/get_scno.c		\
	linux/sh64/get_syscall_args.c	\
	linux/sh64/get_syscall_result.c	\
	linux/sh64/ioctls_arch0.h	\
	linux/sh64/ioctls_inc0.h	\
	linux/sh64/rt_sigframe.h	\
	linux/sh64/set_error.c		\
	linux/sh64/set_scno.c		\
	linux/sh64/syscallent.h		\
	linux/sh64/userent.h		\
	linux/signalent.h		\
	linux/sock_diag.h		\
	linux/sparc/arch_getrval2.c	\
	linux/sparc/arch_regs.c		\
	linux/sparc/arch_regs.h		\
	linux/sparc/arch_rt_sigframe.c	\
	linux/sparc/arch_sigreturn.c	\
	linux/sparc/errnoent.h		\
	linux/sparc/gen.pl		\
	linux/sparc/get_error.c		\
	linux/sparc/get_scno.c		\
	linux/sparc/get_syscall_args.c	\
	linux/sparc/ioctls_arch0.h	\
	linux/sparc/ioctls_inc0.h	\
	linux/sparc/rt_sigframe.h	\
	linux/sparc/set_error.c		\
	linux/sparc/set_scno.c		\
	linux/sparc/signalent.h		\
	linux/sparc/syscallent.h	\
	linux/sparc/userent.h		\
	linux/sparc64/arch_getrval2.c	\
	linux/sparc64/arch_regs.c	\
	linux/sparc64/arch_regs.h	\
	linux/sparc64/arch_rt_sigframe.c\
	linux/sparc64/arch_sigreturn.c	\
	linux/sparc64/errnoent.h	\
	linux/sparc64/errnoent1.h	\
	linux/sparc64/get_error.c	\
	linux/sparc64/get_scno.c	\
	linux/sparc64/get_syscall_args.c\
	linux/sparc64/ioctls_arch0.h	\
	linux/sparc64/ioctls_arch1.h	\
	linux/sparc64/ioctls_inc0.h	\
	linux/sparc64/ioctls_inc1.h	\
	linux/sparc64/rt_sigframe.h	\
	linux/sparc64/set_error.c	\
	linux/sparc64/set_scno.c	\
	linux/sparc64/signalent.h	\
	linux/sparc64/signalent1.h	\
	linux/sparc64/syscallent.h	\
	linux/sparc64/syscallent1.h	\
	linux/sparc64/userent.h		\
	linux/subcall.h			\
	linux/syscall.h			\
	linux/tile/arch_regs.c		\
	linux/tile/arch_regs.h		\
	linux/tile/arch_rt_sigframe.c	\
	linux/tile/arch_sigreturn.c	\
	linux/tile/errnoent1.h		\
	linux/tile/get_error.c		\
	linux/tile/get_scno.c		\
	linux/tile/get_syscall_args.c	\
	linux/tile/ioctls_arch0.h	\
	linux/tile/ioctls_arch1.h	\
	linux/tile/ioctls_inc0.h	\
	linux/tile/ioctls_inc1.h	\
	linux/tile/rt_sigframe.h	\
	linux/tile/set_error.c		\
	linux/tile/set_scno.c		\
	linux/tile/signalent1.h		\
	linux/tile/syscallent.h		\
	linux/tile/syscallent1.h	\
	linux/tile/userent.h		\
	linux/ubi-user.h		\
	linux/unix_diag.h		\
	linux/userent.h			\
	linux/userent0.h		\
	linux/x32/arch_regs.c		\
	linux/x32/arch_regs.h		\
	linux/x32/arch_rt_sigframe.c	\
	linux/x32/arch_sigreturn.c	\
	linux/x32/errnoent1.h		\
	linux/x32/get_error.c		\
	linux/x32/get_scno.c		\
	linux/x32/get_syscall_args.c	\
	linux/x32/ioctls_arch0.h	\
	linux/x32/ioctls_arch1.h	\
	linux/x32/ioctls_inc0.h		\
	linux/x32/ioctls_inc1.h		\
	linux/x32/rt_sigframe.h		\
	linux/x32/set_error.c		\
	linux/x32/set_scno.c		\
	linux/x32/signalent1.h		\
	linux/x32/syscallent.h		\
	linux/x32/syscallent1.h		\
	linux/x32/userent.h		\
	linux/x86_64/arch_regs.c	\
	linux/x86_64/arch_regs.h	\
	linux/x86_64/arch_rt_sigframe.c	\
	linux/x86_64/arch_sigreturn.c	\
	linux/x86_64/errnoent1.h	\
	linux/x86_64/errnoent2.h	\
	linux/x86_64/gentab.pl		\
	linux/x86_64/get_error.c	\
	linux/x86_64/get_scno.c		\
	linux/x86_64/get_syscall_args.c	\
	linux/x86_64/getregs_old.c	\
	linux/x86_64/getregs_old.h	\
	linux/x86_64/ioctls_arch0.h	\
	linux/x86_64/ioctls_arch1.h	\
	linux/x86_64/ioctls_arch2.h	\
	linux/x86_64/ioctls_inc0.h	\
	linux/x86_64/ioctls_inc1.h	\
	linux/x86_64/ioctls_inc2.h	\
	linux/x86_64/rt_sigframe.h	\
	linux/x86_64/set_error.c	\
	linux/x86_64/set_scno.c		\
	linux/x86_64/signalent1.h	\
	linux/x86_64/signalent2.h	\
	linux/x86_64/syscallent.h	\
	linux/x86_64/syscallent1.h	\
	linux/x86_64/syscallent2.h	\
	linux/x86_64/userent.h		\
	linux/xtensa/arch_regs.c	\
	linux/xtensa/arch_rt_sigframe.c	\
	linux/xtensa/get_error.c	\
	linux/xtensa/get_scno.c		\
	linux/xtensa/get_syscall_args.c	\
	linux/xtensa/get_syscall_result.c	\
	linux/xtensa/ioctls_arch0.h	\
	linux/xtensa/ioctls_inc0.h	\
	linux/xtensa/set_error.c	\
	linux/xtensa/set_scno.c		\
	linux/xtensa/syscallent.h	\
	linux/xtensa/userent.h		\
	maint/ioctls_gen.sh		\
	maint/ioctls_hex.sh		\
	maint/ioctls_sym.sh		\
	maint/print_ioctlent.c		\
	mpers.awk			\
	mpers.sh			\
	mpers_test.sh			\
	mpers_xlat.h			\
	scno.head			\
	signalent.sh			\
	strace-graph			\
	strace-log-merge		\
	strace.spec			\
	syscallent.sh			\
	$(XLAT_INPUT_FILES)		\
	$(XLAT_HEADER_FILES)		\
	xlat/gen.sh			\
	xlate.el

.PHONY: check-valgrind-local
check-valgrind-local:

.PHONY: srpm
srpm: dist-xz
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.xz

$(srcdir)/.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

strace_SOURCES_c = \
	$(filter %.c,$(strace_SOURCES)) $(filter %.c,$(libstrace_a_SOURCES))

sys_func.h: $(patsubst %,$(srcdir)/%,$(strace_SOURCES_c))
	for f in $^; do \
		sed -n 's/^SYS_FUNC(.*/extern &;/p' $$f; \
	done | sort -u > $@

syscallent_names = subcall.h syscallent.h syscallent1.h \
		   syscallent-n32.h syscallent-n64.h syscallent-o32.h
syscallent_patterns = $(patsubst %,\%/%,$(syscallent_names))
syscallent_files = $(filter $(syscallent_patterns),$(EXTRA_DIST))

sen.h: $(patsubst %,$(srcdir)/%,$(syscallent_files))
	for f in $^; do cat -- $$f; done | \
		$(srcdir)/generate_sen.sh > $@

dist-hook:
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version
	${AM_V_GEN}echo $(COPYRIGHT_YEAR) > $(distdir)/.year

today = $(shell date +%Y-%m-%d)
version_regexp = $(subst .,\.,$(VERSION))
news_check_regexp = 'Noteworthy changes in release $(version_regexp) ($(today))'

news-check: NEWS
	$(AM_V_GEN)if head -1 $< |				\
	  grep -x $(news_check_regexp) >/dev/null; then		\
		:;						\
	else							\
		echo >&2 '$<: check failed';			\
		exit 1;						\
	fi

ioctlsort_CC = $(CC_FOR_BUILD)
ioctlsort_DEFS = $(DEFS)
ioctlsort_INCLUDES = $(DEFAULT_INCLUDES) $(INCLUDES)
ioctlsort_CPPFLAGS = $(AM_CPPFLAGS_FOR_BUILD) $(CPPFLAGS_FOR_BUILD)
ioctlsort_CFLAGS = $(AM_CFLAGS_FOR_BUILD) $(CFLAGS_FOR_BUILD)
ioctlsort_LDFLAGS = $(AM_LDFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD)

ioctls_inc_h = $(wildcard $(srcdir)/$(OS)/$(ARCH)/ioctls_inc*.h)
ioctlent_h = $(patsubst $(srcdir)/$(OS)/$(ARCH)/ioctls_inc%,ioctlent%,$(ioctls_inc_h))
ioctl_redefs_h = $(filter-out ioctl_redefs0.h,$(subst ioctlent,ioctl_redefs,$(ioctlent_h)))

ioctl_redefs%.h: ioctlent%.h ioctlent0.h
	sort $< > $<-t
	sort ioctlent0.h | comm -23 $<-t - | \
		sed -r -n 's/^\{ "([^"]+)", (0x[[:xdigit:]]+) \},$$/#ifdef \1\n# undef \1\n# define \1 \2\n#endif/p' \
		> $@-t
	rm -f $<-t
	mv $@-t $@

ioctlent%.h: ioctlsort%
	./$< > $@

# Need to pick up <linux/ioctl.h> definitions *for host* while compiling
# ioctlsort *for build*, hence this magic.
ioctl_iocdef.i: $(srcdir)/ioctl_iocdef.c
	$(CPP) -P $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(strace_CPPFLAGS) $(CPPFLAGS) $< -o $@

ioctl_iocdef.h: ioctl_iocdef.i
	sed -n 's/^DEFINE HOST/#define /p' $< > $@

ioctlsort%$(BUILD_EXEEXT): ioctlsort%.o
	$(ioctlsort_CC) $(ioctlsort_CFLAGS) $(ioctlsort_LDFLAGS) $< -o $@

ioctlsort%.o: ioctls_all%.h ioctl_iocdef.h $(srcdir)/ioctlsort.c
	$(ioctlsort_CC) $(ioctlsort_DEFS) $(ioctlsort_INCLUDES) $(ioctlsort_CPPFLAGS) $(ioctlsort_CFLAGS) -DIOCTLSORT_INC=\"$<\" -c -o $@ $(srcdir)/ioctlsort.c

ioctls_all%.h: $(srcdir)/$(OS)/$(ARCH)/ioctls_inc%.h $(srcdir)/$(OS)/$(ARCH)/ioctls_arch%.h
	cat $^ > $@

BUILT_SOURCES = $(ioctl_redefs_h) $(ioctlent_h) \
		native_printer_decls.h native_printer_defs.h printers.h sen.h sys_func.h .version
CLEANFILES    = $(ioctl_redefs_h) $(ioctlent_h) $(mpers_preproc_files) \
		ioctl_iocdef.h ioctl_iocdef.i \
		native_printer_decls.h native_printer_defs.h printers.h sen.h sys_func.h
DISTCLEANFILES = gnu/stubs-32.h gnu/stubs-x32.h

include scno.am

$(strace_OBJECTS): scno.h

# defines mpers_source_files
include mpers.am
srcdir_mpers_source_files = $(patsubst %,$(srcdir)/%,$(mpers_source_files))
mpers_preproc_files = $(mpers_source_files:.c=.c.mpers.i)

mpers_NAME =
mpers_PREFIX = $(mpers_NAME)_
mpers_DEFS = $(DEFS)
mpers_INCLUDES = $(DEFAULT_INCLUDES) $(INCLUDES)
mpers_CPPFLAGS = $(strace_CPPFLAGS) $(CPPFLAGS)
mpers_sh_opts = $(mpers_DEFS) $(mpers_INCLUDES) $(mpers_CPPFLAGS)
libmpers_CPPFLAGS = $(strace_CPPFLAGS) -DIN_MPERS
libmpers_CFLAGS = $(strace_CFLAGS)

# mpers targets

mpers-m%.stamp: $(srcdir_mpers_source_files) | printers.h
	for f in $^; do \
		CC="$(CC)" CFLAGS="$(mpers_sh_opts) -DMPERS_IS_$(mpers_NAME)" \
		CPP="$(CPP)" CPPFLAGS="$(mpers_sh_opts) -DIN_MPERS -DMPERS_IS_$(mpers_NAME)" \
		$(srcdir)/mpers.sh -$(mpers_NAME) $$f || exit; \
	done
	> $@

m%_type_defs.h: $(srcdir_mpers_source_files)
	for f in $^; do \
		sed -r -n 's/^#[[:space:]]*include DEF_MPERS_TYPE\(([^)]+)\)/#ifdef MPERS_$(mpers_PREFIX)\1\n# define \1 MPERS_$(mpers_PREFIX)\1\n#endif/p' $$f || exit; \
	done > $@-t
	echo '#undef MPERS_PRINTER_NAME' >> $@-t
	echo '#define MPERS_PRINTER_NAME(printer_name) printer_name' >> $@-t
	echo '#include "$(mpers_PREFIX)printer_decls.h"' >> $@-t
	echo '#include MPERS_$(mpers_PREFIX)IOCTL_MACROS' >> $@-t
	mv $@-t $@

m%_funcs.h: $(srcdir_mpers_source_files)
	for f in $^; do \
		sed -r -n 's/^SYS_FUNC\(([^)]+)\)/#undef sys_\1\n#define sys_\1 $(mpers_PREFIX)sys_\1/p' $$f || exit; \
	done > $@-t && \
	echo '#include "sys_func.h"' >> $@-t
	mv $@-t $@

# printers

%.c.mpers.i: $(srcdir)/%.c
	$(CPP) -P $(mpers_sh_opts) -DIN_MPERS_BOOTSTRAP $< -o $@

mpers_printer_decl_pattern = ^MPERS_PRINTER_DECL\(([^,)]+),[[:space:]]*([^,)]+),[[:space:]]*([^)]+)\)$$

printers.h: $(mpers_preproc_files)
	echo '/* Generated by Makefile from $^; do not edit. */' > $@-t
	echo 'typedef struct {' >> $@-t
	for f in $^; do \
		sed -r -n 's/$(mpers_printer_decl_pattern)/ \1 (*\2)(\3);\n#define \2 MPERS_PRINTER_NAME(\2)\n/p' $$f \
		|| exit; \
	done >> $@-t
	echo '} struct_printers;' >> $@-t
	echo 'extern const struct_printers *printers;' >> $@-t
	echo '#define MPERS_PRINTER_NAME(printer_name) printers->printer_name' >> $@-t
	mv $@-t $@

%_printer_decls.h: $(mpers_preproc_files)
	echo '/* Generated by Makefile from $^; do not edit. */' > $@-t
	for f in $^; do \
		sed -r -n 's/$(mpers_printer_decl_pattern)/extern \1 $(mpers_PREFIX)\2(\3);/p' $$f \
		|| exit; \
	done >> $@-t
	mv $@-t $@

%_printer_defs.h: $(mpers_preproc_files)
	echo '/* Generated by Makefile from $^; do not edit. */' > $@-t
	for f in $^; do \
		sed -r -n 's/$(mpers_printer_decl_pattern)/.\2 = $(mpers_PREFIX)\2,/p' $$f \
		|| exit; \
	done >> $@-t
	mv $@-t $@

native_printer_decls.h native_printer_defs.h: mpers_PREFIX =

if HAVE_M32_MPERS

strace_LDADD += libmpers-m32.a
noinst_LIBRARIES += libmpers-m32.a
libmpers_m32_a_SOURCES = $(mpers_source_files)
libmpers_m32_a_CPPFLAGS = $(libmpers_CPPFLAGS) -DMPERS_IS_m32 -I$(builddir)/mpers-m32
libmpers_m32_a_CFLAGS = $(libmpers_CFLAGS)
mpers_m32_targets = mpers-m32.stamp m32_type_defs.h m32_funcs.h m32_printer_decls.h m32_printer_defs.h

BUILT_SOURCES += $(mpers_m32_targets)
CLEANFILES    += $(mpers_m32_targets)

$(mpers_m32_targets): mpers_NAME = m32

endif # HAVE_M32_MPERS

if HAVE_MX32_MPERS

strace_LDADD += libmpers-mx32.a
noinst_LIBRARIES += libmpers-mx32.a
libmpers_mx32_a_SOURCES = $(mpers_source_files)
libmpers_mx32_a_CPPFLAGS = $(libmpers_CPPFLAGS) -DMPERS_IS_mx32 -I$(builddir)/mpers-mx32
libmpers_mx32_a_CFLAGS = $(libmpers_CFLAGS)
mpers_mx32_targets = mpers-mx32.stamp mx32_type_defs.h mx32_funcs.h mx32_printer_decls.h mx32_printer_defs.h

BUILT_SOURCES += $(mpers_mx32_targets)
CLEANFILES    += $(mpers_mx32_targets)

$(mpers_mx32_targets): mpers_NAME = mx32

endif # HAVE_MX32_MPERS

clean-local:
	-rm -rf mpers-m32 mpers-mx32

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(srcdir)/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(srcdir)/.version
	@rm -f $@.new
	(cd $(srcdir); \
	 ./gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(srcdir)/CREDITS.in $(srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(srcdir)/.version
	$(AM_V_GEN)							\
	(								\
	  cd $(srcdir);							\
	  sed '/^##/,$$d' CREDITS.in;					\
	  { sed -n '1,/^##>/d; s/   */\t/; s/^./&/p' CREDITS.in;	\
	    git log --pretty=format:'%aN	%aE';			\
	  } | LC_ALL=C sort -u						\
	    | awk -F'\t' '{printf("\t%s <%s>\n",$$1,$$2)}';		\
	) > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

endif
