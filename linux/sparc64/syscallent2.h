	{ 1,	0,	sys_setup,	"setup" },		/* 0 */
	{ 1,	TP,	sys_exit,	"exit" },		/* 1 */
	{ 0,	TP,	sys_fork,	"fork" },		/* 2 */
	{ 3,	0,	sys_read,	"read" },		/* 3 */
	{ 3,	0,	sys_write,	"write" },		/* 4 */
	{ 3,	TF,	sys_open,	"open" },		/* 5 */
	{ 1,	0,	sys_close,	"close" },		/* 6 */
	{ 4,	TP,	sys_wait4,	"wait4" },		/* 7 */
	{ 2,	TF,	sys_creat,	"creat" },		/* 8 */
	{ 2,	TF,	sys_link,	"link" },		/* 9 */
	{ 1,	TF,	sys_unlink,	"unlink" },		/* 10 */
	{ 2,    TF|TP,	sys_execv,	"execv" },		/* 11 */
	{ 1,	TF,	sys_chdir,	"chdir" },		/* 12 */
        { 3,    TF,     sys_chown,      "chown"},		/* 13 */
	{ 3,	TF,	sys_mknod,	"mknod" },		/* 14 */
	{ 2,	TF,	sys_chmod,	"chmod" },		/* 15 */
	{ 3,	TF,	sys_chown,	"lchown" },		/* 16 */
	{ 1,	0,	sys_brk,	"brk" },		/* 17 */
	{ 4,	0,	printargs,	"perfctr" },		/* 18 */
	{ 3,	0,	sys_lseek,	"lseek" },		/* 19 */
	{ 0,	0,	sys_getpid,	"getpid" },		/* 20 */
	{ 2,	0,	sys_capget,	"capget" },		/* 21 */
	{ 2,	0,	sys_capset,	"capset" },		/* 22 */
	{ 1,	0,	sys_setuid,	"setuid" },		/* 23 */
	{ 0,	0,	sys_getuid,	"getuid" },		/* 24 */
	{ 1,	0,	sys_time,	"time" },		/* 25 */
	{ 5,	0,	sys_ptrace,	"ptrace" },		/* 26 */
	{ 1,	0,	sys_alarm,	"alarm" },		/* 27 */
	{ 2,	TS,	sys_sigaltstack,"sigaltstack" },	/* 28 */
	{ 0,	TS,	sys_pause,	"pause" },		/* 29 */
	{ 2,	TF,	sys_utime,	"utime" },		/* 30 */
	{ 3,	TF,	sys_chown,	"lchown32" },		/* 31 */
	{ 3,	0,	sys_fchown,	"fchown32" },		/* 32 */
	{ 2,	TF,	sys_access,	"access" },		/* 33 */
	{ 1,	0,	sys_nice,	"nice" },		/* 34 */
	{ 3,	TF,	sys_chown,	"chown32" },		/* 35 */
	{ 0,	0,	sys_sync,	"sync" },		/* 36 */
	{ 2,	TS,	sys_kill,	"kill" },		/* 37 */
	{ 2,	TF,	sys_stat,	"stat" },		/* 38 */
	{ 4,	TF,	sys_sendfile,	"sendfile" },		/* 39 */
	{ 2,	TF,	sys_lstat,	"lstat" },		/* 40 */
	{ 2,	0,	sys_dup,	"dup" },		/* 41 */
	{ 0,	0,	sys_pipe,	"pipe" },		/* 42 */
	{ 1,	0,	sys_times,	"times" },		/* 43 */
	{ 0,	0,	sys_getuid,	"getuid32" },		/* 44 */
	{ 2,	TF,	sys_umount2,	"umount" },		/* 45 */
	{ 1,	0,	sys_setgid,	"setgid" },		/* 46 */
	{ 0,	0,	sys_getgid,	"getgid" },		/* 47 */
	{ 3,	TS,	sys_signal,	"signal" },		/* 48 */
	{ 0,	0,	sys_geteuid,	"geteuid" },		/* 49 */
	{ 0,	0,	sys_getegid,	"getegid" },		/* 50 */
	{ 1,	TF,	sys_acct,	"acct" },		/* 51 */
	{ 2,	0,	printargs,	"memory_ordering" },	/* 52 */
	{ 0,	0,	sys_getgid,	"getgid32" },		/* 53 */
	{ 3,	0,	sys_ioctl,	"ioctl" },		/* 54 */
	{ 3,	0,	sys_reboot,	"reboot" },		/* 55 */
	{ 6,	0,	sys_mmap,	"mmap2" },		/* 56 */
	{ 2,	TF,	sys_symlink,	"symlink" },		/* 57 */
	{ 3,	TF,	sys_readlink,	"readlink" },		/* 58 */
	{ 3,	TF|TP,	sys_execve,	"execve" },		/* 59 */
	{ 1,	0,	sys_umask,	"umask" },		/* 60 */
	{ 1,	TF,	sys_chroot,	"chroot" },		/* 61 */
	{ 2,	0,	sys_fstat,	"fstat" },		/* 62 */
	{ 2,	TF,	sys_fstat64,	"fstat64" },		/* 63 */
	{ 0,	0,	sys_getpagesize,"getpagesize" },	/* 64 */
	{ 3,	0,	sys_msync,	"msync" },		/* 65 */
	{ 0,	TP,	sys_vfork,	"vfork" },		/* 66 */
	{ 5,	TF,	sys_pread,	"pread" },		/* 67 */
	{ 5,	TF,	sys_pwrite,	"pwrite" },		/* 68 */
	{ 0,    0,	sys_geteuid,	"geteuid32" },		/* 69 */
	{ 0,	0,	sys_getegid,	"getegid32" },		/* 70 */
	{ 6,	0,	sys_mmap,	"mmap" },		/* 71 */
	{ 2,	0,	sys_setreuid,	"setreuid32" },		/* 72 */
	{ 2,	0,	sys_munmap,	"munmap" },		/* 73 */
	{ 3,	0,	sys_mprotect,	"mprotect" },		/* 74 */
	{ 3,	0,	sys_madvise,	"madvise" },		/* 75 */
	{ 0,	0,	sys_vhangup,	"vhangup" },		/* 76 */
	{ 3,	TF,	sys_truncate64,	"truncate64" },		/* 77 */
	{ 3,	0,	sys_mincore,	"mincore" },		/* 78 */
	{ 2,	0,	sys_getgroups,	"getgroups" },		/* 79 */
	{ 2,	0,	sys_setgroups,	"setgroups" },		/* 80 */
	{ 0,	0,	sys_getpgrp,	"getpgrp" },		/* 81 */
	{ 2,	0,	sys_setgroups,	"setgroups32" },	/* 82 */
	{ 3,	0,	sys_setitimer,	"setitimer" },		/* 83 */
	{ 2,	0,	sys_ftruncate,	"ftruncate64" },	/* 84 */
	{ 1,	TF,	sys_swapon,	"swapon" },		/* 85 */
	{ 2,	0,	sys_getitimer,	"getitimer" },		/* 86 */
	{ 1,	0,	sys_setuid,	"setuid32" },		/* 87 */
	{ 2,	0,	sys_sethostname,"sethostname" },	/* 88 */
	{ 1,	0,	sys_setgid,	"setgid32" },		/* 89 */
	{ 2,	0,	sys_dup2,	"dup2" },		/* 90 */
	{ 1,	0,	sys_setfsuid,	"setfsuid32" },		/* 91 */
	{ 3,	0,	sys_fcntl,	"fcntl" },		/* 92 */
	{ 5,	0,	sys_select,	"select" },		/* 93 */
	{ 1,	0,	sys_setfsgid,	"setfsgid32" },		/* 94 */
	{ 1,	0,	sys_fsync,	"fsync" },		/* 95 */
	{ 3,	0,	sys_setpriority,"setpriority" },	/* 96 */
	{ 3,	TN,	sys_socket,	"socket" },		/* 97 */
	{ 3,	TN,	sys_connect,	"connect" },		/* 98 */
	{ 3,	TN,	sys_accept,	"accept" },		/* 99 */
	{ 2,	0,	sys_getpriority,"getpriority" },	/* 100 */
	{ 1,	TS,	printargs,	"rt_sigreturn" },	/* 101 */
	{ 4,	TS,	sys_rt_sigaction,"rt_sigaction" },	/* 102 */
	{ 4,	TS,	sys_rt_sigprocmask,"rt_sigprocmask" },	/* 103 */
	{ 2,	TS,	sys_rt_sigpending,"rt_sigpending" },	/* 104 */
	{ 4,	TS,	sys_rt_sigtimedwait,"rt_sigtimedwait" },/* 105 */
	{ 3,	TS,	sys_rt_sigqueueinfo,"rt_sigqueueinfo" },/* 106 */
	{ 2,	TS,	sys_rt_sigsuspend,"rt_sigsuspend" },	/* 107 */
	{ 3,	TS,	sys_setresuid,	"setresuid" },		/* 108 */
	{ 3,    TS,	sys_getresuid,	"getresuid" },		/* 109 */
	{ 3,	TS,	sys_setresgid,	"setresgid" },		/* 110 */
	{ 3,	TS,	sys_getresgid,	"getresgid" },		/* 111 */
	{ 2,	TS,	sys_setresgid,	"setresgid32" },	/* 112 */
	{ 5,	TN,	sys_recvmsg,	"recvmsg" },		/* 113 */
	{ 5,	TN,	sys_sendmsg,	"sendmsg" },		/* 114 */
	{ 2,	0,	sys_getgroups,	"getgroups32" },	/* 115 */
	{ 2,	0,	sys_gettimeofday,"gettimeofday" },	/* 116 */
	{ 2,	0,	sys_getrusage,	"getrusage" },		/* 117 */
	{ 5,	TN,	sys_getsockopt,	"getsockopt" },		/* 118 */
	{ 2,	TF,	sys_getcwd,	"getcwd" },		/* 119 */
	{ 3,	0,	sys_readv,	"readv" },		/* 120 */
	{ 3,	0,	sys_writev,	"writev" },		/* 121 */
	{ 2,	0,	sys_settimeofday,"settimeofday" },	/* 122 */
	{ 3,	0,	sys_fchown,	"fchown" },		/* 123 */
	{ 2,	0,	sys_fchmod,	"fchmod" },		/* 124 */
	{ 6,	TN,	sys_recvfrom,	"recvfrom" },		/* 125 */
	{ 2,	0,	sys_setreuid,	"setreuid" },		/* 126 */
	{ 2,	0,	sys_setregid,	"setregid" },		/* 127 */
	{ 2,	TF,	sys_rename,	"rename" },		/* 128 */
	{ 2,	TF,	sys_truncate,	"truncate" },		/* 129 */
	{ 2,	0,	sys_ftruncate,	"ftruncate" },		/* 130 */
	{ 2,	0,	sys_flock,	"flock" },		/* 131 */
	{ 2,	TF,	sys_lstat64,	"lstat64" },		/* 132 */
	{ 6,	TN,	sys_sendto,	"sendto" },		/* 133 */
	{ 2,	TN,	sys_shutdown,	"shutdown" },		/* 134 */
	{ 4,	TN,	sys_socketpair,	"socketpair" },		/* 135 */
	{ 2,	TF,	sys_mkdir,	"mkdir" },		/* 136 */
	{ 1,	TF,	sys_rmdir,	"rmdir" },		/* 137 */
	{ 2,	TF,	sys_utimes,	"utimes" },		/* 138 */
	{ 2,	TF,	sys_stat64,	"stat64" },		/* 139 */
	{ 4,    TF,	sys_sendfile64,	"sendfile64" },		/* 140 */
	{ 3,	TN,	sys_getpeername,"getpeername" },	/* 141 */
	{ 5,    0,	sys_futex,	"futex" },		/* 142 */
	{ 0,	0,	printargs,	"gettid" },		/* 143 */
	{ 2,	0,	sys_getrlimit,	"getrlimit" },		/* 144 */
	{ 2,	0,	sys_setrlimit,	"setrlimit" },		/* 145 */
	{ 2,	TF,	sys_pivotroot,	"pivot_root" },		/* 146 */
	{ 5,	0,	sys_prctl,	"prctl" },		/* 147 */
	{ 5,	0,	printargs,	"pciconfig_read" },	/* 148 */
	{ 5,	0,	printargs,	"pciconfig_write" },	/* 149 */
	{ 3,	TN,	sys_getsockname,"getsockname" },	/* 150 */
	{ 4,	TN,	sys_getmsg,	"getmsg" },		/* 151 */
	{ 4,	TN,	sys_putmsg,	"putmsg" },		/* 152 */
	{ 3,	0,	sys_poll,	"poll" },		/* 153 */
	{ 4,	0,	sys_getdents64,	"getdents64" },		/* 154 */
	{ 3,	0,	sys_fcntl,	"fcntl64" },		/* 155 */
	{ 4,	0,	printargs,	"getdirentries" },	/* 156 */
	{ 2,	TF,	sys_statfs,	"statfs" },		/* 157 */
	{ 2,	0,	sys_fstatfs,	"fstatfs" },		/* 158 */
	{ 1,	TF,	sys_umount,	"oldumount" },		/* 159 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity" },/* 160 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity" },/* 161 */
	{ 2,	0,	printargs,	"getdomainname" },	/* 162 */
	{ 2,	0,	sys_setdomainname,"setdomainname" },	/* 163 */
	{ 5,	0,	printargs,	"utrap_install" },	/* 164 */
	{ 4,	0,	sys_quotactl,	"quotactl" },		/* 165 */
	{ 1,	0,	printargs,	"set_tid_address" },	/* 166 */
	{ 5,	TF,	sys_mount,	"mount" },		/* 167 */
	{ 2,	0,	sys_ustat,	"ustat" },		/* 168 */
	{ 5,	TF,	sys_setxattr,	"setxattr" },		/* 169 */
	{ 5,	TF,	sys_setxattr,	"lsetxattr" },		/* 170 */
	{ 5,	0,	sys_fsetxattr,	"fsetxattr" },		/* 171 */
	{ 4,	TF,	sys_getxattr,	"getxattr" },		/* 172 */
	{ 4,	TF,	sys_getxattr,	"lgetxattr" },		/* 173 */
	{ 3,	0,	sys_getdents,	"getdents" },		/* 174 */
	{ 0,	0,	sys_setsid,	"setsid" },		/* 175 */
	{ 1,	0,	sys_fchdir,	"fchdir" },		/* 176 */
	{ 4,    0,	sys_fgetxattr,	"fgetxattr" },		/* 177 */
	{ 3,	TF,	sys_listxattr,	"listxattr" },		/* 178 */
	{ 3,	TF,	sys_listxattr,	"llistxattr" },		/* 179 */
	{ 3,	0,	sys_flistxattr,	"flistxattr" },		/* 180 */
	{ 2,	TF,	sys_removexattr,"removexattr" },	/* 181 */
	{ 2,	TF,	sys_removexattr,"lremovexattr" },	/* 182 */
	{ 1,	TS,	sys_sigpending,	"sigpending" },		/* 183 */
	{ 5,	0,	sys_query_module,"query_module" },	/* 184 */
	{ 2,	0,	sys_setpgid,	"setpgid" },		/* 185 */
	{ 2,	0,	sys_fremovexattr,"fremovexattr" },	/* 186 */
	{ 2,	TS,	sys_kill,	"tkill" },		/* 187 */
	{ 1,	TP,	sys_exit,	"exit_group" },		/* 188 */
	{ 1,	0,	sys_uname,	"uname" },		/* 189 */
	{ 2,	0,	sys_init_module,"init_module" },	/* 190 */
	{ 1,	0,	sys_personality,"personality" },	/* 191 */
	{ 5,	0,	sys_remap_file_pages,"remap_file_pages" },/* 192 */
	{ 1,	0,	sys_epoll_create,"epoll_create" },	/* 193 */
	{ 4,	0,	sys_epoll_ctl,	"epoll_ctl" },		/* 194 */
	{ 4,	0,	sys_epoll_wait,	"epoll_wait" },		/* 195 */
	{ 2,	0,	sys_ulimit,	"ulimit" },		/* 196 */
	{ 0,	0,	sys_getppid,	"getppid" },		/* 197 */
	{ 3,	TS,	sys_sigaction,	"sigaction" },		/* 198 */
	{ 5,	0,	printargs,	"sgetmask" },		/* 199 */
	{ 5,	0,	printargs,	"ssetmask" },		/* 200 */
	{ 3,	TS,	sys_sigsuspend,	"sigsuspend" },		/* 201 */
	{ 2,	TF,	sys_lstat,	"lstat" },		/* 202 */
	{ 1,	TF,	sys_uselib,	"uselib" },		/* 203 */
	{ 3,	0,	sys_readdir,	"readdir" },		/* 204 */
	{ 4,	0,	sys_readahead,	"readahead" },		/* 205 */
	{ 2,	0,	sys_socketcall,	"socketcall" },		/* 206 */
	{ 3,	0,	sys_syslog,	"syslog" },		/* 207 */
	{ 4,	0,	printargs,	"lookup_dcookie" },	/* 208 */
	{ 6,	0,	printargs,	"fadvise64" },		/* 209 */
	{ 6,	0,	printargs,	"fadvise64_64" },	/* 210 */
	{ 3,	TS,	sys_tgkill,	"tgkill" },		/* 211 */
	{ 3,	TP,	sys_waitpid,	"waitpid" },		/* 212 */
	{ 1,	0,	sys_swapoff,	"swapoff" },		/* 213 */
	{ 1,	0,	sys_sysinfo,	"sysinfo" },		/* 214 */
	{ 5,	0,	sys_ipc,	"ipc" },		/* 215 */
	{ 1,	TS,	sys_sigreturn,	"sigreturn" },		/* 216 */
	{ 5,	TP,	sys_clone,	"clone" },		/* 217 */
	{ 3,	0,	sys_modify_ldt,	"modify_ldt" },		/* 218 */
	{ 1,	0,	sys_adjtimex,	"adjtimex" },		/* 219 */
	{ 3,	TS,	sys_sigprocmask,"sigprocmask" },	/* 220 */
	{ 2,	0,	sys_create_module,"create_module" },	/* 221 */
	{ 1,	0,	sys_delete_module,"delete_module" },
	{ 1,	0,	sys_get_kernel_syms,"get_kernel_syms"},	/* 223 */
	{ 1,	0,	sys_getpgid,	"getpgid" },		/* 224 */
	{ 0,	0,	sys_bdflush,	"bdflush" },		/* 225 */
	{ 3,	0,	sys_sysfs,	"sysfs" },		/* 226 */
	{ 5,	0,	sys_afs_syscall,"afs_syscall" },	/* 227 */
	{ 1,	0,	sys_setfsuid,	"setfsuid" },		/* 228 */
	{ 1,	0,	sys_setfsgid,	"setfsgid" },		/* 229 */
	{ 5,	0,	sys_select,	"select" },		/* 230 */
	{ 1,	0,	sys_time,	"time" },		/* 231 */
	{ 2,	TF,	sys_stat,	"stat" },		/* 232 */
	{ 1,	0,	sys_stime,	"stime" },		/* 233 */
	{ 3,	TF,	sys_statfs64,	"statfs64" },		/* 234 */
	{ 3,	0,	sys_fstatfs64,	"fstatfs64" },		/* 235 */
	{ 5,	0,	sys_llseek,	"_llseek" },		/* 236 */
	{ 2,	0,	sys_mlock,	"mlock" },		/* 237 */
	{ 2,	0,	sys_munlock,	"munlock" },		/* 238 */
	{ 2,	0,	sys_mlockall,	"mlockall" },		/* 239 */
	{ 0,	0,	sys_munlockall,	"munlockall" },		/* 240 */
	{ 2,	0,	sys_sched_setparam,"sched_setparam"},	/* 241 */
	{ 2,	0,	sys_sched_getparam,"sched_getparam"},	/* 242 */
	{ 3,	0,	sys_sched_setscheduler,"sched_setscheduler"},/* 243 */
	{ 1,	0,	sys_sched_getscheduler,"sched_getscheduler"},/* 244 */
	{ 0,	0,	sys_sched_yield,"sched_yield" },	/* 245 */
	{ 1,0,sys_sched_get_priority_max,"sched_get_priority_max"},/* 246 */
	{ 1,0,sys_sched_get_priority_min,"sched_get_priority_min"},/* 247 */
	{ 2,	0,sys_sched_rr_get_interval,"sched_rr_get_interval"},/* 248 */
	{ 2,	0,	sys_nanosleep,	"nanosleep" },		/* 249 */
	{ 4,	0,	sys_mremap,	"mremap" },		/* 250 */
	{ 1,	0,	sys_sysctl,	"_sysctl" },		/* 251 */
	{ 1,	0,	sys_getsid,	"getsid" },		/* 252 */
	{ 1,	0,	sys_fdatasync,	"fdatasync" },		/* 253 */
	{ 3,	0,	printargs,	"nfsservctl" },		/* 254 */
	{ 5,	0,	printargs,	"aplib" },		/* 255 */
	{ 2,	0,	sys_clock_settime,"clock_settime" },	/* 256 */
	{ 2,	0,	sys_clock_gettime,"clock_gettime" },	/* 257 */
	{ 2,	0,	sys_clock_getres,"clock_getres" },	/* 258 */
	{ 4,	0,	sys_clock_nanosleep,"clock_nanosleep" },/* 259 */
	{ 3,	0,	sys_sched_setaffinity,"sched_setaffinity" },/* 260 */
	{ 3,	0,	sys_sched_getaffinity,"sched_getaffinity" },/* 261 */
	{ 4,	0,	sys_timer_settime,"timer_settime" },	/* 262 */
	{ 2,	0,	sys_timer_gettime,"timer_gettime" },	/* 263 */
	{ 1,	0,	sys_timer_getoverrun,"timer_getoverrun" },/* 264 */
	{ 1,	0,	sys_timer_delete,"timer_delete" },	/* 265 */
	{ 3,	0,	sys_timer_create,"timer_create" },	/* 266 */
	{ 5,	0,	printargs,	"SYS_267" },		/* 267 */
	{ 5,	0,	printargs,	"SYS_268" },		/* 268 */
	{ 5,	0,	printargs,	"SYS_269" },		/* 269 */
	{ 5,	0,	printargs,	"SYS_270" },		/* 270 */
	{ 5,	0,	printargs,	"SYS_271" },		/* 271 */
	{ 5,	0,	printargs,	"SYS_272" },		/* 272 */
	{ 5,	0,	printargs,	"SYS_273" },		/* 273 */
	{ 5,	0,	printargs,	"SYS_274" },		/* 274 */
	{ 5,	0,	printargs,	"SYS_275" },		/* 275 */
	{ 5,	0,	printargs,	"SYS_276" },		/* 276 */
	{ 5,	0,	printargs,	"SYS_277" },		/* 277 */
	{ 5,	0,	printargs,	"SYS_278" },		/* 278 */
	{ 5,	0,	printargs,	"SYS_279" },		/* 279 */
	{ 5,	0,	printargs,	"SYS_280" },		/* 280 */
	{ 5,	0,	printargs,	"SYS_281" },		/* 281 */
	{ 5,	0,	printargs,	"SYS_282" },		/* 282 */
	{ 5,	0,	printargs,	"SYS_283" },		/* 283 */
	{ 5,	0,	printargs,	"SYS_284" },		/* 284 */
	{ 5,	0,	printargs,	"SYS_285" },		/* 285 */
	{ 5,	0,	printargs,	"SYS_286" },		/* 286 */
	{ 5,	0,	printargs,	"SYS_287" },		/* 287 */
	{ 5,	0,	printargs,	"SYS_288" },		/* 288 */
	{ 5,	0,	printargs,	"SYS_289" },		/* 289 */
	{ 5,	0,	printargs,	"SYS_290" },		/* 290 */
	{ 5,	0,	printargs,	"SYS_291" },		/* 291 */
	{ 5,	0,	printargs,	"SYS_292" },		/* 292 */
	{ 5,	0,	printargs,	"SYS_293" },		/* 293 */
	{ 5,	0,	printargs,	"SYS_294" },		/* 294 */
	{ 5,	0,	printargs,	"SYS_295" },		/* 295 */
	{ 5,	0,	printargs,	"SYS_296" },		/* 296 */
	{ 5,	0,	printargs,	"SYS_297" },		/* 297 */
	{ 5,	0,	printargs,	"SYS_298" },		/* 298 */
	{ 5,	0,	printargs,	"SYS_299" },		/* 299 */
	{ 5,	0,	printargs,	"SYS_300" },		/* 300 */
	{ 5,	0,	printargs,	"SYS_301" },		/* 301 */
	{ 5,	0,	printargs,	"SYS_302" },		/* 302 */
	{ 5,	0,	printargs,	"SYS_303" },		/* 303 */
	{ 5,	0,	printargs,	"SYS_304" },		/* 304 */
	{ 5,	0,	printargs,	"SYS_305" },		/* 305 */
	{ 5,	0,	printargs,	"SYS_306" },		/* 306 */
	{ 5,	0,	printargs,	"SYS_307" },		/* 307 */
	{ 5,	0,	printargs,	"SYS_308" },		/* 308 */
	{ 5,	0,	printargs,	"SYS_309" },		/* 309 */
	{ 5,	0,	printargs,	"SYS_310" },		/* 310 */
	{ 5,	0,	printargs,	"SYS_311" },		/* 311 */
	{ 5,	0,	printargs,	"SYS_312" },		/* 312 */
	{ 5,	0,	printargs,	"SYS_313" },		/* 313 */
	{ 5,	0,	printargs,	"SYS_314" },		/* 314 */
	{ 5,	0,	printargs,	"SYS_315" },		/* 315 */
	{ 5,	0,	printargs,	"SYS_316" },		/* 316 */
	{ 5,	0,	printargs,	"SYS_317" },		/* 317 */
	{ 5,	0,	printargs,	"SYS_318" },		/* 318 */
	{ 5,	0,	printargs,	"SYS_319" },		/* 319 */
	{ 5,	0,	printargs,	"SYS_320" },		/* 320 */
	{ 5,	0,	printargs,	"SYS_321" },		/* 321 */
	{ 5,	0,	printargs,	"SYS_322" },		/* 322 */
	{ 5,	0,	printargs,	"SYS_323" },		/* 323 */
	{ 5,	0,	printargs,	"SYS_324" },		/* 324 */
	{ 5,	0,	printargs,	"SYS_325" },		/* 325 */
	{ 5,	0,	printargs,	"SYS_326" },		/* 326 */
	{ 5,	0,	printargs,	"SYS_327" },		/* 327 */
	{ 5,	0,	printargs,	"SYS_328" },		/* 328 */
	{ 5,	0,	printargs,	"SYS_329" },		/* 329 */
	{ 5,	0,	printargs,	"SYS_330" },		/* 330 */
	{ 5,	0,	printargs,	"SYS_331" },		/* 331 */
	{ 5,	0,	printargs,	"SYS_332" },		/* 332 */
	{ 5,	0,	printargs,	"SYS_333" },		/* 333 */
	{ 5,	0,	printargs,	"SYS_334" },		/* 334 */
	{ 5,	0,	printargs,	"SYS_335" },		/* 335 */
	{ 5,	0,	printargs,	"SYS_336" },		/* 336 */
	{ 5,	0,	printargs,	"SYS_337" },		/* 337 */
	{ 5,	0,	printargs,	"SYS_338" },		/* 338 */
	{ 5,	0,	printargs,	"SYS_339" },		/* 339 */
	{ 5,	0,	printargs,	"SYS_340" },		/* 340 */
	{ 5,	0,	printargs,	"SYS_341" },		/* 341 */
	{ 5,	0,	printargs,	"SYS_342" },		/* 342 */
	{ 5,	0,	printargs,	"SYS_343" },		/* 343 */
	{ 5,	0,	printargs,	"SYS_344" },		/* 344 */
	{ 5,	0,	printargs,	"SYS_345" },		/* 345 */
	{ 5,	0,	printargs,	"SYS_346" },		/* 346 */
	{ 5,	0,	printargs,	"SYS_347" },		/* 347 */
	{ 5,	0,	printargs,	"SYS_348" },		/* 348 */
	{ 5,	0,	printargs,	"SYS_349" },		/* 349 */
	{ 5,	0,	printargs,	"SYS_350" },		/* 350 */
	{ 5,	0,	printargs,	"SYS_351" },		/* 351 */
	{ 5,	0,	printargs,	"SYS_352" },		/* 352 */
