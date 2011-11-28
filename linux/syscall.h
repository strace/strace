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
 *
 *	$Id$
 */

#include "dummy.h"

/* primary syscalls */

int sys_restart_syscall();
int sys_setup();
int sys_exit();
int sys_fork();
int sys_read();
int sys_write();
int sys_open();
int sys_close();
int sys_waitpid();
int sys_creat();
int sys_link();
int sys_execve();
int sys_chdir();
int sys_time();
int sys_mknod();
int sys_chmod();
int sys_chown();
int sys_break();
int sys_oldstat();
int sys_lseek();
int sys_getpid();
int sys_mount();
int sys_umount();
int sys_umount2();
int sys_setuid();
int sys_getuid();
int sys_stime();
int sys_ptrace();
int sys_alarm();
int sys_oldfstat();
int sys_pause();
int sys_utime();
int sys_stty();
int sys_gtty();
int sys_access();
int sys_nice();
int sys_ftime();
int sys_sync();
int sys_kill();
int sys_mkdir();
int sys_pipe();
int sys_times();
int sys_prof();
int sys_brk();
int sys_setgid();
int sys_getgid();
int sys_signal();
int sys_geteuid();
int sys_getegid();
int sys_acct();
int sys_phys();
int sys_lock();
int sys_ioctl();
int sys_fcntl();
int sys_mpx();
int sys_setpgid();
int sys_ulimit();
int sys_olduname();
int sys_umask();
int sys_ustat();
int sys_dup2();
int sys_getppid();
int sys_getpgrp();
int sys_setsid();
int sys_sigaction();
int sys_siggetmask();
int sys_sigsetmask();
int sys_setreuid();
int sys_setregid();
int sys_sigsuspend();
int sys_sigpending();
int sys_sethostname();
int sys_setrlimit();
int sys_getrlimit();
int sys_getrusage();
int sys_gettimeofday();
int sys_settimeofday();
int sys_getgroups();
int sys_setgroups();
int sys_setgroups32();
int sys_getgroups32();
int sys_oldselect();
int sys_oldlstat();
int sys_readlink();
int sys_uselib();
int sys_swapon();
int sys_reboot();
int sys_readdir();
int sys_mmap();
int sys_munmap();
int sys_truncate();
int sys_ftruncate();
int sys_fchmod();
int sys_fchown();
int sys_getpriority();
int sys_setpriority();
int sys_profil();
int sys_statfs();
int sys_fstatfs();
int sys_ioperm();
int sys_socketcall();
int sys_syslog();
int sys_setitimer();
int sys_getitimer();
int sys_stat();
int sys_lstat();
int sys_fstat();
int sys_uname();
int sys_iopl();
int sys_vhangup();
int sys_idle();
int sys_vm86();
int sys_wait4();
int sys_swapoff();
int sys_ipc();
int sys_sigreturn();
int sys_fsync();
int sys_clone();
int sys_setdomainname();
int sys_sysinfo();
int sys_modify_ldt();
int sys_adjtimex();
int sys_mprotect();
int sys_sigprocmask();
int sys_create_module();
int sys_init_module();
int sys_delete_module();
int sys_get_kernel_syms();
int sys_quotactl();
int sys_getpgid();
int sys_bdflush();
int sys_sysfs();
int sys_personality();
int sys_afs_syscall();
int sys_setfsuid();
int sys_setfsgid();
int sys_llseek();
int sys_getdents();
int sys_flock();
int sys_msync();
int sys_readv();
int sys_writev();
int sys_select();
int sys_getsid();
int sys_fdatasync();
int sys_sysctl();
int sys_mlock();
int sys_munlock();
int sys_mlockall();
int sys_munlockall();
int sys_madvise();
int sys_sched_setparam();
int sys_sched_getparam();
int sys_sched_setscheduler();
int sys_sched_getscheduler();
int sys_sched_yield();
int sys_sched_get_priority_max();
int sys_sched_get_priority_min();
int sys_sched_rr_get_interval();
int sys_nanosleep();
int sys_mremap();
int sys_sendmsg();
int sys_recvmsg();
int sys_setresuid();
int sys_setresgid();
int sys_getresuid();
int sys_getresgid();
int sys_pread();
int sys_pwrite();
int sys_getcwd();
int sys_preadv();
int sys_pwritev();
int sys_sigaltstack();
int sys_rt_sigprocmask();
int sys_rt_sigaction();
int sys_rt_sigpending();
int sys_rt_sigsuspend();
int sys_rt_sigqueueinfo();
int sys_rt_sigtimedwait();
int sys_prctl();
int sys_poll();
int sys_vfork();
int sys_sendfile();
int sys_old_mmap();
int sys_stat64();
int sys_lstat64();
int sys_fstat64();
int sys_truncate64();
int sys_ftruncate64();
int sys_getdents64();
int sys_getpmsg();
int sys_putpmsg();
int sys_readahead();
int sys_sendfile64();
int sys_setxattr();
int sys_fsetxattr();
int sys_getxattr();
int sys_fgetxattr();
int sys_listxattr();
int sys_flistxattr();
int sys_removexattr();
int sys_fremovexattr();
int sys_sched_setaffinity();
int sys_sched_getaffinity();
int sys_futex();
int sys_set_thread_area();
int sys_get_thread_area();
int sys_remap_file_pages();
int sys_timer_create();
int sys_timer_delete();
int sys_timer_getoverrun();
int sys_timer_gettime();
int sys_timer_settime();
int sys_clock_settime();
int sys_clock_gettime();
int sys_clock_getres();
int sys_clock_nanosleep();
int sys_semtimedop();
int sys_statfs64();
int sys_fstatfs64();
int sys_tgkill();
int sys_mq_open();
int sys_mq_timedsend();
int sys_mq_timedreceive();
int sys_mq_notify();
int sys_mq_getsetattr();
int sys_epoll_create();
int sys_epoll_ctl();
int sys_epoll_wait();
int sys_waitid();
int sys_fadvise64();
int sys_fadvise64_64();
int sys_mbind();
int sys_get_mempolicy();
int sys_set_mempolicy();
int sys_move_pages();
int sys_arch_prctl();
int sys_io_setup();
int sys_io_submit();
int sys_io_cancel();
int sys_io_getevents();
int sys_io_destroy();
int sys_utimensat();
int sys_epoll_pwait();
int sys_signalfd();
int sys_timerfd();
int sys_eventfd();
int sys_getcpu();
int sys_fallocate();
int sys_timerfd_create();
int sys_timerfd_settime();
int sys_timerfd_gettime();
int sys_signalfd4();
int sys_eventfd2();
int sys_epoll_create1();
int sys_dup3();
int sys_pipe2();

