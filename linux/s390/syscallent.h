/*
 * Copyright (c) 2000 IBM Deutschland Entwicklung GmbH, IBM Coporation
 * Authors: Ulrich Weigand <Ulrich.Weigand@de.ibm.com>
 *          D.J. Barrow  <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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
	{ 1,	0,	sys_time,		"time"		}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 16 */
	{ -1,	0,	printargs,		"SYS_17"	}, /* 17 */
	{ -1,	0,	printargs,		"SYS_18"	}, /* 18 */
	{ 3,	0,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	0,	sys_getuid,		"getuid"	}, /* 24 */
	{ 1,	0,	sys_stime,		"stime"		}, /* 25 */
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
	{ 0,	0,	sys_geteuid,		"geteuid"	}, /* 49 */
	{ 0,	0,	sys_getegid,		"getegid"	}, /* 50 */
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
	{ -1,	0,	printargs,		"SYS_82"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ -1,	0,	printargs,		"SYS_84"	}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 1,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ 3,	0,	sys_readdir,		"readdir"	}, /* 89 */
	{ 6,	0,	sys_old_mmap,		"mmap"		}, /* 90 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	0,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	0,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ 3,	0,	sys_fchown,		"fchown"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ -1,	0,	printargs,		"SYS_98"	}, /* 98 */
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
	{ 2,	TP,	sys_clone,		"clone"		}, /* 120 */
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
	{ 1,	0,	sys_munlockall,		"munlockall"	}, /* 153 */
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
	{ -1,	0,	printargs,		"SYS_166"	}, /* 166 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 167 */
	{ 3,	0,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	printargs,		"nfsservctl"	}, /* 169 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 170 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 171 */
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
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 182 */
	{ 2,	0,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TF,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ 5,	0,	printargs,		"SYS_189"	}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork"		}, /* 190 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 191 */
	{ 6,	0,	sys_mmap,		"mmap2"		}, /* 192 */
	{ 2,	TF,	printargs,		"truncate64"	}, /* 193 */
	{ 2,	TF,	printargs,		"ftruncate64"	}, /* 194 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 195 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 196 */
	{ 2,	TF,	sys_fstat64,		"fstat64"	}, /* 197 */
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
	{ 3,	0,	sys_fcntl,		"fcntl64"	}, /* 221 */
	{ -1,	0,	printargs,		"SYS_222"	}, /* 222 */
	{ -1,	0,	printargs,		"SYS_223"	}, /* 223 */
	{ -1,	0,	printargs,		"SYS_224"	}, /* 224 */
	{ -1,	0,	printargs,		"SYS_225"	}, /* 225 */
	{ -1,	0,	printargs,		"SYS_226"	}, /* 226 */
	{ -1,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ -1,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ -1,	0,	printargs,		"SYS_229"	}, /* 229 */
	{ -1,	0,	printargs,		"SYS_230"	}, /* 230 */
	{ -1,	0,	printargs,		"SYS_231"	}, /* 231 */
	{ -1,	0,	printargs,		"SYS_232"	}, /* 232 */
	{ -1,	0,	printargs,		"SYS_233"	}, /* 233 */
	{ -1,	0,	printargs,		"SYS_234"	}, /* 234 */
	{ -1,	0,	printargs,		"SYS_235"	}, /* 235 */
	{ -1,	0,	printargs,		"SYS_236"	}, /* 236 */
	{ -1,	0,	printargs,		"SYS_237"	}, /* 237 */
	{ -1,	0,	printargs,		"SYS_238"	}, /* 238 */
	{ -1,	0,	printargs,		"SYS_239"	}, /* 239 */
	{ -1,	0,	printargs,		"SYS_240"	}, /* 240 */
	{ -1,	0,	printargs,		"SYS_241"	}, /* 241 */
	{ -1,	0,	printargs,		"SYS_242"	}, /* 242 */
	{ -1,	0,	printargs,		"SYS_243"	}, /* 243 */
	{ -1,	0,	printargs,		"SYS_244"	}, /* 244 */
	{ -1,	0,	printargs,		"SYS_245"	}, /* 245 */
	{ -1,	0,	printargs,		"SYS_246"	}, /* 246 */
	{ -1,	0,	printargs,		"SYS_247"	}, /* 247 */
	{ -1,	0,	printargs,		"SYS_248"	}, /* 248 */
	{ -1,	0,	printargs,		"SYS_249"	}, /* 249 */
	{ -1,	0,	printargs,		"SYS_250"	}, /* 250 */
	{ -1,	0,	printargs,		"SYS_251"	}, /* 251 */
	{ -1,	0,	printargs,		"SYS_252"	}, /* 252 */
	{ -1,	0,	printargs,		"SYS_253"	}, /* 253 */
	{ -1,	0,	printargs,		"SYS_254"	}, /* 254 */
	{ -1,	0,	printargs,		"SYS_255"	}, /* 255 */

	{ 8,	0,	printargs,		"socket_subcall"}, /* 256 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 257 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 258 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 259 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 260 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 261 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 262 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 263 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 264 */
	{ 4,	TN,	sys_send,		"send"		}, /* 265 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 266 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 267 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 268 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 269 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 270 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 271 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 272 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 273 */

	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 274 */
	{ 4,	TI,	printargs,		"semop"		}, /* 275 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 276 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 277 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 278 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 279 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 280 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 281 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 282 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 283 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 284 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 285 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 286 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 287 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 288 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 289 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 290 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 291 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 292 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 293 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 294 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 295 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 296 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 297 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 298 */
	{ 5,	0,	printargs,		"SYS_299"	}, /* 299 */
	{ 5,	0,	printargs,		"SYS_300"	}, /* 300 */
	{ 5,	0,	printargs,		"SYS_301"	}, /* 301 */
	{ 5,	0,	printargs,		"SYS_302"	}, /* 302 */
	{ 5,	0,	printargs,		"SYS_303"	}, /* 303 */
