dnl ### A macro to find the include directory, useful for cross-compiling.
AC_DEFUN(AC_INCLUDEDIR,
[AC_REQUIRE([AC_PROG_AWK])dnl
AC_SUBST(includedir)
AC_MSG_CHECKING(for primary include directory)
includedir=/usr/include
if test -n "$GCC"
then
	>conftest.c
	new_includedir=`
		$CC -v -E conftest.c 2>&1 | $AWK '
			/^End of search list/ { print last; exit }
			{ last = [$]1 }
		'
	`
	rm -f conftest.c
	if test -n "$new_includedir" && test -d "$new_includedir"
	then
		includedir=$new_includedir
	fi
fi
AC_MSG_RESULT($includedir)
])

dnl ### A macro to automatically set different CC and HOSTCC if using gcc.
define(AC_PROG_HOSTCC,
[AC_SUBST(HOSTCC)dnl
if test -z "$HOSTCC"
then
	HOSTCC="$CC"
	if test -n "$GCC"
	then
		# Find out if gcc groks our host.
		worked=
		last=
		for i in $1
		do
			test "x$i" = "x$last" && continue
			last="$i"
			CC="$HOSTCC -b $i"
			AC_MSG_CHECKING([for working $CC])
			AC_TRY_LINK(,,
				worked=1
				break
			)
			AC_MSG_RESULT(no)
		done
		if test -z "$worked"
		then
			CC="$HOSTCC"
		else
			AC_MSG_RESULT(yes)
		fi
	fi
fi
])

dnl ### A macro to set gcc warning flags.
define(AC_WARNFLAGS,
[AC_SUBST(WARNFLAGS)
if test -z "$WARNFLAGS"
then
	if test -n "$GCC"
	then
		# If we're using gcc we want warning flags.
		WARNFLAGS=-Wall
	fi
fi
])

dnl ### A macro to determine if we have a "MP" type procfs
AC_DEFUN(AC_MP_PROCFS,
[AC_MSG_CHECKING(for MP procfs)
AC_CACHE_VAL(ac_cv_mp_procfs,
[AC_TRY_RUN([
#include <stdio.h>
#include <signal.h>
#include <sys/procfs.h>

main()
{
	int pid;
	char proc[32];
	FILE *ctl;
	FILE *status;
	int cmd;
	struct pstatus pstatus;

	if ((pid = fork()) == 0) {
		pause();
		exit(0);
	}
	sprintf(proc, "/proc/%d/ctl", pid);
	if ((ctl = fopen(proc, "w")) == NULL)
		goto fail;
	sprintf(proc, "/proc/%d/status", pid);
	if ((status = fopen (proc, "r")) == NULL)
		goto fail;
	cmd = PCSTOP;
	if (write (fileno (ctl), &cmd, sizeof cmd) < 0)
		goto fail;
	if (read (fileno (status), &pstatus, sizeof pstatus) < 0)
		goto fail;
	kill(pid, SIGKILL);
	exit(0);
fail:
	kill(pid, SIGKILL);
	exit(1);
}
],
ac_cv_mp_procfs=yes,
ac_cv_mp_procfs=no,
[
# Guess or punt.
case "$host_os" in
svr4.2*|svr5*)
	ac_cv_mp_procfs=yes
	;;
*)
	ac_cv_mp_procfs=no
	;;
esac
])])
AC_MSG_RESULT($ac_cv_mp_procfs)
if test "$ac_cv_mp_procfs" = yes
then
	AC_DEFINE(HAVE_MP_PROCFS)
fi
])