/* sys_socketcall subcalls */

int sys_socket();
int sys_bind();
int sys_connect();
int sys_listen();
int sys_accept4();
int sys_accept();
int sys_getsockname();
int sys_getpeername();
int sys_socketpair();
int sys_send();
int sys_recv();
int sys_sendto();
int sys_recvfrom();
int sys_shutdown();
int sys_setsockopt();
int sys_getsockopt();
int sys_recvmmsg();

/* *at syscalls */
int sys_fchmodat();
int sys_newfstatat();
int sys_unlinkat();
int sys_fchownat();
int sys_openat();
int sys_renameat();
int sys_symlinkat();
int sys_readlinkat();
int sys_linkat();
int sys_faccessat();
int sys_mkdirat();
int sys_mknodat();
int sys_futimesat();

/* new ones */
int sys_query_module();
int sys_poll();
int sys_mincore();
int sys_inotify_add_watch();
int sys_inotify_rm_watch();
int sys_inotify_init1();
int sys_pselect6();
int sys_ppoll();
int sys_unshare();
int sys_tee();
int sys_splice();
int sys_vmsplice();

/* architecture-specific calls */
#ifdef ALPHA
int sys_osf_select();
int sys_osf_gettimeofday();
int sys_osf_settimeofday();
int sys_osf_getitimer();
int sys_osf_setitimer();
int sys_osf_getrusage();
int sys_osf_wait4();
int sys_osf_utimes();
#endif


