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

	{ 0,	0,	sys_setup,		"setup"		}, /* 0 */
	{ 1,	TP,	sys_exit,		"_exit"		}, /* 1 */
	{ 0,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ 3,	0,	sys_read,		"read"		}, /* 3 */
	{ 3,	0,	sys_write,		"write"		}, /* 4 */
	{ 3,	TF,	sys_open,		"open"		}, /* 5 */
	{ 1,	0,	sys_close,		"close"		}, /* 6 */
	{ 3,	TP,	sys_waitpid,		"waitpid"	}, /* 7 */
	{ 2,	TF,	sys_creat,		"creat"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ 1,	0,	sys_time,		"time"		}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
#ifdef M68K
	{ 3,	TF,	sys_chown,		"chown"		}, /* 16 */
#else
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 16 */
#endif
	{ 0,	0,	sys_break,		"break"		}, /* 17 */
	{ 2,	TF,	sys_oldstat,		"oldstat"	}, /* 18 */
	{ 3,	0,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	0,	sys_getuid,		"getuid"	}, /* 24 */
	{ 1,	0,	sys_stime,		"stime"		}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 1,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ 2,	0,	sys_oldfstat,		"oldfstat"	}, /* 28 */
	{ 0,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ 2,	0,	sys_stty,		"stty"		}, /* 31 */
	{ 2,	0,	sys_gtty,		"gtty"		}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 1,	0,	sys_nice,		"nice"		}, /* 34 */
	{ 0,	0,	sys_ftime,		"ftime"		}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 38 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 39 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 40 */
	{ 1,	0,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	0,	sys_pipe,		"pipe"		}, /* 42 */
	{ 1,	0,	sys_times,		"times"		}, /* 43 */
	{ 0,	0,	sys_prof,		"prof"		}, /* 44 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 45 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 46 */
	{ 0,	0,	sys_getgid,		"getgid"	}, /* 47 */
	{ 3,	TS,	sys_signal,		"signal"	}, /* 48 */
	{ 0,	0,	sys_geteuid,		"geteuid"	}, /* 49 */
	{ 0,	0,	sys_getegid,		"getegid"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 2,	TF,	sys_umount2,		"umount"	}, /* 52 */
	{ 0,	0,	sys_lock,		"lock"		}, /* 53 */
	{ 3,	0,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 3,	0,	sys_fcntl,		"fcntl"		}, /* 55 */
	{ 0,	0,	sys_mpx,		"mpx"		}, /* 56 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 57 */
	{ 2,	0,	sys_ulimit,		"ulimit"	}, /* 58 */
	{ 1,	0,	sys_oldolduname,	"oldolduname"	}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 62 */
	{ 2,	0,	sys_dup2,		"dup2"		}, /* 63 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 64 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 65 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 66 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 67 */
	{ 0,	TS,	sys_siggetmask,		"siggetmask"	}, /* 68 */
	{ 1,	TS,	sys_sigsetmask,		"sigsetmask"	}, /* 69 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 70 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 71 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 72 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 73 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 74 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 75 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 76 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 77 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 78 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 79 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 80 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 81 */
	{ 1,	0,	sys_oldselect,		"oldselect"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ 2,	TF,	sys_oldlstat,		"oldlstat"	}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 1,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ 3,	0,	sys_readdir,		"readdir"	}, /* 89 */
	{ 6,	0,	sys_old_mmap,		"old_mmap"	}, /* 90 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	0,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	0,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ 3,	0,	sys_fchown,		"fchown"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ 4,	0,	sys_profil,		"profil"	}, /* 98 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 99 */
	{ 2,	0,	sys_fstatfs,		"fstatfs"	}, /* 100 */
	{ 3,	0,	sys_ioperm,		"ioperm"	}, /* 101 */
	{ 2,	0,	sys_socketcall,		"socketcall"	}, /* 102 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 103 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 104 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 105 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 106 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 107 */
	{ 2,	0,	sys_fstat,		"fstat"		}, /* 108 */
	{ 1,	0,	sys_olduname,		"olduname"	}, /* 109 */
	{ 1,	0,	sys_iopl,		"iopl"		}, /* 110 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 111 */
	{ 0,	0,	sys_idle,		"idle"		}, /* 112 */
	{ 1,	0,	sys_vm86old,		"vm86old"	}, /* 113 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 114 */
	{ 1,	0,	sys_swapoff,		"swapoff"	}, /* 115 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 116 */
	{ 5,	0,	sys_ipc,		"ipc"		}, /* 117 */
	{ 1,	0,	sys_fsync,		"fsync"		}, /* 118 */
	{ 1,	TS,	sys_sigreturn,		"sigreturn"	}, /* 119 */
	{ 2,	TP,	sys_clone,		"clone"		}, /* 120 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 121 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 122 */
