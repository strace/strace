# Configure template for strace.
#
# Copyright (c) 1999-2001 Wichert Akkerman <wichert@deephackmode.org>
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

AC_PREREQ(2.57)
AC_INIT([strace],
	m4_esyscmd([./git-version-gen .tarball-version]),
	[strace-devel@lists.sourceforge.net],
	[strace],
	[https://strace.io])
m4_define([copyright_year], m4_esyscmd([./copyright-year-gen .year]))
AC_COPYRIGHT([Copyright (C) 1999-]copyright_year[ The strace developers.])
AC_CONFIG_SRCDIR([strace.c])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign nostdinc dist-xz silent-rules parallel-tests])
AM_EXTRA_RECURSIVE_TARGETS([check-valgrind])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

RPM_CHANGELOGTIME="$(LC_TIME=C date '+%a %b %d %Y')"
AC_SUBST(RPM_CHANGELOGTIME)
DEB_CHANGELOGTIME="$(LC_TIME=C date -R)"
AC_SUBST(DEB_CHANGELOGTIME)

AC_PROG_CC
AC_PROG_CPP
st_WARN_CFLAGS
AX_PROG_CC_FOR_BUILD
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_USE_SYSTEM_EXTENSIONS
AX_CODE_COVERAGE

AC_DEFINE([COPYRIGHT_YEAR], "[copyright_year]", [Current copyright year.])
AC_SUBST([COPYRIGHT_YEAR], [copyright_year])

AC_MSG_CHECKING([for supported architecture])
arch_m32=
arch_mx32=
case "$host_cpu" in
bfin)
	arch=bfin
	AC_DEFINE([BFIN], 1, [Define for the Blackfin architecture.])
	;;
i[[3456]]86|pentium)
	arch=i386
	AC_DEFINE([I386], 1, [Define for the i386 architecture.])
	;;
ia64)
	arch=ia64
	AC_DEFINE([IA64], 1, [Define for the IA64 architecture.])
	;;
m68k)
	arch=m68k
	AC_DEFINE([M68K], 1, [Define for the m68k architecture.])
	;;
sparc64*)
	arch=sparc64
	arch_m32=sparc
	AC_DEFINE([SPARC64], 1, [Define for the SPARC64 architecture.])
	;;
sparc*)
	arch=sparc
	AC_DEFINE([SPARC], 1, [Define for the SPARC architecture.])
	;;
metag*)
	arch=metag
	AC_DEFINE([METAG], 1, [Define for the Meta architecture.])
	;;
mips*)
	arch=mips
	AC_DEFINE([MIPS], 1, [Define for the MIPS architecture.])
	;;
alpha*)
	arch=alpha
	AC_DEFINE([ALPHA], 1, [Define for the Alpha architecture.])
	;;
powerpc*)
	AC_DEFINE([POWERPC], 1, [Define for the PowerPC architecture.])
	AC_TRY_COMPILE(
[#ifndef __LP64__
# error 32 bit
#endif], [], arch=powerpc64, arch=powerpc)
	if test "$arch" = "powerpc64"; then
		arch_m32=powerpc
		AC_DEFINE([POWERPC64], 1, [Define for the PowerPC64 architecture.])
	fi
	;;
arm*)
	arch=arm
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
aarch64*)
	arch=aarch64
	arch_m32=arm
	AC_DEFINE([AARCH64], 1, [Define for the AArch64 architecture.])
	;;
avr32*)
	arch=avr32
	AC_DEFINE([AVR32], 1, [Define for the AVR32 architecture.])
	;;
arc*)
	arch=arc
	AC_DEFINE([ARC], 1, [Define for the ARC architecture.])
	;;
s390)
	arch=s390
	AC_DEFINE([S390], 1, [Define for the S390 architecture.])
	;;
s390x)
	arch=s390x
	AC_DEFINE([S390X], 1, [Define for the S390x architecture.])
	;;
hppa*|parisc*)
	arch=hppa
	AC_DEFINE([HPPA], 1, [Define for the HPPA architecture.])
	;;
