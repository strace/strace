# Configure template for strace.
#
# Copyright (c) 1999-2001 Wichert Akkerman <wichert@deephackmode.org>
# Copyright (c) 2002-2009 Roland McGrath <roland@redhat.com>
# Copyright (c) 2006-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2008-2015 Mike Frysinger <vapier@gentoo.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2002-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_PREREQ(2.57)
AC_INIT([strace],
	st_esyscmd_s([./build-aux/git-version-gen .tarball-version]),
	[strace-devel@lists.strace.io],
	[strace],
	[https://strace.io])
m4_define([copyright_year], st_esyscmd_s([./build-aux/copyright-year-gen .year]))
m4_define([manpage_date], st_esyscmd_s([./build-aux/file-date-gen doc/strace.1.in]))
m4_define([linux_version_code],
	  st_esyscmd_s([sed '/^#define LINUX_VERSION_CODE /!d;s///;q' bundled/linux/include/uapi/linux/version.h]))
AC_COPYRIGHT([Copyright (c) 1999-]copyright_year[ The strace developers.])
AC_CONFIG_SRCDIR([src/strace.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([foreign nostdinc dist-xz silent-rules parallel-tests subdir-objects 1.13])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

RPM_CHANGELOGTIME="$(LC_TIME=C date -u '+%a %b %d %Y')"
AC_SUBST(RPM_CHANGELOGTIME)
DEB_CHANGELOGTIME="$(LC_TIME=C date -u -R)"
AC_SUBST(DEB_CHANGELOGTIME)

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LN_S

AC_USE_SYSTEM_EXTENSIONS
AX_CODE_COVERAGE

AC_DEFINE([COPYRIGHT_YEAR], "[copyright_year]", [Current copyright year.])
AC_SUBST([COPYRIGHT_YEAR], [copyright_year])

AC_DEFINE([MANPAGE_DATE], "[manpage_date]", [Date])
AC_SUBST([MANPAGE_DATE], [manpage_date])

AC_C_BIGENDIAN

dnl arch-specific default for --enable-gcc-Werror
arch_enable_Werror=yes

AC_MSG_CHECKING([for supported architecture])
arch_m32=
arch_mx32=
cc_flags_m32=-m32
cc_flags_mx32=-mx32
karch=
karch_m32=
karch_mx32=
case "$host_cpu" in
aarch64*)
	arch=aarch64
	karch=arm64
	arch_m32=arm
	cc_flags_m32=
	AC_DEFINE([AARCH64], 1, [Define for the AArch64 architecture.])
	;;
alpha*)
	arch=alpha
	arch_enable_Werror=no
	AC_DEFINE([ALPHA], 1, [Define for the Alpha architecture.])
	;;
arc*)
	arch=arc
	AC_DEFINE([ARC], 1, [Define for the ARC architecture.])
	;;
arm*)
	arch=arm
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
avr32*)
	arch=avr32
	AC_DEFINE([AVR32], 1, [Define for the AVR32 architecture.])
	;;
bfin)
	arch=bfin
	arch_enable_Werror=no
	AC_DEFINE([BFIN], 1, [Define for the Blackfin architecture.])
	;;
csky*)
	arch=csky
	AC_DEFINE([CSKY], 1, [Define for the C-SKY architecture])
	;;
hppa*|parisc*)
	arch=hppa
	karch=parisc
	AC_DEFINE([HPPA], 1, [Define for the HPPA architecture.])
	;;
i[[3456]]86|pentium)
	arch=i386
	karch=x86
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
metag*)
	arch=metag
	AC_DEFINE([METAG], 1, [Define for the Meta architecture.])
	;;
microblaze*)
	arch=microblaze
	arch_enable_Werror=no
	AC_DEFINE([MICROBLAZE], 1, [Define for the MicroBlaze architecture.])
	;;
mips*)
	arch=mips
	AC_DEFINE([MIPS], 1, [Define for the MIPS architecture.])
	;;
nios2*)
	arch=nios2
	AC_DEFINE([NIOS2], 1, [Define for the Nios-II architecture.])
	;;