dnl ### A macro to determine if procfs is pollable.
AC_DEFUN(AC_POLLABLE_PROCFS,
[AC_MSG_CHECKING(for pollable procfs)
AC_CACHE_VAL(ac_cv_pollable_procfs,
[AC_TRY_RUN([
#include <stdio.h>
#include <signal.h>
#include <sys/procfs.h>
#include <sys/stropts.h>
#include <poll.h>

#ifdef HAVE_MP_PROCFS
#define PIOCSTOP	PCSTOP
#define POLLWANT	POLLWRNORM
#define PROC		"/proc/%d/ctl"
#define PROC_MODE	"w"
int IOCTL (int fd, int cmd, int arg) {
	return write (fd, &cmd, sizeof cmd);
}
#else
#define POLLWANT	POLLPRI
#define	PROC		"/proc/%d"
#define PROC_MODE	"r+"
#define IOCTL		ioctl
#endif

main()
{
	int pid;
	char proc[32];
	FILE *pfp;
	struct pollfd pfd;

	if ((pid = fork()) == 0) {
		pause();
		exit(0);
	}
	sprintf(proc, PROC, pid);
	if ((pfp = fopen(proc, PROC_MODE)) == NULL)
		goto fail;
	if (IOCTL(fileno(pfp), PIOCSTOP, NULL) < 0)
		goto fail;
	pfd.fd = fileno(pfp);
	pfd.events = POLLWANT;
	if (poll(&pfd, 1, 0) < 0)
		goto fail;
	if (!(pfd.revents & POLLWANT))
		goto fail;
	kill(pid, SIGKILL);
	exit(0);
fail:
	kill(pid, SIGKILL);
	exit(1);
}
],
ac_cv_pollable_procfs=yes,
ac_cv_pollable_procfs=no,
[
# Guess or punt.
case "$host_os" in
solaris2*|irix5*|svr4.2uw*|svr5*)
	ac_cv_pollable_procfs=yes
	;;
*)
	ac_cv_pollable_procfs=no
	;;
esac
])])
AC_MSG_RESULT($ac_cv_pollable_procfs)
if test "$ac_cv_pollable_procfs" = yes
then
	AC_DEFINE(HAVE_POLLABLE_PROCFS)
fi
])

dnl ### A macro to determine if the prstatus structure has a pr_syscall member.
AC_DEFUN(AC_STRUCT_PR_SYSCALL,
[AC_MSG_CHECKING(for pr_syscall in struct prstatus)
AC_CACHE_VAL(ac_cv_struct_pr_syscall,
[AC_TRY_COMPILE([#include <sys/procfs.h>],
[#ifdef HAVE_MP_PROCFS
pstatus_t s;
s.pr_lwp.pr_syscall
#else
prstatus_t s;
s.pr_syscall
#endif],
ac_cv_struct_pr_syscall=yes,
ac_cv_struct_pr_syscall=no)])
AC_MSG_RESULT($ac_cv_struct_pr_syscall)
if test "$ac_cv_struct_pr_syscall" = yes
then
	AC_DEFINE(HAVE_PR_SYSCALL)
fi
])

dnl ### A macro to detect the presence of the sig_atomic_t in signal.h
AC_DEFUN(AC_SIG_ATOMIC_T,
[AC_MSG_CHECKING(for sig_atomic_t in signal.h)
AC_CACHE_VAL(ac_cv_sig_atomic_t,
[AC_TRY_COMPILE([#include <signal.h>],
[sig_atomic_t x;],
ac_cv_sig_atomic_t=yes,
ac_cv_sig_atomic_t=no)])
AC_MSG_RESULT($ac_cv_sig_atomic_t)
if test "$ac_cv_sig_atomic_t" = yes
then
	AC_DEFINE(HAVE_SIG_ATOMIC_T)
fi
])

dnl ### A macro to determine if sys_errlist is declared.
AC_DEFUN(AC_DECL_SYS_ERRLIST,
[AC_MSG_CHECKING([for sys_errlist declaration])
AC_CACHE_VAL(ac_cv_decl_sys_errlist,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
/* Somebody might declare sys_errlist in unistd.h.  */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif], [char *msg = *(sys_errlist + 1);],
  ac_cv_decl_sys_errlist=yes, ac_cv_decl_sys_errlist=no)])dnl
AC_MSG_RESULT($ac_cv_decl_sys_errlist)
if test $ac_cv_decl_sys_errlist = yes; then
  AC_DEFINE(SYS_ERRLIST_DECLARED)
fi
])

dnl ### A macro to determine if _sys_siglist is declared.
AC_DEFUN(AC_DECL__SYS_SIGLIST,
[AC_MSG_CHECKING([for _sys_siglist declaration])
AC_CACHE_VAL(ac_cv_decl__sys_siglist,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <signal.h>
/* Somebody might declare _sys_siglist in unistd.h.  */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif], [char *msg = *(_sys_siglist + 1);],
  ac_cv_decl__sys_siglist=yes, ac_cv_decl__sys_siglist=no)])dnl
AC_MSG_RESULT($ac_cv_decl__sys_siglist)
if test $ac_cv_decl__sys_siglist = yes; then
  AC_DEFINE(SYS_SIGLIST_DECLARED)
fi
])

dnl ### A macro to determine if the msghdr structure has a msg_control member.
AC_DEFUN(AC_STRUCT_MSG_CONTROL,
[AC_MSG_CHECKING(for msg_control in struct msghdr)
AC_CACHE_VAL(ac_cv_struct_msg_control,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
[#undef msg_control
struct msghdr m; m.msg_control;],
ac_cv_struct_msg_control=yes,
ac_cv_struct_msg_control=no)])
AC_MSG_RESULT($ac_cv_struct_msg_control)
if test "$ac_cv_struct_msg_control" = yes
then
	AC_DEFINE(HAVE_MSG_CONTROL)
