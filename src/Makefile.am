# Automake input for strace.
#
# Copyright (c) 2002-2009 Roland McGrath <roland@redhat.com>
# Copyright (c) 2006-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2008-2015 Mike Frysinger <vapier@gentoo.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2002-2024 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

bin_PROGRAMS = strace
bin_SCRIPTS = strace-log-merge

OS		= linux
# ARCH is `x86_64', `i386', `aarch64', `powerpc64le', etc.
ARCH		= @arch@
# KARCH is `x86', `arm64', `arm', `powerpc', etc.
KARCH		= @karch@

READELF		= @READELF@

bundled_CPPFLAGS =
if USE_BUNDLED_HEADERS
bundled_CPPFLAGS += -I$(top_srcdir)/bundled/$(OS)/arch/$(KARCH)/include/uapi \
		    -I$(top_srcdir)/bundled/$(OS)/include/uapi
endif

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir)/$(OS)/$(ARCH) \
	      -I$(srcdir)/$(OS)/$(ARCH) \
	      -I$(builddir)/$(OS)/generic \
	      -I$(srcdir)/$(OS)/generic \
	      -I$(builddir) \
	      -I$(srcdir) \
	      $(bundled_CPPFLAGS)

AM_CFLAGS_FOR_BUILD = $(WARN_CFLAGS_FOR_BUILD)
AM_CPPFLAGS_FOR_BUILD = $(AM_CPPFLAGS)

include xlat/Makemodule.am

strace_CPPFLAGS = $(AM_CPPFLAGS) -DIN_STRACE=1
strace_CFLAGS = $(AM_CFLAGS)
strace_LDFLAGS =
strace_LDADD = libstrace.a $(clock_LIBS) $(timer_LIBS)
strace_SOURCES = strace.c

noinst_PROGRAMS = disable_ptrace_get_syscall_info disable_ptrace_getregset
disable_ptrace_get_syscall_info_LDADD = $(strace_LDADD)
disable_ptrace_getregset_LDADD = $(strace_LDADD)

