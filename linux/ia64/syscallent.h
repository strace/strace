/*
 * Copyright (c) 1999, 2001 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
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

/*
 * IA-32 syscalls that have pointer arguments which are incompatible
 * with 64-bit layout get redirected to printargs.
 */
#define sys_getrlimit		printargs
#define sys_afs_syscall		printargs
#define sys_getpmsg		printargs
#define sys_putpmsg		printargs
#define sys_ugetrlimit		printargs
#define sys_waitpid		printargs
#define sys_time		printargs
#define sys_break		printargs
#define sys_oldstat		printargs
#define sys_lseek		printargs
#undef sys_stime
#define sys_stime		printargs
#define sys_ptrace		printargs
#define sys_oldfstat		printargs
#define sys_pause		printargs
#define sys_utime		printargs
#define sys_stty		printargs
#define sys_gtty		printargs
#define sys_ftime		printargs
#define sys_pipe		printargs
#define sys_times		printargs
#define sys_prof		printargs
#define sys_signal		printargs
#define sys_lock		printargs
#define sys_ioctl		printargs
#define sys_fcntl		printargs
#define sys_mpx			printargs
#define sys_ulimit		printargs
#define sys_oldolduname		printargs
#define sys_sigaction		printargs
#define sys_siggetmask		printargs
#define sys_sigsetmask		printargs
#define sys_sigsuspend		printargs
#define sys_sigpending		printargs
#define sys_setrlimit		printargs
#define sys_getrusage		printargs
#define sys_gettimeofday	printargs
#define sys_settimeofday	printargs
#define sys_getgroups		printargs
#define sys_setgroups		printargs
#define sys_select		printargs
#undef sys_oldlstat
#define sys_oldlstat		printargs
#define sys_readdir		printargs
#define sys_profil		printargs
#define sys_statfs		printargs
#define sys_fstatfs		printargs
#define sys_ioperm		printargs
#define sys_setitimer		printargs
#define sys_getitimer		printargs
#define sys_stat		printargs
#undef sys_lstat
#define sys_lstat		printargs
#define sys_fstat		printargs
#define sys_olduname		printargs
#define sys_iopl		printargs
#define sys_idle		printargs
#define sys_vm86old		printargs
#define sys_wait4		printargs
#define sys_sysinfo		printargs
#define sys_sigreturn		printargs
#define sys_uname		printargs
#define sys_modify_ldt		printargs
#define sys_adjtimex		printargs
#define sys_sigprocmask		printargs
#define sys_create_module	printargs
#define sys_init_module		printargs
#define sys_get_kernel_syms	printargs
#define sys_quotactl		printargs
#define sys_bdflush		printargs
#define sys_personality		printargs
#define sys_getdents		printargs
#define sys__newselect		printargs
#define sys_msync		printargs
#define sys_readv		printargs
#define sys_writev		printargs
#define sys__sysctl		printargs
#define sys_sched_rr_get_interval printargs
#define sys_getresuid		printargs
#define sys_vm86		printargs
#define sys_query_module	printargs
#define sys_nfsservctl		printargs
#define sys_rt_sigreturn	printargs
#define sys_rt_sigaction	printargs
#define sys_rt_sigprocmask	printargs
#define sys_rt_sigtimedwait	printargs
#define sys_rt_sigqueueinfo	printargs
#define sys_rt_sigsuspend	printargs
#define sys_pread		printargs
#define sys_pwrite		printargs
#define sys_sigaltstack		printargs
#define sys_sendfile		printargs
#define sys_truncate64		printargs
#define sys_ftruncate64		printargs
#define sys_stat64		printargs
#undef sys_lstat64
#define sys_lstat64		printargs
#define sys_fstat64		printargs
#define sys_fcntl64		printargs

#include "../i386/syscallent.h"

