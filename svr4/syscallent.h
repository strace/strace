/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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

#ifdef MIPS
	/* TODO: explain to non-mips people how this arch can have syscall #-1 */
	{ MA,	0,	printargs,		"SYS_-1"	}, /* -1 */
#endif /* MIPS */
	{ MA,	0,	sys_syscall,		"syscall"	}, /* 0 */
	{ MA,	TP,	sys_exit,		"_exit"		}, /* 1 */
	{ MA,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ MA,	TD,	sys_read,		"read"		}, /* 3 */
	{ MA,	TD,	sys_write,		"write"		}, /* 4 */
	{ MA,	TD|TF,	sys_open,		"open"		}, /* 5 */
	{ MA,	TD,	sys_close,		"close"		}, /* 6 */
	{ MA,	TP,	sys_wait,		"wait"		}, /* 7 */
	{ MA,	TD|TF,	sys_creat,		"creat"		}, /* 8 */
	{ MA,	TF,	sys_link,		"link"		}, /* 9 */
	{ MA,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ MA,	TF|TP,	sys_exec,		"exec"		}, /* 11 */
	{ MA,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ MA,	0,	sys_time,		"time"		}, /* 13 */
	{ MA,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ MA,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ MA,	TF,	sys_chown,		"chown"		}, /* 16 */
	{ MA,	0,	sys_brk,		"brk"		}, /* 17 */
	{ MA,	TF,	sys_stat,		"stat"		}, /* 18 */
	{ MA,	TD,	sys_lseek,		"lseek"		}, /* 19 */
	{ MA,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ MA,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ MA,	TF,	sys_umount,		"umount"	}, /* 22 */
	{ MA,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ MA,	0,	sys_getuid,		"getuid"	}, /* 24 */
	{ MA,	0,	sys_stime,		"stime"		}, /* 25 */
	{ MA,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ MA,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ MA,	TD,	sys_fstat,		"fstat"		}, /* 28 */
	{ MA,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ MA,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ MA,	0,	sys_stty,		"stty"		}, /* 31 */
	{ MA,	0,	sys_gtty,		"gtty"		}, /* 32 */
	{ MA,	TF,	sys_access,		"access"	}, /* 33 */
	{ MA,	0,	sys_nice,		"nice"		}, /* 34 */
	{ MA,	TF,	sys_statfs,		"statfs"	}, /* 35 */
	{ MA,	0,	sys_sync,		"sync"		}, /* 36 */
	{ MA,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ MA,	TD,	sys_fstatfs,		"fstatfs"	}, /* 38 */
#ifdef MIPS
	{ MA,	0,	sys_setpgrp,		"setpgrp"	}, /* 39 */
#else /* !MIPS */
	{ MA,	0,	sys_pgrpsys,		"pgrpsys"	}, /* 39 */
#endif /* !MIPS */
#ifdef MIPS
	{ MA,	0,	sys_syssgi,		"syssgi"	}, /* 40 */
#else /* !MIPS */
	{ MA,	0,	sys_xenix,		"xenix"		}, /* 40 */
#endif /* !MIPS */
	{ MA,	TD,	sys_dup,		"dup"		}, /* 41 */
	{ MA,	TD,	sys_pipe,		"pipe"		}, /* 42 */
	{ MA,	0,	sys_times,		"times"		}, /* 43 */
	{ MA,	0,	sys_profil,		"profil"	}, /* 44 */
	{ MA,	0,	sys_plock,		"plock"		}, /* 45 */
	{ MA,	0,	sys_setgid,		"setgid"	}, /* 46 */
	{ MA,	0,	sys_getgid,		"getgid"	}, /* 47 */
	{ MA,	0,	sys_sigcall,		"sigcall"	}, /* 48 */
	{ MA,	TI,	sys_msgsys,		"msgsys"	}, /* 49 */
#ifdef SPARC
	{ MA,	0,	sys_syssun,		"syssun"	}, /* 50 */
#else /* !SPARC */
#ifdef I386
	{ MA,	0,	sys_sysi86,		"sysi86"	}, /* 50 */
#else /* !I386 */
#ifdef MIPS
	{ MA,	0,	sys_sysmips,		"sysmips"	}, /* 50 */
#else /* !MIPS */
	{ MA,	0,	sys_sysmachine,		"sysmachine"	}, /* 50 */
#endif /* !MIPS */
#endif /* !I386 */
#endif /* !SPARC */
	{ MA,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ MA,	TI,	sys_shmsys,		"shmsys"	}, /* 52 */
	{ MA,	TI,	sys_semsys,		"semsys"	}, /* 53 */
	{ MA,	TD,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ MA,	0,	sys_uadmin,		"uadmin"	}, /* 55 */
	{ MA,	0,	sys_sysmp,		"sysmp"		}, /* 56 */
	{ MA,	0,	sys_utssys,		"utssys"	}, /* 57 */
	{ MA,	0,	sys_fdsync,		"fdsync"	}, /* 58 */
	{ MA,	TF|TP,	sys_execve,		"execve"	}, /* 59 */
	{ MA,	0,	sys_umask,		"umask"		}, /* 60 */
	{ MA,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ MA,	TD,	sys_fcntl,		"fcntl"		}, /* 62 */
	{ MA,	0,	sys_ulimit,		"ulimit"	}, /* 63 */
	{ MA,	0,	printargs,		"SYS_64"	}, /* 64 */
	{ MA,	0,	printargs,		"SYS_65"	}, /* 65 */
	{ MA,	0,	printargs,		"SYS_66"	}, /* 66 */
	{ MA,	0,	printargs,		"SYS_67"	}, /* 67 */
	{ MA,	0,	printargs,		"SYS_68"	}, /* 68 */
	{ MA,	0,	printargs,		"SYS_69"	}, /* 69 */
	{ MA,	0,	printargs,		"SYS_70"	}, /* 70 */
	{ MA,	0,	printargs,		"SYS_71"	}, /* 71 */
	{ MA,	0,	printargs,		"SYS_72"	}, /* 72 */
	{ MA,	0,	printargs,		"SYS_73"	}, /* 73 */
	{ MA,	0,	printargs,		"SYS_74"	}, /* 74 */
	{ MA,	0,	printargs,		"SYS_75"	}, /* 75 */
	{ MA,	0,	printargs,		"SYS_76"	}, /* 76 */
	{ MA,	0,	printargs,		"SYS_77"	}, /* 77 */
	{ MA,	0,	printargs,		"SYS_78"	}, /* 78 */
	{ MA,	TF,	sys_rmdir,		"rmdir"		}, /* 79 */
	{ MA,	TF,	sys_mkdir,		"mkdir"		}, /* 80 */
	{ MA,	TD,	sys_getdents,		"getdents"	}, /* 81 */
	{ MA,	0,	sys_sginap,		"sginap"	}, /* 82 */
	{ MA,	0,	sys_sgikopt,		"sgikopt"	}, /* 83 */
	{ MA,	0,	sys_sysfs,		"sysfs"		}, /* 84 */
	{ MA,	TN,	sys_getmsg,		"getmsg"	}, /* 85 */
	{ MA,	TN,	sys_putmsg,		"putmsg"	}, /* 86 */
	{ MA,	TN,	sys_poll,		"poll"		}, /* 87 */
