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

/* common syscalls */

int sys_accept();
int sys_accept4();
int sys_access();
int sys_adjtimex();
int sys_alarm();
int sys_arch_prctl();
int sys_bind();
int sys_brk();
int sys_capget();
int sys_capset();
int sys_chdir();
int sys_chmod();
int sys_chown();
int sys_clock_gettime();
int sys_clock_nanosleep();
int sys_clock_settime();
int sys_clone();
int sys_close();
int sys_connect();
int sys_creat();
int sys_create_module();
int sys_dup2();
int sys_dup3();
int sys_epoll_create();
int sys_epoll_create1();
int sys_epoll_ctl();
int sys_epoll_pwait();
int sys_epoll_wait();
int sys_eventfd();
int sys_eventfd2();
int sys_execve();
int sys_exit();
int sys_faccessat();
int sys_fadvise64();
int sys_fadvise64_64();
int sys_fallocate();
int sys_fchmod();
int sys_fchmodat();
int sys_fchown();
int sys_fchownat();
int sys_fcntl();
int sys_fgetxattr();
int sys_flistxattr();
int sys_flock();
int sys_fork();
int sys_fremovexattr();
int sys_fsetxattr();
int sys_fstat();
int sys_fstat64();
int sys_fstatfs();
int sys_fstatfs64();
int sys_fsync();
int sys_ftruncate();
int sys_ftruncate64();
int sys_futex();
int sys_futimesat();
int sys_get_mempolicy();
int sys_get_thread_area();
int sys_getcpu();
int sys_getcwd();
int sys_getdents();
int sys_getdents64();
int sys_getdtablesize();
int sys_getgroups();
int sys_getgroups32();
int sys_gethostname();
int sys_getitimer();
int sys_getpeername();
int sys_getpmsg();
int sys_getpriority();
int sys_getresgid();
int sys_getresuid();
int sys_getrlimit();
int sys_getrusage();
int sys_getsid();
int sys_getsockname();
int sys_getsockopt();
int sys_gettimeofday();
int sys_getuid();
int sys_getxattr();
int sys_init_module();
int sys_inotify_add_watch();
int sys_inotify_init1();
int sys_inotify_rm_watch();
int sys_io_cancel();
int sys_io_destroy();
int sys_io_getevents();
int sys_io_setup();
int sys_io_submit();
int sys_ioctl();
int sys_kill();
int sys_link();
int sys_linkat();
int sys_listen();
int sys_listxattr();
int sys_llseek();
int sys_lseek();
int sys_lstat();
int sys_lstat64();
int sys_madvise();
int sys_mbind();
int sys_mincore();
int sys_mkdir();
int sys_mkdirat();
int sys_mknod();
int sys_mknodat();
int sys_mlockall();
int sys_mmap();
int sys_modify_ldt();
int sys_mount();
int sys_move_pages();
int sys_mprotect();
int sys_mq_getsetattr();
int sys_mq_notify();
int sys_mq_open();
int sys_mq_timedreceive();
int sys_mq_timedsend();
int sys_mremap();
int sys_msgctl();
int sys_msgget();
int sys_msgrcv();
int sys_msgsnd();
int sys_msync();
int sys_munmap();
int sys_nanosleep();
int sys_newfstatat();
int sys_nice();
int sys_old_mmap();
int sys_oldfstat();
int sys_oldlstat();
int sys_oldselect();
int sys_oldstat();
int sys_open();
int sys_openat();
int sys_personality();
int sys_pipe();
int sys_pipe2();
int sys_poll();
int sys_poll();
int sys_ppoll();
int sys_prctl();
int sys_pread();
int sys_preadv();
int sys_pselect6();
int sys_ptrace();
int sys_process_vm_readv();
int sys_putpmsg();
int sys_pwrite();
int sys_pwritev();
int sys_query_module();
int sys_quotactl();
int sys_read();
int sys_readahead();
int sys_readdir();
int sys_readlink();
int sys_readlinkat();
int sys_readv();
int sys_reboot();
int sys_recv();
int sys_recvfrom();
int sys_recvmmsg();
int sys_recvmsg();
int sys_remap_file_pages();
int sys_removexattr();
int sys_renameat();
int sys_restart_syscall();
int sys_rt_sigaction();
int sys_rt_sigpending();
int sys_rt_sigprocmask();
int sys_rt_sigqueueinfo();
int sys_rt_sigsuspend();
int sys_rt_sigtimedwait();
int sys_sched_get_priority_min();
int sys_sched_getaffinity();
int sys_sched_getparam();
int sys_sched_getscheduler();
int sys_sched_setaffinity();
int sys_sched_setparam();
int sys_sched_setscheduler();
int sys_select();
int sys_semctl();
int sys_semget();
int sys_semop();
int sys_semtimedop();
int sys_send();
int sys_sendfile();
int sys_sendfile64();
int sys_sendmsg();
int sys_sendto();
int sys_set_mempolicy();
int sys_set_thread_area();
int sys_setdomainname();
int sys_setfsuid();
int sys_setgid();
int sys_setgroups();
int sys_setgroups32();
int sys_sethostname();
int sys_setitimer();
int sys_setpgid();
int sys_setpgrp();
int sys_setpriority();
int sys_setregid();
int sys_setresgid();
int sys_setresuid();
int sys_setreuid();
int sys_setrlimit();
int sys_setsockopt();
int sys_settimeofday();
int sys_setuid();
int sys_setxattr();
int sys_shmat();
int sys_shmctl();
int sys_shmdt();
int sys_shmget();
int sys_shutdown();
int sys_sigaction();
int sys_sigaltstack();
int sys_siggetmask();
int sys_signal();
int sys_signalfd();
int sys_signalfd4();
int sys_sigpending();
int sys_sigprocmask();
int sys_sigreturn();
int sys_sigsetmask();
int sys_sigsuspend();
int sys_socket();
int sys_socketpair();
int sys_splice();
int sys_stat();
int sys_stat64();
int sys_statfs();
int sys_statfs64();
int sys_stime();
int sys_swapon();
int sys_symlinkat();
int sys_sysctl();
int sys_sysinfo();
int sys_tee();
int sys_tgkill();
int sys_time();
int sys_timer_create();
int sys_timer_gettime();
int sys_timer_settime();
int sys_timerfd();
int sys_timerfd_create();
int sys_timerfd_gettime();
int sys_timerfd_settime();
int sys_times();
int sys_truncate();
int sys_truncate64();
int sys_umask();
int sys_umount2();
int sys_uname();
int sys_unlinkat();
int sys_unshare();
int sys_utime();
int sys_utimensat();
int sys_utimes();
int sys_vfork();
int sys_vmsplice();
int sys_wait4();
int sys_waitid();
int sys_waitpid();
int sys_write();
int sys_writev();

