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
	{ 2,	0,	printargs,	"perfctr" },		/* 18 */
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
	{ 2,	0,	sys_stty,	"stty" },		/* 31 */
	{ 2,	0,	sys_gtty,	"gtty" },		/* 32 */
	{ 2,	TF,	sys_access,	"access" },		/* 33 */
	{ 1,	0,	sys_nice,	"nice" },		/* 34 */
	{ 1,	0,	sys_ftime,	"ftime" },		/* 35 */
	{ 0,	0,	sys_sync,	"sync" },		/* 36 */
	{ 2,	TS,	sys_kill,	"kill" },		/* 37 */
	{ 2,	TF,	sys_stat,	"stat" },		/* 38 */
	{ 4,	TF,	sys_sendfile,	"sendfile" },		/* 39 */
	{ 2,	TF,	sys_lstat,	"lstat" },		/* 40 */
	{ 2,	0,	sys_dup,	"dup" },		/* 41 */
	{ 0,	0,	sys_pipe,	"pipe" },		/* 42 */
	{ 1,	0,	sys_times,	"times" },		/* 43 */
	{ 4,	0,	sys_profil,	"profil" },		/* 44 */
	{ 2,	TF,	sys_umount2,	"umount" },		/* 45 */
	{ 1,	0,	sys_setgid,	"setgid" },		/* 46 */
	{ 0,	0,	sys_getgid,	"getgid" },		/* 47 */
	{ 2,	0,	sys_signal,	"signal" },		/* 48 */
	{ 0,	0,	sys_geteuid,	"geteuid" },		/* 49 */
	{ 0,	0,	sys_getegid,	"getegid" },		/* 50 */
	{ 1,	0,	sys_acct,	"acct" },		/* 51 */
	{ 0,	0,	printargs,	"SYS_52" },		/* 52 */
	{ 4,	0,	sys_mctl,	"mctl" },		/* 53 */
	{ 3,	0,	sys_ioctl,	"ioctl" },		/* 54 */
	{ 2,	0,	sys_reboot,	"reboot" },		/* 55 */
	{ 3,	0,	printargs,	"SYS_56" },		/* 56 */
	{ 2,	TF,	sys_symlink,	"symlink" },		/* 57 */
	{ 3,	TF,	sys_readlink,	"readlink" },		/* 58 */
	{ 3,	TF|TP,	sys_execve,	"execve" },		/* 59 */
	{ 1,	0,	sys_umask,	"umask" },		/* 60 */
	{ 1,	TF,	sys_chroot,	"chroot" },		/* 61 */
	{ 2,	0,	sys_fstat,	"fstat" },		/* 62 */
	{ 2,	TF,	sys_fstat64,	"fstat64" },		/* 63 */
	{ 1,	0,	sys_getpagesize,"getpagesize" },	/* 64 */
	{ 3,	0,	sys_msync,	"msync" },		/* 65 */
	{ 0,	TP,	sys_vfork,	"vfork" },		/* 66 */
	{ 5,	TF,	sys_pread,	"pread" },		/* 67 */
	{ 5,	TF,	sys_pwrite,	"pwrite" },		/* 68 */
	{ 1,    0,	sys_sbrk,	"sbrk" },		/* 69 */
	{ 1,	0,	printargs,	"sstk" },		/* 70 */
	{ 6,	0,	sys_mmap,	"mmap" },		/* 71 */
	{ 1,	0,	printargs,	"vadvise" },		/* 72 */
	{ 2,	0,	sys_munmap,	"munmap" },		/* 73 */
	{ 3,	0,	sys_mprotect,	"mprotect" },		/* 74 */
	{ 3,	0,	sys_madvise,	"madvise" },		/* 75 */
	{ 1,	0,	sys_vhangup,	"vhangup" },		/* 76 */
	{ 2,	TF,	printargs,	"truncate64" },		/* 77 */
	{ 3,	0,	sys_mincore,	"mincore" },		/* 78 */
	{ 2,	0,	sys_getgroups,	"getgroups" },		/* 79 */
	{ 2,	0,	sys_setgroups,	"setgroups" },		/* 80 */
	{ 1,	0,	sys_getpgrp,	"getpgrp" },		/* 81 */
	{ 2,	0,	sys_setpgrp,	"setpgrp" },		/* 82 */
	{ 3,	0,	sys_setitimer,	"setitimer" },		/* 83 */
	{ 0,	0,	printargs,	"SYS_84" },		/* 84 */
	{ 1,	TF,	sys_swapon,	"swapon" },		/* 85 */
	{ 2,	0,	sys_getitimer,	"getitimer" },		/* 86 */
	{ 2,	0,	sys_gethostname,"gethostname" },	/* 87 */
	{ 2,	0,	sys_sethostname,"sethostname" },	 /* 88 */
	{ 0,	0,	sys_getdtablesize,"getdtablesize" },	 /* 89 */
	{ 2,	0,	sys_dup2,	"dup2" },		/* 90 */
	{ 2,	0,	printargs,	"getdopt" },		/* 91 */
	{ 3,	0,	sys_fcntl,	"fcntl" },		/* 92 */
	{ 5,	0,	sys_oldselect,	"select" },		/* 93 */
	{ 2,	0,	printargs,	"setdopt" },		/* 94 */
	{ 1,	0,	sys_fsync,	"fsync" },		/* 95 */
	{ 3,	0,	sys_setpriority,"setpriority" },	/* 96 */
	{ 3,	TN,	sys_socket,	"socket" },		/* 97 */
	{ 3,	TN,	sys_connect,	"connect" },		/* 98 */
	{ 3,	TN,	sys_accept,	"accept" },		/* 99 */
	{ 2,	0,	sys_getpriority,"getpriority" },	/* 100 */
	{ 0,	TS,	printargs,	"rt_sigreturn" },	/* 101 */
	{ 5,	TS,	sys_rt_sigaction,"rt_sigaction" },	/* 102 */
	{ 4,	TS,	sys_rt_sigprocmask,"rt_sigprocmask" },	/* 103 */
	{ 2,	TS,	sys_rt_sigpending,"rt_sigpending" },	/* 104 */
	{ 4,	TS,	sys_rt_sigtimedwait,"rt_sigtimedwait" },/* 105 */
	{ 3,	TS,	sys_rt_sigqueueinfo,"rt_sigqueueinfo" },/* 106 */
	{ 2,	TS,	sys_rt_sigsuspend,"rt_sigsuspend" },	/* 107 */
	{ 3,	TS,	printargs,	"sigvec" },		/* 108 */
	{ 1,    TS,	sys_sigblock,	"sigblock" },		/* 109 */
	{ 1,	TS,	sys_sigsetmask,	"sigsetmask" },		/* 110 */
	{ 1,	TS,	printargs,	"sigpause" },		/* 111 */
	{ 2,	TS,	printargs,	"sigstack" },		/* 112 */
	{ 3,	TN,	sys_recvmsg,	"recvmsg" },		/* 113 */
	{ 3,	TN,	sys_sendmsg,	"sendmsg" },		/* 114 */
	{ 3,	0,	printargs,	"vtrace" },		/* 115 */
	{ 2,	0,	sys_gettimeofday,"gettimeofday" },	/* 116 */
	{ 2,	0,	sys_getrusage,	"getrusage" },		/* 117 */
	{ 5,	TN,	sys_getsockopt,	"getsockopt" },		/* 118 */
	{ 2,	0,	sys_getcwd,	"getcwd" },		/* 119 */
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
	{ 5,	TN,	sys_socketpair,	"socketpair" },		/* 135 */
	{ 2,	TF,	sys_mkdir,	"mkdir" },		/* 136 */
	{ 1,	TF,	sys_rmdir,	"rmdir" },		/* 137 */
	{ 2,	TF,	sys_utimes,	"utimes" },		/* 138 */
	{ 2,	TF,	sys_stat64,	"stat64" },		/* 139 */
	{ 2,   0,	sys_adjtime,	"adjtime" },		/* 140 */
	{ 3,	TN,	sys_getpeername,"getpeername" },	/* 141 */
	{ 2,   0,	sys_gethostid,	"gethostid" },		/* 142 */
	{ 0,	0,	printargs,	"SYS_143" },		/* 143 */
	{ 2,	0,	sys_getrlimit,	"getrlimit" },		/* 144 */
	{ 2,	0,	sys_setrlimit,	"setrlimit" },		/* 145 */
	{ 2,	TF,	sys_pivotroot,	"pivot_root" },		/* 146 */