#ifdef MIPS
	{ MA,	TS,	sys_sigreturn,		"sigreturn"	}, /* 88 */
	{ MA,	TN,	sys_accept,		"accept"	}, /* 89 */
	{ MA,	TN,	sys_bind,		"bind"		}, /* 90 */
	{ MA,	TN,	sys_connect,		"connect"	}, /* 91 */
	{ MA,	0,	sys_gethostid,		"gethostid"	}, /* 92 */
	{ MA,	TN,	sys_getpeername,	"getpeername"	}, /* 93 */
	{ MA,	TN,	sys_getsockname,	"getsockname"	}, /* 94 */
	{ MA,	TN,	sys_getsockopt,		"getsockopt"	}, /* 95 */
	{ MA,	TN,	sys_listen,		"listen"	}, /* 96 */
	{ MA,	TN,	sys_recv,		"recv"		}, /* 97 */
	{ MA,	TN,	sys_recvfrom,		"recvfrom"	}, /* 98 */
	{ MA,	TN,	sys_recvmsg,		"recvmsg"	}, /* 99 */
	{ MA,	TD,	sys_select,		"select"	}, /* 100 */
	{ MA,	TN,	sys_send,		"send"		}, /* 101 */
	{ MA,	TN,	sys_sendmsg,		"sendmsg"	}, /* 102 */
	{ MA,	TN,	sys_sendto,		"sendto"	}, /* 103 */
	{ MA,	0,	sys_sethostid,		"sethostid"	}, /* 104 */
	{ MA,	TN,	sys_setsockopt,		"setsockopt"	}, /* 105 */
	{ MA,	TN,	sys_shutdown,		"shutdown"	}, /* 106 */
	{ MA,	TN,	sys_socket,		"socket"	}, /* 107 */
	{ MA,	0,	sys_gethostname,	"gethostname"	}, /* 108 */
	{ MA,	0,	sys_sethostname,	"sethostname"	}, /* 109 */
	{ MA,	0,	sys_getdomainname,	"getdomainname"	}, /* 110 */
	{ MA,	0,	sys_setdomainname,	"setdomainname"	}, /* 111 */
	{ MA,	TF,	sys_truncate,		"truncate"	}, /* 112 */
	{ MA,	TD,	sys_ftruncate,		"ftruncate"	}, /* 113 */
	{ MA,	TF,	sys_rename,		"rename"	}, /* 114 */
	{ MA,	TF,	sys_symlink,		"symlink"	}, /* 115 */
	{ MA,	TF,	sys_readlink,		"readlink"	}, /* 116 */
	{ MA,	0,	printargs,		"SYS_117"	}, /* 117 */
	{ MA,	0,	printargs,		"SYS_118"	}, /* 118 */
	{ MA,	0,	sys_nfssvc,		"nfssvc"	}, /* 119 */
	{ MA,	0,	sys_getfh,		"getfh"		}, /* 120 */
	{ MA,	0,	sys_async_daemon,	"async_daemon"	}, /* 121 */
	{ MA,	0,	sys_exportfs,		"exportfs"	}, /* 122 */
	{ MA,	0,	sys_setregid,		"setregid"	}, /* 123 */
	{ MA,	0,	sys_setreuid,		"setreuid"	}, /* 124 */
	{ MA,	0,	sys_getitimer,		"getitimer"	}, /* 125 */
	{ MA,	0,	sys_setitimer,		"setitimer"	}, /* 126 */
	{ MA,	0,	sys_adjtime,		"adjtime"	}, /* 127 */
	{ MA,	0,	sys_BSD_getime,		"BSD_getime"	}, /* 128 */
	{ MA,	0,	sys_sproc,		"sproc"		}, /* 129 */
	{ MA,	0,	sys_prctl,		"prctl"		}, /* 130 */
	{ MA,	0,	sys_procblk,		"procblk"	}, /* 131 */
	{ MA,	0,	sys_sprocsp,		"sprocsp"	}, /* 132 */
	{ MA,	0,	printargs,		"SYS_133"	}, /* 133 */
	{ MA,	0,	sys_mmap,		"mmap"		}, /* 134 */
	{ MA,	0,	sys_munmap,		"munmap"	}, /* 135 */
	{ MA,	0,	sys_mprotect,		"mprotect"	}, /* 136 */
	{ MA,	0,	sys_msync,		"msync"		}, /* 137 */
	{ MA,	0,	sys_madvise,		"madvise"	}, /* 138 */
	{ MA,	0,	sys_pagelock,		"pagelock"	}, /* 139 */
	{ MA,	0,	sys_getpagesize,	"getpagesize"	}, /* 140 */
	{ MA,	0,	sys_quotactl,		"quotactl"	}, /* 141 */
	{ MA,	0,	printargs,		"SYS_142"	}, /* 142 */
	{ MA,	0,	sys_BSDgetpgrp,		"BSDgetpgrp"	}, /* 143 */
	{ MA,	0,	sys_BSDsetpgrp,		"BSDsetpgrp"	}, /* 144 */
	{ MA,	0,	sys_vhangup,		"vhangup"	}, /* 145 */
	{ MA,	TD,	sys_fsync,		"fsync"		}, /* 146 */
	{ MA,	TD,	sys_fchdir,		"fchdir"	}, /* 147 */
	{ MA,	0,	sys_getrlimit,		"getrlimit"	}, /* 148 */
	{ MA,	0,	sys_setrlimit,		"setrlimit"	}, /* 149 */
	{ MA,	0,	sys_cacheflush,		"cacheflush"	}, /* 150 */
	{ MA,	0,	sys_cachectl,		"cachectl"	}, /* 151 */
	{ MA,	TD,	sys_fchown,		"fchown"	}, /* 152 */
	{ MA,	TD,	sys_fchmod,		"fchmod"	}, /* 153 */
	{ MA,	0,	printargs,		"SYS_154"	}, /* 154 */
	{ MA,	TN,	sys_socketpair,		"socketpair"	}, /* 155 */
	{ MA,	0,	sys_sysinfo,		"sysinfo"	}, /* 156 */
	{ MA,	0,	sys_nuname,		"nuname"	}, /* 157 */
	{ MA,	TF,	sys_xstat,		"xstat"		}, /* 158 */
	{ MA,	TF,	sys_lxstat,		"lxstat"	}, /* 159 */
	{ MA,	0,	sys_fxstat,		"fxstat"	}, /* 160 */
	{ MA,	TF,	sys_xmknod,		"xmknod"	}, /* 161 */
	{ MA,	TS,	sys_ksigaction,		"sigaction"	}, /* 162 */
	{ MA,	TS,	sys_sigpending,		"sigpending"	}, /* 163 */
	{ MA,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 164 */
	{ MA,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 165 */
	{ MA,	TS,	sys_sigpoll,		"sigpoll"	}, /* 166 */
	{ MA,	0,	sys_swapctl,		"swapctl"	}, /* 167 */
	{ MA,	0,	sys_getcontext,		"getcontext"	}, /* 168 */
	{ MA,	0,	sys_setcontext,		"setcontext"	}, /* 169 */
	{ MA,	TP,	sys_waitid,		"waitid"	}, /* 170 */
	{ MA,	TS,	sys_sigstack,		"sigstack"	}, /* 171 */
	{ MA,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 172 */
	{ MA,	TS,	sys_sigsendset,		"sigsendset"	}, /* 173 */
	{ MA,	TF,	sys_statvfs,		"statvfs"	}, /* 174 */
	{ MA,	0,	sys_fstatvfs,		"fstatvfs"	}, /* 175 */
	{ MA,	TN,	sys_getpmsg,		"getpmsg"	}, /* 176 */
	{ MA,	TN,	sys_putpmsg,		"putpmsg"	}, /* 177 */
	{ MA,	TF,	sys_lchown,		"lchown"	}, /* 178 */
	{ MA,	0,	sys_priocntl,		"priocntl"	}, /* 179 */
	{ MA,	TS,	sys_ksigqueue,		"ksigqueue"	}, /* 180 */
	{ MA,	0,	printargs,		"SYS_181"	}, /* 181 */
	{ MA,	0,	printargs,		"SYS_182"	}, /* 182 */
	{ MA,	0,	printargs,		"SYS_183"	}, /* 183 */
	{ MA,	0,	printargs,		"SYS_184"	}, /* 184 */
	{ MA,	0,	printargs,		"SYS_185"	}, /* 185 */
	{ MA,	0,	printargs,		"SYS_186"	}, /* 186 */
	{ MA,	0,	printargs,		"SYS_187"	}, /* 187 */
	{ MA,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ MA,	0,	printargs,		"SYS_189"	}, /* 189 */
	{ MA,	0,	printargs,		"SYS_190"	}, /* 190 */
	{ MA,	0,	printargs,		"SYS_191"	}, /* 191 */
	{ MA,	0,	printargs,		"SYS_192"	}, /* 192 */
	{ MA,	0,	printargs,		"SYS_193"	}, /* 193 */
	{ MA,	0,	printargs,		"SYS_194"	}, /* 194 */
	{ MA,	0,	printargs,		"SYS_195"	}, /* 195 */
	{ MA,	0,	printargs,		"SYS_196"	}, /* 196 */
	{ MA,	0,	printargs,		"SYS_197"	}, /* 197 */
	{ MA,	0,	printargs,		"SYS_198"	}, /* 198 */
	{ MA,	0,	printargs,		"SYS_199"	}, /* 199 */
	{ MA,	0,	printargs,		"SYS_200"	}, /* 200 */
	{ MA,	0,	printargs,		"SYS_201"	}, /* 201 */
	{ MA,	0,	printargs,		"SYS_202"	}, /* 202 */
	{ MA,	0,	printargs,		"SYS_203"	}, /* 203 */
	{ MA,	0,	printargs,		"SYS_204"	}, /* 204 */
	{ MA,	0,	printargs,		"SYS_205"	}, /* 205 */
	{ MA,	0,	printargs,		"SYS_206"	}, /* 206 */
	{ MA,	0,	printargs,		"SYS_207"	}, /* 207 */
	{ MA,	0,	printargs,		"SYS_208"	}, /* 208 */
	{ MA,	0,	printargs,		"SYS_209"	}, /* 209 */
	{ MA,	0,	printargs,		"SYS_210"	}, /* 210 */
	{ MA,	0,	printargs,		"SYS_211"	}, /* 211 */
	{ MA,	0,	printargs,		"SYS_212"	}, /* 212 */
	{ MA,	0,	printargs,		"SYS_213"	}, /* 213 */
	{ MA,	0,	printargs,		"SYS_214"	}, /* 214 */
	{ MA,	0,	printargs,		"SYS_215"	}, /* 215 */
	{ MA,	0,	printargs,		"SYS_216"	}, /* 216 */
	{ MA,	0,	printargs,		"SYS_217"	}, /* 217 */
	{ MA,	0,	printargs,		"SYS_218"	}, /* 218 */
	{ MA,	0,	printargs,		"SYS_219"	}, /* 219 */
	{ MA,	0,	printargs,		"SYS_220"	}, /* 220 */
	{ MA,	0,	printargs,		"SYS_221"	}, /* 221 */
	{ MA,	0,	printargs,		"SYS_222"	}, /* 222 */
	{ MA,	0,	printargs,		"SYS_223"	}, /* 223 */
	{ MA,	0,	printargs,		"SYS_224"	}, /* 224 */
	{ MA,	0,	printargs,		"SYS_225"	}, /* 225 */
	{ MA,	0,	printargs,		"SYS_226"	}, /* 226 */
	{ MA,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ MA,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ MA,	0,	printargs,		"SYS_229"	}, /* 229 */
	{ MA,	0,	printargs,		"SYS_230"	}, /* 230 */
	{ MA,	0,	printargs,		"SYS_231"	}, /* 231 */
	{ MA,	0,	printargs,		"SYS_232"	}, /* 232 */
	{ MA,	0,	printargs,		"SYS_233"	}, /* 233 */
	{ MA,	0,	printargs,		"SYS_234"	}, /* 234 */
	{ MA,	0,	printargs,		"SYS_235"	}, /* 235 */
	{ MA,	0,	printargs,		"SYS_236"	}, /* 236 */
	{ MA,	0,	printargs,		"SYS_237"	}, /* 237 */
	{ MA,	0,	printargs,		"SYS_238"	}, /* 238 */
	{ MA,	0,	printargs,		"SYS_239"	}, /* 239 */
	{ MA,	0,	printargs,		"SYS_240"	}, /* 240 */
	{ MA,	0,	printargs,		"SYS_241"	}, /* 241 */
	{ MA,	0,	printargs,		"SYS_242"	}, /* 242 */
	{ MA,	0,	printargs,		"SYS_243"	}, /* 243 */
	{ MA,	0,	printargs,		"SYS_244"	}, /* 244 */
	{ MA,	0,	printargs,		"SYS_245"	}, /* 245 */
	{ MA,	0,	printargs,		"SYS_246"	}, /* 246 */
	{ MA,	0,	printargs,		"SYS_247"	}, /* 247 */
	{ MA,	0,	printargs,		"SYS_248"	}, /* 248 */
	{ MA,	0,	printargs,		"SYS_249"	}, /* 249 */
	{ MA,	0,	printargs,		"SYS_250"	}, /* 250 */
	{ MA,	0,	printargs,		"SYS_251"	}, /* 251 */
	{ MA,	0,	printargs,		"SYS_252"	}, /* 252 */
	{ MA,	0,	printargs,		"SYS_253"	}, /* 253 */
	{ MA,	0,	printargs,		"SYS_254"	}, /* 254 */
	{ MA,	0,	printargs,		"SYS_255"	}, /* 255 */
#else /* !MIPS */
	{ MA,	TF,	sys_lstat,		"lstat"		}, /* 88 */
	{ MA,	TF,	sys_symlink,		"symlink"	}, /* 89 */
	{ MA,	TF,	sys_readlink,		"readlink"	}, /* 90 */
	{ MA,	0,	sys_setgroups,		"setgroups"	}, /* 91 */
	{ MA,	0,	sys_getgroups,		"getgroups"	}, /* 92 */
	{ MA,	TD,	sys_fchmod,		"fchmod"	}, /* 93 */
	{ MA,	TD,	sys_fchown,		"fchown"	}, /* 94 */
	{ MA,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 95 */
	{ MA,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 96 */
	{ MA,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 97 */
	{ MA,	TS,	sys_sigaction,		"sigaction"	}, /* 98 */
	{ MA,	0,	sys_spcall,		"spcall"	}, /* 99 */
	{ MA,	0,	sys_context,		"context"	}, /* 100 */
	{ MA,	0,	sys_evsys,		"evsys"		}, /* 101 */
	{ MA,	0,	sys_evtrapret,		"evtrapret"	}, /* 102 */
	{ MA,	TF,	sys_statvfs,		"statvfs"	}, /* 103 */
	{ MA,	0,	sys_fstatvfs,		"fstatvfs"	}, /* 104 */
	{ MA,	0,	printargs,		"SYS_105"	}, /* 105 */
	{ MA,	0,	sys_nfssys,		"nfssys"	}, /* 106 */
#if UNIXWARE
	{ MA,	TP,	sys_waitsys,		"waitsys"	}, /* 107 */
#else
	{ MA,	TP,	sys_waitid,		"waitid"	}, /* 107 */
#endif
	{ MA,	0,	sys_sigsendsys,		"sigsendsys"	}, /* 108 */
	{ MA,	0,	sys_hrtsys,		"hrtsys"	}, /* 109 */
	{ MA,	0,	sys_acancel,		"acancel"	}, /* 110 */
	{ MA,	0,	sys_async,		"async"		}, /* 111 */
	{ MA,	0,	sys_priocntlsys,	"priocntlsys"	}, /* 112 */
	{ MA,	TF,	sys_pathconf,		"pathconf"	}, /* 113 */
	{ MA,	0,	sys_mincore,		"mincore"	}, /* 114 */
	{ MA,	0,	sys_mmap,		"mmap"		}, /* 115 */
	{ MA,	0,	sys_mprotect,		"mprotect"	}, /* 116 */
	{ MA,	0,	sys_munmap,		"munmap"	}, /* 117 */
	{ MA,	0,	sys_fpathconf,		"fpathconf"	}, /* 118 */
	{ MA,	TP,	sys_vfork,		"vfork"		}, /* 119 */
	{ MA,	TD,	sys_fchdir,		"fchdir"	}, /* 120 */
	{ MA,	TD,	sys_readv,		"readv"		}, /* 121 */
	{ MA,	TD,	sys_writev,		"writev"	}, /* 122 */
	{ MA,	TF,	sys_xstat,		"xstat"		}, /* 123 */
	{ MA,	TF,	sys_lxstat,		"lxstat"	}, /* 124 */
	{ MA,	0,	sys_fxstat,		"fxstat"	}, /* 125 */
	{ MA,	TF,	sys_xmknod,		"xmknod"	}, /* 126 */
	{ MA,	0,	sys_clocal,		"clocal"	}, /* 127 */
	{ MA,	0,	sys_setrlimit,		"setrlimit"	}, /* 128 */
	{ MA,	0,	sys_getrlimit,		"getrlimit"	}, /* 129 */
	{ MA,	TF,	sys_lchown,		"lchown"	}, /* 130 */
	{ MA,	0,	sys_memcntl,		"memcntl"	}, /* 131 */
	{ MA,	TN,	sys_getpmsg,		"getpmsg"	}, /* 132 */
	{ MA,	TN,	sys_putpmsg,		"putpmsg"	}, /* 133 */
	{ MA,	TF,	sys_rename,		"rename"	}, /* 134 */
	{ MA,	0,	sys_uname,		"uname"		}, /* 135 */
	{ MA,	0,	sys_setegid,		"setegid"	}, /* 136 */
	{ MA,	0,	sys_sysconfig,		"sysconfig"	}, /* 137 */
	{ MA,	0,	sys_adjtime,		"adjtime"	}, /* 138 */
	{ MA,	0,	sys_sysinfo,		"sysinfo"	}, /* 139 */
	{ MA,	0,	printargs,		"SYS_140"	}, /* 140 */
#if UNIXWARE >= 2
	{ MA,	0,	sys_seteuid,		"seteuid"	}, /* 141 */
	{ MA,	0,	printargs,		"SYS_142"	}, /* 142 */
	{ MA,	0,	sys_keyctl,		"keyctl"	}, /* 143 */
	{ MA,	0,	sys_secsys,		"secsys"	}, /* 144 */
	{ MA,	0,	sys_filepriv,		"filepriv"	}, /* 145 */
	{ MA,	0,	sys_procpriv,		"procpriv"	}, /* 146 */
	{ MA,	0,	sys_devstat,		"devstat"	}, /* 147 */
	{ MA,	0,	sys_aclipc,		"aclipc"	}, /* 148 */
	{ MA,	0,	sys_fdevstat,		"fdevstat"	}, /* 149 */
	{ MA,	0,	sys_flvlfile,		"flvlfile"	}, /* 150 */
	{ MA,	0,	sys_lvlfile,		"lvlfile"	}, /* 151 */
	{ MA,	0,	printargs,		"SYS_152"	}, /* 152 */
	{ MA,	0,	sys_lvlequal,		"lvlequal"	}, /* 153 */
	{ MA,	0,	sys_lvlproc,		"lvlproc"	}, /* 154 */
	{ MA,	0,	printargs,		"SYS_155"	}, /* 155 */
	{ MA,	0,	sys_lvlipc,		"lvlipc"	}, /* 156 */
	{ MA,	0,	sys_acl,		"acl"		}, /* 157 */
	{ MA,	0,	sys_auditevt,		"auditevt"	}, /* 158 */
	{ MA,	0,	sys_auditctl,		"auditctl"	}, /* 159 */
	{ MA,	0,	sys_auditdmp,		"auditdmp"	}, /* 160 */
	{ MA,	0,	sys_auditlog,		"auditlog"	}, /* 161 */
	{ MA,	0,	sys_auditbuf,		"auditbuf"	}, /* 162 */
	{ MA,	0,	sys_lvldom,		"lvldom"	}, /* 163 */
	{ MA,	0,	sys_lvlvfs,		"lvlvfs"	}, /* 164 */
	{ MA,	0,	sys_mkmld,		"mkmld"		}, /* 165 */
	{ MA,	0,	sys_mldmode,		"mldmode"	}, /* 166 */
	{ MA,	0,	sys_secadvise,		"secadvise"	}, /* 167 */
	{ MA,	0,	sys_online,		"online"	}, /* 168 */
	{ MA,	0,	sys_setitimer,		"setitimer"	}, /* 169 */
	{ MA,	0,	sys_getitimer,		"getitimer"	}, /* 170 */
	{ MA,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 171 */
	{ MA,	0,	sys_settimeofday,	"settimeofday"	}, /* 172 */
	{ MA,	0,	sys_lwp_create,		"lwpcreate"	}, /* 173 */
	{ MA,	0,	sys_lwp_exit,		"lwpexit"	}, /* 174 */
	{ MA,	0,	sys_lwp_wait,		"lwpwait"	}, /* 175 */
	{ MA,	0,	sys_lwp_self,		"lwpself"	}, /* 176 */
	{ MA,	0,	sys_lwpinfo,		"lwpinfo"	}, /* 177 */
	{ MA,	0,	sys_lwpprivate,		"lwpprivate"	}, /* 178 */
	{ MA,	0,	sys_processor_bind,	"processor_bind"}, /* 179 */
	{ MA,	0,	sys_processor_exbind,	"processor_exbind"}, /* 180 */
	{ MA,	0,	printargs,		"SYS_181"	}, /* 181 */
	{ MA,	0,	printargs,		"SYS_182"	}, /* 182 */
	{ MA,	0,	sys_prepblock,		"prepblock"	}, /* 183 */
	{ MA,	0,	sys_block,		"block"		}, /* 184 */
	{ MA,	0,	sys_rdblock,		"rdblock"	}, /* 185 */
	{ MA,	0,	sys_unblock,		"unblock"	}, /* 186 */
	{ MA,	0,	sys_cancelblock,	"cancelblock"	}, /* 187 */
	{ MA,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ MA,	TD,	sys_pread,		"pread"		}, /* 189 */
	{ MA,	TD,	sys_pwrite,		"pwrite"	}, /* 190 */
	{ MA,	TF,	sys_truncate,		"truncate"	}, /* 191 */
	{ MA,	TD,	sys_ftruncate,		"ftruncate"	}, /* 192 */
	{ MA,	0,	sys_lwpkill,		"lwpkill"	}, /* 193 */
	{ MA,	0,	sys_sigwait,		"sigwait"	}, /* 194 */
	{ MA,	0,	sys_fork1,		"fork1"		}, /* 195 */
	{ MA,	0,	sys_forkall,		"forkall"	}, /* 196 */
	{ MA,	0,	sys_modload,		"modload"	}, /* 197 */
	{ MA,	0,	sys_moduload,		"moduload"	}, /* 198 */
	{ MA,	0,	sys_modpath,		"modpath"	}, /* 199 */
	{ MA,	0,	sys_modstat,		"modstat"	}, /* 200 */
	{ MA,	0,	sys_modadm,		"modadm"	}, /* 201 */
	{ MA,	0,	sys_getksym,		"getksym"	}, /* 202 */
	{ MA,	0,	sys_lwpsuspend,		"lwpsuspend"	}, /* 203 */
	{ MA,	0,	sys_lwpcontinue,	"lwpcontinue"	}, /* 204 */
	{ MA,	0,	sys_priocntllst,	"priocntllst"	}, /* 205 */
	{ MA,	0,	sys_sleep,		"sleep"		}, /* 206 */
	{ MA,	0,	sys_lwp_sema_wait,	"lwp_sema_wait"	}, /* 207 */
	{ MA,	0,	sys_lwp_sema_post,	"lwp_sema_post"	}, /* 208 */
	{ MA,	0,	sys_lwp_sema_trywait,	"lwp_sema_trywait"}, /* 209 */
	{ MA,	0,	printargs,		"SYS_210"	}, /* 210 */
	{ MA,	0,	printargs,		"SYS_211"	}, /* 211 */
	{ MA,	0,	printargs,		"SYS_212"	}, /* 212 */
	{ MA,	0,	printargs,		"SYS_213"	}, /* 213 */
	{ MA,	0,	printargs,		"SYS_214"	}, /* 214 */
	{ MA,	0,	printargs,		"SYS_215"	}, /* 215 */
#if UNIXWARE >= 7
	{ MA,	0,	sys_fstatvfs64,		"fstatvfs64"	}, /* 216 */
	{ MA,	TF,	sys_statvfs64,		"statvfs64"	}, /* 217 */
	{ MA,	TD,	sys_ftruncate64,	"ftruncate64"	}, /* 218 */
	{ MA,	TF,	sys_truncate64,		"truncate64"	}, /* 219 */
	{ MA,	0,	sys_getrlimit64,	"getrlimit64"	}, /* 220 */
	{ MA,	0,	sys_setrlimit64,	"setrlimit64"	}, /* 221 */
	{ MA,	TF,	sys_lseek64,		"lseek64"	}, /* 222 */
	{ MA,	TF,	sys_mmap64,		"mmap64"	}, /* 223 */
	{ MA,	TF,	sys_pread64,		"pread64"	}, /* 224 */
	{ MA,	TF,	sys_pwrite64,		"pwrite64"	}, /* 225 */
	{ MA,	TD|TF,	sys_creat64,		"creat64"	}, /* 226 */
	{ MA,	0,	sys_dshmsys,		"dshmsys"	}, /* 227 */
	{ MA,	0,	sys_invlpg,		"invlpg"	}, /* 228 */
	{ MA,	0,	sys_rfork1,		"rfork1"	}, /* 229 */
	{ MA,	0,	sys_rforkall,		"rforkall"	}, /* 230 */
	{ MA,	0,	sys_rexecve,		"rexecve"	}, /* 231 */
	{ MA,	0,	sys_migrate,		"migrate"	}, /* 232 */
	{ MA,	0,	sys_kill3,		"kill3"		}, /* 233 */
	{ MA,	0,	sys_ssisys,		"ssisys"	}, /* 234 */
	{ MA,	TN,	sys_xaccept,		"xaccept"	}, /* 235 */
	{ MA,	TN,	sys_xbind,		"xbind"		}, /* 236 */
	{ MA,	TN,	sys_xbindresvport,	"xbindresvport"	}, /* 237 */
	{ MA,	TN,	sys_xconnect,		"xconnect"	}, /* 238 */
	{ MA,	TN,	sys_xgetsockaddr,	"xgetsockaddr"	}, /* 239 */
	{ MA,	TN,	sys_xgetsockopt,	"xgetsockopt"	}, /* 240 */
	{ MA,	TN,	sys_xlisten,		"xlisten"	}, /* 241 */
	{ MA,	TN,	sys_xrecvmsg,		"xrecvmsg"	}, /* 242 */
	{ MA,	TN,	sys_xsendmsg,		"xsendmsg"	}, /* 243 */
	{ MA,	TN,	sys_xsetsockaddr,	"xsetsockaddr"	}, /* 244 */
	{ MA,	TN,	sys_xsetsockopt,	"xsetsockopt"	}, /* 245 */
	{ MA,	TN,	sys_xshutdown,		"xshutdown"	}, /* 246 */
	{ MA,	TN,	sys_xsocket,		"xsocket"	}, /* 247 */
	{ MA,	TN,	sys_xsocketpair,	"xsocketpair"	}, /* 248 */
#else	/* UNIXWARE 2 */
	{ MA,	0,	printargs,		"SYS_216"	}, /* 216 */
	{ MA,	0,	printargs,		"SYS_217"	}, /* 217 */
	{ MA,	0,	printargs,		"SYS_218"	}, /* 218 */
	{ MA,	0,	printargs,		"SYS_219"	}, /* 219 */
	{ MA,	0,	printargs,		"SYS_220"	}, /* 220 */
	{ MA,	0,	printargs,		"SYS_221"	}, /* 221 */
	{ MA,	0,	printargs,		"SYS_222"	}, /* 222 */
	{ MA,	0,	printargs,		"SYS_223"	}, /* 223 */
	{ MA,	0,	printargs,		"SYS_224"	}, /* 224 */
	{ MA,	0,	printargs,		"SYS_225"	}, /* 225 */
	{ MA,	0,	printargs,		"SYS_226"	}, /* 226 */
	{ MA,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ MA,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ MA,	0,	printargs,		"SYS_229"	}, /* 229 */
	{ MA,	0,	printargs,		"SYS_230"	}, /* 230 */
	{ MA,	0,	printargs,		"SYS_231"	}, /* 231 */
	{ MA,	0,	printargs,		"SYS_232"	}, /* 232 */
	{ MA,	0,	printargs,		"SYS_233"	}, /* 233 */
	{ MA,	0,	printargs,		"SYS_234"	}, /* 234 */
	{ MA,	0,	printargs,		"SYS_235"	}, /* 235 */
	{ MA,	0,	printargs,		"SYS_236"	}, /* 236 */
	{ MA,	0,	printargs,		"SYS_237"	}, /* 237 */
	{ MA,	0,	printargs,		"SYS_238"	}, /* 238 */
	{ MA,	0,	printargs,		"SYS_239"	}, /* 239 */
	{ MA,	0,	printargs,		"SYS_240"	}, /* 240 */
	{ MA,	0,	printargs,		"SYS_241"	}, /* 241 */
	{ MA,	0,	printargs,		"SYS_242"	}, /* 242 */
	{ MA,	0,	printargs,		"SYS_243"	}, /* 243 */
	{ MA,	0,	printargs,		"SYS_244"	}, /* 244 */
	{ MA,	0,	printargs,		"SYS_245"	}, /* 245 */
	{ MA,	0,	printargs,		"SYS_246"	}, /* 246 */
	{ MA,	0,	printargs,		"SYS_247"	}, /* 247 */
	{ MA,	0,	printargs,		"SYS_248"	}, /* 248 */
#endif	/* UNIXWARE 2 */
	{ MA,	0,	printargs,		"SYS_249"	}, /* 249 */
	{ MA,	0,	printargs,		"SYS_250"	}, /* 250 */
	{ MA,	0,	printargs,		"SYS_251"	}, /* 251 */
	{ MA,	0,	printargs,		"SYS_252"	}, /* 252 */
	{ MA,	0,	printargs,		"SYS_253"	}, /* 253 */
	{ MA,	0,	printargs,		"SYS_254"	}, /* 254 */
	{ MA,	0,	printargs,		"SYS_255"	}, /* 255 */
#else   /* !UNIXWARE */
	{ MA,	0,	sys_seteuid,		"seteuid"	}, /* 141 */
	{ MA,	0,	sys_vtrace,		"vtrace"	}, /* 142 */
	{ MA,	TP,	sys_fork1,		"fork1"		}, /* 143 */
	{ MA,	TS,	sys_sigtimedwait,	"sigtimedwait"	}, /* 144 */
	{ MA,	0,	sys_lwp_info,		"lwp_info"	}, /* 145 */
	{ MA,	0,	sys_yield,		"yield"		}, /* 146 */
	{ MA,	0,	sys_lwp_sema_wait,	"lwp_sema_wait"	}, /* 147 */
	{ MA,	0,	sys_lwp_sema_post,	"lwp_sema_post"	}, /* 148 */
	{ MA,	0,	sys_lwp_sema_trywait,"lwp_sema_trywait"	}, /* 149 */
	{ MA,	0,	printargs,		"SYS_150"	}, /* 150 */
	{ MA,	0,	printargs,		"SYS_151"	}, /* 151 */
	{ MA,	0,	sys_modctl,		"modctl"	}, /* 152 */
	{ MA,	0,	sys_fchroot,		"fchroot"	}, /* 153 */
	{ MA,	TF,	sys_utimes,		"utimes"	}, /* 154 */
	{ MA,	0,	sys_vhangup,		"vhangup"	}, /* 155 */
	{ MA,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 156 */
	{ MA,	0,	sys_getitimer,		"getitimer"	}, /* 157 */
	{ MA,	0,	sys_setitimer,		"setitimer"	}, /* 158 */
	{ MA,	0,	sys_lwp_create,		"lwp_create"	}, /* 159 */
	{ MA,	0,	sys_lwp_exit,		"lwp_exit"	}, /* 160 */
	{ MA,	0,	sys_lwp_suspend,	"lwp_suspend"	}, /* 161 */
	{ MA,	0,	sys_lwp_continue,	"lwp_continue"	}, /* 162 */
	{ MA,	0,	sys_lwp_kill,		"lwp_kill"	}, /* 163 */
	{ MA,	0,	sys_lwp_self,		"lwp_self"	}, /* 164 */
	{ MA,	0,	sys_lwp_setprivate,	"lwp_setprivate"}, /* 165 */
	{ MA,	0,	sys_lwp_getprivate,	"lwp_getprivate"}, /* 166 */
	{ MA,	0,	sys_lwp_wait,		"lwp_wait"	}, /* 167 */
	{ MA,	0,	sys_lwp_mutex_unlock,	"lwp_mutex_unlock"}, /* 168 */
	{ MA,	0,	sys_lwp_mutex_lock,	"lwp_mutex_lock"}, /* 169 */
	{ MA,	0,	sys_lwp_cond_wait,	"lwp_cond_wait"}, /* 170 */
	{ MA,	0,	sys_lwp_cond_signal,	"lwp_cond_signal"}, /* 171 */
	{ MA,	0,	sys_lwp_cond_broadcast,	"lwp_cond_broadcast"}, /* 172 */
	{ MA,	TD,	sys_pread,		"pread"		}, /* 173 */
	{ MA,	TD,	sys_pwrite,		"pwrite"	}, /* 174 */
	{ MA,	TD,	sys_llseek,		"llseek"	}, /* 175 */
	{ MA,	0,	sys_inst_sync,		"inst_sync"	}, /* 176 */
	{ MA,	0,	printargs,		"srmlimitsys"	}, /* 177 */
	{ MA,	0,	sys_kaio,		"kaio"		}, /* 178 */
	{ MA,	0,	printargs,		"cpc"		}, /* 179 */
	{ MA,	0,	printargs,		"SYS_180"	}, /* 180 */
	{ MA,	0,	printargs,		"SYS_181"	}, /* 181 */
	{ MA,	0,	printargs,		"SYS_182"	}, /* 182 */
	{ MA,	0,	printargs,		"SYS_183"	}, /* 183 */
	{ MA,	0,	sys_tsolsys,		"tsolsys"	}, /* 184 */
#ifdef HAVE_SYS_ACL_H
	{ MA,	TF,	sys_acl,		"acl"		}, /* 185 */
#else
	{ MA,	0,	printargs,		"SYS_185"	}, /* 185 */
#endif
	{ MA,	0,	sys_auditsys,		"auditsys"	}, /* 186 */
	{ MA,	0,	sys_processor_bind,	"processor_bind"}, /* 187 */
	{ MA,	0,	sys_processor_info,	"processor_info"}, /* 188 */
	{ MA,	0,	sys_p_online,		"p_online"	}, /* 189 */
	{ MA,	0,	sys_sigqueue,		"sigqueue"	}, /* 190 */
	{ MA,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 191 */
	{ MA,	0,	sys_clock_settime,	"clock_settime"	}, /* 192 */
	{ MA,	0,	sys_clock_getres,	"clock_getres"	}, /* 193 */
	{ MA,	0,	sys_timer_create,	"timer_create"	}, /* 194 */
	{ MA,	0,	sys_timer_delete,	"timer_delete"	}, /* 195 */
	{ MA,	0,	sys_timer_settime,	"timer_settime"	}, /* 196 */
	{ MA,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 197 */
	{ MA,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 198 */
	{ MA,	0,	sys_nanosleep,		"nanosleep"	}, /* 199 */
#ifdef HAVE_SYS_ACL_H
	{ MA,	0,	sys_facl,		"facl"		}, /* 200 */
#else
	{ MA,	0,	printargs,		"SYS_200"	}, /* 200 */
#endif
#ifdef HAVE_SYS_DOOR_H
	{ MA,	0,	sys_door,		"door"		}, /* 201 */
#else
	{ MA,	0,	printargs,		"SYS_201"	}, /* 201 */
#endif
	{ MA,	0,	sys_setreuid,		"setreuid"	}, /* 202 */
	{ MA,	0,	sys_setregid,		"setregid"	}, /* 203 */
	{ MA,	0,	sys_install_utrap,	"install_utrap"	}, /* 204 */
	{ MA,	0,	sys_signotify,		"signotify"	}, /* 205 */
	{ MA,	0,	sys_schedctl,		"schedctl"	}, /* 206 */
	{ MA,	0,	sys_pset,		"pset"	}, /* 207 */
	{ MA,	0,	printargs,		"__sparc_utrap_install"	}, /* 208 */
	{ MA,	0,	sys_resolvepath,	"resolvepath"	}, /* 209 */
	{ MA,	0,	sys_signotifywait,	"signotifywait"	}, /* 210 */
	{ MA,	0,	sys_lwp_sigredirect,	"lwp_sigredirect"	}, /* 211 */
	{ MA,	0,	sys_lwp_alarm,		"lwp_alarm"	}, /* 212 */
	{ MA,	TD,	sys_getdents64,		"getdents64"	}, /* 213 */
	{ MA,	0,	sys_mmap64,		"mmap64"	}, /* 214 */
	{ MA,	0,	sys_stat64,		"stat64"	}, /* 215 */
	{ MA,	0,	sys_lstat64,		"lstat64"	}, /* 216 */
	{ MA,	TD,	sys_fstat64,		"fstat64"	}, /* 217 */
	{ MA,	0,	sys_statvfs64,		"statvfs64"	}, /* 218 */
	{ MA,	0,	sys_fstatvfs64,		"fstatvfs64"	}, /* 219 */
	{ MA,	0,	sys_setrlimit64,	"setrlimit64"	}, /* 220 */
	{ MA,	0,	sys_getrlimit64,	"getrlimit64"	}, /* 221 */
	{ MA,	TD,	sys_pread64,		"pread64"	}, /* 222 */
	{ MA,	TD,	sys_pwrite64,		"pwrite64"	}, /* 223 */
	{ MA,	0,	sys_creat64,		"creat64"	}, /* 224 */
	{ MA,	0,	sys_open64,		"open64"	}, /* 225 */
	{ MA,	0,	sys_rpcsys,		"rpcsys"	}, /* 226 */
	{ MA,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ MA,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ MA,	0,	printargs,		"SYS_229"	}, /* 229 */
	{ MA,	TN,	sys_so_socket,		"so_socket"	}, /* 230 */
	{ MA,	TN,	sys_so_socketpair,	"so_socketpair"	}, /* 231 */
	{ MA,	TN,	sys_bind,		"bind"		}, /* 232 */
	{ MA,	TN,	sys_listen,		"listen"	}, /* 233 */
	{ MA,	TN,	sys_accept,		"accept"	}, /* 234 */
	{ MA,	TN,	sys_connect,		"connect"	}, /* 235 */
	{ MA,	TN,	sys_shutdown,		"shutdown"	}, /* 236 */
	{ MA,	TN,	sys_recv,		"recv"		}, /* 237 */
	{ MA,	TN,	sys_recvfrom,		"recvfrom"	}, /* 238 */
	{ MA,	TN,	sys_recvmsg,		"recvmsg"	}, /* 239 */
	{ MA,	TN,	sys_send,		"send"		}, /* 240 */
	{ MA,	TN,	sys_sendmsg,		"sendmsg"	}, /* 241 */
	{ MA,	TN,	sys_sendto,		"sendto"	}, /* 242 */
	{ MA,	TN,	sys_getpeername,	"getpeername"	}, /* 243 */
	{ MA,	TN,	sys_getsockname,	"getsockname"	}, /* 244 */
	{ MA,	TN,	sys_getsockopt,		"getsockopt"	}, /* 245 */
	{ MA,	TN,	sys_setsockopt,		"setsockopt"	}, /* 246 */
	{ MA,	TN,	sys_sockconfig,		"sockconfig"	}, /* 247 */
	{ MA,	0,	sys_ntp_gettime,	"ntp_gettime"	}, /* 248 */
	{ MA,	0,	sys_ntp_adjtime,	"ntp_adjtime"	}, /* 249 */
	{ MA,	0,	printargs,		"lwp_mutex_unlock"	}, /* 250 */
	{ MA,	0,	printargs,		"lwp_mutex_trylock"	}, /* 251 */
	{ MA,	0,	printargs,		"lwp_mutex_init"	}, /* 252 */
	{ MA,	0,	printargs,		"cladm"		}, /* 253 */
	{ MA,	0,	printargs,		"lwp_sig_timedwait"	}, /* 254 */
	{ MA,	0,	printargs,		"umount2"	}, /* 255 */
#endif /* !UNIXWARE */
#endif /* !MIPS */
	{ MA,	0,	printargs,		"SYS_256"	}, /* 256 */
	{ MA,	0,	printargs,		"SYS_257"	}, /* 257 */
	{ MA,	0,	printargs,		"SYS_258"	}, /* 258 */
	{ MA,	0,	printargs,		"SYS_259"	}, /* 259 */
	{ MA,	0,	printargs,		"SYS_260"	}, /* 260 */
	{ MA,	0,	printargs,		"SYS_261"	}, /* 261 */
	{ MA,	0,	printargs,		"SYS_262"	}, /* 262 */
	{ MA,	0,	printargs,		"SYS_263"	}, /* 263 */
	{ MA,	0,	printargs,		"SYS_264"	}, /* 264 */
	{ MA,	0,	printargs,		"SYS_265"	}, /* 265 */
	{ MA,	0,	printargs,		"SYS_266"	}, /* 266 */
	{ MA,	0,	printargs,		"SYS_267"	}, /* 267 */
	{ MA,	0,	printargs,		"SYS_268"	}, /* 268 */
	{ MA,	0,	printargs,		"SYS_269"	}, /* 269 */
	{ MA,	0,	printargs,		"SYS_270"	}, /* 270 */
	{ MA,	0,	printargs,		"SYS_271"	}, /* 271 */
	{ MA,	0,	printargs,		"SYS_272"	}, /* 272 */
	{ MA,	0,	printargs,		"SYS_273"	}, /* 273 */
	{ MA,	0,	printargs,		"SYS_274"	}, /* 274 */
	{ MA,	0,	printargs,		"SYS_275"	}, /* 275 */
	{ MA,	0,	printargs,		"SYS_276"	}, /* 276 */
	{ MA,	0,	printargs,		"SYS_277"	}, /* 277 */
	{ MA,	0,	printargs,		"SYS_278"	}, /* 278 */
	{ MA,	0,	printargs,		"SYS_279"	}, /* 279 */
	{ MA,	0,	printargs,		"SYS_280"	}, /* 280 */
	{ MA,	0,	printargs,		"SYS_281"	}, /* 281 */
	{ MA,	0,	printargs,		"SYS_282"	}, /* 282 */
	{ MA,	0,	printargs,		"SYS_283"	}, /* 283 */
	{ MA,	0,	printargs,		"SYS_284"	}, /* 284 */
	{ MA,	0,	printargs,		"SYS_285"	}, /* 285 */
	{ MA,	0,	printargs,		"SYS_286"	}, /* 286 */
	{ MA,	0,	printargs,		"SYS_287"	}, /* 287 */
	{ MA,	0,	printargs,		"SYS_288"	}, /* 288 */
	{ MA,	0,	printargs,		"SYS_289"	}, /* 289 */
	{ MA,	0,	printargs,		"SYS_290"	}, /* 290 */
	{ MA,	0,	printargs,		"SYS_291"	}, /* 291 */
	{ MA,	0,	printargs,		"SYS_292"	}, /* 292 */
	{ MA,	0,	printargs,		"SYS_293"	}, /* 293 */
	{ MA,	0,	printargs,		"SYS_294"	}, /* 294 */
	{ MA,	0,	printargs,		"SYS_295"	}, /* 295 */
	{ MA,	0,	printargs,		"SYS_296"	}, /* 296 */
	{ MA,	0,	printargs,		"SYS_297"	}, /* 297 */
	{ MA,	0,	printargs,		"SYS_298"	}, /* 298 */
	{ MA,	0,	printargs,		"SYS_299"	}, /* 299 */

	{ MA,	0,	sys_getpgrp,		"getpgrp"	}, /* 300 */
	{ MA,	0,	sys_setpgrp,		"setpgrp"	}, /* 301 */
	{ MA,	0,	sys_getsid,		"getsid"	}, /* 302 */
	{ MA,	0,	sys_setsid,		"setsid"	}, /* 303 */
	{ MA,	0,	sys_getpgid,		"getpgid"	}, /* 304 */
	{ MA,	0,	sys_setpgid,		"setpgid"	}, /* 305 */
	{ MA,	0,	printargs,		"SYS_306"	}, /* 306 */
	{ MA,	0,	printargs,		"SYS_307"	}, /* 307 */
	{ MA,	0,	printargs,		"SYS_308"	}, /* 308 */
	{ MA,	0,	printargs,		"SYS_309"	}, /* 309 */

	{ MA,	TS,	sys_signal,		"signal"	}, /* 310 */
	{ MA,	TS,	sys_sigset,		"sigset"	}, /* 311 */
	{ MA,	TS,	sys_sighold,		"sighold"	}, /* 312 */
	{ MA,	TS,	sys_sigrelse,		"sigrelse"	}, /* 313 */
	{ MA,	TS,	sys_sigignore,		"sigignore"	}, /* 314 */
	{ MA,	TS,	sys_sigpause,		"sigpause"	}, /* 315 */
	{ MA,	0,	printargs,		"SYS_316"	}, /* 316 */
	{ MA,	0,	printargs,		"SYS_317"	}, /* 317 */
	{ MA,	0,	printargs,		"SYS_318"	}, /* 318 */
	{ MA,	0,	printargs,		"SYS_319"	}, /* 319 */

	{ MA,	TI,	sys_msgget,		"msgget"	}, /* 320 */
	{ MA,	TI,	sys_msgctl,		"msgctl"	}, /* 321 */
	{ MA,	TI,	sys_msgrcv,		"msgrcv"	}, /* 322 */
	{ MA,	TI,	sys_msgsnd,		"msgsnd"	}, /* 323 */
	{ MA,	0,	printargs,		"SYS_324"	}, /* 324 */
	{ MA,	0,	printargs,		"SYS_325"	}, /* 325 */
	{ MA,	0,	printargs,		"SYS_326"	}, /* 326 */
	{ MA,	0,	printargs,		"SYS_327"	}, /* 327 */
	{ MA,	0,	printargs,		"SYS_328"	}, /* 328 */
	{ MA,	0,	printargs,		"SYS_329"	}, /* 329 */

	{ MA,	TI,	sys_shmat,		"shmat"		}, /* 330 */
	{ MA,	TI,	sys_shmctl,		"shmctl"	}, /* 331 */
	{ MA,	TI,	sys_shmdt,		"shmdt"		}, /* 332 */
	{ MA,	TI,	sys_shmget,		"shmget"	}, /* 333 */
	{ MA,	0,	printargs,		"SYS_334"	}, /* 334 */
	{ MA,	0,	printargs,		"SYS_335"	}, /* 335 */
	{ MA,	0,	printargs,		"SYS_336"	}, /* 336 */
	{ MA,	0,	printargs,		"SYS_337"	}, /* 337 */
	{ MA,	0,	printargs,		"SYS_338"	}, /* 338 */
	{ MA,	0,	printargs,		"SYS_339"	}, /* 339 */

	{ MA,	TI,	sys_semctl,		"semctl"	}, /* 340 */
	{ MA,	TI,	sys_semget,		"semget"	}, /* 341 */
	{ MA,	TI,	sys_semop,		"semop"		}, /* 342 */
	{ MA,	0,	printargs,		"SYS_343"	}, /* 343 */
	{ MA,	0,	printargs,		"SYS_344"	}, /* 344 */
	{ MA,	0,	printargs,		"SYS_345"	}, /* 345 */
	{ MA,	0,	printargs,		"SYS_346"	}, /* 346 */
	{ MA,	0,	printargs,		"SYS_347"	}, /* 347 */
	{ MA,	0,	printargs,		"SYS_348"	}, /* 348 */
	{ MA,	0,	printargs,		"SYS_349"	}, /* 349 */

	{ MA,	0,	sys_olduname,		"olduname"	}, /* 350 */
	{ MA,	0,	printargs,		"utssys1"	}, /* 351 */
	{ MA,	0,	sys_ustat,		"ustat"		}, /* 352 */
	{ MA,	0,	sys_fusers,		"fusers"	}, /* 353 */
	{ MA,	0,	printargs,		"SYS_354"	}, /* 354 */
	{ MA,	0,	printargs,		"SYS_355"	}, /* 355 */
	{ MA,	0,	printargs,		"SYS_356"	}, /* 356 */
	{ MA,	0,	printargs,		"SYS_357"	}, /* 357 */
	{ MA,	0,	printargs,		"SYS_358"	}, /* 358 */
	{ MA,	0,	printargs,		"SYS_359"	}, /* 359 */

	{ MA,	0,	printargs,		"sysfs0"	}, /* 360 */
	{ MA,	0,	sys_sysfs1,		"sysfs1"	}, /* 361 */
	{ MA,	0,	sys_sysfs2,		"sysfs2"	}, /* 362 */
	{ MA,	0,	sys_sysfs3,		"sysfs3"	}, /* 363 */
	{ MA,	0,	printargs,		"SYS_364"	}, /* 364 */
	{ MA,	0,	printargs,		"SYS_365"	}, /* 365 */
	{ MA,	0,	printargs,		"SYS_366"	}, /* 366 */
	{ MA,	0,	printargs,		"SYS_367"	}, /* 367 */
	{ MA,	0,	printargs,		"SYS_368"	}, /* 368 */
	{ MA,	0,	printargs,		"SYS_369"	}, /* 369 */

	{ MA,	0,	printargs,		"spcall0"	}, /* 370 */
	{ MA,	TS,	sys_sigpending,		"sigpending"	}, /* 371 */
	{ MA,	TS,	sys_sigfillset,		"sigfillset"	}, /* 372 */
	{ MA,	0,	printargs,		"SYS_373"	}, /* 373 */
	{ MA,	0,	printargs,		"SYS_374"	}, /* 374 */
	{ MA,	0,	printargs,		"SYS_375"	}, /* 375 */
	{ MA,	0,	printargs,		"SYS_376"	}, /* 376 */
	{ MA,	0,	printargs,		"SYS_377"	}, /* 377 */
	{ MA,	0,	printargs,		"SYS_378"	}, /* 378 */
	{ MA,	0,	printargs,		"SYS_379"	}, /* 379 */

	{ MA,	0,	sys_getcontext,		"getcontext"	}, /* 380 */
	{ MA,	0,	sys_setcontext,		"setcontext"	}, /* 381 */
	{ MA,	0,	printargs,		"SYS_382"	}, /* 382 */
	{ MA,	0,	printargs,		"SYS_383"	}, /* 383 */
	{ MA,	0,	printargs,		"SYS_384"	}, /* 384 */
	{ MA,	0,	printargs,		"SYS_385"	}, /* 385 */
	{ MA,	0,	printargs,		"SYS_386"	}, /* 386 */
	{ MA,	0,	printargs,		"SYS_387"	}, /* 387 */
	{ MA,	0,	printargs,		"SYS_388"	}, /* 388 */
	{ MA,	0,	printargs,		"SYS_389"	}, /* 389 */

	{ MA,	0,	printargs,		"door_create"	}, /* 390 */
	{ MA,	0,	printargs,		"door_revoke"	}, /* 391 */
	{ MA,	0,	printargs,		"door_info"	}, /* 392 */
	{ MA,	0,	printargs,		"door_call"	}, /* 393 */
	{ MA,	0,	printargs,		"door_return"	}, /* 394 */
	{ MA,	0,	printargs,		"door_cred"	}, /* 395 */
	{ MA,	0,	printargs,		"SYS_396"	}, /* 396 */
	{ MA,	0,	printargs,		"SYS_397"	}, /* 397 */
	{ MA,	0,	printargs,		"SYS_398"	}, /* 398 */
	{ MA,	0,	printargs,		"SYS_399"	}, /* 399 */

#ifdef HAVE_SYS_AIO_H
	{ MA,	TF,	sys_aioread,		"aioread"	}, /* 400 */
	{ MA,	TF,	sys_aiowrite,		"aiowrite"	}, /* 401 */
	{ MA,	TF,	sys_aiowait,		"aiowait"	}, /* 402 */
	{ MA,	TF,	sys_aiocancel,		"aiocancel"	}, /* 403 */
	{ MA,	TF,	sys_aionotify,		"aionotify"	}, /* 404 */
	{ MA,	TF,	sys_aioinit,		"aioinit"	}, /* 405 */
	{ MA,	TF,	sys_aiostart,		"aiostart"	}, /* 406 */
	{ MA,	TF,	sys_aiolio,		"aiolio"	}, /* 407 */
	{ MA,	TF,	sys_aiosuspend,		"aiosuspend"	}, /* 408 */
	{ MA,	TF,	sys_aioerror,		"aioerror"	}, /* 409 */
	{ MA,	TF,	sys_aioliowait,		"aioliowait"	}, /* 410 */
	{ MA,	TF,	sys_aioaread,		"aioaread"	}, /* 411 */
	{ MA,	TF,	sys_aioawrite,		"aioawrite"	}, /* 412 */
	{ MA,	TF,	sys_aiolio64,		"aiolio64"	}, /* 413 */
	{ MA,	TF,	sys_aiosuspend64,	"aiosuspend64"	}, /* 414 */
	{ MA,	TF,	sys_aioerror64,		"aioerror64"	}, /* 415 */
	{ MA,	TF,	sys_aioliowait64,	"aioliowait64"	}, /* 416 */
	{ MA,	TF,	sys_aioaread64,		"aioaread64"	}, /* 417 */
	{ MA,	TF,	sys_aioawrite64,	"aioawrite64"	}, /* 418 */
	{ MA,	TF,	sys_aiocancel64,	"aiocancel64"	}, /* 419 */
	{ MA,	TF,	sys_aiofsync,		"aiofsync"	}, /* 420 */
#endif