sh64*)
	arch=sh64
	AC_DEFINE([SH64], 1, [Define for the SH64 architecture.])
	;;
sh*)
	arch=sh
	AC_DEFINE([SH], 1, [Define for the SH architecture.])
	;;
x86?64*)
	arch_m32=i386
	AC_TRY_COMPILE(
[#ifndef __ILP32__
# error not x32
#endif], [], arch=x32, arch=x86_64)
	if test "$arch" = "x86_64"; then
		arch_mx32=x32
		AC_DEFINE([X86_64], 1, [Define for the 64bit AMD x86-64 architecture.])
	else
		AC_DEFINE([X32], 1, [Define for the 32bit AMD x86-64 architecture.])
	fi
	;;
cris|crisv10)
	arch=crisv10
	AC_DEFINE([CRISV10], 1, [Define for the CRISv10 architecture.])
	;;
crisv32)
	arch=crisv32
	AC_DEFINE([CRISV32], 1, [Define for the CRISv32 architecture.])
	;;
tile*)
	arch=tile
	AC_DEFINE([TILE], 1, [Define for the Tile architecture])
	;;
microblaze*)
	arch=microblaze
	AC_DEFINE([MICROBLAZE], 1, [Define for the MicroBlaze architecture.])
	;;
nios2*)
	arch=nios2
	AC_DEFINE([NIOS2], 1, [Define for the Nios-II architecture.])
	;;

or1k*)
	arch=or1k
	AC_DEFINE([OR1K], 1, [Define for the OpenRISC 1000 architecture.])
	;;

xtensa*)
	arch=xtensa
	AC_DEFINE([XTENSA], 1, [Define for the Xtensa architecture])
	;;

riscv*)
	arch=riscv
	AC_DEFINE([RISCV], 1, [Define for the RISC-V architecture])
	;;

*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([architecture $host_cpu is not supported by strace])
	;;
esac
AC_MSG_RESULT($arch)

test -n "$arch_m32" ||
	arch_m32=$arch
test -n "$arch_mx32" ||
	arch_mx32=$arch
AC_SUBST(arch)
AC_SUBST(arch_m32)
AC_SUBST(arch_mx32)

MIPS_ABI=
if test "$arch" = mips; then
	AC_CACHE_CHECK([for _MIPS_SIM], [st_cv__MIPS_SIM],
		       [AC_COMPUTE_INT([st_cv__MIPS_SIM], [_MIPS_SIM], ,
				       [AC_MSG_ERROR([_MIPS_SIM cannot be determined])])])

	# requires GCC >= 3.4
	AC_CACHE_CHECK([for MIPS ABI], [st_cv_mips_abi],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(, [[int i[_MIPS_SIM == _ABIO32 ? 1 : - 1];]])],
			[st_cv_mips_abi=o32],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(, [[int i[_MIPS_SIM == _ABIN32 ? 1 : - 1];]])],
			[st_cv_mips_abi=n32],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(, [[int i[_MIPS_SIM == _ABI64 ? 1 : - 1];]])],
			[st_cv_mips_abi=n64],
			[st_cv_mips_abi=unknown])])])])

	case "$st_cv_mips_abi" in
		o32) AC_DEFINE([LINUX_MIPSO32], [1], [Define for _ABIO32.]);;
		n32) AC_DEFINE([LINUX_MIPSN32], [1], [Define for _ABIN32.]);;
		n64) AC_DEFINE([LINUX_MIPSN64], [1], [Define for _ABI64.]);;
		*) AC_MSG_ERROR([Unsupported _MIPS_SIM]);;
	esac
	MIPS_ABI="$st_cv_mips_abi"
fi
AC_SUBST(MIPS_ABI)

AC_ARG_ENABLE([arm-oabi],
	      [AS_HELP_STRING([--enable-arm-oabi],
			      [enable OABI support on ARM EABI])],
	      [], [enable_arm_oabi=no])
case "$enable_arm_oabi" in
	yes) enable_arm_oabi=1 ;;
	no) enable_arm_oabi=0 ;;
	*) AC_MSG_ERROR([bad value $enable_arm_oabi for arm-oabi option]) ;;