or1k*)
	arch=or1k
	AC_DEFINE([OR1K], 1, [Define for the OpenRISC 1000 architecture.])
	;;
powerpc*)
	karch=powerpc
	karch_m32=powerpc
	AC_DEFINE([POWERPC], 1, [Define for the PowerPC architecture.])
	AC_TRY_COMPILE(
[#ifndef __LP64__
# error 32 bit
#endif], [], arch=powerpc64, arch=powerpc)
	if test "$arch" = "powerpc64"; then
		# $ac_cv_c_bigendian is defined by AC_C_BIGENDIAN
		case "$ac_cv_c_bigendian" in
		no)
			arch=powerpc64le
			AC_DEFINE([POWERPC64LE], 1,
				  [Define for the little endian PowerPC64 architecture.])
			;;
		*)
			arch_m32=powerpc
			AC_DEFINE([POWERPC64], 1,
				  [Define for the big endian PowerPC64 architecture.])
			;;
		esac
	fi
	;;
riscv64*)
	arch=riscv64
	karch=riscv
	AC_DEFINE([RISCV64], 1, [Define for the RISC-V 64-bit architecture])
	;;
s390)
	arch=s390
	AC_DEFINE([S390], 1, [Define for the S390 architecture.])
	;;
s390x)
	arch=s390x
	karch=s390
	arch_m32=s390
	cc_flags_m32=-m31
	AC_DEFINE([S390X], 1, [Define for the S390x architecture.])
	;;
sh64*)
	arch=sh64
	AC_DEFINE([SH64], 1, [Define for the SH64 architecture.])
	;;
sh*)
	arch=sh
	AC_DEFINE([SH], 1, [Define for the SH architecture.])
	;;
sparc64*)
	arch=sparc64
	karch=sparc
	arch_m32=sparc
	AC_DEFINE([SPARC64], 1, [Define for the SPARC64 architecture.])
	;;
sparc*)
	arch=sparc
	AC_DEFINE([SPARC], 1, [Define for the SPARC architecture.])
	;;
tile*)
	arch=tile
	AC_DEFINE([TILE], 1, [Define for the Tile architecture])
	;;
x86?64*)
	karch=x86
	karch_m32=x86
	karch_mx32=x86
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
xtensa*)
	arch=xtensa
	AC_DEFINE([XTENSA], 1, [Define for the Xtensa architecture])
	;;
*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([architecture $host_cpu is not supported by strace])
	;;
esac
AC_MSG_RESULT($arch)

arch_native=$arch

test -n "$arch_m32" ||
	arch_m32=$arch
test -n "$arch_mx32" ||
	arch_mx32=$arch

test -n "$karch" ||
	karch=$arch
test -n "$karch_m32" ||
	karch_m32=$arch_m32
test -n "$karch_mx32" ||
	karch_mx32=$arch_mx32

AC_SUBST(arch)
AC_SUBST(arch_m32)
AC_SUBST(arch_mx32)
AC_SUBST(karch)
AC_SUBST(karch_m32)
AC_SUBST(karch_mx32)
AC_SUBST(cc_flags_m32)
AC_SUBST(cc_flags_mx32)
AC_SUBST(arch_native)

AC_ARG_ENABLE([bundled],
	[AS_HELP_STRING([--enable-bundled=yes|no|check],
		[whether to use bundled linux kernel headers,
		 default is check.])],
	[case "$enableval" in
		yes|no|check) enable_bundled="$enableval" ;;
		*) AC_MSG_ERROR([bad value $enableval for enable-bundled option.
Valid options are: yes, no, check.]
		   )
		;;
	 esac],
	[enable_bundled=check]
)