noinst_LIBRARIES = libstrace.a
libstrace_a_CPPFLAGS = $(strace_CPPFLAGS)
libstrace_a_CFLAGS = $(strace_CFLAGS)
libstrace_a_SOURCES =	\
	access.c	\
	affinity.c	\
	aio.c		\
	alpha.c		\
	arch_defs.h	\
	basic_filters.c	\
	bind.c		\
	bjm.c		\
	block.c		\
	bpf.c		\
	bpf_attr.h	\
	bpf_filter.c	\
	bpf_filter.h	\
	bpf_fprog.h	\
	bpf_seccomp_filter.c \
	bpf_sock_filter.c \
	btrfs.c		\
	cacheflush.c	\
	cachestat.c	\
	cachestat.h	\
	capability.c	\
	caps0.h		\
	caps1.h		\
	chdir.c		\
	chmod.c		\
	clone.c		\
	close_range.c	\
	copy_file_range.c \
	count.c		\
	counter_ioctl.c	\
	defs.h		\
	delay.c		\
	delay.h		\
	desc.c		\
	dirent.c	\
	dirent64.c	\
	dirent_types.c	\
	dm.c		\
	dup.c		\
	dyxlat.c	\
	empty.h		\
	epoll.c		\
	error_prints.c	\
	error_prints.h	\
	evdev.c		\
	evdev_mpers.c	\
	eventfd.c	\
	execve.c	\
	exitkill.c	\
	exitkill.h	\
	fadvise.c	\
	fallocate.c	\
	fanotify.c	\
	fchownat.c	\
	fcntl.c		\
	fetch_bpf_fprog.c \
	fetch_indirect_syscall_args.c \
	fetch_struct_flock.c \
	fetch_struct_iovec.c \
	fetch_struct_keyctl_kdf_params.c \
	fetch_struct_mmsghdr.c \
	fetch_struct_msghdr.c \
	fetch_struct_stat.c \
	fetch_struct_stat64.c \
	fetch_struct_statfs.c \
	fetch_struct_xfs_quotastat.c \
	file_handle.c	\
	filter.h	\
	filter_qualify.c \
	filter_seccomp.c \
	filter_seccomp.h \
	flock.c		\
	fs_0x94_ioctl.c	\
	fs_f_ioctl.c	\
	fs_x_ioctl.c	\
	fsconfig.c	\
	fsmount.c	\
	fsopen.c	\
	fspick.c	\
	fstatfs.c \
	fstatfs64.c \
	futex.c		\
	futex2.c	\
	gcc_compat.h	\
	gen/gen_hdio.c	\
	gen/generated.h	\
	get_personality.c \
	get_personality.h \
	get_robust_list.c \
	getcpu.c	\
	getcwd.c	\
	getpagesize.c \
	getpid.c	\
	getrandom.c	\
	gpio_ioctl.c	\
	hdio.c		\
	hostname.c	\
	inotify.c	\
	inotify_ioctl.c	\
	io.c		\
	io_uring.c	\
	ioctl.c		\
	ioperm.c	\
	ioprio.c	\
	iovec.h		\
	ipc.c \
	ipc_defs.h	\
	ipc_msg.c	\
	ipc_msgctl.c	\
	ipc_sem.c	\
	ipc_semctl.c	\
	ipc_shm.c	\
	ipc_shmctl.c	\
	kcmp.c		\
	kd_ioctl.c	\
	kd_mpers_ioctl.c \
	kernel_dirent.h	\
	kernel_fcntl.h	\
	kernel_rusage.h	\
	kernel_time_types.h \
	kernel_timespec.h \
	kernel_timeval.h \
	kernel_timex.h	\
	kernel_types.h	\
	kernel_v4l2_types.h \
	kexec.c		\
	keyctl.c	\
	keyctl_kdf_params.h \
	kill_save_errno.h \
	kvm.c		\
	landlock.c	\
	largefile_wrappers.h \
	ldt.c		\
	link.c		\
	linux/x32/asm_stat.h \
	linux/x86_64/asm_stat.h \
	lirc_ioctl.c	\
	list.h		\
	listen.c	\
	listmount.c	\
	lookup_dcookie.c \
	loop.c		\
	lseek.c		\
	lsm.c		\
	macros.h	\
	map_shadow_stack.c \
	mem.c		\
	membarrier.c	\
	memfd_create.c	\
	memfd_secret.c	\
	mknod.c		\
	mmap_cache.c	\
	mmap_cache.h	\
	mmap_notify.c	\
	mmap_notify.h	\
	mmsghdr.c	\
	mount.c		\
	mount_setattr.c	\
	move_mount.c	\
	mpers_type.h	\
	mq.c		\
	msghdr.c	\
	msghdr.h	\
	mtd.c		\
	native_defs.h	\
	nbd_ioctl.c	\
	negated_errno.h	\
	net.c		\
	netlink.c       \
	netlink.h	\
	netlink_crypto.c \
	netlink_inet_diag.c \
	netlink_kobject_uevent.c \
	netlink_kobject_uevent.h \
	netlink_netfilter.c \
	netlink_netlink_diag.c \
	netlink_packet_diag.c \
	netlink_route.c	\
	netlink_route.h	\
	netlink_selinux.c \
	netlink_smc_diag.c \
	netlink_sock_diag.c \
	netlink_sock_diag.h \
	netlink_unix_diag.c \
	nlattr.c	\
	nlattr.h	\
	nsfs.c          \
	nsig.h		\
	numa.c		\
	number_set.c	\
	number_set.h	\
	oldstat.c	\
	open.c		\
	open_tree.c	\
	or1k_atomic.c	\
	pathtrace.c	\
	perf.c		\
	perf_event_struct.h \
	perf_ioctl.c	\
	personality.c	\
	pidfd_getfd.c	\
	pidfd_open.c	\
	pidns.c		\
	pkeys.c		\
	poke.c		\
	poke.h		\
	poll.c		\
	prctl.c		\
	print_dev_t.c	\
	print_fields.h	\
	print_group_req.c \
	print_ifindex.c	\
	print_instruction_pointer.c \
	print_kernel_sigset.c \
	print_kernel_version.c \
	print_mac.c	\
	print_mq_attr.c	\
	print_msgbuf.c	\
	print_sg_req_info.c \
	print_sigevent.c \
	print_statfs.c	\
	print_struct_stat.c \
	print_syscall_number.c \
	print_time.c	\
	print_timespec.h \
	print_timespec32.c \
	print_timespec64.c \
	print_timeval.c	\
	print_timeval64.c \
	print_timex.c	\
	print_timex.h	\
	print_utils.h	\
	printmode.c	\
	printrusage.c	\
	printsiginfo.c	\
	printsiginfo.h	\
	process_vm.c	\
	ptp.c		\
	ptrace.c	\
	ptrace.h	\
	ptrace_syscall_info.c \
	ptrace_syscall_info.h \
	quota.c		\
	random_ioctl.c	\
	readahead.c	\
	readlink.c	\
	reboot.c	\
	regs.h		\
	regset.c	\
	renameat.c	\
	resource.c	\
	retval.c	\
	retval.h	\
	riscv.c		\
	rt_sigframe.c	\
	rt_sigreturn.c	\
	rtc.c		\
	rtnl_addr.c	\
	rtnl_addrlabel.c \
	rtnl_cachereport.c \
	rtnl_dcb.c	\
	rtnl_link.c	\
	rtnl_mdb.c	\
	rtnl_neigh.c	\
	rtnl_neightbl.c	\
	rtnl_netconf.c	\
	rtnl_nh.c	\
	rtnl_nsid.c	\
	rtnl_route.c	\
	rtnl_rule.c	\
	rtnl_stats.c	\
	rtnl_tc.c	\
	rtnl_tc_action.c \
	s390.c		\
	sched.c		\
	sched_attr.h	\
	scsi.c		\
	seccomp.c	\
	seccomp_ioctl.c	\
	sendfile.c	\
	sg_io_v3.c	\
	sg_io_v4.c	\
	shutdown.c	\
	sigaltstack.c	\
	sigevent.h	\
	signal.c	\
	signalfd.c	\
	sigreturn.c	\
	sock.c		\
	sockaddr.c	\
	socketcall.c \
	socketutils.c	\
	sparc.c		\
	sram_alloc.c	\
	stage_output.c	\
	stat.c		\
	stat.h		\
	stat64.c	\
	statfs.c	\
	statfs.c \
	statfs.h	\
	statfs64.c \
	static_assert.h	\
	statmount.c	\
	statx.c		\
	statx.h		\
	strauss.c	\
	strauss.h	\
	string_to_uint.c \
	string_to_uint.h \
	swapon.c	\
	sync_file_range.c \
	sync_file_range2.c \
	syscall.c	\
	syscall.h	\
	syscall_dummy.h	\
	syscall_name.c	\
	sysctl.c	\
	sysent.h	\
	sysent_shorthand_defs.h \
	sysent_shorthand_undefs.h \
	sysinfo.c	\
	syslog.c	\
	sysmips.c	\
	tee.c		\
	term.c		\
	time.c		\
	times.c		\
	trace_event.h	\
	trie.c		\
	trie.h		\
	truncate.c	\
	ubi.c		\
	ucopy.c		\
	uid.c		\
	uid16.c		\
	umask.c		\
	umount.c	\
	uname.c		\
	upeek.c		\
	upoke.c		\
	userfaultfd.c	\
	ustat.c		\
	util.c		\
	utime.c		\
	utimes.c	\
	v4l2.c		\
	wait.c		\
	wait.h		\
	watchdog_ioctl.c \
	xattr.c		\
	xgetdents.c	\
	xgetdents.h	\
	xlat.c		\
	xlat.h		\
	xmalloc.c	\
	xmalloc.h	\
	xstring.h	\
	$(TYPES_HEADER_FILES) \
	$(strace_SOURCES_check) \
	# end of libstrace_a_SOURCES