#undef sys_getrlimit
#undef sys_afs_syscall
#undef sys_getpmsg
#undef sys_putpmsg
#undef sys_ugetrlimit
#undef sys_waitpid
#undef sys_time
#undef sys_break
#undef sys_oldstat
#undef sys_lseek
#undef sys_stime
#undef sys_ptrace
#undef sys_oldfstat
#undef sys_pause
#undef sys_utime
#undef sys_stty
#undef sys_gtty
#undef sys_ftime
#undef sys_pipe
#undef sys_times
#undef sys_prof
#undef sys_signal
#undef sys_lock
#undef sys_ioctl
#undef sys_fcntl
#undef sys_mpx
#undef sys_ulimit
#undef sys_oldolduname
#undef sys_sigaction
#undef sys_siggetmask
#undef sys_sigsetmask
#undef sys_sigsuspend
#undef sys_sigpending
#undef sys_setrlimit
#undef sys_getrusage
#undef sys_gettimeofday
#undef sys_settimeofday
#undef sys_getgroups
#undef sys_setgroups
#undef sys_select
#undef sys_oldlstat
#undef sys_readdir
#undef sys_profil
#undef sys_statfs
#undef sys_fstatfs
#undef sys_ioperm
#undef sys_setitimer
#undef sys_getitimer
#undef sys_stat
#undef sys_lstat
#undef sys_fstat
#undef sys_olduname
#undef sys_iopl
#undef sys_idle
#undef sys_vm86old
#undef sys_wait4
#undef sys_sysinfo
#undef sys_sigreturn
#undef sys_uname
#undef sys_modify_ldt
#undef sys_adjtimex
#undef sys_sigprocmask
#undef sys_create_module
#undef sys_init_module
#undef sys_get_kernel_syms
#undef sys_quotactl
#undef sys_bdflush
#undef sys_personality
#undef sys_getdents
#undef sys__newselect
#undef sys_msync
#undef sys_readv
#undef sys_writev
#undef sys__sysctl
#undef sys_sched_rr_get_interval
#undef sys_getresuid
#undef sys_vm86
#undef sys_query_module
#undef sys_nfsservctl
#undef sys_rt_sigreturn
#undef sys_rt_sigaction
#undef sys_rt_sigprocmask
#undef sys_rt_sigtimedwait
#undef sys_rt_sigqueueinfo
#undef sys_rt_sigsuspend
#undef sys_pread
#undef sys_pwrite
#undef sys_sigaltstack
#undef sys_sendfile
#undef sys_truncate64
#undef sys_ftruncate64
#undef sys_stat64
#undef sys_lstat64
#undef sys_fstat64
#undef sys_fcntl64

#include "../dummy.h"