AC_CACHE_CHECK(
	[whether to use bundled linux kernel headers],
	[st_cv_use_bundled_headers],
	[AS_IF([test $enable_bundled = check],
	       [AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([#include <linux/version.h>
#define USE_OS_HEADERS (LINUX_VERSION_CODE > ]linux_version_code[ ? 1 : -1)],
					 [[int i[USE_OS_HEADERS];]]
			 )
			],
			[st_cv_use_bundled_headers=no],
			[st_cv_use_bundled_headers=yes]
		)
	       ],
	       [st_cv_use_bundled_headers=$enable_bundled]
	 )
	]
)

AM_CONDITIONAL([USE_BUNDLED_HEADERS], [test $st_cv_use_bundled_headers = yes])
AS_IF([test $st_cv_use_bundled_headers = yes],
      [CPPFLAGS="$CPPFLAGS -I$srcdir/bundled/linux/arch/$karch/include/uapi"
       CPPFLAGS="$CPPFLAGS -I$srcdir/bundled/linux/include/uapi"
      ]
)

MIPS_ABI=
if test "$arch" = mips; then
	AC_CACHE_CHECK([for _MIPS_SIM], [st_cv__MIPS_SIM],
		       [AC_COMPUTE_INT([st_cv__MIPS_SIM], [_MIPS_SIM], ,
				       [AC_MSG_FAILURE([_MIPS_SIM cannot be determined])])])

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

st_WARN_CFLAGS
st_BROKEN_CC
AX_PROG_CC_FOR_BUILD
AM_PROG_AR

AC_C_TYPEOF

AC_TYPE_UID_T

AC_CHECK_FUNCS(m4_normalize([
	accept4
	be64toh
	fallocate
	fanotify_mark
	fcntl64
	fopen64
	fork
	fputs_unlocked
	fstatat
	ftruncate
	futimens
	iconv_open
	if_indextoname
	mempcpy
	open64
	open_memstream
	preadv
	process_vm_readv
	process_vm_writev
	pwritev
	readahead
	signalfd
	stpcpy
	strerror
	strndup
	sync_file_range
]))