strace_SOURCES_check = bpf_attr_check.c $(TYPES_CHECK_FILES)

if ENABLE_STACKTRACE
libstrace_a_SOURCES += unwind.c unwind.h
if USE_LIBDW
libstrace_a_SOURCES += unwind-libdw.c
strace_CPPFLAGS += $(libdw_CPPFLAGS)
strace_CFLAGS += $(libdw_CFLAGS)
strace_LDFLAGS += $(libdw_LDFLAGS)
strace_LDADD += $(libdw_LIBS)
endif
if USE_LIBUNWIND
libstrace_a_SOURCES += unwind-libunwind.c
strace_CPPFLAGS += $(libunwind_CPPFLAGS)
strace_LDFLAGS += $(libunwind_LDFLAGS)
strace_LDADD += $(libunwind_LIBS)
endif
if USE_DEMANGLE
strace_CPPFLAGS += $(libiberty_CPPFLAGS)
strace_LDFLAGS += $(libiberty_LDFLAGS)
strace_LDADD += $(libiberty_LIBS)
endif
endif

if ENABLE_SECONTEXT
libstrace_a_SOURCES +=	\
	secontext.c	\
	secontext.h
strace_CPPFLAGS += $(libselinux_CPPFLAGS)
strace_LDFLAGS += $(libselinux_LDFLAGS)
strace_LDADD += $(libselinux_LIBS)
endif

strace_CPPFLAGS += $(CODE_COVERAGE_CPPFLAGS)
strace_CFLAGS += $(CODE_COVERAGE_CFLAGS)
strace_LDADD += $(CODE_COVERAGE_LIBS)

# Enable this to get link map generated
#strace_LDFLAGS += -Wl,-Map=strace.mapfile

