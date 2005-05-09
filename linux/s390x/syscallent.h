/*
 * Copyright (c) 2000 IBM Deutschland Entwicklung GmbH, IBM Coporation
 * Author: Ulrich Weigand <Ulrich.Weigand@de.ibm.com>
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
 */

	{ 0,	0,	sys_setup,		"setup"		}, /* 0 */
	{ 1,	TP,	sys_exit,		"_exit"		}, /* 1 */
	{ 0,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ 3,	0,	sys_read,		"read"		}, /* 3 */
	{ 3,	0,	sys_write,		"write"		}, /* 4 */
	{ 3,	TF,	sys_open,		"open"		}, /* 5 */
	{ 1,	0,	sys_close,		"close"		}, /* 6 */
	{ -1,	0,	printargs,		"SYS_7"		}, /* 7 */
	{ 2,	TF,	sys_creat,		"creat"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ -1,	0,	printargs,		"SYS_13"	}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ -1,	0,	printargs,		"SYS_16"	}, /* 16 */
	{ -1,	0,	printargs,		"SYS_17"	}, /* 17 */
	{ -1,	0,	printargs,		"SYS_18"	}, /* 18 */
	{ 3,	0,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ -1,	0,	printargs,		"SYS_23"	}, /* 23 */
	{ -1,	0,	printargs,		"SYS_24"	}, /* 24 */
	{ -1,	0,	printargs,		"SYS_25"	}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 1,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ -1,	0,	printargs,		"SYS_28"	}, /* 28 */
	{ 0,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ -1,	0,	printargs,		"SYS_31"	}, /* 31 */
	{ -1,	0,	printargs,		"SYS_32"	}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 1,	0,	sys_nice,		"nice"		}, /* 34 */
	{ -1,	0,	printargs,		"SYS_35"	}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 38 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 39 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 40 */
	{ 1,	0,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	0,	sys_pipe,		"pipe"		}, /* 42 */
	{ 1,	0,	sys_times,		"times"		}, /* 43 */
	{ -1,	0,	printargs,		"SYS_44"	}, /* 44 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 45 */
	{ -1,	0,	printargs,		"SYS_46"	}, /* 46 */
	{ -1,	0,	printargs,		"SYS_47"	}, /* 47 */
	{ 3,	TS,	sys_signal,		"signal"	}, /* 48 */
	{ -1,	0,	printargs,		"SYS_49"	}, /* 49 */
	{ -1,	0,	printargs,		"SYS_50"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 2,	TF,	sys_umount2,		"umount"	}, /* 52 */
	{ -1,	0,	printargs,		"SYS_53"	}, /* 53 */
	{ 3,	0,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 3,	0,	sys_fcntl,		"fcntl"		}, /* 55 */
	{ -1,	0,	printargs,		"SYS_56"	}, /* 56 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 57 */
	{ -1,	0,	printargs,		"SYS_58"	}, /* 58 */
	{ -1,	0,	printargs,		"SYS_59"	}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 62 */
	{ 2,	0,	sys_dup2,		"dup2"		}, /* 63 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 64 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 65 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 66 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 67 */
	{ -1,	0,	printargs,		"SYS_68"	}, /* 68 */
	{ -1,	0,	printargs,		"SYS_69"	}, /* 69 */
	{ -1,	0,	printargs,		"SYS_70"	}, /* 70 */
	{ -1,	0,	printargs,		"SYS_71"	}, /* 71 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 72 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 73 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 74 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 75 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 76 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 77 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 78 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 79 */
	{ -1,	0,	printargs,		"SYS_80"	}, /* 80 */
	{ -1,	0,	printargs,		"SYS_81"	}, /* 81 */
	{ -1,	0,	printargs,		"SYS_82"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ -1,	0,	printargs,		"SYS_84"	}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 1,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ -1,	0,	printargs,		"SYS_89"	}, /* 89 */
	{ 6,	0,	sys_old_mmap,		"mmap"		}, /* 90 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	0,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	0,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ -1,	0,	printargs,		"SYS_95"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ -1,	0,	printargs,		"SYS_98"	}, /* 98 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 99 */
	{ 2,	0,	sys_fstatfs,		"fstatfs"	}, /* 100 */
	{ -1,	0,	printargs,		"SYS_101"	}, /* 101 */
	{ 2,	0,	sys_socketcall,		"socketcall"	}, /* 102 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 103 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 104 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 105 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 106 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 107 */
	{ 2,	0,	sys_fstat,		"fstat"		}, /* 108 */
	{ -1,	0,	printargs,		"SYS_109"	}, /* 109 */
	{ -1,	0,	printargs,		"SYS_110"	}, /* 110 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 111 */
	{ 0,	0,	sys_idle,		"idle"		}, /* 112 */
	{ -1,	0,	printargs,		"SYS_113"	}, /* 113 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 114 */
	{ 1,	0,	sys_swapoff,		"swapoff"	}, /* 115 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 116 */
	{ 5,	0,	sys_ipc,		"ipc"		}, /* 117 */
	{ 1,	0,	sys_fsync,		"fsync"		}, /* 118 */
	{ 1,	TS,	sys_sigreturn,		"sigreturn"	}, /* 119 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 120 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 121 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 122 */
	{ -1,	0,	printargs,		"SYS_123"	}, /* 123 */
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
	{ -1,	0,	printargs,		"SYS_138"	}, /* 138 */
	{ -1,	0,	printargs,		"SYS_139"	}, /* 139 */
	{ -1,	0,	printargs,		"SYS_140"	}, /* 140 */
	{ 3,	0,	sys_getdents,		"getdents"	}, /* 141 */
	{ 5,	0,	sys_select,		"select"	}, /* 142 */
	{ 2,	0,	sys_flock,		"flock"		}, /* 143 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 144 */
	{ 3,	0,	sys_readv,		"readv"		}, /* 145 */
	{ 3,	0,	sys_writev,		"writev"	}, /* 146 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 147 */
	{ 1,	0,	sys_fdatasync,		"fdatasync"	}, /* 148 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 149 */
	{ 2,	0,	sys_mlock,		"mlock"		}, /* 150 */
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
	{ -1,	0,	printargs,		"SYS_164"	}, /* 164 */
	{ -1,	0,	printargs,		"SYS_165"	}, /* 165 */
	{ -1,	0,	printargs,		"SYS_166"	}, /* 166 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 167 */
	{ 3,	0,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	printargs,		"nfsservctl"	}, /* 169 */
	{ -1,	0,	printargs,		"SYS_170"	}, /* 170 */
	{ -1,	0,	printargs,		"SYS_171"	}, /* 171 */
	{ 5,	0,	printargs,		"prctl"		}, /* 172 */
	{ 1,	TS,	sys_sigreturn,		"rt_sigreturn"	}, /* 173 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 174 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 175 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 176 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 177 */
	{ 3,	TS,	sys_rt_sigqueueinfo,	"rt_sigqueueinfo"}, /* 178 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 179 */
	{ 5,	TF,	sys_pread,		"pread"		}, /* 180 */
	{ 5,	TF,	sys_pwrite,		"pwrite"	}, /* 181 */
	{ -1,	0,	printargs,		"SYS_182"	}, /* 182 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TF,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	sys_getpmsg,		"getpmsg"	}, /* 188 */
	{ 5,	0,	sys_putpmsg,		"putpmsg"	}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork"		}, /* 190 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 191 */
 	{ -1,	0,	printargs,		"SYS_192"	}, /* 192 */
 	{ -1,	0,	printargs,		"SYS_193"	}, /* 193 */
 	{ -1,	0,	printargs,		"SYS_194"	}, /* 194 */
 	{ -1,	0,	printargs,		"SYS_195"	}, /* 195 */
 	{ -1,	0,	printargs,		"SYS_196"	}, /* 196 */
 	{ -1,	0,	printargs,		"SYS_197"	}, /* 197 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 198 */
	{ 0,	0,	sys_getuid,		"getuid"	}, /* 199 */
	{ 0,	0,	sys_getgid,		"getgid"	}, /* 200 */
	{ 0,	0,	sys_geteuid,		"geteuid"	}, /* 201 */
	{ 0,	0,	sys_getegid,		"getegid"	}, /* 202 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 203 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 204 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 205 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 206 */
	{ 3,	0,	sys_fchown,		"fchown"	}, /* 207 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 208 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 209 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 210 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 211 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 212 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 213 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 214 */
	{ 1,	0,	sys_setfsuid,		"setfsuid"	}, /* 215 */
	{ 1,	0,	sys_setfsgid,		"setfsgid"	}, /* 216 */
        { 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 217 */
	{ 3,	0,	sys_mincore,	         "mincore"      }, /* 218 */
	{ 3,	0,	sys_madvise,		"madvise"	}, /* 219 */
	{ 3,	0,	sys_getdents64,		"getdents64"	}, /* 220 */
	{ -1,	0,	printargs,		"SYS_221"	}, /* 221 */
	{ 4,	0,	sys_readahead,		"readahead"	}, /* 222 */
	{ 4,	TF,	sys_sendfile64,		"sendfile64"	}, /* 223 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 224 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 225 */
	{ 5,	0,	sys_fsetxattr,		"fsetxattr"	}, /* 226 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 227 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 228 */
	{ 4,	0,	sys_fgetxattr,		"fgetxattr"	}, /* 229 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 230 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 231 */
	{ 3,	0,	sys_flistxattr,		"flistxattr"	}, /* 232 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 233 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 234 */
	{ 2,	0,	sys_fremovexattr,	"fremovexattr"	}, /* 235 */
	{ 0,	0,	printargs,		"gettid"	}, /* 236 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 237 */
	{ 5,	0,	sys_futex,		"futex"		}, /* 238 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity" },/* 239 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity" },/* 240 */
	{ -1,	0,	printargs,		"SYS_241"	}, /* 241 */
	{ -1,	0,	printargs,		"SYS_242"	}, /* 242 */
	{ 2,	0,	sys_io_setup,		"io_setup"	}, /* 243 */
	{ 1,	0,	sys_io_destroy,		"io_destroy"	}, /* 244 */
	{ 5,	0,	sys_io_getevents,		"io_getevents"	}, /* 245 */
	{ 3,	0,	sys_io_submit,		"io_submit"	}, /* 246 */
	{ 3,	0,	sys_io_cancel,		"io_cancel"	}, /* 247 */
	{ 1,	TP,	sys_exit,		"exit_group"	}, /* 248 */
	{ 1,	0,	sys_epoll_create,	"epoll_create"	}, /* 249 */
	{ 4,	0,	sys_epoll_ctl,		"epoll_ctl"	}, /* 250 */
	{ 4,	0,	sys_epoll_wait,		"epoll_wait"	}, /* 251 */
	{ 1,	0,	printargs,		"set_tid_address"}, /* 252 */
	{ 5,	0,	printargs,		"fadvise64"	}, /* 253 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 254 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 255 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 256 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 257 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 258 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 259 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 260 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 261 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 262 */

	{ 5,	0,	printargs,		"SYS_263"	}, /* 263 */
	{ 5,	0,	printargs,		"SYS_264"	}, /* 264 */
	{ 5,	0,	printargs,		"SYS_265"	}, /* 265 */
	{ 5,	0,	printargs,		"SYS_266"	}, /* 266 */
	{ 5,	0,	printargs,		"SYS_267"	}, /* 267 */
	{ 5,	0,	printargs,		"SYS_268"	}, /* 268 */
	{ 5,	0,	printargs,		"SYS_269"	}, /* 269 */
	{ 5,	0,	printargs,		"SYS_270"	}, /* 270 */
	{ 5,	0,	printargs,		"SYS_271"	}, /* 271 */
	{ 5,	0,	printargs,		"SYS_272"	}, /* 272 */
	{ 5,	0,	printargs,		"SYS_273"	}, /* 273 */
	{ 5,	0,	printargs,		"SYS_274"	}, /* 274 */
	{ 5,	0,	printargs,		"SYS_275"	}, /* 275 */
	{ 5,	0,	printargs,		"SYS_276"	}, /* 276 */
	{ 5,	0,	printargs,		"SYS_277"	}, /* 277 */
	{ 5,	0,	printargs,		"SYS_278"	}, /* 278 */
	{ 5,	0,	printargs,		"SYS_279"	}, /* 279 */
	{ 5,	0,	printargs,		"SYS_280"	}, /* 280 */
	{ 5,	0,	printargs,		"SYS_281"	}, /* 281 */
	{ 5,	0,	printargs,		"SYS_282"	}, /* 282 */
	{ 5,	0,	printargs,		"SYS_283"	}, /* 283 */
	{ 5,	0,	printargs,		"SYS_284"	}, /* 284 */
	{ 5,	0,	printargs,		"SYS_285"	}, /* 285 */
	{ 5,	0,	printargs,		"SYS_286"	}, /* 286 */
	{ 5,	0,	printargs,		"SYS_287"	}, /* 287 */
	{ 5,	0,	printargs,		"SYS_288"	}, /* 288 */
	{ 5,	0,	printargs,		"SYS_289"	}, /* 289 */
	{ 5,	0,	printargs,		"SYS_290"	}, /* 290 */
	{ 5,	0,	printargs,		"SYS_291"	}, /* 291 */
	{ 5,	0,	printargs,		"SYS_292"	}, /* 292 */
	{ 5,	0,	printargs,		"SYS_293"	}, /* 293 */
	{ 5,	0,	printargs,		"SYS_294"	}, /* 294 */
	{ 5,	0,	printargs,		"SYS_295"	}, /* 295 */
	{ 5,	0,	printargs,		"SYS_296"	}, /* 296 */
	{ 5,	0,	printargs,		"SYS_297"	}, /* 297 */
	{ 5,	0,	printargs,		"SYS_298"	}, /* 298 */
	{ 5,	0,	printargs,		"SYS_299"	}, /* 299 */

#if SYS_socket_subcall != 300
 #error fix me
#endif
	{ 8,	0,	printargs,		"socket_subcall"}, /* 300 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 301 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 302 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 303 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 304 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 305 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 306 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 307 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 308 */
	{ 4,	TN,	sys_send,		"send"		}, /* 309 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 310 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 311 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 312 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 313 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 314 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 315 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 316 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 317 */

#if SYS_ipc_subcall != 318
 #error fix me
#endif
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 318 */
	{ 4,	TI,	sys_semop,		"semop"		}, /* 319 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 320 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 321 */
	{ 5,	TI,	sys_semtimedop,		"semtimedop"	}, /* 322 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 323 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 324 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 325 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 326 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 327 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 328 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 329 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 330 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 331 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 332 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 333 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 334 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 335 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 336 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 337 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 338 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 339 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 340 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 341 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 342 */
