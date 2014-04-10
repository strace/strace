dnl Process this file with autoconf to create configure.  Use autoreconf.
AC_PREREQ(2.57)
AC_INIT([strace],
	m4_esyscmd([./git-version-gen .tarball-version]),
	[strace-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([strace.c])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign dist-xz no-dist-gzip silent-rules parallel-tests])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS

AC_MSG_CHECKING([for supported architecture])
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
	arch=powerpc
	AC_DEFINE([POWERPC], 1, [Define for the PowerPC architecture.])
	AC_TRY_COMPILE(
[#ifndef __LP64__
# error 32 bit
#endif], [], ppc_bits=64, ppc_bits=32)
	if test "$ppc_bits" = "64"; then
		AC_DEFINE([POWERPC64], 1, [Define for the PowerPC64 architecture.])
	fi
	;;
arm*)
	arch=arm
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
aarch64*)
	arch=aarch64
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
	AC_TRY_COMPILE(
[#ifndef __ILP32__
# error not x32
#endif], [], arch=x32, arch=x86_64)
	if test "$arch" = "x86_64"; then
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
or1k*)
	arch=or1k
	AC_DEFINE([OR1K], 1, [Define for the OpenRISC 1000 architecture.])
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

AC_SUBST(arch)

AM_CONDITIONAL([I386], [test x$arch = xi386])
AM_CONDITIONAL([X86_64], [test x$arch = xx86_64])
AM_CONDITIONAL([X32], [test x$arch = xx32])

AC_INCLUDEDIR

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

gl_WARN_ADD([-Wall])
gl_WARN_ADD([-Wwrite-strings])
AC_ARG_ENABLE([gcc-Werror],
  [AS_HELP_STRING([--enable-gcc-Werror], [turn on gcc's -Werror option])],
  [case $enableval in
     yes) gl_WARN_ADD([-Werror]) ;;
     no)  ;;
     *)   AC_MSG_ERROR([bad value $enableval for gcc-Werror option]) ;;
   esac]
)
AC_SUBST([WARN_CFLAGS])

AC_PROG_CPP
AC_PROG_INSTALL
AC_C_CONST
AC_C_BIGENDIAN
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_CHECK_MEMBERS(m4_normalize([
	struct stat.st_aclcnt,
	struct stat.st_blksize,
	struct stat.st_blocks,
	struct stat.st_flags,
	struct stat.st_fstype,
	struct stat.st_gen,
	struct stat.st_level,
	struct stat.st_rdev
]))
AC_STAT64
AC_STATFS64

AC_TYPE_SIGNAL
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_GETGROUPS
AC_HEADER_MAJOR
AC_CHECK_TYPES([sig_atomic_t, siginfo_t],,, [#include <signal.h>])
AC_CHECK_MEMBERS([struct sockaddr_in6.sin6_scope_id],,,
[#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>])
AC_LITTLE_ENDIAN_LONG_LONG

AC_CHECK_FUNCS(m4_normalize([
	fopen64
	fork
	fputs_unlocked
	if_indextoname
	inet_ntop
	prctl
	process_vm_readv
	sendmsg
	sigaction
	stpcpy
	strerror
	strsignal
]))
AC_CHECK_HEADERS(m4_normalize([
	asm/cachectl.h
	asm/sysmips.h
	elf.h
	inttypes.h
	ioctls.h
	linux/capability.h
	linux/perf_event.h
	linux/ptrace.h
	linux/utsname.h
	mqueue.h
	netinet/sctp.h
	poll.h
	scsi/sg.h
	stropts.h
	sys/acl.h
	sys/asynch.h
	sys/conf.h
	sys/epoll.h
	sys/filio.h
	sys/ioctl.h
	sys/poll.h
	sys/ptrace.h
	sys/reg.h
	sys/uio.h
	sys/vfs.h
]))
AC_CHECK_HEADERS([linux/icmp.h linux/in6.h linux/netlink.h linux/if_packet.h],
                 [], [], [#include <stddef.h>
#include <sys/socket.h>
#include <asm/types.h>])
AC_CHECK_HEADERS([asm/sigcontext.h], [], [], [#include <signal.h>])
AC_CHECK_TYPES([struct sigcontext],,, [#include <signal.h>])
AC_CHECK_HEADERS([netinet/tcp.h netinet/udp.h],,, [#include <netinet/in.h>])

AC_CHECK_MEMBERS([struct msghdr.msg_control],,, [#include <sys/socket.h>])

AC_CHECK_TYPES([struct __old_kernel_stat],,, [#include <asm/stat.h>])

AC_CHECK_TYPES([struct pt_all_user_regs, struct ia64_fpreg, struct ptrace_peeksiginfo_args],,,
	       [#include <sys/ptrace.h>])

AC_CHECK_MEMBERS([struct utsname.domainname],,, [#include <sys/utsname.h>])

AC_CHECK_MEMBERS([struct sigevent._sigev_un._pad,
		  struct sigevent.__pad,
		  siginfo_t.si_syscall],,, [#include <signal.h>])

AC_CHECK_TYPES([struct flock64],,, [#include <fcntl.h>])

AC_CHECK_DECLS([sys_errlist])
AC_CHECK_DECLS(m4_normalize([
	PTRACE_EVENT_CLONE,
	PTRACE_EVENT_EXEC,
	PTRACE_EVENT_EXIT,
	PTRACE_EVENT_FORK,
	PTRACE_EVENT_VFORK,
	PTRACE_EVENT_VFORK_DONE,
	PTRACE_GETEVENTMSG,
	PTRACE_GETSIGINFO,
	PTRACE_O_TRACECLONE,
	PTRACE_O_TRACEEXEC,
	PTRACE_O_TRACEEXIT,
	PTRACE_O_TRACEFORK,
	PTRACE_O_TRACESYSGOOD,
	PTRACE_O_TRACEVFORK,
	PTRACE_PEEKUSER,
	PTRACE_POKEUSER,
	PTRACE_SETOPTIONS
]),,, [#include <sys/ptrace.h>])

AC_CHECK_DECLS(m4_normalize([
	LO_FLAGS_AUTOCLEAR,
	LO_FLAGS_PARTSCAN
]),,, [#include <linux/loop.h>])

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
AC_CHECK_SIZEOF([rlim_t],,[#include <sys/resource.h>])

AC_CACHE_CHECK([for SA_RESTORER], [st_cv_sa_restorer],
	       [st_cv_sa_restorer="$(echo SA_RESTORER |
			$CPP $CPPFLAGS -P -imacros asm/signal.h - |
			grep ^0x)"
	        test -n "$st_cv_sa_restorer" || st_cv_sa_restorer=no])
if test "x$st_cv_sa_restorer" != xno; then
	AC_DEFINE_UNQUOTED([ASM_SA_RESTORER], [$st_cv_sa_restorer],
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

AC_PATH_PROG([PERL], [perl])

AC_CONFIG_FILES([Makefile tests/Makefile])
AC_OUTPUT