#ifndef SYS_waitid
# ifdef I386
#  define SYS_waitid 284
# elif defined ALPHA
#  define SYS_waitid 438
# elif defined ARM
#  define SYS_waitid (NR_SYSCALL_BASE + 280)
# elif defined IA64
#  define SYS_waitid 1270
# elif defined M68K
#  define SYS_waitid 277
# elif defined POWERPC
#  define SYS_waitid 272
# elif defined S390 || defined S390X
#  define SYS_waitid 281
# elif defined SH64
#  define SYS_waitid 312
# elif defined SH64
#  define SYS_waitid 312
# elif defined SH
#  define SYS_waitid 284
# elif defined SPARC || defined SPARC64
#  define SYS_waitid 279
# elif defined X86_64
#  define SYS_waitid 247
# endif
#endif

#if !defined(ALPHA) && !defined(MIPS) && !defined(HPPA) && \
	!defined(__ARM_EABI__)
# ifdef	IA64
/*
 *  IA64 syscall numbers (the only ones available from standard header
 *  files) are disjoint from IA32 syscall numbers.  We need to define
 *  the IA32 socket call number here.
 */
#  define SYS_socketcall	102

#  undef SYS_socket
#  undef SYS_bind
#  undef SYS_connect
#  undef SYS_listen
#  undef SYS_accept
#  undef SYS_getsockname
#  undef SYS_getpeername
#  undef SYS_socketpair
#  undef SYS_send
#  undef SYS_recv
#  undef SYS_sendto
#  undef SYS_recvfrom
#  undef SYS_shutdown
#  undef SYS_setsockopt
#  undef SYS_getsockopt
#  undef SYS_sendmsg
#  undef SYS_recvmsg
# endif /* IA64 */
# if defined(SPARC) || defined(SPARC64)
#  define SYS_socket_subcall	353
# else
#  define SYS_socket_subcall	400
# endif
#define SYS_sub_socket		(SYS_socket_subcall + 1)
#define SYS_sub_bind		(SYS_socket_subcall + 2)
#define SYS_sub_connect		(SYS_socket_subcall + 3)
#define SYS_sub_listen		(SYS_socket_subcall + 4)
#define SYS_sub_accept		(SYS_socket_subcall + 5)
#define SYS_sub_getsockname	(SYS_socket_subcall + 6)
#define SYS_sub_getpeername	(SYS_socket_subcall + 7)
#define SYS_sub_socketpair	(SYS_socket_subcall + 8)
#define SYS_sub_send		(SYS_socket_subcall + 9)
#define SYS_sub_recv		(SYS_socket_subcall + 10)
#define SYS_sub_sendto		(SYS_socket_subcall + 11)
#define SYS_sub_recvfrom	(SYS_socket_subcall + 12)
#define SYS_sub_shutdown	(SYS_socket_subcall + 13)
#define SYS_sub_setsockopt	(SYS_socket_subcall + 14)
#define SYS_sub_getsockopt	(SYS_socket_subcall + 15)
#define SYS_sub_sendmsg		(SYS_socket_subcall + 16)
#define SYS_sub_recvmsg		(SYS_socket_subcall + 17)
#define SYS_sub_accept4		(SYS_socket_subcall + 18)
#define SYS_sub_recvmmsg	(SYS_socket_subcall + 19)

#define SYS_socket_nsubcalls	20
#endif /* !(ALPHA || MIPS || HPPA) */

/* sys_ipc subcalls */

int sys_semget();
int sys_semctl();
int sys_semop();
int sys_msgsnd();
int sys_msgrcv();
int sys_msgget();
int sys_msgctl();
int sys_shmat();
int sys_shmdt();
int sys_shmget();
int sys_shmctl();

#if !defined(ALPHA) && !defined(MIPS) && !defined(HPPA) && \
	!defined(__ARM_EABI__)
# ifdef	IA64
   /*
    * IA64 syscall numbers (the only ones available from standard
    * header files) are disjoint from IA32 syscall numbers.  We need
    * to define the IA32 socket call number here.  Fortunately, this
    * symbol, `SYS_ipc', is not used by any of the IA64 code so
    * re-defining this symbol will not cause a problem.
   */