esac
AC_DEFINE_UNQUOTED([ENABLE_ARM_OABI], [$enable_arm_oabi],
		   [Define to 1 if you want OABI support on ARM EABI.])

AC_C_BIGENDIAN
AC_C_TYPEOF

AC_TYPE_UID_T

AC_CHECK_FUNCS(m4_normalize([
	accept4
	fallocate
	fanotify_mark
	fopen64
	fork
	fputs_unlocked
	fstatat
	ftruncate
	futimens
	if_indextoname
	open64
	prctl
	preadv
	process_vm_readv
	pwritev
	readahead
	signalfd
	stpcpy
	strerror
	strsignal
	sync_file_range
	utimensat
]))

AC_CHECK_TYPES([sig_atomic_t, struct sigcontext],,, [#include <signal.h>])

AC_CHECK_TYPES([struct mmsghdr],,, [#include <sys/socket.h>])

AC_CHECK_TYPES([__kernel_long_t, __kernel_ulong_t],,,
[#include <asm/posix_types.h>])

AC_CHECK_TYPES([struct stat64, struct __old_kernel_stat],,,
[#include <sys/types.h>
#include <asm/stat.h>])

AC_CHECK_TYPES([struct user_desc],,, [#include <asm/ldt.h>])

AC_CHECK_MEMBERS([struct stat.st_mtime_nsec, struct stat64.st_mtime_nsec],,,
[#include <sys/types.h>
#include <asm/stat.h>])

AC_CHECK_TYPES(m4_normalize([
	struct pt_all_user_regs,
	struct ia64_fpreg,
	struct ptrace_peeksiginfo_args
]),,, [#include <sys/ptrace.h>])

AC_CHECK_TYPES(m4_normalize([
	struct flock,
	struct flock64,
	struct __kernel_flock,
	struct __kernel_flock64
]),,,
[#include <sys/types.h>
#include <linux/fcntl.h>])

AC_CHECK_MEMBERS([struct timex.tai],,, [#include <sys/timex.h>])

AC_CHECK_MEMBERS([struct utsname.domainname],,, [#include <sys/utsname.h>])

AC_CHECK_MEMBERS(m4_normalize([
	siginfo_t.si_syscall,
	siginfo_t.si_timerid,
	siginfo_t.si_overrun
]),,, [#include <signal.h>])

AC_CHECK_MEMBERS(m4_normalize([
	struct perf_event_attr.precise_ip,
	struct perf_event_attr.mmap_data,
	struct perf_event_attr.sample_id_all,
	struct perf_event_attr.exclude_host,
	struct perf_event_attr.exclude_guest,
	struct perf_event_attr.exclude_callchain_kernel,
	struct perf_event_attr.exclude_callchain_user,
	struct perf_event_attr.mmap2,
	struct perf_event_attr.comm_exec,
	struct perf_event_attr.use_clockid,
	struct perf_event_attr.context_switch,
	struct perf_event_attr.write_backward,
	struct perf_event_attr.bp_type,
	struct perf_event_attr.config1,
	struct perf_event_attr.config2,
	struct perf_event_attr.branch_sample_type,
	struct perf_event_attr.sample_regs_user,
	struct perf_event_attr.sample_stack_user,
	struct perf_event_attr.sample_regs_intr,
	struct perf_event_attr.aux_watermark,
	struct perf_event_attr.sample_max_stack
]),,, [#include <linux/perf_event.h>])

AC_CHECK_HEADERS(m4_normalize([
	asm/cachectl.h
	asm/sysmips.h
	bluetooth/bluetooth.h
	elf.h
	inttypes.h
	linux/bsg.h
	linux/dm-ioctl.h
	linux/dqblk_xfs.h
	linux/falloc.h
	linux/fiemap.h
	linux/filter.h
	linux/hiddev.h
	linux/ip_vs.h
	linux/ipc.h
	linux/mmtimer.h
	linux/msg.h
	linux/nsfs.h
	linux/perf_event.h
	linux/quota.h
	linux/seccomp.h
	linux/securebits.h
	linux/sem.h
	linux/shm.h
	linux/userfaultfd.h
	linux/utsname.h
	mqueue.h
	netinet/sctp.h
	netipx/ipx.h
	scsi/sg.h
	stropts.h
	sys/conf.h
	sys/eventfd.h
	sys/fanotify.h
	sys/ipc.h
	sys/msg.h
	sys/quota.h
	sys/reg.h
	sys/sem.h
	sys/shm.h
	sys/signalfd.h
	sys/xattr.h
	ustat.h
]))

AC_CHECK_HEADERS([asm/sigcontext.h],,, [#include <signal.h>])

AC_CHECK_HEADERS([netinet/tcp.h netinet/udp.h],,, [#include <netinet/in.h>])

AC_CHECK_HEADERS([linux/mqueue.h],,, [#include <linux/types.h>])

AC_CHECK_HEADERS(m4_normalize([
	linux/netfilter_arp/arp_tables.h
	linux/netfilter_bridge/ebtables.h
	linux/netfilter_ipv4/ip_tables.h
	linux/netfilter_ipv6/ip6_tables.h
]),,, [#include <netinet/in.h>
#include <net/if.h>])

AC_CHECK_HEADERS([linux/input.h], [
	AC_CHECK_MEMBERS([struct input_absinfo.resolution],,, [#include <linux/input.h>])
])

AC_CHECK_HEADERS([linux/bpf.h], [
	AC_CACHE_CHECK([whether union bpf_attr.log_buf initialization works],
		       [st_cv_have_union_bpf_attr_log_buf],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <linux/bpf.h>]],
					 [[union bpf_attr a = { .log_buf = 0 };]])],
			[st_cv_have_union_bpf_attr_log_buf=yes],
			[st_cv_have_union_bpf_attr_log_buf=no])])
	if test $st_cv_have_union_bpf_attr_log_buf = yes; then
		AC_DEFINE(HAVE_UNION_BPF_ATTR_LOG_BUF, [1],
			  [Define to 1 if union bpf_attr.log_buf initialization works])
	fi
	AC_CACHE_CHECK([whether union bpf_attr.bpf_fd initialization works],
		       [st_cv_have_union_bpf_attr_bpf_fd],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <linux/bpf.h>]],
					 [[union bpf_attr a = { .bpf_fd = 0 };]])],
			[st_cv_have_union_bpf_attr_bpf_fd=yes],
			[st_cv_have_union_bpf_attr_bpf_fd=no])])
	if test $st_cv_have_union_bpf_attr_bpf_fd = yes; then
		AC_DEFINE(HAVE_UNION_BPF_ATTR_BPF_FD, [1],
			  [Define to 1 if union bpf_attr.bpf_fd initialization works])
	fi
	AC_CACHE_CHECK([whether union bpf_attr.attach_flags initialization works],
		       [st_cv_have_union_bpf_attr_attach_flags],
		[AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <linux/bpf.h>]],
					 [[union bpf_attr a = { .attach_flags = 0 };]])],
			[st_cv_have_union_bpf_attr_attach_flags=yes],
			[st_cv_have_union_bpf_attr_attach_flags=no])])
	if test $st_cv_have_union_bpf_attr_attach_flags = yes; then
		AC_DEFINE(HAVE_UNION_BPF_ATTR_ATTACH_FLAGS, [1],
			  [Define to 1 if union bpf_attr.attach_flags initialization works])
	fi
])

AC_CHECK_TYPES([struct statfs], [
	AC_CHECK_MEMBERS([struct statfs.f_frsize],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs.f_flags],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs.f_fsid.val],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs.f_fsid.__val],,, [#include <linux/types.h>
#include <asm/statfs.h>])
],, [#include <linux/types.h>
#include <asm/statfs.h>])

AC_CHECK_TYPES([struct statfs64], [
	AC_CHECK_MEMBERS([struct statfs64.f_frsize],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs64.f_flags],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs64.f_fsid.val],,, [#include <linux/types.h>
#include <asm/statfs.h>])
	AC_CHECK_MEMBERS([struct statfs64.f_fsid.__val],,, [#include <linux/types.h>
#include <asm/statfs.h>])
],, [#include <linux/types.h>
#include <asm/statfs.h>])

AC_CHECK_TYPES([struct blk_user_trace_setup],,, [#include <linux/blktrace_api.h>])

AC_CHECK_HEADERS([linux/btrfs.h], [
	AC_CHECK_MEMBERS(m4_normalize([
		struct btrfs_ioctl_feature_flags.compat_flags,
		struct btrfs_ioctl_fs_info_args.nodesize,
		struct btrfs_ioctl_defrag_range_args.start,
		struct btrfs_ioctl_search_args_v2.buf_size
		]),,, [ #include <stdio.h>
#include <linux/btrfs.h>])
        AC_CHECK_DECLS(m4_normalize([BTRFS_COMPRESS_NONE, BTRFS_COMPRESS_ZLIB,
	                        BTRFS_COMPRESS_LZO]),,,[ #include <stdio.h>
#include <linux/btrfs.h>])])

AC_CHECK_DECLS([sys_errlist])
AC_CHECK_DECLS(m4_normalize([
	PTRACE_PEEKUSER,
	PTRACE_POKEUSER
]),,, [#include <sys/ptrace.h>])

AC_CHECK_DECLS(m4_normalize([
	P_PID,
	P_PPID,
	P_PGID,
	P_SID,
	P_CID,
	P_UID,
	P_GID,
	P_ALL,
	P_LWPID
]),,, [#include <sys/wait.h>])

AC_CHECK_DECLS(m4_normalize([
	LO_FLAGS_READ_ONLY,
	LO_FLAGS_USE_AOPS,
	LO_FLAGS_AUTOCLEAR,
	LO_FLAGS_PARTSCAN,
	LO_FLAGS_DIRECT_IO
]),,, [#include <linux/loop.h>])

AC_CHECK_DECLS(m4_normalize([
	CTL_PROC,
	CTL_ARLAN,
	CTL_S390DBF,
	CTL_SUNRPC,
	CTL_PM,
	CTL_FRV,
	KERN_PRINTK_RATELIMIT,
	KERN_PRINTK_RATELIMIT_BURST,
	KERN_PTY,
	KERN_NGROUPS_MAX,
	KERN_SPARC_SCONS_PWROFF,
	KERN_HZ_TIMER,
	KERN_UNKNOWN_NMI_PANIC,
	KERN_BOOTLOADER_TYPE,
	KERN_RANDOMIZE,
	KERN_SETUID_DUMPABLE,
	KERN_SPIN_RETRY,
	KERN_ACPI_VIDEO_FLAGS,
	KERN_IA64_UNALIGNED,
	KERN_COMPAT_LOG,
	KERN_MAX_LOCK_DEPTH,
	KERN_NMI_WATCHDOG,
	KERN_PANIC_ON_NMI,
	NET_LLC,
	NET_NETFILTER,
	NET_DCCP,
	NET_IRDA,
	NET_CORE_DESTROY_DELAY,
	NET_CORE_BUDGET,
	NET_CORE_AEVENT_ETIME,
	NET_CORE_AEVENT_RSEQTH,
	NET_CORE_WARNINGS,
	NET_IPV4_IGMP_MAX_MSF,
	NET_TCP_NO_METRICS_SAVE,
	NET_TCP_DEFAULT_WIN_SCALE,
	NET_TCP_MODERATE_RCVBUF,
	NET_TCP_TSO_WIN_DIVISOR,
	NET_TCP_BIC_BETA,
	NET_IPV4_ICMP_ERRORS_USE_INBOUND_IFADDR,
	NET_TCP_CONG_CONTROL,
	NET_TCP_ABC,
	NET_IPV4_IPFRAG_MAX_DIST,
	NET_TCP_MTU_PROBING,
	NET_TCP_BASE_MSS,
	NET_IPV4_TCP_WORKAROUND_SIGNED_WINDOWS,
	NET_TCP_DMA_COPYBREAK,
	NET_TCP_SLOW_START_AFTER_IDLE,
	NET_CIPSOV4_CACHE_ENABLE,
	NET_CIPSOV4_CACHE_BUCKET_SIZE,
	NET_CIPSOV4_RBM_OPTFMT,
	NET_CIPSOV4_RBM_STRICTVALID,
	NET_TCP_AVAIL_CONG_CONTROL,
	NET_TCP_ALLOWED_CONG_CONTROL,
	NET_TCP_MAX_SSTHRESH,
	NET_TCP_FRTO_RESPONSE,
	NET_IPV4_ROUTE_GC_MIN_INTERVAL_MS,
	NET_IPV4_CONF_FORCE_IGMP_VERSION,
	NET_IPV4_CONF_ARP_ANNOUNCE,
	NET_IPV4_CONF_ARP_IGNORE,
	NET_IPV4_CONF_PROMOTE_SECONDARIES,
	NET_IPV4_CONF_ARP_ACCEPT,
	NET_IPV4_CONF_ARP_NOTIFY,
	NET_IPV6_MLD_MAX_MSF,
	NET_IPV6_ROUTE_GC_MIN_INTERVAL_MS,
	VM_MAX_MAP_COUNT,
	VM_LAPTOP_MODE,
	VM_BLOCK_DUMP,
	VM_HUGETLB_GROUP,
	VM_VFS_CACHE_PRESSURE,
	VM_LEGACY_VA_LAYOUT,
	VM_SWAP_TOKEN_TIMEOUT
]),,,
[#include <sys/types.h>
#include <linux/sysctl.h>])

AC_CHECK_DECLS(m4_normalize([
	V4L2_FIELD_ANY,
	V4L2_FIELD_NONE,
	V4L2_FIELD_TOP,
	V4L2_FIELD_BOTTOM,
	V4L2_FIELD_INTERLACED,
	V4L2_FIELD_SEQ_TB,
	V4L2_FIELD_SEQ_BT,
	V4L2_FIELD_ALTERNATE,
	V4L2_FIELD_INTERLACED_TB,
	V4L2_FIELD_INTERLACED_BT,
	V4L2_BUF_TYPE_VIDEO_CAPTURE,
	V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
	V4L2_BUF_TYPE_VIDEO_OUTPUT,
	V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
	V4L2_BUF_TYPE_VIDEO_OVERLAY,
	V4L2_BUF_TYPE_VBI_CAPTURE,
	V4L2_BUF_TYPE_VBI_OUTPUT,
	V4L2_BUF_TYPE_SLICED_VBI_CAPTURE,
	V4L2_BUF_TYPE_SLICED_VBI_OUTPUT,
	V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY,
	V4L2_BUF_TYPE_SDR_CAPTURE,
	V4L2_BUF_TYPE_SDR_OUTPUT,
	V4L2_TUNER_RADIO,
	V4L2_TUNER_ANALOG_TV,
	V4L2_TUNER_DIGITAL_TV,
	V4L2_MEMORY_MMAP,
	V4L2_MEMORY_USERPTR,
	V4L2_MEMORY_OVERLAY,
	V4L2_MEMORY_DMABUF,
	V4L2_COLORSPACE_SMPTE170M,
	V4L2_COLORSPACE_SMPTE240M,
	V4L2_COLORSPACE_REC709,
	V4L2_COLORSPACE_BT878,
	V4L2_COLORSPACE_470_SYSTEM_M,
	V4L2_COLORSPACE_470_SYSTEM_BG,
	V4L2_COLORSPACE_JPEG,
	V4L2_COLORSPACE_SRGB,
	V4L2_PRIORITY_UNSET,
	V4L2_PRIORITY_BACKGROUND,
	V4L2_PRIORITY_INTERACTIVE,
	V4L2_PRIORITY_RECORD,
	V4L2_FRMSIZE_TYPE_DISCRETE,
	V4L2_FRMSIZE_TYPE_CONTINUOUS,
	V4L2_FRMSIZE_TYPE_STEPWISE,
	V4L2_FRMIVAL_TYPE_DISCRETE,
	V4L2_FRMIVAL_TYPE_CONTINUOUS,
	V4L2_FRMIVAL_TYPE_STEPWISE,
	V4L2_CTRL_TYPE_INTEGER,
	V4L2_CTRL_TYPE_BOOLEAN,
	V4L2_CTRL_TYPE_MENU,
	V4L2_CTRL_TYPE_BUTTON,
	V4L2_CTRL_TYPE_INTEGER64,
	V4L2_CTRL_TYPE_CTRL_CLASS,
	V4L2_CTRL_TYPE_STRING,
	V4L2_CTRL_TYPE_BITMASK,
	V4L2_CTRL_TYPE_INTEGER_MENU
]),,, [#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>])

AC_CHECK_MEMBERS(m4_normalize([
	struct v4l2_window.global_alpha,
	struct v4l2_sdr_format.buffersize
]),,, [#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>])

AC_CACHE_CHECK([for BLKGETSIZE64], [ac_cv_have_blkgetsize64],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <stdlib.h>
#include <linux/fs.h>], [return !BLKGETSIZE64;])],
			   [ac_cv_have_blkgetsize64=yes], [ac_cv_have_blkgetsize64=no])]
	if test $ac_cv_have_blkgetsize64 = yes; then
		AC_DEFINE([HAVE_BLKGETSIZE64], [1], [Define to 1 if you have BLKGETSIZE64.])
	fi)

AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([off_t],,[#include <sys/types.h>])
AC_CHECK_SIZEOF([kernel_long_t],,[#include "$srcdir/kernel_types.h"])
AC_CHECK_SIZEOF([struct i64_i32],,[struct i64_i32 {long long i64; int i32;};])

AC_CACHE_CHECK([for SIGRTMIN], [st_cv_SIGRTMIN],
	       [[st_cv_SIGRTMIN="$(echo SIGRTMIN |
			$CPP $CPPFLAGS -P -imacros asm/signal.h - |
			grep '^[0-9]')"
	         test -n "$st_cv_SIGRTMIN" || st_cv_SIGRTMIN=no]])
if test "x$st_cv_SIGRTMIN" != xno; then
	AC_DEFINE_UNQUOTED([ASM_SIGRTMIN], [$st_cv_SIGRTMIN],
			   [SIGRTMIN defined in <asm/signal.h>])
fi
AC_CACHE_CHECK([for SA_RESTORER], [st_cv_SA_RESTORER],
	       [st_cv_SA_RESTORER="$(echo SA_RESTORER |
			$CPP $CPPFLAGS -P -imacros asm/signal.h - |
			grep ^0x)"
	        test -n "$st_cv_SA_RESTORER" || st_cv_SA_RESTORER=no])
if test "x$st_cv_SA_RESTORER" != xno; then
	AC_DEFINE_UNQUOTED([ASM_SA_RESTORER], [$st_cv_SA_RESTORER],
			   [SA_RESTORER defined in <asm/signal.h>])
fi

AC_CACHE_CHECK([for __builtin_popcount], [st_cv_have___builtin_popcount],
	       [AC_LINK_IFELSE([AC_LANG_PROGRAM([], [__builtin_popcount(0)])],
			       [st_cv_have___builtin_popcount=yes],
			       [st_cv_have___builtin_popcount=no])])
if test "x$st_cv_have___builtin_popcount" = xyes; then
	AC_DEFINE([HAVE___BUILTIN_POPCOUNT], [1],
		  [Define to 1 if the system provides __builtin_popcount function])
fi

AC_CHECK_LIB([dl], [dladdr], [dl_LIBS='-ldl'], [dl_LIBS=])
if test "x$ac_cv_lib_dl_dladdr" = xyes; then
	AC_DEFINE([HAVE_DLADDR], [1], [Define to 1 if the system provides dladdr])
fi
AC_SUBST(dl_LIBS)

AC_PATH_PROG([PERL], [perl])

dnl stack trace with libunwind
libunwind_CPPFLAGS=
libunwind_LDFLAGS=
libunwind_LIBS=
AC_ARG_WITH([libunwind],
            [AS_HELP_STRING([--with-libunwind],
                            [use libunwind to implement stack tracing support])],
            [case "${withval}" in
             yes|no|check) ;;
             *) with_libunwind=yes
                libunwind_CPPFLAGS="-I${withval}/include"
                libunwind_LDFLAGS="-L${withval}/lib" ;;
             esac],
            [with_libunwind=check]
)

use_libunwind=no
AS_IF([test "x$with_libunwind" != xno],
      [saved_CPPFLAGS="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $libunwind_CPPFLAGS"

       AC_CHECK_HEADERS([libunwind-ptrace.h],
         [saved_LDFLAGS="$LDFLAGS"
          LDFLAGS="$LDFLAGS $libunwind_LDFLAGS"

          AC_CHECK_LIB([unwind], [backtrace],
            [libunwind_LIBS="-lunwind $libunwind_LIBS"

             AC_MSG_CHECKING([for unw_create_addr_space in libunwind-generic])
             saved_LIBS="$LIBS"
             LIBS="-lunwind-generic $libunwind_LIBS $LIBS"

             AC_LINK_IFELSE(
               [AC_LANG_PROGRAM([[#include <libunwind-ptrace.h>]],
                                [[return !unw_create_addr_space(0, 0)]])
               ],
               [AC_MSG_RESULT([yes])
                libunwind_LIBS="-lunwind-generic $libunwind_LIBS"

                AC_CHECK_LIB([unwind-ptrace], [_UPT_create],
                  [libunwind_LIBS="-lunwind-ptrace $libunwind_LIBS"
                   use_libunwind=yes
                  ],
                  [if test "x$with_libunwind" != xcheck; then
                     AC_MSG_FAILURE([failed to find _UPT_create in libunwind-ptrace])
                   fi
                  ],
                  [$libunwind_LIBS]
                )
               ],
               [AC_MSG_RESULT([no])
                if test "x$with_libunwind" != xcheck; then
                  AC_MSG_FAILURE([failed to find unw_create_addr_space in libunwind-generic])
                fi
               ]
             )

             LIBS="$saved_LIBS"
            ],
            [if test "x$with_libunwind" != xcheck; then
               AC_MSG_FAILURE([failed to find libunwind])
             fi
            ],
            [$libunwind_LIBS]
          )

          LDFLAGS="$saved_LDFLAGS"
         ],
         [if test "x$with_libunwind" != xcheck; then
            AC_MSG_FAILURE([failed to find libunwind-ptrace.h])
          fi
         ]
       )

       CPPFLAGS="$saved_CPPFLAGS"
      ]
)

dnl enable libunwind
AC_MSG_CHECKING([whether to enable stack tracing support using libunwind])
if test "x$use_libunwind" = xyes; then
	AC_DEFINE([USE_LIBUNWIND], 1, [Compile stack tracing functionality])
	AC_SUBST(libunwind_LIBS)
	AC_SUBST(libunwind_LDFLAGS)
	AC_SUBST(libunwind_CPPFLAGS)
fi
AM_CONDITIONAL([USE_LIBUNWIND], [test "x$use_libunwind" = xyes])
AC_MSG_RESULT([$use_libunwind])

if test "$arch" = mips && test "$no_create" != yes; then
	mkdir -p linux/mips
	if $srcdir/linux/mips/genstub.sh linux/mips; then
		AC_MSG_RESULT([Generated MIPS syscallent stubs])
	else
		AC_MSG_FAILURE([Failed to generate syscallent stubs])
	fi
fi

st_MPERS([m32], [aarch64|powerpc64|sparc64|tile|x32|x86_64])
st_MPERS([mx32], [x86_64])

AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

AC_CONFIG_FILES([Makefile
		 tests/Makefile
		 tests-m32/Makefile
		 tests-mx32/Makefile
		 strace.spec
		 debian/changelog])
AC_OUTPUT
