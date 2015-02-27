#ifdef NEED_PTRACE_PROTOTYPE_WORKAROUND
# define ptrace xptrace
# include <sys/ptrace.h>
# undef ptrace
extern long ptrace(int, int, char *, long);
#else
# include <sys/ptrace.h>
#endif

#ifdef HAVE_STRUCT_IA64_FPREG
# define ia64_fpreg XXX_ia64_fpreg
#endif
#ifdef HAVE_STRUCT_PT_ALL_USER_REGS
# define pt_all_user_regs XXX_pt_all_user_regs
#endif
#ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
# define ptrace_peeksiginfo_args XXX_ptrace_peeksiginfo_args
#endif

#include <linux/ptrace.h>

#ifdef HAVE_STRUCT_IA64_FPREG
# undef ia64_fpreg
#endif
#ifdef HAVE_STRUCT_PT_ALL_USER_REGS
# undef pt_all_user_regs
#endif
#ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
# undef ptrace_peeksiginfo_args
#endif

#ifndef PTRACE_EVENT_FORK
# define PTRACE_EVENT_FORK	1
#endif
#ifndef PTRACE_EVENT_VFORK
# define PTRACE_EVENT_VFORK	2
#endif
#ifndef PTRACE_EVENT_CLONE
# define PTRACE_EVENT_CLONE	3
#endif
#ifndef PTRACE_EVENT_EXEC
# define PTRACE_EVENT_EXEC	4
#endif
#ifndef PTRACE_EVENT_VFORK_DONE
# define PTRACE_EVENT_VFORK_DONE	5
#endif
#ifndef PTRACE_EVENT_EXIT
# define PTRACE_EVENT_EXIT	6
#endif
#ifndef PTRACE_EVENT_SECCOMP
# define PTRACE_EVENT_SECCOMP	7
#endif
#ifdef PTRACE_EVENT_STOP
/* Linux 3.1 - 3.3 releases had a broken value.  It was fixed in 3.4.  */
# if PTRACE_EVENT_STOP == 7
#  undef PTRACE_EVENT_STOP
# endif
#endif
#ifndef PTRACE_EVENT_STOP
# define PTRACE_EVENT_STOP	128
#endif

#ifndef PTRACE_O_TRACESYSGOOD
# define PTRACE_O_TRACESYSGOOD	1
#endif
#ifndef PTRACE_O_TRACEFORK
# define PTRACE_O_TRACEFORK	(1 << PTRACE_EVENT_FORK)
#endif
#ifndef PTRACE_O_TRACEVFORK
# define PTRACE_O_TRACEVFORK	(1 << PTRACE_EVENT_VFORK)
#endif
#ifndef PTRACE_O_TRACECLONE
# define PTRACE_O_TRACECLONE	(1 << PTRACE_EVENT_CLONE)
#endif
#ifndef PTRACE_O_TRACEEXEC
# define PTRACE_O_TRACEEXEC	(1 << PTRACE_EVENT_EXEC)
#endif
#ifndef PTRACE_O_TRACEVFORKDONE
# define PTRACE_O_TRACEVFORKDONE	(1 << PTRACE_EVENT_VFORK_DONE)
#endif
#ifndef PTRACE_O_TRACEEXIT
# define PTRACE_O_TRACEEXIT	(1 << PTRACE_EVENT_EXIT)
#endif
#ifndef PTRACE_O_TRACESECCOMP
# define PTRACE_O_TRACESECCOMP	(1 << PTRACE_EVENT_SECCOMP)
#endif
#ifndef PTRACE_O_EXITKILL
# define PTRACE_O_EXITKILL	(1 << 20)
#endif

#ifndef PTRACE_SETOPTIONS
# define PTRACE_SETOPTIONS	0x4200
#endif
#ifndef PTRACE_GETEVENTMSG
# define PTRACE_GETEVENTMSG	0x4201
#endif
#ifndef PTRACE_GETSIGINFO
# define PTRACE_GETSIGINFO	0x4202
#endif
#ifndef PTRACE_SETSIGINFO
# define PTRACE_SETSIGINFO	0x4203
#endif
#ifndef PTRACE_GETREGSET
# define PTRACE_GETREGSET	0x4204
#endif
#ifndef PTRACE_SETREGSET
# define PTRACE_SETREGSET	0x4205
#endif
#ifndef PTRACE_SEIZE
# define PTRACE_SEIZE		0x4206
#endif
#ifndef PTRACE_INTERRUPT
# define PTRACE_INTERRUPT	0x4207
#endif
#ifndef PTRACE_LISTEN
# define PTRACE_LISTEN		0x4208
#endif
#ifndef PTRACE_PEEKSIGINFO
# define PTRACE_PEEKSIGINFO	0x4209
#endif
#ifndef PTRACE_GETSIGMASK
# define PTRACE_GETSIGMASK	0x420a
#endif
#ifndef PTRACE_SETSIGMASK
# define PTRACE_SETSIGMASK	0x420b
#endif

#if !HAVE_DECL_PTRACE_PEEKUSER
# define PTRACE_PEEKUSER PTRACE_PEEKUSR
#endif
#if !HAVE_DECL_PTRACE_POKEUSER
# define PTRACE_POKEUSER PTRACE_POKEUSR
#endif