EXTRA_DIST =				\
	disable_ptrace_request.c	\
	gen_bpf_attr_check.sh		\
	generate_sen.sh			\
	ioctl_iocdef.c			\
	ioctls_zfs.h			\
	ioctlsort.c			\
	linux/32/ioctls_inc.h		\
	linux/32/ioctls_inc_align16.h	\
	linux/32/ioctls_inc_align32.h	\
	linux/32/ioctls_inc_align64.h	\
	linux/32/subcallent.h		\
	linux/32/syscallent-common-32.h	\
	linux/32/syscallent-time32.h	\
	linux/32/syscallent.h		\
	linux/64/ioctls_inc.h		\
	linux/64/subcallent.h		\
	linux/64/syscallent.h		\
	linux/aarch64/arch_defs_.h	\
	linux/aarch64/arch_get_personality.c \
	linux/aarch64/arch_prstatus_regset.c \
	linux/aarch64/arch_prstatus_regset.h \
	linux/aarch64/arch_regs.c	\
	linux/aarch64/arch_sigreturn.c	\
	linux/aarch64/get_error.c	\
	linux/aarch64/get_scno.c	\
	linux/aarch64/get_syscall_args.c	\
	linux/aarch64/ioctls_arch0.h	\
	linux/aarch64/ioctls_arch1.h	\
	linux/aarch64/ioctls_inc0.h	\
	linux/aarch64/ioctls_inc1.h	\
	linux/aarch64/nr_prefix.c	\
	linux/aarch64/raw_syscall.h	\
	linux/aarch64/set_error.c	\
	linux/aarch64/set_scno.c	\
	linux/aarch64/shuffle_scno.c	\
	linux/aarch64/syscallent.h	\
	linux/aarch64/syscallent1.h	\
	linux/alpha/arch_defs_.h	\
	linux/alpha/arch_getrval2.c	\
	linux/alpha/arch_regs.c		\
	linux/alpha/arch_sigreturn.c	\
	linux/alpha/errnoent.h		\
	linux/alpha/get_error.c		\
	linux/alpha/get_scno.c		\
	linux/alpha/get_syscall_args.c	\
	linux/alpha/get_syscall_result.c	\
	linux/alpha/ioctls_arch0.h	\
	linux/alpha/ioctls_inc0.h	\
	linux/alpha/raw_syscall.h	\
	linux/alpha/set_error.c		\
	linux/alpha/set_scno.c		\
	linux/alpha/signalent.h		\
	linux/alpha/syscallent.h	\
	linux/alpha/userent.h		\
	linux/arc/arch_defs_.h		\
	linux/arc/arch_regs.c		\
	linux/arc/get_error.c		\
	linux/arc/get_scno.c		\
	linux/arc/get_syscall_args.c	\
	linux/arc/ioctls_arch0.h	\
	linux/arc/ioctls_inc0.h		\
	linux/arc/raw_syscall.h		\
	linux/arc/set_error.c		\
	linux/arc/set_scno.c		\
	linux/arc/syscallent.h		\
	linux/arm/arch_defs_.h		\
	linux/arm/arch_prstatus_regset.c \
	linux/arm/arch_prstatus_regset.h \
	linux/arm/arch_regs.c		\
	linux/arm/arch_sigreturn.c	\
	linux/arm/get_error.c		\
	linux/arm/get_scno.c		\
	linux/arm/get_syscall_args.c	\
	linux/arm/ioctls_arch0.h	\
	linux/arm/ioctls_inc0.h		\
	linux/arm/nr_prefix.c		\
	linux/arm/raw_syscall.h		\
	linux/arm/set_error.c		\
	linux/arm/set_scno.c		\
	linux/arm/shuffle_scno.c	\
	linux/arm/syscallent.h		\
	linux/arm/userent.h		\
	linux/avr32/arch_defs_.h	\
	linux/avr32/arch_regs.c		\
	linux/avr32/get_error.c		\
	linux/avr32/get_scno.c		\
	linux/avr32/get_syscall_args.c	\
	linux/avr32/ioctls_arch0.h	\
	linux/avr32/ioctls_inc0.h	\
	linux/avr32/raw_syscall.h	\
	linux/avr32/set_error.c		\
	linux/avr32/set_scno.c		\
	linux/avr32/syscallent.h	\
	linux/avr32/userent.h		\
	linux/bfin/arch_defs_.h		\
	linux/bfin/arch_regs.c		\
	linux/bfin/get_error.c		\
	linux/bfin/get_scno.c		\
	linux/bfin/get_syscall_args.c	\
	linux/bfin/get_syscall_result.c	\
	linux/bfin/ioctls_arch0.h	\
	linux/bfin/ioctls_inc0.h	\
	linux/bfin/raw_syscall.h	\
	linux/bfin/rt_sigframe.h	\
	linux/bfin/set_error.c		\
	linux/bfin/set_scno.c		\
	linux/bfin/syscallent.h		\
	linux/bfin/userent.h		\
	linux/csky/arch_defs_.h		\
	linux/csky/arch_regs.c		\
	linux/csky/get_error.c		\
	linux/csky/get_scno.c		\
	linux/csky/get_syscall_args.c	\
	linux/csky/ioctls_arch0.h	\
	linux/csky/ioctls_inc0.h	\
	linux/csky/raw_syscall.h	\
	linux/csky/set_error.c		\
	linux/csky/set_scno.c		\
	linux/csky/syscallent.h		\
	linux/generic/arch_fpregset.c		\
	linux/generic/arch_fpregset.h		\
	linux/generic/arch_kvm.c		\
	linux/generic/arch_prstatus_regset.c	\
	linux/generic/arch_prstatus_regset.h	\
	linux/generic/arch_pt_fpregs.c		\
	linux/generic/arch_pt_regs.c		\
	linux/generic/arch_pt_regs64.c		\
	linux/generic/arch_regs.h		\
	linux/generic/arch_rt_sigframe.c	\
	linux/generic/arch_sigreturn.c		\
	linux/generic/asm_stat.h		\
	linux/generic/check_scno.c		\
	linux/generic/errnoent.h		\
	linux/generic/getregs_old.h		\
	linux/generic/nr_prefix.c		\
	linux/generic/ptrace_pokeuser.c		\
	linux/generic/raw_syscall.h		\
	linux/generic/rt_sigframe.h		\
	linux/generic/shuffle_scno.c		\
	linux/generic/signal.h.in		\
	linux/generic/signalent.h		\
	linux/generic/socket.h.in		\
	linux/generic/subcallent.h		\
	linux/generic/syscallent-common.h	\
	linux/generic/syscallent_base_nr.h	\
	linux/generic/userent.h			\
	linux/generic/userent0.h		\
	linux/hppa/arch_defs_.h		\
	linux/hppa/arch_regs.c		\
	linux/hppa/arch_rt_sigframe.c	\
	linux/hppa/errnoent.h		\
	linux/hppa/get_error.c		\
	linux/hppa/get_scno.c		\
	linux/hppa/get_syscall_args.c	\
	linux/hppa/ioctls_arch0.h	\
	linux/hppa/ioctls_inc0.h	\
	linux/hppa/raw_syscall.h	\
	linux/hppa/rt_sigframe.h	\
	linux/hppa/set_error.c		\
	linux/hppa/set_scno.c		\
	linux/hppa/signalent.h		\
	linux/hppa/syscallent.h		\
	linux/hppa/userent.h		\
	linux/i386/arch_defs_.h		\
	linux/i386/arch_fpregset.c	\
	linux/i386/arch_fpregset.h	\
	linux/i386/arch_kvm.c		\
	linux/i386/arch_prstatus_regset.c \
	linux/i386/arch_prstatus_regset.h \
	linux/i386/arch_regs.c		\
	linux/i386/arch_rt_sigframe.c	\
	linux/i386/arch_sigreturn.c	\
	linux/i386/get_error.c		\
	linux/i386/get_scno.c		\
	linux/i386/get_syscall_args.c	\
	linux/i386/ioctls_arch0.h	\
	linux/i386/ioctls_inc0.h	\
	linux/i386/raw_syscall.h	\
	linux/i386/rt_sigframe.h	\
	linux/i386/set_error.c		\
	linux/i386/set_scno.c		\
	linux/i386/syscallent.h		\
	linux/i386/userent.h		\
	linux/i386/userent0.h		\
	linux/ia64/arch_defs_.h		\
	linux/ia64/arch_getrval2.c	\
	linux/ia64/arch_regs.c		\
	linux/ia64/arch_regs.h		\
	linux/ia64/arch_rt_sigframe.c	\
	linux/ia64/get_error.c		\
	linux/ia64/get_scno.c		\
	linux/ia64/get_syscall_args.c	\
	linux/ia64/ioctls_arch0.h	\
	linux/ia64/ioctls_inc0.h	\
	linux/ia64/raw_syscall.h	\
	linux/ia64/rt_sigframe.h	\
	linux/ia64/set_error.c		\
	linux/ia64/set_scno.c		\
	linux/ia64/shuffle_scno.c	\
	linux/ia64/syscallent.h		\
	linux/ia64/syscallent_base_nr.h	\
	linux/ia64/userent.h		\
	linux/loongarch64/arch_defs_.h	\
	linux/loongarch64/arch_fpregset.c	\
	linux/loongarch64/arch_fpregset.h	\
	linux/loongarch64/arch_prstatus_regset.c	\
	linux/loongarch64/arch_prstatus_regset.h	\
	linux/loongarch64/arch_regs.c	\
	linux/loongarch64/get_error.c	\
	linux/loongarch64/get_scno.c	\
	linux/loongarch64/get_syscall_args.c	\
	linux/loongarch64/ioctls_arch0.h	\
	linux/loongarch64/ioctls_inc0.h	\
	linux/loongarch64/raw_syscall.h	\
	linux/loongarch64/set_error.c	\
	linux/loongarch64/set_scno.c	\
	linux/loongarch64/syscallent.h	\
	linux/m68k/arch_defs_.h		\
	linux/m68k/arch_regs.c		\
	linux/m68k/arch_rt_sigframe.c	\
	linux/m68k/arch_sigreturn.c	\
	linux/m68k/get_error.c		\
	linux/m68k/get_scno.c		\
	linux/m68k/get_syscall_args.c	\
	linux/m68k/ioctls_arch0.h	\
	linux/m68k/ioctls_inc0.h	\
	linux/m68k/raw_syscall.h	\
	linux/m68k/rt_sigframe.h	\
	linux/m68k/set_error.c		\
	linux/m68k/set_scno.c		\
	linux/m68k/syscallent.h		\
	linux/m68k/userent.h		\
	linux/metag/arch_defs_.h	\
	linux/metag/arch_regs.c		\
	linux/metag/arch_rt_sigframe.c	\
	linux/metag/get_error.c		\
	linux/metag/get_scno.c		\
	linux/metag/get_syscall_args.c	\
	linux/metag/ioctls_arch0.h	\
	linux/metag/ioctls_inc0.h	\
	linux/metag/raw_syscall.h	\
	linux/metag/set_error.c		\
	linux/metag/set_scno.c		\
	linux/metag/syscallent.h	\
	linux/microblaze/arch_defs_.h	\
	linux/microblaze/arch_regs.c	\
	linux/microblaze/arch_sigreturn.c	\
	linux/microblaze/get_error.c	\
	linux/microblaze/get_scno.c	\
	linux/microblaze/get_syscall_args.c	\
	linux/microblaze/get_syscall_result.c	\
	linux/microblaze/ioctls_arch0.h	\
	linux/microblaze/ioctls_inc0.h	\
	linux/microblaze/raw_syscall.h	\
	linux/microblaze/set_error.c	\
	linux/microblaze/set_scno.c	\
	linux/microblaze/syscallent.h	\
	linux/microblaze/userent.h	\
	linux/mips/arch_defs_.h		\
	linux/mips/arch_getrval2.c	\
	linux/mips/arch_prstatus_regset.c \
	linux/mips/arch_prstatus_regset.h \
	linux/mips/arch_pt_regs.c	\
	linux/mips/arch_regs.c		\
	linux/mips/arch_sigreturn.c	\
	linux/mips/errnoent.h		\
	linux/mips/genstub.sh		\
	linux/mips/get_error.c		\
	linux/mips/get_scno.c		\
	linux/mips/get_syscall_args.c	\
	linux/mips/ioctls_arch0.h	\
	linux/mips/ioctls_inc0.h	\
	linux/mips/raw_syscall.h	\
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
	linux/nios2/arch_defs_.h	\
	linux/nios2/arch_regs.c		\
	linux/nios2/get_error.c		\
	linux/nios2/get_scno.c		\
	linux/nios2/get_syscall_args.c	\
	linux/nios2/ioctls_arch0.h	\
	linux/nios2/ioctls_inc0.h	\
	linux/nios2/raw_syscall.h	\
	linux/nios2/set_error.c		\
	linux/nios2/set_scno.c		\
	linux/nios2/syscallent.h	\
	linux/or1k/arch_defs_.h		\
	linux/or1k/arch_regs.c		\
	linux/or1k/get_error.c		\
	linux/or1k/get_scno.c		\
	linux/or1k/get_syscall_args.c	\
	linux/or1k/ioctls_arch0.h	\
	linux/or1k/ioctls_inc0.h	\
	linux/or1k/raw_syscall.h	\
	linux/or1k/set_error.c		\
	linux/or1k/set_scno.c		\
	linux/or1k/syscallent.h		\
	linux/or1k/userent.h		\
	linux/powerpc/arch_defs_.h	\
	linux/powerpc/arch_fpregset.c	\
	linux/powerpc/arch_fpregset.h	\
	linux/powerpc/arch_prstatus_regset.c \
	linux/powerpc/arch_prstatus_regset.h \
	linux/powerpc/arch_pt_regs64.c	\
	linux/powerpc/arch_pt_regs64.h	\
	linux/powerpc/arch_regs.c	\
	linux/powerpc/arch_rt_sigframe.c	\
	linux/powerpc/arch_sigreturn.c	\
	linux/powerpc/errnoent.h	\
	linux/powerpc/get_error.c	\
	linux/powerpc/get_scno.c	\
	linux/powerpc/get_syscall_args.c	\
	linux/powerpc/ioctls_arch0.h	\
	linux/powerpc/ioctls_inc0.h	\
	linux/powerpc/raw_syscall.h	\
	linux/powerpc/set_error.c	\
	linux/powerpc/set_scno.c	\
	linux/powerpc/syscallent.h	\
	linux/powerpc/userent.h		\
	linux/powerpc64/arch_defs_.h	\
	linux/powerpc64/arch_fpregset.c	\
	linux/powerpc64/arch_fpregset.h	\
	linux/powerpc64/arch_get_personality.c \
	linux/powerpc64/arch_prstatus_regset.c \
	linux/powerpc64/arch_prstatus_regset.h \
	linux/powerpc64/arch_pt_regs64.c	\
	linux/powerpc64/arch_pt_regs64.h	\
	linux/powerpc64/arch_regs.c	\
	linux/powerpc64/arch_rt_sigframe.c	\
	linux/powerpc64/arch_sigreturn.c	\
	linux/powerpc64/errnoent.h	\
	linux/powerpc64/get_error.c	\
	linux/powerpc64/get_scno.c	\
	linux/powerpc64/get_syscall_args.c	\
	linux/powerpc64/ioctls_arch0.h	\
	linux/powerpc64/ioctls_arch1.h	\
	linux/powerpc64/ioctls_inc0.h	\
	linux/powerpc64/ioctls_inc1.h	\
	linux/powerpc64/raw_syscall.h	\
	linux/powerpc64/rt_sigframe.h	\
	linux/powerpc64/set_error.c	\
	linux/powerpc64/set_scno.c	\
	linux/powerpc64/syscallent.h	\
	linux/powerpc64/syscallent1.h	\
	linux/powerpc64/userent.h	\
	linux/powerpc64le/arch_defs_.h	\
	linux/powerpc64le/arch_fpregset.c	\
	linux/powerpc64le/arch_fpregset.h	\
	linux/powerpc64le/arch_prstatus_regset.c \
	linux/powerpc64le/arch_prstatus_regset.h \
	linux/powerpc64le/arch_pt_regs64.c	\
	linux/powerpc64le/arch_regs.c	\
	linux/powerpc64le/arch_rt_sigframe.c	\
	linux/powerpc64le/errnoent.h	\
	linux/powerpc64le/get_error.c	\
	linux/powerpc64le/get_scno.c	\
	linux/powerpc64le/get_syscall_args.c	\
	linux/powerpc64le/ioctls_arch0.h	\
	linux/powerpc64le/ioctls_inc0.h	\
	linux/powerpc64le/raw_syscall.h	\
	linux/powerpc64le/rt_sigframe.h	\
	linux/powerpc64le/set_error.c	\
	linux/powerpc64le/set_scno.c	\
	linux/powerpc64le/syscallent.h	\
	linux/powerpc64le/userent.h	\
	linux/riscv64/arch_defs_.h		\
	linux/riscv64/arch_prstatus_regset.c \
	linux/riscv64/arch_prstatus_regset.h \
	linux/riscv64/arch_regs.c		\
	linux/riscv64/get_error.c		\
	linux/riscv64/get_scno.c		\
	linux/riscv64/get_syscall_args.c	\
	linux/riscv64/ioctls_arch0.h	\
	linux/riscv64/ioctls_inc0.h	\
	linux/riscv64/raw_syscall.h	\
	linux/riscv64/set_error.c		\
	linux/riscv64/set_scno.c		\
	linux/riscv64/syscallent.h	\
	linux/s390/arch_defs_.h		\
	linux/s390/arch_prstatus_regset.c \
	linux/s390/arch_prstatus_regset.h \
	linux/s390/arch_regs.c		\
	linux/s390/arch_sigreturn.c	\
	linux/s390/check_scno.c		\
	linux/s390/get_error.c		\
	linux/s390/get_scno.c		\
	linux/s390/get_syscall_args.c	\
	linux/s390/ioctls_arch0.h	\
	linux/s390/ioctls_inc0.h	\
	linux/s390/raw_syscall.h	\
	linux/s390/rt_sigframe.h	\
	linux/s390/set_error.c		\
	linux/s390/set_scno.c		\
	linux/s390/syscallent.h		\
	linux/s390/userent.h		\
	linux/s390/userent0.h		\
	linux/s390/userent1.h		\
	linux/s390x/arch_defs_.h	\
	linux/s390x/arch_get_personality.c \
	linux/s390x/arch_prstatus_regset.c \
	linux/s390x/arch_prstatus_regset.h \
	linux/s390x/arch_regs.c		\
	linux/s390x/arch_sigreturn.c	\
	linux/s390x/check_scno.c	\
	linux/s390x/get_error.c		\
	linux/s390x/get_scno.c		\
	linux/s390x/get_syscall_args.c	\
	linux/s390x/ioctls_arch0.h	\
	linux/s390x/ioctls_arch1.h	\
	linux/s390x/ioctls_inc0.h	\
	linux/s390x/ioctls_inc1.h	\
	linux/s390x/raw_syscall.h	\
	linux/s390x/rt_sigframe.h	\
	linux/s390x/set_error.c		\
	linux/s390x/set_scno.c		\
	linux/s390x/syscallent.h	\
	linux/s390x/syscallent1.h	\
	linux/s390x/userent.h		\
	linux/sh/arch_defs_.h		\
	linux/sh/arch_getrval2.c	\
	linux/sh/arch_regs.c		\
	linux/sh/get_error.c		\
	linux/sh/get_scno.c		\
	linux/sh/get_syscall_args.c	\
	linux/sh/ioctls_arch0.h		\
	linux/sh/ioctls_inc0.h		\
	linux/sh/raw_syscall.h		\
	linux/sh/set_error.c		\
	linux/sh/set_scno.c		\
	linux/sh/syscallent.h		\
	linux/sh/userent.h		\
	linux/sh/userent0.h		\
	linux/sh64/arch_defs_.h		\
	linux/sh64/arch_regs.c		\
	linux/sh64/get_error.c		\
	linux/sh64/get_scno.c		\
	linux/sh64/get_syscall_args.c	\
	linux/sh64/ioctls_arch0.h	\
	linux/sh64/ioctls_inc0.h	\
	linux/sh64/raw_syscall.h	\
	linux/sh64/rt_sigframe.h	\
	linux/sh64/set_error.c		\
	linux/sh64/set_scno.c		\
	linux/sh64/syscallent.h		\
	linux/sh64/userent.h		\
	linux/sparc/arch_defs_.h	\
	linux/sparc/arch_getrval2.c	\
	linux/sparc/arch_prstatus_regset.c \
	linux/sparc/arch_prstatus_regset.h \
	linux/sparc/arch_pt_regs.c	\
	linux/sparc/arch_regs.c		\
	linux/sparc/arch_sigreturn.c	\
	linux/sparc/errnoent.h		\
	linux/sparc/get_error.c		\
	linux/sparc/get_scno.c		\
	linux/sparc/get_syscall_args.c	\
	linux/sparc/ioctls_arch0.h	\
	linux/sparc/ioctls_inc0.h	\
	linux/sparc/raw_syscall.h	\
	linux/sparc/rt_sigframe.h	\
	linux/sparc/set_error.c		\
	linux/sparc/set_scno.c		\
	linux/sparc/signalent.h		\
	linux/sparc/syscallent.h	\
	linux/sparc/userent.h		\
	linux/sparc64/arch_defs_.h	\
	linux/sparc64/arch_get_personality.c \
	linux/sparc64/arch_getrval2.c	\
	linux/sparc64/arch_prstatus_regset.c \
	linux/sparc64/arch_prstatus_regset.h \
	linux/sparc64/arch_pt_regs.c	\
	linux/sparc64/arch_pt_regs64.c	\
	linux/sparc64/arch_regs.c	\
	linux/sparc64/arch_rt_sigframe.c	\
	linux/sparc64/arch_sigreturn.c	\
	linux/sparc64/errnoent.h	\
	linux/sparc64/get_error.c	\
	linux/sparc64/get_scno.c	\
	linux/sparc64/get_syscall_args.c	\
	linux/sparc64/ioctls_arch0.h	\
	linux/sparc64/ioctls_arch1.h	\
	linux/sparc64/ioctls_inc0.h	\
	linux/sparc64/ioctls_inc1.h	\
	linux/sparc64/raw_syscall.h	\
	linux/sparc64/rt_sigframe.h	\
	linux/sparc64/set_error.c	\
	linux/sparc64/set_scno.c	\
	linux/sparc64/signalent.h	\
	linux/sparc64/syscallent.h	\
	linux/sparc64/syscallent1.h	\
	linux/sparc64/userent.h		\
	linux/tile/arch_defs_.h		\
	linux/tile/arch_get_personality.c \
	linux/tile/arch_regs.c		\
	linux/tile/arch_sigreturn.c	\
	linux/tile/get_error.c		\
	linux/tile/get_scno.c		\
	linux/tile/get_syscall_args.c	\
	linux/tile/ioctls_arch0.h	\
	linux/tile/ioctls_arch1.h	\
	linux/tile/ioctls_inc0.h	\
	linux/tile/ioctls_inc1.h	\
	linux/tile/raw_syscall.h	\
	linux/tile/rt_sigframe.h	\
	linux/tile/set_error.c		\
	linux/tile/set_scno.c		\
	linux/tile/syscallent.h		\
	linux/tile/syscallent1.h	\
	linux/tile/userent.h		\
	linux/x32/arch_defs_.h		\
	linux/x32/arch_fpregset.c	\
	linux/x32/arch_fpregset.h	\
	linux/x32/arch_get_personality.c \
	linux/x32/arch_kvm.c		\
	linux/x32/arch_prstatus_regset.c \
	linux/x32/arch_prstatus_regset.h \
	linux/x32/arch_regs.c		\
	linux/x32/arch_regs.h		\
	linux/x32/arch_rt_sigframe.c	\
	linux/x32/arch_sigreturn.c	\
	linux/x32/check_scno.c		\
	linux/x32/get_error.c		\
	linux/x32/get_scno.c		\
	linux/x32/get_syscall_args.c	\
	linux/x32/ioctls_arch0.h	\
	linux/x32/ioctls_arch1.h	\
	linux/x32/ioctls_inc0.h		\
	linux/x32/ioctls_inc1.h		\
	linux/x32/ptrace_pokeuser.c	\
	linux/x32/raw_syscall.h		\
	linux/x32/rt_sigframe.h		\
	linux/x32/set_error.c		\
	linux/x32/set_scno.c		\
	linux/x32/shuffle_scno.c	\
	linux/x32/syscallent.h		\
	linux/x32/syscallent1.h		\
	linux/x32/userent.h		\
	linux/x86_64/arch_defs_.h	\
	linux/x86_64/arch_fpregset.c	\
	linux/x86_64/arch_fpregset.h	\
	linux/x86_64/arch_get_personality.c \
	linux/x86_64/arch_kvm.c		\
	linux/x86_64/arch_prstatus_regset.c \
	linux/x86_64/arch_prstatus_regset.h \
	linux/x86_64/arch_regs.c	\
	linux/x86_64/arch_regs.h	\
	linux/x86_64/arch_rt_sigframe.c	\
	linux/x86_64/arch_sigreturn.c	\
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
	linux/x86_64/raw_syscall.h	\
	linux/x86_64/rt_sigframe.h	\
	linux/x86_64/set_error.c	\
	linux/x86_64/set_scno.c		\
	linux/x86_64/shuffle_scno.c	\
	linux/x86_64/syscallent.h	\
	linux/x86_64/syscallent1.h	\
	linux/x86_64/syscallent2.h	\
	linux/x86_64/userent.h		\
	linux/xtensa/arch_defs_.h		\
	linux/xtensa/arch_regs.c	\
	linux/xtensa/get_error.c	\
	linux/xtensa/get_scno.c		\
	linux/xtensa/get_syscall_args.c	\
	linux/xtensa/ioctls_arch0.h	\
	linux/xtensa/ioctls_inc0.h	\
	linux/xtensa/raw_syscall.h	\
	linux/xtensa/set_error.c	\
	linux/xtensa/set_scno.c		\
	linux/xtensa/syscallent.h	\
	linux/xtensa/userent.h		\
	mpers.awk			\
	mpers.sh			\
	mpers_test.sh			\
	mpers_xlat.h			\
	scno.head			\
	strace-graph			\
	strace-log-merge		\
	$(XLAT_INPUT_FILES)		\
	$(XLAT_HEADER_FILES)		\
	xlat/gen.sh			\
	# end of EXTRA_DIST