#  undef SYS_ipc
#  define SYS_ipc		117
#  undef SYS_semop
#  undef SYS_semget
#  undef SYS_semctl
#  undef SYS_semtimedop
#  undef SYS_msgsnd
#  undef SYS_msgrcv
#  undef SYS_msgget
#  undef SYS_msgctl
#  undef SYS_shmat
#  undef SYS_shmdt
#  undef SYS_shmget
#  undef SYS_shmctl
# endif /* IA64 */
#define SYS_ipc_subcall		((SYS_socket_subcall)+(SYS_socket_nsubcalls))
#define SYS_sub_semop		(SYS_ipc_subcall + 1)
#define SYS_sub_semget		(SYS_ipc_subcall + 2)
#define SYS_sub_semctl		(SYS_ipc_subcall + 3)
#define SYS_sub_semtimedop	(SYS_ipc_subcall + 4)
#define SYS_sub_msgsnd		(SYS_ipc_subcall + 11)
#define SYS_sub_msgrcv		(SYS_ipc_subcall + 12)
#define SYS_sub_msgget		(SYS_ipc_subcall + 13)
#define SYS_sub_msgctl		(SYS_ipc_subcall + 14)
#define SYS_sub_shmat		(SYS_ipc_subcall + 21)
#define SYS_sub_shmdt		(SYS_ipc_subcall + 22)
#define SYS_sub_shmget		(SYS_ipc_subcall + 23)
#define SYS_sub_shmctl		(SYS_ipc_subcall + 24)

#define SYS_ipc_nsubcalls	25
#endif /* !(ALPHA || MIPS || HPPA) */

#if defined SYS_ipc_subcall && !defined SYS_ipc
# define SYS_ipc SYS_ipc_subcall
#endif
#if defined SYS_socket_subcall && !defined SYS_socketcall
# define SYS_socketcall SYS_socket_subcall
#endif

#ifdef IA64
  /*
   * IA64 syscall numbers (the only ones available from standard header
   * files) are disjoint from IA32 syscall numbers.  We need to define
   * some IA32 specific syscalls here.
   */
# define SYS_fork	2
# define SYS_vfork	190
# define SYS32_exit	1
# define SYS_waitpid	7
# define SYS32_wait4	114
# define SYS32_execve	11
#endif /* IA64 */

#if defined(ALPHA) || defined(IA64)
int sys_getpagesize();
#endif

#ifdef ALPHA
int osf_statfs();
int osf_fstatfs();
#endif

#ifdef IA64
/* STREAMS stuff */
int sys_getpmsg();
int sys_putpmsg();
#endif

#ifdef MIPS
int sys_sysmips();
#endif

int sys_setpgrp();
int sys_gethostname();
int sys_getdtablesize();
int sys_utimes();
int sys_capget();
int sys_capset();

#if defined M68K || defined SH
int sys_cacheflush();
#endif

int sys_pread64();
int sys_pwrite64();

#ifdef POWERPC
int sys_subpage_prot();
#endif

#ifdef BFIN
int sys_sram_alloc();
int sys_cacheflush();
#endif

#if defined SPARC || defined SPARC64
#include "sparc/syscall1.h"
int sys_execv();
int sys_getpagesize();
int sys_getmsg();
int sys_putmsg();

int sys_semsys();
int sys_semctl();
int sys_semget();
#define SYS_semsys_subcall	200
#define SYS_semsys_nsubcalls	3
#define SYS_semctl		(SYS_semsys_subcall + 0)
#define SYS_semget		(SYS_semsys_subcall + 1)
#define SYS_semop		(SYS_semsys_subcall + 2)
int sys_msgsys();
int sys_msgget();
int sys_msgctl();
int sys_msgrcv();
int sys_msgsnd();
#define SYS_msgsys_subcall	203
#define SYS_msgsys_nsubcalls	4
#define SYS_msgget		(SYS_msgsys_subcall + 0)
#define SYS_msgctl		(SYS_msgsys_subcall + 1)
#define SYS_msgrcv		(SYS_msgsys_subcall + 2)
#define SYS_msgsnd		(SYS_msgsys_subcall + 3)
int sys_shmsys();
int sys_shmat();
int sys_shmctl();
int sys_shmdt();
int sys_shmget();
#define SYS_shmsys_subcall	207
#define SYS_shmsys_nsubcalls	4
#define SYS_shmat		(SYS_shmsys_subcall + 0)
#define SYS_shmctl		(SYS_shmsys_subcall + 1)
#define SYS_shmdt		(SYS_shmsys_subcall + 2)
#define SYS_shmget		(SYS_shmsys_subcall + 3)
#endif
