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
	sprintf(proc, "/proc/%d", pid);
	if ((pfp = fopen(proc, "r+")) == NULL)
		goto fail;
	if (ioctl(fileno(pfp), PIOCSTOP, NULL) < 0)
		goto fail;
	pfd.fd = fileno(pfp);
	pfd.events = POLLPRI;
	if (poll(&pfd, 1, 0) < 0)
		goto fail;
	if (!(pfd.revents & POLLPRI))
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
solaris2*|irix5*)
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
[struct prstatus s; s.pr_syscall;],
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