.PHONY: check-valgrind-local
check-valgrind-local:

bpf_attr_check.c: $(srcdir)/bpf_attr.h $(srcdir)/gen_bpf_attr_check.sh
	D="$(D)" \
	$(srcdir)/gen_bpf_attr_check.sh $< > $@-t && mv $@-t $@

strace_SOURCES_c = \
	$(filter %.c,$(strace_SOURCES)) $(filter %.c,$(libstrace_a_SOURCES))

sys_func_h_sources = $(filter-out $(strace_SOURCES_check),$(strace_SOURCES_c))

sys_func.h: $(patsubst %,$(srcdir)/%,$(sys_func_h_sources))
	for f in $^; do \
		sed -n 's/^SYS_FUNC(.*/extern &;/p' $$f; \
	done | sort -u > $@

syscallent_names = subcallent.h syscallent.h syscallent1.h \
		   syscallent-common.h syscallent-common-32.h \
		   syscallent-n32.h syscallent-n64.h syscallent-o32.h
syscallent_patterns = $(patsubst %,\%/%,$(syscallent_names))
syscallent_files = $(filter $(syscallent_patterns),$(EXTRA_DIST))

sen.h: $(patsubst %,$(srcdir)/%,$(syscallent_files))
	for f in $^; do cat -- $$f; done | \
		D="$(D)" \
		$(srcdir)/generate_sen.sh > $@

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
		sed -E -n 's/^\{ "([^"]+)", (0x[[:xdigit:]]+) \},$$/#ifdef \1\n# undef \1\n# define \1 \2\n#endif/p' \
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