/*	{ 2,	TS,	sys_killpg,	"killpg" },		   146 SunOS killpkg, overridden by Linux pivot_root */
	{ 5,	0,	printargs,	"prctl" },		/* 147 */
	{ 5,	0,	printargs,	"pciconfig_read" },	/* 148 */
	{ 5,	0,	printargs,	"pciconfig_write" },	/* 149 */
	{ 3,	TN,	sys_getsockname,"getsockname" },	/* 150 */
	{ 4,	TN,	sys_getmsg,	"getmsg" },		/* 151 */
	{ 4,	TN,	sys_putmsg,	"putmsg" },		/* 152 */
	{ 3,	0,	sys_poll,	"poll" },		/* 153 */
	{ 4,	0,	printargs,	"getdents64" },		/* 154 */
	{ 1,	0,	printargs,	"nfssvc" },		/* 155 */
	{ 4,	0,	printargs,	"getdirentries" },	/* 156 */
	{ 2,	TF,	sys_statfs,	"statfs" },		/* 157 */
	{ 2,	0,	sys_fstatfs,	"fstatfs" },		/* 158 */
	{ 1,	TF,	sys_umount,	"oldumount" },		/* 159 */
	{ 0,	0,	printargs,	"async_daemon" },	/* 160 */
	{ 2,	0,	printargs,	"getfh" },		/* 161 */
	{ 2,	0,	printargs,	"getdomainname" },	/* 162 */
	{ 2,	0,	sys_setdomainname,"setdomainname" },	/* 163 */
	{ 5,	0,	printargs,	"SYS_164" },		/* 164 */
	{ 4,	0,	sys_quotactl,	"quotactl" },		/* 165 */
	{ 2,	0,	printargs,	"exportfs" },		/* 166 */
	{ 4,	TF,	sys_mount,	"mount" },		/* 167 */
	{ 2,	0,	sys_ustat,	"ustat" },		/* 168 */
	{ 5,	TI,	printargs,	"semsys" },		/* 169 */
	{ 5,	TI,	printargs,	"msgsys" },		/* 170 */
	{ 5,	TI,	printargs,	"shmsys" },		/* 171 */
	{ 4,	0,	printargs,	"auditsys" },		/* 172 */
	{ 5,	0,	printargs,	"rfssys" },		/* 173 */
	{ 3,	0,	sys_getdents,	"getdents" },		/* 174 */
	{ 1,	0,	sys_setsid,	"setsid" },		/* 175 */
	{ 1,	0,	sys_fchdir,	"fchdir" },		/* 176 */
	{ 1,    0,	sys_fchroot,	"fchroot" },		/* 177 */
	{ 2,	0,	printargs,	"vpixsys" },		/* 178 */
	{ 6,	0,	printargs,	"aioread" },		/* 179 */
	{ 6,	0,	printargs,	"aiowrite" },		/* 180 */
	{ 1,	0,	printargs,	"aiowait" },		/* 181 */
	{ 1,	0,	printargs,	"aiocancel" },		/* 182 */
	{ 1,	TS,	sys_sigpending,	"sigpending" },		/* 183 */
	{ 5,	0,	sys_query_module,"query_module" },	/* 184 */
	{ 2,	0,	sys_setpgid,	"setpgid" },		/* 185 */
	{ 2,	TF,	printargs,	"pathconf" },		/* 186 */
	{ 2,	0,	printargs,	"fpathconf" },		/* 187 */
	{ 1,	0,	printargs,	"sysconf" },		/* 188 */
	{ 1,	0,	sys_uname,	"uname" },		/* 189 */

	    /* Linux only system calls */

	{ 4,	0,	sys_init_module,"init_module" },	/* 190 */
	{ 1,	0,	sys_personality,"personality" },	/* 191 */
	{ 0,	0,	sys_prof,	"prof" },		/* 192 */
	{ 0,	0,	sys_break,	"break" },		/* 193 */
	{ 0,	0,	sys_lock,	"lock" },		/* 194 */
	{ 0,	0,	sys_mpx,	"mpx" },		/* 195 */
	{ 2,	0,	sys_ulimit,	"ulimit" },		/* 196 */
	{ 0,	0,	sys_getppid,	"getppid" },		/* 197 */
	{ 3,	TS,	sys_sigaction,	"sigaction" },		/* 198 */
	{ 5,	0,	printargs,	"sgetmask" },		/* 199 */
	{ 5,	0,	printargs,	"ssetmask" },		/* 200 */
	{ 3,	TS,	sys_sigsuspend,	"sigsuspend" },		/* 201 */
	{ 2,	TF,	sys_lstat,	"lstat" },		/* 202 */
	{ 1,	TF,	sys_uselib,	"uselib" },		/* 203 */
	{ 3,	0,	sys_readdir,	"readdir" },		/* 204 */
	{ 3,	0,	sys_ioperm,	"ioperm" },		/* 205 */
	{ 2,	0,	sys_socketcall,	"socketcall" },		/* 206 */
	{ 3,	0,	sys_syslog,	"syslog" },		/* 207 */
	{ 1,	0,	sys_olduname,	"olduname" },		/* 208 */
	{ 1,	0,	sys_iopl,	"iopl" },		/* 209 */
	{ 0,	0,	sys_idle,	"idle" },		/* 210 */
	{ 1,	0,	sys_vm86old,	"vm86" },		/* 211 */
	{ 3,	TP,	sys_waitpid,	"waitpid" },		/* 212 */
	{ 1,	0,	sys_swapoff,	"swapoff" },		/* 213 */
	{ 1,	0,	sys_sysinfo,	"sysinfo" },		/* 214 */
	{ 5,	0,	sys_ipc,	"ipc" },		/* 215 */
	{ 1,	TS,	sys_sigreturn,	"sigreturn" },		/* 216 */
	{ 2,	TP,	sys_clone,	"clone" },		/* 217 */
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
	{ 2,	TF,	printargs,	"nis_syscall" },	/* 232 */
	{ 1,	0,	sys_stime,	"stime" },		/* 233 */
	{ 2,	0,	printargs,	"nis_syscall" },	/* 234 */
	{ 2,	0,	printargs,	"nis_syscall" },	/* 235 */
	{ 5,	0,	printargs,	"_llseek" },		/* 236 */
	{ 5,	0,	sys_mlock,	"mlock" },		/* 237 */
	{ 5,	0,	sys_munlock,	"munlock" },		/* 238 */
	{ 5,	0,	sys_mlockall,	"mlockall" },		/* 239 */
	{ 0,	0,	sys_munlockall,	"munlockall" },		/* 240 */
	{ 5,	0,	sys_sched_setparam,"sched_setparam"},	/* 241 */
	{ 5,	0,	sys_sched_getparam,"sched_getparam"},	/* 242 */
{ 5,	0,	sys_sched_setscheduler,"sched_setscheduler"},	/* 243 */
{ 5,	0,	sys_sched_getscheduler,"sched_getscheduler"},	/* 244 */
{ 5,	0,	sys_sched_yield,"sched_yield" },		/* 245 */
{ 5,	0,sys_sched_get_priority_max,"sched_get_priority_max"},	/* 246 */
{ 5,	0,sys_sched_get_priority_min,"sched_get_priority_min"},	/* 247 */
{ 5,	0,sys_sched_rr_get_interval,"sched_rr_get_interval"},	/* 248 */
	{ 5,	0,	sys_nanosleep,	"nanosleep" },		/* 249 */
	{ 5,	0,	sys_mremap,	"mremap" },		/* 250 */
	{ 5,	0,	sys_sysctl,	"_sysctl" },		/* 251 */
	{ 5,	0,	sys_getsid,	"getsid" },		/* 252 */
	{ 5,	0,	sys_fdatasync,	"fdatasync" },		/* 253 */
	{ 5,	0,	printargs,	"nfsservctl" },		/* 254 */
	{ 5,	0,	printargs,	"aplib" },		/* 255 */
	{ 5,	0,	printargs,	"nis_syscall" },	/* 256 */
	{ 5,	0,	printargs,	"SYS_257" },		/* 257 */
	{ 5,	0,	printargs,	"SYS_258" },		/* 258 */
	{ 5,	0,	printargs,	"SYS_259" },		/* 259 */
	{ 5,	0,	printargs,	"SYS_260" },		/* 260 */
	{ 5,	0,	printargs,	"SYS_261" },		/* 261 */
	{ 5,	0,	printargs,	"SYS_262" },		/* 262 */
	{ 5,	0,	printargs,	"SYS_263" },		/* 263 */
	{ 5,	0,	printargs,	"SYS_264" },		/* 264 */
	{ 5,	0,	printargs,	"SYS_265" },		/* 265 */
	{ 5,	0,	printargs,	"SYS_266" },		/* 266 */
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