/* You must be careful to check ../i386/syscallent.h so that this table
   starts where that one leaves off.
*/
[(SYS_ipc_subcall + SYS_ipc_nsubcalls) ... 1023] = { },
[1024] = { 0,	0,		printargs,			"ni_syscall"		},
[1025] = { 1,	TP|SE,		sys_exit,			"exit"			},
[1026] = { 3,	TD,		sys_read,			"read"			},
[1027] = { 3,	TD,		sys_write,			"write"			},
[1028] = { 3,	TD|TF,		sys_open,			"open"			},
[1029] = { 1,	TD,		sys_close,			"close"			},
[1030] = { 2,	TD|TF,		sys_creat,			"creat"			},
[1031] = { 2,	TF,		sys_link,			"link"			},
[1032] = { 1,	TF,		sys_unlink,			"unlink"		},
[1033] = { 3,	TF|TP|SE|SI,	sys_execve,			"execve"		},
[1034] = { 1,	TF,		sys_chdir,			"chdir"			},
[1035] = { 1,	TD,		sys_fchdir,			"fchdir"		},
[1036] = { 2,	TF,		sys_utimes,			"utimes"		},
[1037] = { 3,	TF,		sys_mknod,			"mknod"			},
[1038] = { 2,	TF,		sys_chmod,			"chmod"			},
[1039] = { 3,	TF,		sys_chown,			"chown"			},
[1040] = { 3,	TD,		sys_lseek,			"lseek"			},
[1041] = { 0,	0,		sys_getpid,			"getpid"		},
[1042] = { 0,	0,		sys_getppid,			"getppid"		},
[1043] = { 5,	TF,		sys_mount,			"mount"			},
[1044] = { 2,	TF,		sys_umount2,			"umount"		},
[1045] = { 1,	0,		sys_setuid,			"setuid"		},
[1046] = { 0,	NF,		sys_getuid,			"getuid"		},
[1047] = { 0,	NF,		sys_geteuid,			"geteuid"		},
[1048] = { 4,	0,		sys_ptrace,			"ptrace"		},
[1049] = { 2,	TF,		sys_access,			"access"		},
[1050] = { 0,	0,		sys_sync,			"sync"			},
[1051] = { 1,	TD,		sys_fsync,			"fsync"			},
[1052] = { 1,	TD,		sys_fdatasync,			"fdatasync"		},
[1053] = { 2,	TS,		sys_kill,			"kill"			},
[1054] = { 2,	TF,		sys_rename,			"rename"		},
[1055] = { 2,	TF,		sys_mkdir,			"mkdir"			},
[1056] = { 1,	TF,		sys_rmdir,			"rmdir"			},
[1057] = { 1,	TD,		sys_dup,			"dup"			},
[1058] = { 1,	TD,		sys_pipe,			"pipe"			},
[1059] = { 1,	0,		sys_times,			"times"			},
[1060] = { 1,	TM|SI,		sys_brk,			"brk"			},
[1061] = { 1,	0,		sys_setgid,			"setgid"		},
[1062] = { 0,	NF,		sys_getgid,			"getgid"		},
[1063] = { 0,	NF,		sys_getegid,			"getegid"		},
[1064] = { 1,	TF,		sys_acct,			"acct"			},
[1065] = { 3,	TD,		sys_ioctl,			"ioctl"			},
[1066] = { 3,	TD,		sys_fcntl,			"fcntl"			},
[1067] = { 1,	0,		sys_umask,			"umask"			},
[1068] = { 1,	TF,		sys_chroot,			"chroot"		},
[1069] = { 2,	0,		sys_ustat,			"ustat"			},
[1070] = { 2,	TD,		sys_dup2,			"dup2"			},
[1071] = { 2,	0,		sys_setreuid,			"setreuid"		},
[1072] = { 2,	0,		sys_setregid,			"setregid"		},
[1073] = { 3,	0,		sys_getresuid,			"getresuid"		},
[1074] = { 3,	0,		sys_setresuid,			"setresuid"		},
[1075] = { 3,	0,		sys_getresgid,			"getresgid"		},
[1076] = { 3,	0,		sys_setresgid,			"setresgid"		},
[1077] = { 2,	0,		sys_getgroups,			"getgroups"		},
[1078] = { 2,	0,		sys_setgroups,			"setgroups"		},
[1079] = { 1,	0,		sys_getpgid,			"getpgid"		},
[1080] = { 2,	0,		sys_setpgid,			"setpgid"		},
[1081] = { 0,	0,		sys_setsid,			"setsid"		},
[1082] = { 1,	0,		sys_getsid,			"getsid"		},
[1083] = { 2,	0,		sys_sethostname,		"sethostname"		},
[1084] = { 2,	0,		sys_setrlimit,			"setrlimit"		},
[1085] = { 2,	0,		sys_getrlimit,			"getrlimit"		},
[1086] = { 2,	0,		sys_getrusage,			"getrusage"		},
[1087] = { 2,	0,		sys_gettimeofday,		"gettimeofday"		},
[1088] = { 2,	0,		sys_settimeofday,		"settimeofday"		},
[1089] = { 5,	TD,		sys_select,			"select"		},
[1090] = { 3,	TD,		sys_poll,			"poll"			},
[1091] = { 2,	TF,		sys_symlink,			"symlink"		},
[1092] = { 3,	TF,		sys_readlink,			"readlink"		},
[1093] = { 1,	TF,		sys_uselib,			"uselib"		},
[1094] = { 2,	TF,		sys_swapon,			"swapon"		},
[1095] = { 1,	TF,		sys_swapoff,			"swapoff"		},
[1096] = { 4,	0,		sys_reboot,			"reboot"		},
[1097] = { 2,	TF,		sys_truncate,			"truncate"		},
[1098] = { 2,	TD,		sys_ftruncate,			"ftruncate"		},
[1099] = { 2,	TD,		sys_fchmod,			"fchmod"		},
[1100] = { 3,	TD,		sys_fchown,			"fchown"		},
[1101] = { 2,	0,		sys_getpriority,		"getpriority"		},
[1102] = { 3,	0,		sys_setpriority,		"setpriority"		},
[1103] = { 2,	TF,		sys_statfs,			"statfs"		},
[1104] = { 2,	TD,		sys_fstatfs,			"fstatfs"		},
[1105] = { 3,	0,		sys_gettid,			"gettid"		},
[1106] = { 3,	TI,		sys_semget,			"semget"		},
[1107] = { 3,	TI,		printargs,			"semop"			},
[1108] = { 4,	TI,		sys_semctl,			"semctl"		},
[1109] = { 2,	TI,		sys_msgget,			"msgget"		},
[1110] = { 4,	TI,		sys_msgsnd,			"msgsnd"		},
[1111] = { 5,	TI,		sys_msgrcv,			"msgrcv"		},
[1112] = { 3,	TI,		sys_msgctl,			"msgctl"		},
[1113] = { 3,	TI,		sys_shmget,			"shmget"		},
[1114] = { 3,	TI|TM|SI,	sys_shmat,			"shmat"			},
[1115] = { 1,	TI|TM|SI,	sys_shmdt,			"shmdt"			},
[1116] = { 3,	TI,		sys_shmctl,			"shmctl"		},
[1117] = { 3,	0,		sys_syslog,			"syslog"		},
[1118] = { 3,	0,		sys_setitimer,			"setitimer"		},
[1119] = { 2,	0,		sys_getitimer,			"getitimer"		},
[1120] = { 2,	TF,		sys_stat,			"stat"			},
[1121] = { 2,	TF,		sys_lstat,			"lstat"			},
[1122] = { 2,	TD,		sys_fstat,			"fstat"			},
[1123] = { 0,	0,		sys_vhangup,			"vhangup"		},
[1124] = { 3,	TF,		sys_chown,			"lchown"		},
[1125] = { 5,	0,		sys_vm86,			"vm86"			},
[1126] = { 4,	TP,		sys_wait4,			"wait4"			},
[1127] = { 1,	0,		sys_sysinfo,			"sysinfo"		},
[1128] = { 5,	TP,		sys_clone,			"clone"			},
[1129] = { 2,	0,		sys_setdomainname,		"setdomainname"		},
[1130] = { 1,	0,		sys_uname,			"uname"			},
[1131] = { 1,	0,		sys_adjtimex,			"adjtimex"		},
[1132] = { 2,	0,		sys_create_module,		"create_module"		},
[1133] = { 4,	0,		sys_init_module,		"init_module"		},
[1134] = { 2,	0,		sys_delete_module,		"delete_module"		},
[1135] = { 1,	0,		sys_get_kernel_syms,		"get_kernel_syms"	},
[1136] = { 5,	0,		sys_query_module,		"query_module"		},
[1137] = { 4,	TF,		sys_quotactl,			"quotactl"		},
[1138] = { 0,	0,		sys_bdflush,			"bdflush"		},
[1139] = { 3,	0,		sys_sysfs,			"sysfs"			},
[1140] = { 1,	0,		sys_personality,		"personality"		},
[1141] = { 5,	0,		sys_afs_syscall,		"afs_syscall"		},
[1142] = { 1,	NF,		sys_setfsuid,			"setfsuid"		},
[1143] = { 1,	NF,		sys_setfsgid,			"setfsgid"		},
[1144] = { 3,	TD,		sys_getdents,			"getdents"		},
[1145] = { 2,	TD,		sys_flock,			"flock"			},
[1146] = { 5,	TD,		sys_readv,			"readv"			},
[1147] = { 5,	TD,		sys_writev,			"writev"		},
[1148] = { 4,	TD,		sys_pread,			"pread"			},
[1149] = { 4,	TD,		sys_pwrite,			"pwrite"		},
[1150] = { 1,	0,		printargs,			"_sysctl"		},
[1151] = { 6,	TD|TM|SI,	sys_mmap,			"mmap"			},
[1152] = { 2,	TM|SI,		sys_munmap,			"munmap"		},
[1153] = { 2,	TM,		sys_mlock,			"mlock"			},
[1154] = { 1,	TM,		sys_mlockall,			"mlockall"		},
[1155] = { 3,	TM|SI,		sys_mprotect,			"mprotect"		},
[1156] = { 5,	TM|SI,		sys_mremap,			"mremap"		},
[1157] = { 3,	TM,		sys_msync,			"msync"			},
[1158] = { 2,	TM,		sys_munlock,			"munlock"		},
[1159] = { 0,	TM,		sys_munlockall,			"munlockall"		},
[1160] = { 2,	0,		sys_sched_getparam,		"sched_getparam"	},
[1161] = { 2,	0,		sys_sched_setparam,		"sched_setparam"	},
[1162] = { 2,	0,		sys_sched_getscheduler,		"sched_getscheduler"	},
[1163] = { 3,	0,		sys_sched_setscheduler,		"sched_setscheduler"	},
[1164] = { 0,	0,		sys_sched_yield,		"sched_yield"		},
[1165] = { 1,	0,		sys_sched_get_priority_max,	"sched_get_priority_max"},
[1166] = { 1,	0,		sys_sched_get_priority_min,	"sched_get_priority_min"},
[1167] = { 2,	0,		sys_sched_rr_get_interval,	"sched_rr_get_interval"	},
[1168] = { 2,	0,		sys_nanosleep,			"nanosleep"		},
[1169] = { 3,	0,		sys_nfsservctl,			"nfsservctl"		},
[1170] = { 5,	0,		sys_prctl,			"prctl"			},
[1171] = { 1,	0,		sys_getpagesize,		"getpagesize"		},
[1172] = { 6,	TD|TM|SI,	sys_mmap_pgoff,			"mmap2"			},
[1173] = { 5,	0,		printargs,			"pciconfig_read"	},
[1174] = { 5,	0,		printargs,			"pciconfig_write"	},
[1175] = { MA,	0,		printargs,			"perfmonctl"		},
[1176] = { 2,	TS,		sys_sigaltstack,		"sigaltstack"		},
[1177] = { 4,	TS,		sys_rt_sigaction,		"rt_sigaction"		},
[1178] = { 2,	TS,		sys_rt_sigpending,		"rt_sigpending"		},
[1179] = { 4,	TS,		sys_rt_sigprocmask,		"rt_sigprocmask"	},
[1180] = { 3,	TS,		sys_rt_sigqueueinfo,		"rt_sigqueueinfo"	},
[1181] = { 0,	TS,		sys_sigreturn,			"rt_sigreturn"		},
[1182] = { 2,	TS,		sys_rt_sigsuspend,		"rt_sigsuspend"		},
[1183] = { 4,	TS,		sys_rt_sigtimedwait,		"rt_sigtimedwait"	},
[1184] = { 2,	TF,		sys_getcwd,			"getcwd"		},
[1185] = { 2,	0,		sys_capget,			"capget"		},
[1186] = { 2,	0,		sys_capset,			"capset"		},
[1187] = { 4,	TD|TN,		sys_sendfile,			"sendfile"		},
[1188] = { 5,	TN,		printargs,			"getpmsg"		},
[1189] = { 5,	TN,		printargs,			"putpmsg"		},
[1190] = { 3,	TN,		sys_socket,			"socket"		},
[1191] = { 3,	TN,		sys_bind,			"bind"			},
[1192] = { 3,	TN,		sys_connect,			"connect"		},
[1193] = { 2,	TN,		sys_listen,			"listen"		},
[1194] = { 3,	TN,		sys_accept,			"accept"		},
[1195] = { 3,	TN,		sys_getsockname,		"getsockname"		},
[1196] = { 3,	TN,		sys_getpeername,		"getpeername"		},
[1197] = { 4,	TN,		sys_socketpair,			"socketpair"		},
[1198] = { 4,	TN,		sys_send,			"send"			},
[1199] = { 6,	TN,		sys_sendto,			"sendto"		},
[1200] = { 4,	TN,		sys_recv,			"recv"			},
[1201] = { 6,	TN,		sys_recvfrom,			"recvfrom"		},
[1202] = { 2,	TN,		sys_shutdown,			"shutdown"		},
[1203] = { 5,	TN,		sys_setsockopt,			"setsockopt"		},
[1204] = { 5,	TN,		sys_getsockopt,			"getsockopt"		},
[1205] = { 3,	TN,		sys_sendmsg,			"sendmsg"		},
[1206] = { 3,	TN,		sys_recvmsg,			"recvmsg"		},
[1207] = { 2,	TF,		sys_pivotroot,			"pivot_root"		},
[1208] = { 3,	TM,		sys_mincore,			"mincore"		},
[1209] = { 3,	TM,		sys_madvise,			"madvise"		},
[1210] = { 2,	TF,		sys_stat,			"stat"			},
[1211] = { 2,	TF,		sys_lstat,			"lstat"			},
[1212] = { 2,	TD,		sys_fstat,			"fstat"			},
[1213] = { 6,	TP,		sys_clone,			"clone2"		},
[1214] = { 3,	TD,		sys_getdents64,			"getdents64"		},
[1215] = { 2,	0,		printargs,			"getunwind"		},
[1216] = { 3,	TD,		sys_readahead,			"readahead"		},
[1217] = { 5,	TF,		sys_setxattr,			"setxattr"		},
[1218] = { 5,	TF,		sys_setxattr,			"lsetxattr"		},
[1219] = { 5,	TD,		sys_fsetxattr,			"fsetxattr"		},
[1220] = { 4,	TF,		sys_getxattr,			"getxattr"		},
[1221] = { 4,	TF,		sys_getxattr,			"lgetxattr"		},
[1222] = { 4,	TD,		sys_fgetxattr,			"fgetxattr"		},
[1223] = { 3,	TF,		sys_listxattr,			"listxattr"		},
[1224] = { 3,	TF,		sys_listxattr,			"llistxattr"		},
[1225] = { 3,	TD,		sys_flistxattr,			"flistxattr"		},
[1226] = { 2,	TF,		sys_removexattr,		"removexattr"		},
[1227] = { 2,	TF,		sys_removexattr,		"lremovexattr"		},
[1228] = { 2,	TD,		sys_fremovexattr,		"fremovexattr"		},
[1229] = { 2,	TS,		sys_kill,			"tkill"			},
[1230] = { 6,	0,		sys_futex,			"futex"			},
[1231] = { 3,	0,		sys_sched_setaffinity,		"sched_setaffinity"	},
[1232] = { 3,	0,		sys_sched_getaffinity,		"sched_getaffinity"	},
[1233] = { 1,	0,		sys_set_tid_address,		"set_tid_address"	},
[1234] = { 4,	TD,		sys_fadvise64,			"fadvise64"		},
[1235] = { 3,	TS,		sys_tgkill,			"tgkill"		},
[1236] = { 1,	TP|SE,		sys_exit,			"exit_group"		},
[1237] = { 3,	0,		sys_lookup_dcookie,		"lookup_dcookie"	},
[1238] = { 2,	0,		sys_io_setup,			"io_setup"		},
[1239] = { 1,	0,		sys_io_destroy,			"io_destroy"		},
[1240] = { 5,	0,		sys_io_getevents,		"io_getevents"		},
[1241] = { 3,	0,		sys_io_submit,			"io_submit"		},
[1242] = { 3,	0,		sys_io_cancel,			"io_cancel"		},
[1243] = { 1,	TD,		sys_epoll_create,		"epoll_create"		},
[1244] = { 4,	TD,		sys_epoll_ctl,			"epoll_ctl"		},
[1245] = { 4,	TD,		sys_epoll_wait,			"epoll_wait"		},
[1246] = { 0,	0,		sys_restart_syscall,		"restart_syscall"	},
[1247] = { 4,	TI,		sys_semtimedop,			"semtimedop"		},
[1248] = { 3,	0,		sys_timer_create,		"timer_create"		},
[1249] = { 4,	0,		sys_timer_settime,		"timer_settime"		},
[1250] = { 2,	0,		sys_timer_gettime,		"timer_gettime"		},
[1251] = { 1,	0,		sys_timer_getoverrun,		"timer_getoverrun"	},
[1252] = { 1,	0,		sys_timer_delete,		"timer_delete"		},
[1253] = { 2,	0,		sys_clock_settime,		"clock_settime"		},
[1254] = { 2,	0,		sys_clock_gettime,		"clock_gettime"		},
[1255] = { 2,	0,		sys_clock_getres,		"clock_getres"		},
[1256] = { 4,	0,		sys_clock_nanosleep,		"clock_nanosleep"	},
[1257] = { MA,	0,		printargs,			"fstatfs64"		},
[1258] = { MA,	0,		printargs,			"statfs64"		},
[1259] = { 6,	TM,		sys_mbind,			"mbind"			},
[1260] = { 5,	TM,		sys_get_mempolicy,		"get_mempolicy"		},
[1261] = { 3,	TM,		sys_set_mempolicy,		"set_mempolicy"		},
[1262] = { 4,	0,		sys_mq_open,			"mq_open"		},
[1263] = { 1,	0,		sys_mq_unlink,			"mq_unlink"		},
[1264] = { 5,	0,		sys_mq_timedsend,		"mq_timedsend"		},
[1265] = { 5,	0,		sys_mq_timedreceive,		"mq_timedreceive"	},
[1266] = { 2,	0,		sys_mq_notify,			"mq_notify"		},
[1267] = { 3,	0,		sys_mq_getsetattr,		"mq_getsetattr"		},
[1268] = { 4,	0,		sys_kexec_load,			"kexec_load"		},
[1269] = { 5,	0,		sys_vserver,			"vserver"		},
[1270] = { 5,	TP,		sys_waitid,			"waitid"		},
[1271] = { 5,	0,		sys_add_key,			"add_key"		},
[1272] = { 4,	0,		sys_request_key,		"request_key"		},
[1273] = { 5,	0,		sys_keyctl,			"keyctl"		},
[1274] = { 3,	0,		sys_ioprio_set,			"ioprio_set"		},
[1275] = { 2,	0,		sys_ioprio_get,			"ioprio_get"		},
[1276] = { 6,	TM,		sys_move_pages,			"move_pages"		},
[1277] = { 0,	TD,		sys_inotify_init,		"inotify_init"		},
[1278] = { 3,	TD,		sys_inotify_add_watch,		"inotify_add_watch"	},
[1279] = { 2,	TD,		sys_inotify_rm_watch,		"inotify_rm_watch"	},
[1280] = { 4,	TM,		sys_migrate_pages,		"migrate_pages"		},
[1281] = { 4,	TD|TF,		sys_openat,			"openat"		},
[1282] = { 3,	TD|TF,		sys_mkdirat,			"mkdirat"		},
[1283] = { 4,	TD|TF,		sys_mknodat,			"mknodat"		},
[1284] = { 5,	TD|TF,		sys_fchownat,			"fchownat"		},
[1285] = { 3,	TD|TF,		sys_futimesat,			"futimesat"		},
[1286] = { 4,	TD|TF,		sys_newfstatat,			"newfstatat"		},
[1287] = { 3,	TD|TF,		sys_unlinkat,			"unlinkat"		},
[1288] = { 4,	TD|TF,		sys_renameat,			"renameat"		},
[1289] = { 5,	TD|TF,		sys_linkat,			"linkat"		},
[1290] = { 3,	TD|TF,		sys_symlinkat,			"symlinkat"		},
[1291] = { 4,	TD|TF,		sys_readlinkat,			"readlinkat"		},
[1292] = { 3,	TD|TF,		sys_fchmodat,			"fchmodat"		},
[1293] = { 3,	TD|TF,		sys_faccessat,			"faccessat"		},
[1294] = { 6,	TD,		sys_pselect6,			"pselect6"		},
[1295] = { 5,	TD,		sys_ppoll,			"ppoll"			},
[1296] = { 1,	TP,		sys_unshare,			"unshare"		},
[1297] = { 2,	0,		sys_set_robust_list,		"set_robust_list"	},
[1298] = { 3,	0,		sys_get_robust_list,		"get_robust_list"	},
[1299] = { 6,	TD,		sys_splice,			"splice"		},
[1300] = { 4,	TD,		sys_sync_file_range,		"sync_file_range"	},
[1301] = { 4,	TD,		sys_tee,			"tee"			},
[1302] = { 4,	TD,		sys_vmsplice,			"vmsplice"		},
[1303] = { 4,	TD,		sys_fallocate,			"fallocate"		},
[1304] = { 3,	0,		sys_getcpu,			"getcpu"		},
[1305] = { 6,	TD,		sys_epoll_pwait,		"epoll_pwait"		},
[1306] = { },
[1307] = { 3,	TD|TS,		sys_signalfd,			"signalfd"		},
[1308] = { 4,	TD,		sys_timerfd,			"timerfd"		},
[1309] = { 1,	TD,		sys_eventfd,			"eventfd"		},
[1310] = { 2,	TD,		sys_timerfd_create,		"timerfd_create"	},
[1311] = { 4,	TD,		sys_timerfd_settime,		"timerfd_settime"	},
[1312] = { 2,	TD,		sys_timerfd_gettime,		"timerfd_gettime"	},
[1313] = { 4,	TD|TS,		sys_signalfd4,			"signalfd4"		},
[1314] = { 2,	TD,		sys_eventfd2,			"eventfd2"		},
[1315] = { 1,	TD,		sys_epoll_create1,		"epoll_create1"		},
[1316] = { 3,	TD,		sys_dup3,			"dup3"			},
[1317] = { 2,	TD,		sys_pipe2,			"pipe2"			},
[1318] = { 1,	TD,		sys_inotify_init1,		"inotify_init1"		},
[1319] = { 4,	TD,		sys_preadv,			"preadv"		},
[1320] = { 4,	TD,		sys_pwritev,			"pwritev"		},
[1321] = { 4,	TP|TS,		sys_rt_tgsigqueueinfo,		"rt_tgsigqueueinfo"	},
[1322] = { 5,	TN,		sys_recvmmsg,			"recvmmsg"		},
[1323] = { 2,	TD,		sys_fanotify_init,		"fanotify_init"		},
[1324] = { 5,	TD|TF,		sys_fanotify_mark,		"fanotify_mark"		},
[1325] = { 4,	0,		sys_prlimit64,			"prlimit64"		},
[1326] = { 5,	TD|TF,		sys_name_to_handle_at,		"name_to_handle_at"	},
[1327] = { 3,	TD,		sys_open_by_handle_at,		"open_by_handle_at"	},
[1328] = { 2,	0,		sys_clock_adjtime,		"clock_adjtime"		},
[1329] = { 1,	TD,		sys_syncfs,			"syncfs"		},
[1330] = { 2,	TD,		sys_setns,			"setns"			},
[1331] = { 4,	TN,		sys_sendmmsg,			"sendmmsg"		},
[1332] = { 6,	0,		sys_process_vm_readv,		"process_vm_readv"	},
[1333] = { 6,	0,		sys_process_vm_writev,		"process_vm_writev"	},
[1334] = { 4,	TN,		sys_accept4,			"accept4"		},
[1335] = { 3,	TD,		sys_finit_module,		"finit_module"		},
[1336] = { 3,	0,		sys_sched_setattr,		"sched_setattr"		},
[1337] = { 4,	0,		sys_sched_getattr,		"sched_getattr"		},
[1338] = { 5,	TD|TF,		sys_renameat2,			"renameat2"		},
[1339] = { 3,	0,		sys_getrandom,			"getrandom",		},
[1340] = { 2,	TD,		sys_memfd_create,		"memfd_create",		},
[1341] = { 3,	TD,		sys_bpf,			"bpf",			},
[1342] = { 5,	TD|TF|TP|SE|SI,	sys_execveat,			"execveat",		},