/* architecture-specific calls */
#ifdef ALPHA
int osf_statfs();
int osf_fstatfs();
int sys_osf_getitimer();
int sys_osf_getrusage();
int sys_osf_gettimeofday();
int sys_osf_select();
int sys_osf_setitimer();
int sys_osf_settimeofday();
int sys_osf_utimes();
int sys_osf_wait4();
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

#define SYS_socket_nsubcalls	20
#endif /* !(ALPHA || MIPS || HPPA) */

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
#endif /* IA64 */

#if defined(ALPHA) || defined(IA64) || defined(SPARC) || defined(SPARC64)
int sys_getpagesize();
#endif

#ifdef IA64
/* STREAMS stuff */
int sys_getpmsg();
int sys_putpmsg();
#endif

#ifdef MIPS
int sys_sysmips();
#endif

#if defined M68K || defined SH
int sys_cacheflush();
#endif

#ifdef POWERPC
int sys_subpage_prot();
#endif

#ifdef BFIN
int sys_cacheflush();
int sys_sram_alloc();
#endif

#if defined SPARC || defined SPARC64
#include "sparc/syscall1.h"
int sys_execv();
int sys_getmsg();
int sys_msgsys();
int sys_putmsg();
int sys_semsys();
int sys_shmsys();
#define SYS_semsys_subcall	200
#define SYS_semsys_nsubcalls	3
#define SYS_semctl		(SYS_semsys_subcall + 0)
#define SYS_semget		(SYS_semsys_subcall + 1)
#define SYS_semop		(SYS_semsys_subcall + 2)
#define SYS_msgsys_subcall	203
#define SYS_msgsys_nsubcalls	4
#define SYS_msgget		(SYS_msgsys_subcall + 0)
#define SYS_msgctl		(SYS_msgsys_subcall + 1)
#define SYS_msgrcv		(SYS_msgsys_subcall + 2)
#define SYS_msgsnd		(SYS_msgsys_subcall + 3)
#define SYS_shmsys_subcall	207
#define SYS_shmsys_nsubcalls	4
#define SYS_shmat		(SYS_shmsys_subcall + 0)
#define SYS_shmctl		(SYS_shmsys_subcall + 1)
#define SYS_shmdt		(SYS_shmsys_subcall + 2)
#define SYS_shmget		(SYS_shmsys_subcall + 3)
#endif