ioctlsort%.o: ioctls_all%.h \
	      ioctl_iocdef.h $(srcdir)/ioctls_zfs.h $(srcdir)/ioctlsort.c
	$(ioctlsort_CC) $(ioctlsort_DEFS) $(ioctlsort_INCLUDES) $(ioctlsort_CPPFLAGS) $(ioctlsort_CFLAGS) -DIOCTLSORT_INC=\"$<\" -c -o $@ $(srcdir)/ioctlsort.c

ioctls_all%.h: $(srcdir)/$(OS)/$(ARCH)/ioctls_inc%.h $(srcdir)/$(OS)/$(ARCH)/ioctls_arch%.h
	cat $^ > $@

BUILT_SOURCES = $(ioctl_redefs_h) $(ioctlent_h) \
		bpf_attr_check.c native_printer_decls.h native_printer_defs.h \
		printers.h sen.h sys_func.h
CLEANFILES    = $(ioctl_redefs_h) $(ioctlent_h) $(mpers_preproc_files) \
		ioctl_iocdef.h ioctl_iocdef.i \
		bpf_attr_check.c native_printer_decls.h native_printer_defs.h \
		printers.h sen.h sys_func.h
DISTCLEANFILES = gnu/stubs-32.h \
		 gnu/stubs-x32.h \
		 linux/generic/linux/signal.h \
		 linux/generic/linux/socket.h \
		 #

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
mpers_CPPFLAGS = $(CPPFLAGS)
mpers_sh_opts = $(mpers_DEFS) $(mpers_INCLUDES) $(strace_CPPFLAGS) $(mpers_CPPFLAGS)
libmpers_CPPFLAGS = $(strace_CPPFLAGS) -DIN_MPERS
libmpers_CFLAGS = $(strace_CFLAGS)