AC_CHECK_TYPES([sig_atomic_t, struct sigcontext],,, [#include <signal.h>])

AC_CHECK_TYPES([struct sockaddr_storage],
	       [],
	       [mkdir -p src/linux/generic/linux
		cp $srcdir/src/linux/generic/socket.h.in src/linux/generic/linux/socket.h
		CPPFLAGS="-Isrc/linux/generic $CPPFLAGS"],
	       [#include <sys/socket.h>])

AC_CHECK_TYPES([struct mmsghdr],,, [#include <sys/socket.h>])

AC_CHECK_TYPES([__kernel_long_t, __kernel_ulong_t],,,
[#include <asm/posix_types.h>])

AC_CHECK_TYPES([struct __kernel_timespec, struct __kernel_sock_timeval],,,
[#include <linux/time_types.h>])

AC_CHECK_TYPES([struct stat64, struct __old_kernel_stat],,,
[#include <sys/types.h>
#include <asm/stat.h>])

AC_CHECK_TYPES([struct user_desc],
	       [AC_CHECK_MEMBERS([struct user_desc.lm],,,
				 [#include <asm/ldt.h>])],,
	       [#include <asm/ldt.h>])

AC_CHECK_MEMBERS([struct stat.st_mtime_nsec, struct stat64.st_mtime_nsec],,,
[#include <sys/types.h>
#include <asm/stat.h>])

AC_CHECK_TYPES(m4_normalize([
	struct pt_all_user_regs,
	struct ia64_fpreg,
	struct ptrace_peeksiginfo_args,
	struct __ptrace_syscall_info
]),,, [#include <sys/ptrace.h>])

AC_CHECK_TYPES([struct ptrace_syscall_info],,, [#include <linux/ptrace.h>])

# For kernels that do not have v3.10-rc1~201^2~11
AC_CHECK_TYPES([s390_compat_regs],,, [#include <asm/ptrace.h>])

AC_CHECK_MEMBERS([struct utsname.domainname],,, [#include <sys/utsname.h>])

AC_CHECK_MEMBERS(m4_normalize([
	siginfo_t.si_syscall,
	siginfo_t.si_timerid,
	siginfo_t.si_overrun
]),,, [#include <signal.h>])

AC_CHECK_HEADERS(m4_normalize([
	asm/cachectl.h
	asm/guarded_storage.h
	asm/sysmips.h
	elf.h
	gcov.h
	iconv.h
	mqueue.h
	netinet/sctp.h
	netipx/ipx.h
	paths.h
	scsi/sg.h
	sys/eventfd.h
	sys/fanotify.h
	sys/ipc.h
	sys/quota.h
	sys/signalfd.h
	sys/xattr.h
	ustat.h
]))

AC_CHECK_HEADERS(m4_normalize([
	linux/ipc.h
	linux/kvm.h
]),,, [AC_INCLUDES_DEFAULT
#include <linux/types.h>
])

AC_CHECK_HEADERS([asm/sigcontext.h],,, [#include <signal.h>])

AC_CHECK_HEADERS([netinet/tcp.h netinet/udp.h],,, [#include <netinet/in.h>])

AC_CHECK_HEADERS(m4_normalize([
	linux/netfilter_arp/arp_tables.h
	linux/netfilter_bridge/ebtables.h
	linux/netfilter_ipv4/ip_tables.h
	linux/netfilter_ipv6/ip6_tables.h
]),,, [#include <netinet/in.h>
#include <net/if.h>])

AC_CHECK_HEADERS([linux/bpf.h], [
	AC_CHECK_TYPES(m4_normalize([
		struct bpf_insn,
		struct bpf_map_info,
		struct bpf_prog_info
	]),,, [#include <linux/bpf.h>])
	st_BPF_ATTR
])

AC_CHECK_HEADERS([bluetooth/bluetooth.h], [
	AC_CHECK_MEMBERS([struct sockaddr_hci.hci_channel],,,
			 [#include <bluetooth/bluetooth.h>
			 #include <bluetooth/hci.h>])
	AC_CHECK_MEMBERS([struct sockaddr_l2.l2_bdaddr_type],,,
			 [#include <bluetooth/bluetooth.h>
			 #include <bluetooth/l2cap.h>])
])

AC_CHECK_TYPES([struct statfs], [
	AC_CHECK_MEMBERS(m4_normalize([
		struct statfs.f_frsize,
		struct statfs.f_flags,
		struct statfs.f_fsid.val,
		struct statfs.f_fsid.__val
	]),,, [#include <linux/types.h>
#include <asm/statfs.h>])
],, [#include <linux/types.h>
#include <asm/statfs.h>])

AC_CHECK_TYPES([struct statfs64], [
	AC_CHECK_MEMBERS(m4_normalize([
		struct statfs64.f_frsize,
		struct statfs64.f_flags,
		struct statfs64.f_fsid.val,
		struct statfs64.f_fsid.__val
	]),,, [#include <linux/types.h>
#include <asm/statfs.h>])
],, [#include <linux/types.h>
#include <asm/statfs.h>])

AC_CHECK_TYPES(m4_normalize([
	struct kvm_cpuid2,
	struct kvm_regs,
	struct kvm_sregs,
	struct kvm_userspace_memory_region
]),,, [#include <linux/kvm.h>])

saved_CPPFLAGS="$CPPFLAGS"

AC_CACHE_CHECK([whether <linux/signal.h> can be used along with <signal.h>],
	       [st_cv_linux_signal],
	       [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#include <signal.h>
#include <linux/signal.h>]],[])],
				  [st_cv_linux_signal=yes],
				  [st_cv_linux_signal=no])])
if test "x$st_cv_linux_signal" = xno; then
	mkdir -p src/linux/generic/linux
	cp $srcdir/src/linux/generic/signal.h.in src/linux/generic/linux/signal.h
	CPPFLAGS="-Isrc/linux/generic $CPPFLAGS"
fi

AC_CHECK_TYPES([struct __aio_sigset],,, [#include <linux/aio_abi.h>])

AC_CHECK_MEMBERS(m4_normalize([
		struct iocb.aio_flags,
		struct iocb.aio_rw_flags
		]),,, [#include <linux/aio_abi.h>])

CPPFLAGS="$saved_CPPFLAGS"

st_CHECK_ENUMS

AC_CHECK_DECLS([sys_errlist])
AC_CHECK_DECLS(m4_normalize([
	PTRACE_PEEKUSER,
	PTRACE_POKEUSER
]),,, [#include <sys/ptrace.h>])

AC_CHECK_SIZEOF([long])
SIZEOF_LONG="$ac_cv_sizeof_long"
AC_SUBST(SIZEOF_LONG)
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([off_t],,[#include <sys/types.h>])
AC_CHECK_SIZEOF([kernel_long_t],,[#include "$srcdir/src/kernel_types.h"])
SIZEOF_KERNEL_LONG_T="$ac_cv_sizeof_kernel_long_t"
AC_SUBST(SIZEOF_KERNEL_LONG_T)
AC_CHECK_SIZEOF([struct i64_i32],,[struct i64_i32 {long long i64; int i32;};])
AC_CHECK_SIZEOF([struct msqid64_ds],,[#include <linux/msg.h>])


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

AC_CACHE_CHECK([for program_invocation_name], [st_cv_have_program_invocation_name],
	       [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <errno.h>]],
						[[return !*program_invocation_name]])],
			       [st_cv_have_program_invocation_name=yes],
			       [st_cv_have_program_invocation_name=no])])
if test "x$st_cv_have_program_invocation_name" = xyes; then
	AC_DEFINE([HAVE_PROGRAM_INVOCATION_NAME], [1],
		  [Define to 1 if the system provides program_invocation_name variable])
fi

AC_CACHE_CHECK([for static_assert], [st_cv_have_static_assert],
	       [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <assert.h>]],
						[[static_assert(1,"")]]
					       )
			       ],
			       [st_cv_have_static_assert=yes],
			       [AC_LINK_IFELSE([AC_LANG_PROGRAM([],
								[[_Static_assert(1,"")]]
							       )
					       ],
					       [st_cv_have_static_assert=_Static_assert],
					       [st_cv_have_static_assert=no]
					      )
			       ]
			      )
	       ]
	      )
case "x$st_cv_have_static_assert" in
	xyes)
		AC_DEFINE([HAVE_STATIC_ASSERT], [1],
			  [Define to 1 if the system provides static_assert])
		;;
	x_Static_assert)
		AC_DEFINE([HAVE__STATIC_ASSERT], [1],
			  [Define to 1 if the system provides _Static_assert])
		;;
esac

saved_LIBS="$LIBS"
AC_SEARCH_LIBS([dladdr], [dl])
LIBS="$saved_LIBS"
dl_LIBS=""
if test "$ac_cv_search_dladdr" != no; then
	AC_DEFINE([HAVE_DLADDR], [1], [Define to 1 if the system provides dladdr])
	case "$ac_cv_search_dladdr" in
		-l*) dl_LIBS="$ac_cv_search_dladdr" ;;
	esac
fi
AC_SUBST(dl_LIBS)

saved_LIBS="$LIBS"
AC_SEARCH_LIBS([timer_create], [rt])
LIBS="$saved_LIBS"
case "$ac_cv_search_timer_create" in
	no) AC_MSG_FAILURE([failed to find timer_create]) ;;
	-l*) timer_LIBS="$ac_cv_search_timer_create" ;;
	*) timer_LIBS= ;;
esac
AC_SUBST(timer_LIBS)

saved_LIBS="$LIBS"
AC_SEARCH_LIBS([clock_gettime], [rt])
LIBS="$saved_LIBS"
case "$ac_cv_search_clock_gettime" in
	no) AC_MSG_FAILURE([failed to find clock_gettime]) ;;
	-l*) clock_LIBS="$ac_cv_search_clock_gettime" ;;
	*) clock_LIBS= ;;
esac
AC_SUBST(clock_LIBS)

saved_LIBS="$LIBS"
AC_SEARCH_LIBS([mq_open], [rt])
LIBS="$saved_LIBS"
case "$ac_cv_search_mq_open" in
	-l*) mq_LIBS="$ac_cv_search_mq_open" ;;
	*) mq_LIBS= ;;
esac
AC_SUBST(mq_LIBS)

AC_CHECK_TOOL([READELF], [readelf])

st_STACKTRACE

st_SELINUX

if test "$arch" = mips && test "$no_create" != yes; then
	mkdir -p src/linux/mips
	if $srcdir/src/linux/mips/genstub.sh \
	   $srcdir/src/linux/mips src/linux/mips n32 n64 o32 &&
	   $srcdir/src/linux/mips/genstub.sh \
	   $srcdir/src/linux/32 src/linux/mips common-32 &&
	   $srcdir/src/linux/mips/genstub.sh \
	   $srcdir/src/linux/generic src/linux/mips common; then
		AC_MSG_RESULT([Generated MIPS syscallent stubs])
	else
		AC_MSG_ERROR([Failed to generate syscallent stubs])
	fi
fi

AC_ARG_ENABLE([mpers],
	[AS_HELP_STRING([--enable-mpers=yes|no|check|m32|mx32],
		[whether to enable multiple personalities support required
		 for proper decoding of structures used by tracees with
		 personalities that differ from the personality of strace,
		 default is yes.])],
	[case "$enableval" in
		yes|no|check|m32|mx32) enable_mpers="$enableval" ;;
		*) AC_MSG_ERROR([bad value $enableval for enable-mpers option.
Valid options are: yes, no, check, m32, mx32.])
		;;
	 esac],
	[enable_mpers=yes])

AS_IF([test x$arch = xaarch64],
      [# So far, only aarch64 needs a separate compiler for its compat
       # personality (which is AArch32, that is more or less ARMv7 EABI)
       #
       # Some heuristics regarding possible compiler name:
       #   Debian:  arm-linux-gnueabi{,hf}-gcc
       #   SuSE:    arm7{,hl}-linux-gnueabi-gcc
       #   Red Hat: arm-redhat-linux-gnu-gcc
       #   Fedora:  arm-linux-gnu-gcc
       #   ALT:     armh-alt-linux-gnueabi-gcc
       m4_foreach([triplet1], [arm, arm7, arm7hl, armh], dnl
		  [m4_foreach([triplet2], [, $host_vendor-], dnl
			      [m4_foreach([triplet3], [gnu, gnueabi, gnueabihf], dnl
					  [m4_foreach([triplet_cc], [gcc, cc], dnl
						      [m4_append([arm_compat_compilers], dnl
								 triplet1[-]triplet2[linux-]triplet3[-]triplet_cc)])])])])
       AC_CHECK_PROGS(CC_FOR_M32, arm_compat_compilers)
       AS_IF([test -n "$CC_FOR_M32"],
	     [: ${CPP_FOR_M32=$CC_FOR_M32 -E}
	      : ${CFLAGS_FOR_M32=-std=gnu99}
	      : ${CPPFLAGS_FOR_M32=}
	     ])
      ])

# Set default compiler variables for personalities.
m4_foreach([pers], [M32, MX32], dnl
	   [m4_foreach([var], [CC, CPP, CPPFLAGS], dnl
		       [[: ${]var[_FOR_]pers[=$]var[}]
		        AC_SUBST(var[_FOR_]pers)]) dnl
	    m4_foreach([var], [CFLAGS], dnl
		       [[: ${]var[_FOR_]pers[=][}]
		        AC_SUBST(var[_FOR_]pers)])])

st_MPERS([m32], [aarch64|powerpc64|s390x|sparc64|tile|x32|x86_64])
st_MPERS([mx32], [x86_64])

AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

AC_CONFIG_FILES([Makefile
		 bundled/Makefile
		 debian/changelog
		 doc/strace.1
		 doc/strace-log-merge.1
		 src/Makefile
		 strace.spec
		 tests/Makefile
		 tests-m32/Makefile
		 tests-mx32/Makefile])
AC_OUTPUT
