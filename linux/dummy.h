/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STRACE_LINUX_DUMMY_H
#define STRACE_LINUX_DUMMY_H

#ifndef HAVE_STRUCT___OLD_KERNEL_STAT
#define	sys_oldfstat		printargs
#define	sys_oldstat		printargs
#endif

/* still unfinished */
#define	sys_vm86		printargs
#define	sys_vm86old		printargs

/* machine-specific */
#ifndef HAVE_STRUCT_USER_DESC
# define	sys_modify_ldt		printargs
#endif

#if !(defined HAVE_STRUCT_USER_DESC || defined M68K || defined MIPS)
# define	sys_set_thread_area	printargs
#endif

#if !(defined HAVE_STRUCT_USER_DESC || defined M68K)
# define	sys_get_thread_area	printargs
#endif

#ifdef ALPHA
# define	sys_getdtablesize	printargs
#endif

/* like another call */
#define	sys_acct		sys_chdir
#define	sys_chroot		sys_chdir
#define	sys_clock_getres	sys_clock_gettime
#define	sys_connect		sys_bind
#define	sys_fchdir		sys_close
#define	sys_fdatasync		sys_close
#define	sys_fsync		sys_close
#define	sys_getegid		sys_getuid
#define	sys_getegid16		sys_geteuid16
#define	sys_geteuid		sys_getuid
#define	sys_geteuid16		sys_getuid16
#define	sys_getgid		sys_getuid
#define	sys_getgid16		sys_getuid16
#define	sys_getpeername		sys_getsockname
#define	sys_getresgid		sys_getresuid
#define	sys_getresgid16		sys_getresuid16
#define	sys_lstat		sys_stat
#define	sys_lstat64		sys_stat64
#define	sys_mkdir		sys_chmod
#define	sys_mkdirat		sys_fchmodat
#define	sys_mlock		sys_munmap
#define	sys_mq_unlink		sys_chdir
#define	sys_munlock		sys_munmap
#define	sys_oldlstat		sys_oldstat
#define	sys_pivotroot		sys_link
#define	sys_rename		sys_link
#define	sys_rmdir		sys_chdir
#define	sys_sched_get_priority_max	sys_sched_get_priority_min
#define	sys_set_robust_list	sys_munmap
#define	sys_setdomainname	sys_sethostname
#define	sys_setfsgid		sys_setfsuid
#define	sys_setfsgid16		sys_setfsuid16
#define	sys_setgid		sys_setuid
#define	sys_setgid16		sys_setuid16
#define	sys_setregid		sys_setreuid
#define	sys_setregid16		sys_setreuid16
#define	sys_setresgid		sys_setresuid
#define	sys_setresgid16		sys_setresuid16
#define	sys_stime		sys_time
#define	sys_swapoff		sys_chdir
#define	sys_symlink		sys_link
#define	sys_syncfs		sys_close
#define	sys_umount		sys_chdir
#define	sys_unlink		sys_chdir
#define	sys_uselib		sys_chdir
#define	sys_vfork		sys_fork

/* printargs does the right thing */
#define	sys_getpgrp		printargs
#define	sys_getpid		printargs
#define	sys_getppid		printargs
#define	sys_gettid		printargs
#define	sys_idle		printargs
#define	sys_inotify_init	printargs
#define	sys_munlockall		printargs
#define	sys_pause		printargs
#define	sys_printargs		printargs
#define	sys_sched_yield		printargs
#define	sys_setsid		printargs
#define	sys_set_tid_address	printargs
#define	sys_setup		printargs
#define	sys_sync		printargs
#define	sys_syscall		printargs
#define	sys_vhangup		printargs

/* printargs_u does the right thing */
#define	sys_alarm		printargs_u

/* printargs_d does the right thing */
#define	sys_exit		printargs_d
#define	sys_getpgid		printargs_d
#define	sys_getsid		printargs_d
#define	sys_nice		printargs_d
#define	sys_setpgid		printargs_d
#define	sys_setpgrp		printargs_d
#define	sys_timer_delete	printargs_d
#define	sys_timer_getoverrun	printargs_d

/* unimplemented */
#define	sys_afs_syscall		printargs
#define	sys_break		printargs
#define	sys_create_module	printargs
#define	sys_ftime		printargs
#define	sys_get_kernel_syms	printargs
#define	sys_getpmsg		printargs
#define	sys_gtty		printargs
#define	sys_lock		printargs
#define	sys_mpx			printargs
#define	sys_nfsservctl		printargs
#define	sys_phys		printargs
#define	sys_prof		printargs
#define	sys_profil		printargs
#define	sys_putpmsg		printargs
#define	sys_query_module	printargs
#define	sys_security		printargs
#define	sys_stty		printargs
#define	sys_timerfd		printargs
#define	sys_tuxcall		printargs
#define	sys_ulimit		printargs
#define	sys_vserver		printargs

/* deprecated */
#define	sys_bdflush		printargs
#define	sys_oldolduname		printargs
#define	sys_olduname		printargs
#define	sys_sysfs		printargs

#endif /* !STRACE_LINUX_DUMMY_H */