# mpers targets

# sys_func.h is an auto-generated file. Make sure it's present before
# preprocessor is ran against it. Example use:
#   struct_blk_user_trace_setup.c => syscall.h => sys_func.h (generated)
mpers-m%.stamp: $(srcdir_mpers_source_files) printers.h sys_func.h
	for f in $(srcdir_mpers_source_files); do \
		D="$(D)" \
		READELF="$(READELF)" \
		CC="$(mpers_CC)" \
		CFLAGS="$(mpers_sh_opts) -DMPERS_IS_$(mpers_NAME)" \
		CPP="$(CPP)" \
		CPPFLAGS="$(mpers_sh_opts) -DIN_MPERS -DMPERS_IS_$(mpers_NAME)" \
		$(srcdir)/mpers.sh $(mpers_NAME) "$(mpers_CC_FLAGS)" $$f || exit; \
	done
	> $@

m%_type_defs.h: $(srcdir_mpers_source_files)
	for f in $^; do \
		sed -E -n 's/^#[[:space:]]*include DEF_MPERS_TYPE\(([^)]+)\)/#ifdef MPERS_$(mpers_PREFIX)\1\n# define \1 MPERS_$(mpers_PREFIX)\1\n#endif/p' $$f || exit; \
	done > $@-t
	echo '#undef MPERS_PRINTER_NAME' >> $@-t
	echo '#define MPERS_PRINTER_NAME(printer_name) printer_name' >> $@-t
	echo '#include "$(mpers_PREFIX)printer_decls.h"' >> $@-t
	echo '#include MPERS_$(mpers_PREFIX)IOCTL_MACROS' >> $@-t
	mv $@-t $@