fi
])

dnl ### A macro to determine whether stat64 is defined.
AC_DEFUN(AC_STAT64,
[AC_MSG_CHECKING(for stat64 in (asm|sys)/stat.h)
AC_CACHE_VAL(ac_cv_type_stat64,
[AC_TRY_COMPILE([#ifdef linux
#include <asm/stat.h>
#else
#include <sys/stat.h>
#endif],
[struct stat64 st;],
ac_cv_type_stat64=yes,
ac_cv_type_stat64=no)])
AC_MSG_RESULT($ac_cv_type_stat64)
if test "$ac_cv_type_stat64" = yes
then
	AC_DEFINE(HAVE_STAT64)
fi
])

dnl ### A macro to determine whether we have long long
AC_DEFUN(AC_LONG_LONG,
[AC_MSG_CHECKING(for long long)
AC_CACHE_VAL(ac_cv_type_long_long,
[AC_TRY_COMPILE([],
[long long x = 20;],
ac_cv_type_long_long=yes,
ac_cv_type_long_long=no)])
AC_MSG_RESULT($ac_cv_type_long_long)
if test "$ac_cv_type_long_long" = yes
then
	AC_DEFINE(HAVE_LONG_LONG)
fi
])

dnl ### A macro to determine if off_t is a long long
AC_DEFUN(AC_OFF_T_IS_LONG_LONG,
[AC_MSG_CHECKING(for long long off_t)
AC_CACHE_VAL(ac_cv_have_long_long_off_t,
[AC_TRY_RUN([#include <sys/types.h>
main () {
	if (sizeof (off_t) == sizeof (long long) &&
	    sizeof (off_t) > sizeof (long))
	    return 0;
	return 1;
}
],
ac_cv_have_long_long_off_t=yes,
ac_cv_have_long_long_off_t=no,
[# Should try to guess here
ac_cv_have_long_long_off_t=no
])])
AC_MSG_RESULT($ac_cv_have_long_long_off_t)
if test "$ac_cv_have_long_long_off_t" = yes
then
	AC_DEFINE(HAVE_LONG_LONG_OFF_T)
fi
])

dnl ### A macro to determine if rlim_t is a long long
AC_DEFUN(AC_RLIM_T_IS_LONG_LONG,
[AC_MSG_CHECKING(for long long rlim_t)
AC_CACHE_VAL(ac_cv_have_long_long_rlim_t,
[AC_TRY_RUN([#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
main () {
	if (sizeof (rlim_t) == sizeof (long long) &&
	    sizeof (rlim_t) > sizeof (long))
	    return 0;
	return 1;
}
],
ac_cv_have_long_long_rlim_t=yes,
ac_cv_have_long_long_rlim_t=no,
[# Should try to guess here
ac_cv_have_long_long_rlim_t=no
])])
AC_MSG_RESULT($ac_cv_have_long_long_rlim_t)
if test "$ac_cv_have_long_long_rlim_t" = yes
then
	AC_DEFINE(HAVE_LONG_LONG_RLIM_T)
fi
])

dnl ### A macro to determine whether sin6_scope_id is available.
AC_DEFUN(AC_SIN6_SCOPE_ID,
[AC_MSG_CHECKING(for sin6_scope_id in sockaddr_in6)
AC_CACHE_VAL(ac_cv_have_sin6_scope_id,
[AC_TRY_COMPILE([#include <netinet/in.h>],
[ struct sockaddr_in6 s; s.sin6_scope_id = 0; ],
ac_cv_have_sin6_scope_id=yes,
ac_cv_have_sin6_scope_id=no)])
AC_MSG_RESULT($ac_cv_have_sin6_scope_id)
if test "$ac_cv_have_sin6_scope_id" = "yes" ; then
       AC_DEFINE(HAVE_SIN6_SCOPE_ID)
else
       AC_MSG_CHECKING(for sin6_scope_id in linux sockaddr_in6)
       AC_CACHE_VAL(ac_cv_have_sin6_scope_id_linux,
       [AC_TRY_COMPILE([#include <linux/in6.h>],
       [ struct sockaddr_in6 s; s.sin6_scope_id = 0; ],
       ac_cv_have_sin6_scope_id_linux=yes,
       ac_cv_have_sin6_scope_id_linux=no)])
       AC_MSG_RESULT($ac_cv_have_sin6_scope_id_linux)
       if test "$ac_cv_have_sin6_scope_id_linux" = "yes" ; then
               AC_DEFINE(HAVE_SIN6_SCOPE_ID)
               AC_DEFINE(HAVE_SIN6_SCOPE_ID_LINUX)
       fi
fi
])

dnl ### A macro to check for st_flags in struct stat
AC_DEFUN(AC_ST_FLAGS,
[AC_MSG_CHECKING(for st_flags in struct stat)
AC_CACHE_VAL(ac_cv_have_st_flags,
[AC_TRY_COMPILE([#include <sys/stat.h>],
[struct stat buf;
buf.st_flags = 0;],
ac_cv_have_st_flags=yes,
ac_cv_have_st_flags=no)])
AC_MSG_RESULT($ac_cv_have_st_flags)
if test "$ac_cv_have_st_flags" = yes
then
	AC_DEFINE(HAVE_ST_FLAGS)
fi
])

dnl ### A macro to check for st_aclcnt in struct stat
AC_DEFUN(AC_ST_ACLCNT,
[AC_MSG_CHECKING(for st_aclcnt in struct stat)
AC_CACHE_VAL(ac_cv_have_st_aclcnt,
[AC_TRY_COMPILE([#include <sys/stat.h>],
[struct stat buf;
buf.st_aclcnt = 0;],
ac_cv_have_st_aclcnt=yes,
ac_cv_have_st_aclcnt=no)])
AC_MSG_RESULT($ac_cv_have_st_aclcnt)
if test "$ac_cv_have_st_aclcnt" = yes
then
	AC_DEFINE(HAVE_ST_ACLCNT)
fi
])

dnl ### A macro to check for st_level in struct stat
AC_DEFUN(AC_ST_LEVEL,
[AC_MSG_CHECKING(for st_level in struct stat)
AC_CACHE_VAL(ac_cv_have_st_level,
[AC_TRY_COMPILE([#include <sys/stat.h>],
[struct stat buf;
buf.st_level = 0;],
ac_cv_have_st_level=yes,
ac_cv_have_st_level=no)])
AC_MSG_RESULT($ac_cv_have_st_level)
if test "$ac_cv_have_st_level" = yes
then
	AC_DEFINE(HAVE_ST_LEVEL)
fi
])

dnl ### A macro to check for st_fstype in struct stat
AC_DEFUN(AC_ST_FSTYPE,
[AC_MSG_CHECKING(for st_fstype in struct stat)
AC_CACHE_VAL(ac_cv_have_st_fstype,
[AC_TRY_COMPILE([#include <sys/stat.h>],
[struct stat buf;
buf.st_fstype[0] = 0;],
ac_cv_have_st_fstype=yes,
ac_cv_have_st_fstype=no)])
AC_MSG_RESULT($ac_cv_have_st_fstype)
if test "$ac_cv_have_st_fstype" = yes
then
	AC_DEFINE(HAVE_ST_FSTYPE)
fi
])

dnl ### A macro to check for st_gen in struct stat
AC_DEFUN(AC_ST_GEN,
[AC_MSG_CHECKING(for st_gen in struct stat)
AC_CACHE_VAL(ac_cv_have_st_gen,
[AC_TRY_COMPILE([#include <sys/stat.h>],
[struct stat buf;
buf.st_gen = 0;],
ac_cv_have_st_gen=yes,
ac_cv_have_st_gen=no)])
AC_MSG_RESULT($ac_cv_have_st_gen)
if test "$ac_cv_have_st_gen" = yes
then
	AC_DEFINE(HAVE_ST_GEN)
fi
])

dnl ### A macro to determine endianness of long long
AC_DEFUN(AC_LITTLE_ENDIAN_LONG_LONG,
[AC_MSG_CHECKING(for little endian long long)
AC_CACHE_VAL(ac_cv_have_little_endian_long_long,
[AC_TRY_RUN([
int main () {
	union {
		long long ll;
		long l [2];
	} u;
	u.ll = 0x12345678;
	if (u.l[0] == 0x12345678) 
		return 0;
	return 1;
}
],
ac_cv_have_little_endian_long_long=yes,
ac_cv_have_little_endian_long_long=no,
[# Should try to guess here
ac_cv_have_little_endian_long_long=no
])])
AC_MSG_RESULT($ac_cv_have_little_endian_long_long)
if test "$ac_cv_have_little_endian_long_long" = yes
then
	AC_DEFINE(HAVE_LITTLE_ENDIAN_LONG_LONG)
fi
])

