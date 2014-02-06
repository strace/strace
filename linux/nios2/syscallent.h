/*
 * Copyright (c) 2014 Ezequiel Garcia <ezequiel@vanguardiasur.com.ar>
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

	{ 0,	0,	sys_restart_syscall,	"restart_syscall" }, /* 0 */
	{ 1,	TP,	sys_exit,		"_exit"		}, /* 1 */
	{ 0,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ 3,	TD,	sys_read,		"read"		}, /* 3 */
	{ 3,	TD,	sys_write,		"write"		}, /* 4 */
	{ 3,	TD|TF,	sys_open,		"open"		}, /* 5 */
	{ 1,	TD,	sys_close,		"close"		}, /* 6 */
	{ 3,	TP,	sys_waitpid,		"waitpid"	}, /* 7 */
	{ 2,	TD|TF,	sys_creat,		"creat"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ 1,	0,	sys_time,		"time"		}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 16 */
	{ 0,	TM,	NULL,			NULL		}, /* 17 */
	{ 2,	TF,	NULL,			NULL		}, /* 18 */
	{ 3,	TD,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	NF,	sys_getuid,		"getuid"	}, /* 24 */
	{ 1,	0,	sys_stime,		"stime"		}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 1,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ 2,	TD,	NULL,			NULL		}, /* 28 */
	{ 0,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ 2,	0,	NULL,			NULL		}, /* 31 */
	{ 2,	0,	NULL,			NULL		}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 1,	0,	sys_nice,		"nice"		}, /* 34 */
	{ 0,	0,	NULL,			NULL		}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 38 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 39 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 40 */
	{ 1,	TD,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	TD,	sys_pipe,		"pipe"		}, /* 42 */
	{ 1,	0,	sys_times,		"times"		}, /* 43 */
	{ 0,	0,	NULL,			NULL		}, /* 44 */
	{ 1,	TM,	sys_brk,		"brk"		}, /* 45 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 46 */
	{ 0,	NF,	sys_getgid,		"getgid"	}, /* 47 */
	{ 3,	TS,	NULL,			NULL		}, /* 48 */
	{ 0,	NF,	sys_geteuid,		"geteuid"	}, /* 49 */
	{ 0,	NF,	sys_getegid,		"getegid"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 2,	TF,	sys_umount2,		"umount"	}, /* 52 */
	{ 0,	0,	NULL,			NULL		}, /* 53 */
	{ 3,	TD,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 3,	TD,	sys_fcntl,		"fcntl"		}, /* 55 */
	{ 0,	0,	NULL,			NULL		}, /* 56 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 57 */
	{ 2,	0,	NULL,			NULL		}, /* 58 */
	{ 1,	0,	NULL,			NULL		}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 62 */
	{ 2,	TD,	sys_dup2,		"dup2"		}, /* 63 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 64 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 65 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 66 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 67 */
	{ 0,	TS,	NULL,			NULL		}, /* 68 */
	{ 1,	TS,	NULL,			NULL		}, /* 69 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 70 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 71 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 72 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 73 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 74 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 75 */
	{ 2,	0,	NULL,			NULL		}, /* 76 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 77 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 78 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 79 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 80 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 81 */
	{ 1,	TD,	sys_oldselect,		"oldselect"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ 2,	TF,	NULL,			NULL		}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 2,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 4,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ 3,	TD,	NULL,			NULL		}, /* 89 */
	{ 6,	TD|TM,	sys_mmap,		"old_mmap"	}, /* 90 */
	{ 2,	TM,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	TD,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	TD,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ 3,	TD,	sys_fchown,		"fchown"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ 4,	0,	NULL,			NULL		}, /* 98 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 99 */
	{ 2,	TD,	sys_fstatfs,		"fstatfs"	}, /* 100 */
	{ 3,	0,	NULL,			NULL		}, /* 101 */
	{ 2,	TD,	sys_socketcall,		"socketcall"	}, /* 102 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 103 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 104 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 105 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 106 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 107 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 108 */
	{ 1,	0,	NULL,			NULL		}, /* 109 */
	{ 1,	0,	NULL,			NULL		}, /* 110 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 111 */
	{ 0,	0,	NULL,			NULL		}, /* 112 */
	{ 1,	0,	sys_nios2cmpxchg,	"nios2_cmpxchg"	}, /* 113 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 114 */
	{ 1,	TF,	sys_swapoff,		"swapoff"	}, /* 115 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 116 */
	{ 6,	TI,	sys_ipc,		"ipc"		}, /* 117 */
	{ 1,	TD,	sys_fsync,		"fsync"		}, /* 118 */
	{ 0,	TS,	sys_sigreturn,		"sigreturn"	}, /* 119 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 120 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 121 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 122 */
	{ 3,	0,	sys_cacheflush,		"cacheflush"	}, /* 123 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 124 */
	{ 3,	TM,	sys_mprotect,		"mprotect"	}, /* 125 */
	{ 3,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 126 */
	{ 2,	0,	NULL,			NULL	}, /* 127 */
	{ 3,	0,	sys_init_module,	"init_module"	}, /* 128 */
	{ 2,	0,	sys_delete_module,	"delete_module"	}, /* 129 */
	{ 1,	0,	NULL,			NULL		}, /* 130 */
	{ 4,	TF,	sys_quotactl,		"quotactl"	}, /* 131 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 132 */
	{ 1,	TD,	sys_fchdir,		"fchdir"	}, /* 133 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 134 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 135 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 136 */
	{ 5,	0,	NULL,			NULL		}, /* 137 */
	{ 1,	NF,	sys_setfsuid,		"setfsuid"	}, /* 138 */
	{ 1,	NF,	sys_setfsgid,		"setfsgid"	}, /* 139 */
	{ 5,	TD,	sys_llseek,		"_llseek"	}, /* 140 */
	{ 3,	TD,	sys_getdents,		"getdents"	}, /* 141 */
	{ 5,	TD,	sys_select,		"select"	}, /* 142 */
	{ 2,	TD,	sys_flock,		"flock"		}, /* 143 */
	{ 3,	TM,	sys_msync,		"msync"		}, /* 144 */
	{ 3,	TD,	sys_readv,		"readv"		}, /* 145 */
	{ 3,	TD,	sys_writev,		"writev"	}, /* 146 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 147 */
	{ 1,	TD,	sys_fdatasync,		"fdatasync"	}, /* 148 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 149 */
	{ 2,	TM,	sys_mlock,		"mlock"		}, /* 150 */
	{ 2,	TM,	sys_munlock,		"munlock"	}, /* 151 */
	{ 1,	TM,	sys_mlockall,		"mlockall"	}, /* 152 */
	{ 0,	TM,	sys_munlockall,		"munlockall"	}, /* 153 */
	{ 0,	0,	sys_sched_setparam,	"sched_setparam"}, /* 154 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 155 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 156 */
	{ 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 157 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"}, /* 158 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 159 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 160 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 161 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 162 */
	{ 5,	TM,	sys_mremap,		"mremap"	}, /* 163 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 164 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 165 */
	{ 5,	0,	sys_getpagesize,	"getpagesize"	}, /* 166 */
	{ 5,	0,	NULL,			NULL		}, /* 167 */
	{ 3,	TD,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	NULL,			NULL		}, /* 169 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 170 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 171 */
	{ 5,	0,	sys_prctl,		"prctl"		}, /* 172 */
	{ 0,	TS,	sys_rt_sigreturn,	"rt_sigreturn"	}, /* 173 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 174 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 175 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 176 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 177 */
	{ 3,	TS,	sys_rt_sigqueueinfo,    "rt_sigqueueinfo"}, /* 178 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 179 */
	{ 5,	TD,	sys_pread,		"pread64"	}, /* 180 */
	{ 5,	TD,	sys_pwrite,		"pwrite64"	}, /* 181 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 182 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TD|TN,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	NULL,			NULL		}, /* 188 */
	{ 5,	0,	NULL,			NULL		}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork"		}, /* 190 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 191 */
	{ 6,	TD|TM,	sys_mmap_pgoff,		"mmap2"		}, /* 192 */
	{ 3,	TF,	sys_truncate64,		"truncate64"	}, /* 193 */
	{ 3,	TD,	sys_ftruncate64,	"ftruncate64"	}, /* 194 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 195 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 196 */
	{ 2,	TD,	sys_fstat64,		"fstat64"	}, /* 197 */
	{ 3,	TF,	sys_chown,		"lchown32"	}, /* 198 */
	{ 0,	NF,	sys_getuid,		"getuid32"	}, /* 199 */
	{ 0,	NF,	sys_getgid,		"getgid32"	}, /* 200 */
	{ 0,	NF,	sys_geteuid,		"geteuid32"	}, /* 201 */
	{ 0,	NF,	sys_getegid,		"getegid32"	}, /* 202 */
	{ 2,	0,	sys_setreuid,		"setreuid32"	}, /* 203 */
	{ 2,	0,	sys_setregid,		"setregid32"	}, /* 204 */
	{ 2,	0,	sys_getgroups32,	"getgroups32"	}, /* 205 */
	{ 2,	0,	sys_setgroups32,	"setgroups32"	}, /* 206 */
	{ 3,	TD,	sys_fchown,		"fchown32"	}, /* 207 */
	{ 3,	0,	sys_setresuid,		"setresuid32"	}, /* 208 */
	{ 3,	0,	sys_getresuid,		"getresuid32"	}, /* 209 */
	{ 3,	0,	sys_setresgid,		"setresgid32"	}, /* 210 */
	{ 3,	0,	sys_getresgid,		"getresgid32"	}, /* 211 */
	{ 3,	TF,	sys_chown,		"chown32"	}, /* 212 */
	{ 1,	0,	sys_setuid,		"setuid32"	}, /* 213 */
	{ 1,	0,	sys_setgid,		"setgid32"	}, /* 214 */
	{ 1,	NF,	sys_setfsuid,		"setfsuid32"	}, /* 215 */
	{ 1,	NF,	sys_setfsgid,		"setfsgid32"	}, /* 216 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 217 */
	{ 3,	TM,	NULL,			NULL		}, /* 218 */
	{ 3,	TM,	NULL,			NULL		}, /* 219 */
	{ 3,	TD,	sys_getdents64,		"getdents64"	}, /* 220 */
	{ 3,	TD,	sys_gettid,		"gettid"	}, /* 221 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 222 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 223 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 224 */
	{ 5,	TD,	sys_fsetxattr,		"fsetxattr"	}, /* 225 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 226 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 237 */
	{ 4,	TD,	sys_fgetxattr,		"fgetxattr"	}, /* 228 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 229 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 230 */
	{ 3,	TD,	sys_flistxattr,		"flistxattr"	}, /* 231 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 232 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 233 */
	{ 2,	TD,	sys_fremovexattr,	"fremovexattr"	}, /* 234 */
	{ 6,	0,	sys_futex,		"futex"		}, /* 235 */
	{ 4,	TD|TN,	sys_sendfile64,		"sendfile64"	}, /* 236 */
	{ 3,	TM,	sys_mincore,		"mincore"	}, /* 237 */
	{ 3,	TM,	sys_madvise,		"madvise"	}, /* 238 */
	{ 3,	TD,	sys_fcntl,		"fcntl64"	}, /* 239 */
	{ 4,	TD,	sys_readahead,		"readahead"	}, /* 240 */
	{ 2,	0,	sys_io_setup,		"io_setup"	}, /* 241 */
	{ 1,	0,	sys_io_destroy,		"io_destroy"	}, /* 242 */
	{ 5,	0,	sys_io_getevents,	"io_getevents"	}, /* 243 */
	{ 3,	0,	sys_io_submit,		"io_submit"	}, /* 244 */
	{ 3,	0,	sys_io_cancel,		"io_cancel"	}, /* 245 */
	{ 5,	TD,	sys_fadvise64,		"fadvise64"	}, /* 246 */
	{ 1,	TP,	sys_exit,		"exit_group"	}, /* 247 */
	{ 4,	0,	sys_lookup_dcookie,	"lookup_dcookie"}, /* 248 */
	{ 1,	TD,	sys_epoll_create,	"epoll_create"	}, /* 249 */
	{ 4,	TD,	sys_epoll_ctl,		"epoll_ctl"	}, /* 250 */
	{ 4,	TD,	sys_epoll_wait,		"epoll_wait"	}, /* 251 */
	{ 5,	TM,	sys_remap_file_pages,	"remap_file_pages"}, /* 252 */
	{ 1,	0,	sys_set_tid_address,	"set_tid_address"}, /* 253 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 254 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 255 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 256 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 257 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 258 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 259 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 260 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 261 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 262 */
	{ 3,	TF,	sys_statfs64,		"statfs64"	}, /* 263 */
	{ 3,	TD,	sys_fstatfs64,		"fstatfs64"	}, /* 264 */
	{ 3,	TS,	sys_tgkill,		"tgkill"	}, /* 265 */
	{ 2,	TF,	sys_utimes,		"utimes"	}, /* 266 */
	{ 6,	TD,	sys_fadvise64_64,	"fadvise64_64"	}, /* 267 */
	{ 6,	TM,	sys_mbind,		"mbind"		}, /* 268 */
	{ 5,	TM,	sys_get_mempolicy,	"get_mempolicy"	}, /* 269 */
	{ 3,	TM,	sys_set_mempolicy,	"set_mempolicy"	}, /* 270 */
	{ 4,	0,	sys_mq_open,		"mq_open"	}, /* 271 */
	{ 1,	0,	sys_mq_unlink,		"mq_unlink"	}, /* 272 */
	{ 5,	0,	sys_mq_timedsend,	"mq_timedsend"	}, /* 273 */
	{ 5,	0,	sys_mq_timedreceive,	"mq_timedreceive" }, /* 274 */
	{ 2,	0,	sys_mq_notify,		"mq_notify"	}, /* 275 */
	{ 3,	0,	sys_mq_getsetattr,	"mq_getsetattr"	}, /* 276 */
	{ 5,	TP,	sys_waitid,		"waitid"	}, /* 277 */
	{ 6,	0,	NULL,			NULL		}, /* 278 */
	{ 5,	0,	sys_add_key,		"add_key"	}, /* 279 */
	{ 4,	0,	sys_request_key,	"request_key"	}, /* 280 */
	{ 5,	0,	sys_keyctl,		"keyctl"	}, /* 281 */
	{ 3,	0,	sys_ioprio_set,		"ioprio_set"	}, /* 282 */
	{ 2,	0,	sys_ioprio_get,		"ioprio_get"	}, /* 283 */
	{ 0,	TD,	sys_inotify_init,	"inotify_init"	}, /* 284 */
	{ 3,	TD,	sys_inotify_add_watch,	"inotify_add_watch" }, /* 285 */
	{ 2,	TD,	sys_inotify_rm_watch,	"inotify_rm_watch" }, /* 286 */
	{ 4,	TM,	sys_migrate_pages,	"migrate_pages"	}, /* 287 */
	{ 4,	TD|TF,	sys_openat,		"openat"	}, /* 288 */
	{ 3,	TD|TF,	sys_mkdirat,		"mkdirat"	}, /* 289 */
	{ 4,	TD|TF,	sys_mknodat,		"mknodat"	}, /* 290 */
	{ 5,	TD|TF,	sys_fchownat,		"fchownat"	}, /* 291 */
	{ 3,	TD|TF,	sys_futimesat,		"futimesat"	}, /* 292 */
	{ 4,	TD|TF,	sys_newfstatat,		"fstatat64"	}, /* 293 */
	{ 3,	TD|TF,	sys_unlinkat,		"unlinkat"	}, /* 294 */
	{ 4,	TD|TF,	sys_renameat,		"renameat"	}, /* 295 */
	{ 5,	TD|TF,	sys_linkat,		"linkat"	}, /* 296 */
	{ 3,	TD|TF,	sys_symlinkat,		"symlinkat"	}, /* 297 */
	{ 4,	TD|TF,	sys_readlinkat,		"readlinkat"	}, /* 298 */
	{ 3,	TD|TF,	sys_fchmodat,		"fchmodat"	}, /* 299 */
	{ 3,	TD|TF,	sys_faccessat,		"faccessat"	}, /* 300 */
	{ 6,	TD,	sys_pselect6,		"pselect6"	}, /* 301 */
	{ 5,	TD,	sys_ppoll,		"ppoll"		}, /* 302 */
	{ 1,	TP,	sys_unshare,		"unshare"	}, /* 303 */
	{ 2,	0,	sys_set_robust_list,	"set_robust_list" }, /* 304 */
	{ 3,	0,	sys_get_robust_list,	"get_robust_list" }, /* 305 */
	{ 6,	TD,	sys_splice,		"splice"	}, /* 306 */
	{ 6,	TD,	sys_sync_file_range,	"sync_file_range" }, /* 307 */
	{ 4,	TD,	sys_tee,		"tee"		}, /* 308 */
	{ 4,	TD,	sys_vmsplice,		"vmsplice"	}, /* 309 */
	{ 6,	TM,	sys_move_pages,		"move_pages"	}, /* 310 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity" },/* 311 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity" },/* 312 */
	{ 4,	0,	sys_kexec_load,		"kexec_load"	}, /* 313 */
	{ 3,	0,	sys_getcpu,		"getcpu"	}, /* 314 */
	{ 6,	TD,	sys_epoll_pwait,	"epoll_pwait"	}, /* 315 */
	{ 4,	TD|TF,	sys_utimensat,		"utimensat"	}, /* 316 */
	{ 3,	TD|TS,	sys_signalfd,		"signalfd"	}, /* 317 */
	{ 2,	TD,	sys_timerfd_create,	"timerfd_create"}, /* 318 */
	{ 1,	TD,	sys_eventfd,		"eventfd"	}, /* 319 */
	{ 5,	TD,	sys_pread,		"pread64"	}, /* 320 */
	{ 5,	TD,	sys_pwrite,		"pwrite64"	}, /* 321 */
	{ 6,	TD,	sys_fallocate,		"fallocate"	}, /* 322 */
	{ 4,	TD,	sys_timerfd_settime,	"timerfd_settime"}, /* 323 */
	{ 2,	TD,	sys_timerfd_gettime,	"timerfd_gettime"}, /* 324 */
	{ 4,	TD|TS,	sys_signalfd4,		"signalfd4"	}, /* 325 */
	{ 2,	TD,	sys_eventfd2,		"eventfd2"	}, /* 326 */
	{ 1,	TD,	sys_epoll_create1,	"epoll_create1"	}, /* 327 */
	{ 3,	TD,	sys_dup3,		"dup3"		}, /* 328 */
	{ 2,	TD,	sys_pipe2,		"pipe2"		}, /* 329 */
	{ 1,	TD,	sys_inotify_init1,	"inotify_init1"	}, /* 330 */
	{ 5,	TD,	sys_preadv,		"preadv"	}, /* 331 */
	{ 5,	TD,	sys_pwritev,		"pwritev"	}, /* 332 */
	{ 4,	TP|TS,	sys_rt_tgsigqueueinfo,	"rt_tgsigqueueinfo"}, /* 333 */
	{ 5,	TD,	sys_perf_event_open,	"perf_event_open"}, /* 334 */
	{ 5,	TN,	sys_recvmmsg,		"recvmmsg"	}, /* 335 */
	{ 2,	TD,	sys_fanotify_init,	"fanotify_init"	}, /* 336 */
	{ 5,	TD|TF,	sys_fanotify_mark,	"fanotify_mark"	}, /* 337 */
	{ 4,	0,	sys_prlimit64,		"prlimit64"	}, /* 338 */
	{ 5,	TD|TF,	sys_name_to_handle_at,	"name_to_handle_at"}, /* 339 */
	{ 3,	TD,	sys_open_by_handle_at,	"open_by_handle_at"}, /* 340 */
	{ 2,	0,	sys_clock_adjtime,	"clock_adjtime"	}, /* 341 */
	{ 1,	TD,	sys_syncfs,		"syncfs"	}, /* 342 */