m%_funcs.h: $(srcdir_mpers_source_files)
	for f in $^; do \
		sed -E -n 's/^SYS_FUNC\(([^)]+)\)/#undef sys_\1\n#define sys_\1 $(mpers_PREFIX)sys_\1/p' $$f || exit; \
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
		sed -E -n 's/$(mpers_printer_decl_pattern)/ \1 (*\2)(\3);\n#define \2 MPERS_PRINTER_NAME(\2)\n/p' $$f \
		|| exit; \
	done >> $@-t
	echo '} struct_printers;' >> $@-t
	echo 'extern const struct_printers *printers;' >> $@-t
	echo '#define MPERS_PRINTER_NAME(printer_name) printers->printer_name' >> $@-t
	mv $@-t $@

%_printer_decls.h: $(mpers_preproc_files)
	echo '/* Generated by Makefile from $^; do not edit. */' > $@-t
	for f in $^; do \
		sed -E -n 's/$(mpers_printer_decl_pattern)/extern \1 $(mpers_PREFIX)\2(\3);/p' $$f \
		|| exit; \
	done >> $@-t
	mv $@-t $@

%_printer_defs.h: $(mpers_preproc_files)
	echo '/* Generated by Makefile from $^; do not edit. */' > $@-t
	for f in $^; do \
		sed -E -n 's/$(mpers_printer_decl_pattern)/.\2 = $(mpers_PREFIX)\2,/p' $$f \
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
$(mpers_m32_targets): mpers_CC_FLAGS = @CFLAGS_FOR_M32@ @cc_flags_m32@
$(mpers_m32_targets): mpers_CPPFLAGS = @CPPFLAGS_FOR_M32@
$(mpers_m32_targets): mpers_CC = @CC_FOR_M32@

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
$(mpers_mx32_targets): mpers_CC_FLAGS = @CFLAGS_FOR_MX32@ @cc_flags_mx32@
$(mpers_mx32_targets): mpers_CPPFLAGS = @CPPFLAGS_FOR_MX32@
$(mpers_mx32_targets): mpers_CC = @CC_FOR_MX32@

endif # HAVE_MX32_MPERS

clean-local:
	-rm -rf mpers-m32 mpers-mx32