#ifdef M68K
	{ 4,	0,	sys_cacheflush,		"cacheflush"	}, /* 123 */
#else
	{ 3,	0,	sys_modify_ldt,		"modify_ldt"	}, /* 123 */
#endif
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 124 */
	{ 3,	0,	sys_mprotect,		"mprotect"	}, /* 125 */
	{ 3,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 126 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 127 */
	{ 2,	0,	sys_init_module,	"init_module"	}, /* 128 */
	{ 1,	0,	sys_delete_module,	"delete_module"	}, /* 129 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 130 */
	{ 4,	0,	sys_quotactl,		"quotactl"	}, /* 131 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 132 */
	{ 1,	0,	sys_fchdir,		"fchdir"	}, /* 133 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 134 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 135 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 136 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 137 */
	{ 1,	0,	sys_setfsuid,		"setfsuid"	}, /* 138 */
	{ 1,	0,	sys_setfsgid,		"setfsgid"	}, /* 139 */
	{ 5,	0,	sys_llseek,		"_llseek"	}, /* 140 */
	{ 3,	0,	sys_getdents,		"getdents"	}, /* 141 */
	{ 5,	0,	sys_select,		"select"	}, /* 142 */
	{ 2,	0,	sys_flock,		"flock"		}, /* 143 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 144 */
	{ 3,	0,	sys_readv,		"readv"		}, /* 145 */
	{ 3,	0,	sys_writev,		"writev"	}, /* 146 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 147 */
	{ 1,	0,	sys_fdatasync,		"fdatasync"	}, /* 148 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 149 */
	{ 1,	0,	sys_mlock,		"mlock"		}, /* 150 */
	{ 2,	0,	sys_munlock,		"munlock"	}, /* 151 */
	{ 2,	0,	sys_mlockall,		"mlockall"	}, /* 152 */
	{ 0,	0,	sys_munlockall,		"munlockall"	}, /* 153 */
	{ 0,	0,	sys_sched_setparam,	"sched_setparam"}, /* 154 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 155 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 156 */
	{ 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 157 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"}, /* 158 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 159 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 160 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 161 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 162 */
	{ 4,	0,	sys_mremap,		"mremap"	}, /* 163 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 164 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 165 */
#ifdef M68K
	{ 5,	0,	printargs,		"SYS_166"	}, /* 166 */
#else
	{ 5,	0,	printargs,		"vm86"		}, /* 166 */
#endif
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 167 */
	{ 3,	0,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	printargs,		"nfsservctl"	}, /* 169 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 170 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 171 */
	{ 5,	0,	printargs,		"prctl"		}, /* 172 */
	{ 1,	TS,	printargs,		"rt_sigreturn"	}, /* 173 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"  }, /* 174 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 175 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 176 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 177 */
	{ 3,	TS,	sys_rt_sigqueueinfo,    "rt_sigqueueinfo"}, /* 178 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 179 */

	{ 5,	TF,	sys_pread,		"pread"		}, /* 180 */
	{ 5,	TF,	sys_pwrite,		"pwrite"	}, /* 181 */
#ifdef M68K
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 182 */
#else
	{ 3,	TF,	sys_chown,		"chown"		}, /* 182 */
#endif
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TF,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ 5,	0,	printargs,		"SYS_189"	}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork"		}, /* 190 */
	{ 2,	0,	printargs,		"getrlimit"	}, /* 191 */
	{ 6,	0,	sys_mmap,		"mmap2"		}, /* 192 */
	{ 2,	TF,	printargs,		"truncate64"	}, /* 193 */
	{ 2,	TF,	printargs,		"ftruncate64"	}, /* 194 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 195 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 196 */
	{ 2,	TF,	sys_fstat64,		"fstat64"	}, /* 197 */
/*TODO*/{ 3,	TF,	printargs,		"lchown32"	}, /* 198 */
/*TODO*/{ 0,	0,	printargs,		"getuid32"	}, /* 199 */

	{ 0,	0,	printargs,		"getgid32"	}, /* 200 */
	{ 0,	0,	printargs,		"geteuid32"	}, /* 201 */
	{ 0,	0,	printargs,		"getegid32"	}, /* 202 */
	{ 2,	0,	printargs,		"setreuid32"	}, /* 203 */
	{ 2,	0,	printargs,		"setregid32"	}, /* 204 */
	{ 2,	0,	printargs,		"getgroups32"	}, /* 205 */
	{ 2,	0,	printargs,		"setgroups32"	}, /* 206 */
	{ 3,	0,	printargs,		"fchown32"	}, /* 207 */
	{ 3,	0,	printargs,		"setresuid32"	}, /* 208 */
	{ 3,	0,	printargs,		"getresuid32"	}, /* 209 */
	{ 3,	0,	printargs,		"setresgid32"	}, /* 210 */
	{ 3,	0,	printargs,		"getsetgid32"	}, /* 211 */
	{ 3,	TF,	printargs,		"chown32"	}, /* 212 */
	{ 1,	0,	printargs,		"setuid32"	}, /* 213 */
	{ 1,	0,	printargs,		"setgid32"	}, /* 214 */
	{ 1,	0,	printargs,		"setfsuid32"	}, /* 215 */
	{ 1,	0,	printargs,		"setfsgid32"	}, /* 216 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 217 */
	{ 3,	0,	printargs,		"mincore"	}, /* 218 */
	{ 3,	0,	sys_madvise,		"madvise"	}, /* 219 */
	{ 4,	0,	printargs,		"getdents64"	}, /* 220 */
	{ 3,	0,	sys_fcntl,		"fcntl64"	}, /* 221 */
	{ 4,	0,	printargs,		"SYS_222"	}, /* 222 */
	{ 4,	0,	printargs,		"SYS_223"	}, /* 223 */
	{ 4,	0,	printargs,		"SYS_224"	}, /* 224 */
	{ 5,	0,	printargs,		"SYS_225"	}, /* 225 */
	{ 5,	0,	printargs,		"SYS_226"	}, /* 226 */
	{ 5,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ 5,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ 5,	0,	printargs,		"SYS_229"	}, /* 229 */

	{ 8,	0,	printargs,		"socket_subcall"}, /* 230 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 231 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 232 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 233 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 234 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 235 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 236 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 237 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 238 */
	{ 4,	TN,	sys_send,		"send"		}, /* 239 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 240 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 241 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 242 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 243 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 244 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 245 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 246 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 247 */
	{ 5,	0,	printargs,		"SYS_248"	}, /* 248 */
	{ 5,	0,	printargs,		"SYS_249"	}, /* 249 */

	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 250 */
	{ 4,	TI,	sys_semop,		"semop"		}, /* 251 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 252 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 253 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 254 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 255 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 256 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 257 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 258 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 259 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 260 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 261 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 262 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 263 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 264 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 265 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 266 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 267 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 268 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 269 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 270 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 271 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 272 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 273 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 274 */


