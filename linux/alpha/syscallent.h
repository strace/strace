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

	{ 5,	0,	printargs,		"SYS_0"		}, /* 0 */
	{ 1,	TP,	sys_exit,		"exit"		}, /* 1 */
	{ 0,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ 3,	TF,	sys_read,		"read"		}, /* 3 */
	{ 3,	TF,	sys_write,		"write"		}, /* 4 */
	{ 5,	0,	printargs,		"SYS_5"		}, /* 5 */
	{ 1,	0,	sys_close,		"close"		}, /* 6 */
	{ 4,	TP,	sys_osf_wait4,		"osf_wait4"	}, /* 7 */
	{ 5,	0,	printargs,		"SYS_8"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 5,	0,	printargs,		"SYS_11"	}, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ 1,	TF,	sys_fchdir,		"fchdir"	}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 16 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 17 */
	{ 5,	0,	printargs,		"SYS_18"	}, /* 18 */
	{ 3,	TF,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getxpid"	}, /* 20 */
	{ 4,	0,	printargs,		"osf_mount"	}, /* 21 */
	{ 2,	0,	sys_umount2,		"umount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	0,	sys_getuid,		"getxuid"	}, /* 24 */
	{ 5,	0,	printargs,		"SYS_25"	}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 5,	0,	printargs,		"SYS_27"	}, /* 27 */
	{ 5,	0,	printargs,		"SYS_28"	}, /* 28 */
	{ 5,	0,	printargs,		"SYS_29"	}, /* 29 */
	{ 5,	0,	printargs,		"SYS_30"	}, /* 30 */
	{ 5,	0,	printargs,		"SYS_31"	}, /* 31 */
	{ 5,	0,	printargs,		"SYS_32"	}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 5,	0,	printargs,		"SYS_34"	}, /* 34 */
	{ 5,	0,	printargs,		"SYS_35"	}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 5,	0,	printargs,		"SYS_38"	}, /* 38 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 39 */
	{ 5,	0,	printargs,		"SYS_40"	}, /* 40 */
	{ 1,	0,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	0,	sys_pipe,		"pipe"		}, /* 42 */
	{ 5,	0,	printargs,		"SYS_43"	}, /* 43 */
	{ 5,	0,	printargs,		"SYS_44"	}, /* 44 */
	{ 3,	TF,	sys_open,		"open"		}, /* 45 */
	{ 5,	0,	printargs,		"SYS_46"	}, /* 46 */
	{ 1,	0,	sys_getgid,		"getxgid"	}, /* 47 */
	{ 3,	TS,	printargs,		"osf_sigprocmask"}, /* 48 */
	{ 5,	0,	printargs,		"SYS_49"	}, /* 49 */
	{ 5,	0,	printargs,		"SYS_50"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 52 */
	{ 5,	0,	printargs,		"SYS_53"	}, /* 53 */
	{ 3,	0,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 5,	0,	printargs,		"SYS_55"	}, /* 55 */
	{ 5,	0,	printargs,		"SYS_56"	}, /* 56 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 57 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 58 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 5,	0,	printargs,		"SYS_62"	}, /* 62 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 63 */
	{ 0,	0,	sys_getpagesize,	"getpagesize"	}, /* 64 */
	{ 5,	0,	printargs,		"SYS_65"	}, /* 65 */
	{ 0,	TP,	sys_fork,		"vfork"		}, /* 66 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 67 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 68 */
	{ 5,	0,	printargs,		"SYS_69"	}, /* 69 */
	{ 5,	0,	printargs,		"SYS_70"	}, /* 70 */
	{ 6,	0,	sys_mmap,		"mmap"		}, /* 71 */
	{ 5,	0,	printargs,		"SYS_72"	}, /* 72 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 73 */
	{ 3,	0,	sys_mprotect,		"mprotect"	}, /* 74 */
	{ 0,	0,	printargs,		"madvise"	}, /* 75 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 76 */
	{ 5,	0,	printargs,		"SYS_77"	}, /* 77 */
	{ 5,	0,	printargs,		"SYS_78"	}, /* 78 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 79 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 80 */
	{ 5,	0,	printargs,		"SYS_81"	}, /* 81 */
	{ 2,	0,	sys_setpgrp,		"setpgrp"	}, /* 82 */
	{ 3,	0,	sys_osf_setitimer,	"osf_setitimer"	}, /* 83 */
	{ 5,	0,	printargs,		"SYS_84"	}, /* 84 */
	{ 5,	0,	printargs,		"SYS_85"	}, /* 85 */
	{ 2,	0,	sys_osf_getitimer,	"osf_getitimer"	}, /* 86 */
	{ 2,	0,	sys_gethostname,	"gethostname"	}, /* 87 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 88 */
	{ 0,	0,	sys_getdtablesize,	"getdtablesize"	}, /* 89 */
	{ 2,	0,	sys_dup2,		"dup2"		}, /* 90 */
	{ 2,	0,	sys_fstat,		"fstat"		}, /* 91 */
	{ 3,	0,	sys_fcntl,		"fcntl"		}, /* 92 */
	{ 5,	0,	sys_osf_select,		"osf_select"	}, /* 93 */
	{ 3,	0,	sys_poll,		"poll"		}, /* 94 */
	{ 1,	0,	sys_fsync,		"fsync"		}, /* 95 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 96 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 97 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 98 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 99 */
	{ 2,	0,	sys_getpriority,	"osf_getpriority"}, /* 100 */
	{ 4,	TN,	sys_send,		"send"		}, /* 101 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 102 */
	{ 1,	TS,	sys_sigreturn,		"sigreturn"	}, /* 103 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 104 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 105 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 106 */
	{ 5,	0,	printargs,		"SYS_107"	}, /* 107 */
	{ 5,	0,	printargs,		"SYS_108"	}, /* 108 */
	{ 5,	0,	printargs,		"SYS_109"	}, /* 109 */
	{ 5,	0,	printargs,		"SYS_110"	}, /* 110 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 111 */
	{ 5,	0,	printargs,		"sigstack"	}, /* 112 */
	{ 3,	TN,	sys_recvmsg,		"recvmsg"	}, /* 113 */
	{ 3,	TN,	sys_sendmsg,		"sendmsg"	}, /* 114 */
	{ 5,	0,	printargs,		"SYS_115"	}, /* 115 */
	{ 2,	0,	sys_osf_gettimeofday,	"osf_gettimeofday"}, /* 116 */
	{ 2,	0,	sys_osf_getrusage,	"osf_getrusage"	}, /* 117 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 118 */
	{ 5,	0,	printargs,		"SYS_119"	}, /* 119 */
	{ 3,	0,	sys_readv,		"readv"		}, /* 120 */
	{ 3,	0,	sys_writev,		"writev"	}, /* 121 */
	{ 2,	0,	sys_osf_settimeofday,	"osf_settimeofday"}, /* 122 */
	{ 3,	0,	sys_fchown,		"fchown"	}, /* 123 */
	{ 2,	0,	sys_fchmod,		"fchmod"	}, /* 124 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 125 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 126 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 127 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 128 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 129 */
	{ 2,	0,	sys_ftruncate,		"ftruncate"	}, /* 130 */
	{ 2,	0,	sys_flock,		"flock"		}, /* 131 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 132 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 133 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 134 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 135 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 136 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 137 */
	{ 2,	0,	sys_osf_utimes,		"osf_utimes"	}, /* 138 */
	{ 5,	0,	printargs,		"SYS_139"	}, /* 139 */
	{ 5,	0,	printargs,		"SYS_140"	}, /* 140 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 141 */
	{ 5,	0,	printargs,		"SYS_142"	}, /* 142 */
	{ 5,	0,	printargs,		"SYS_143"	}, /* 143 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 144 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 145 */
	{ 5,	0,	printargs,		"SYS_146"	}, /* 146 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 147 */
	{ 4,	0,	sys_quotactl,		"quotactl"	}, /* 148 */
	{ 5,	0,	printargs,		"SYS_149"	}, /* 149 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 150 */
	{ 5,	0,	printargs,		"SYS_151"	}, /* 151 */
	{ 5,	0,	printargs,		"SYS_152"	}, /* 152 */
	{ 5,	0,	printargs,		"SYS_153"	}, /* 153 */
	{ 5,	0,	printargs,		"SYS_154"	}, /* 154 */
	{ 5,	0,	printargs,		"SYS_155"	}, /* 155 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 156 */
	{ 5,	0,	printargs,		"SYS_157"	}, /* 157 */
	{ 5,	0,	printargs,		"SYS_158"	}, /* 158 */
	{ 4,	0,	printargs,		"osf_getdirentries"}, /* 159 */
	{ 3,	0,	osf_statfs,		"osf_statfs"	}, /* 160 */
	{ 3,	0,	osf_fstatfs,		"osf_fstatfs"	}, /* 161 */
	{ 5,	0,	printargs,		"SYS_162"	}, /* 162 */
	{ 5,	0,	printargs,		"SYS_163"	}, /* 163 */
	{ 5,	0,	printargs,		"SYS_164"	}, /* 164 */
	{ 2,	0,	printargs,		"osf_getdomainname"}, /* 165 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 166 */
	{ 5,	0,	printargs,		"SYS_167"	}, /* 167 */
	{ 5,	0,	printargs,		"SYS_168"	}, /* 168 */
	{ 5,	0,	printargs,		"SYS_169"	}, /* 169 */
	{ 5,	0,	printargs,		"SYS_170"	}, /* 170 */
	{ 5,	0,	printargs,		"SYS_171"	}, /* 171 */
	{ 5,	0,	printargs,		"SYS_172"	}, /* 172 */
	{ 5,	0,	printargs,		"SYS_173"	}, /* 173 */
	{ 5,	0,	printargs,		"SYS_174"	}, /* 174 */
	{ 5,	0,	printargs,		"SYS_175"	}, /* 175 */
	{ 5,	0,	printargs,		"SYS_176"	}, /* 176 */
	{ 5,	0,	printargs,		"SYS_177"	}, /* 177 */
	{ 5,	0,	printargs,		"SYS_178"	}, /* 178 */
	{ 5,	0,	printargs,		"SYS_179"	}, /* 179 */
	{ 5,	0,	printargs,		"SYS_180"	}, /* 180 */
	{ 5,	0,	printargs,		"SYS_181"	}, /* 181 */
	{ 5,	0,	printargs,		"SYS_182"	}, /* 182 */
	{ 5,	0,	printargs,		"SYS_183"	}, /* 183 */
	{ 5,	0,	printargs,		"SYS_184"	}, /* 184 */
	{ 5,	0,	printargs,		"SYS_185"	}, /* 185 */
	{ 5,	0,	printargs,		"SYS_186"	}, /* 186 */
	{ 5,	0,	printargs,		"SYS_187"	}, /* 187 */
	{ 5,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ 5,	0,	printargs,		"SYS_189"	}, /* 189 */
	{ 5,	0,	printargs,		"SYS_190"	}, /* 190 */
	{ 5,	0,	printargs,		"SYS_191"	}, /* 191 */
	{ 5,	0,	printargs,		"SYS_192"	}, /* 192 */
	{ 5,	0,	printargs,		"SYS_193"	}, /* 193 */
	{ 5,	0,	printargs,		"SYS_194"	}, /* 194 */
	{ 5,	0,	printargs,		"SYS_195"	}, /* 195 */
	{ 5,	0,	printargs,		"SYS_196"	}, /* 196 */
	{ 5,	0,	printargs,		"SYS_197"	}, /* 197 */
	{ 5,	0,	printargs,		"SYS_198"	}, /* 198 */
	{ 4,	0,	printargs,		"osf_swapon"	}, /* 199 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 200 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 201 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 202 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 203 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 204 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 205 */
	{ 4,	TI,	printargs,		"semop"		}, /* 206 */
	{ 1,	0,	printargs,		"osf_utsname"	}, /* 207 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 208 */
	{ 3,	TI,	printargs,		"osf_shmat"	}, /* 209 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 210 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 211 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 212 */
	{ 5,	0,	printargs,		"SYS_213"	}, /* 213 */
	{ 5,	0,	printargs,		"SYS_214"	}, /* 214 */
	{ 5,	0,	printargs,		"SYS_215"	}, /* 215 */
	{ 5,	0,	printargs,		"SYS_216"	}, /* 216 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 217 */
	{ 5,	0,	printargs,		"SYS_218"	}, /* 218 */
	{ 5,	0,	printargs,		"SYS_219"	}, /* 219 */
	{ 5,	0,	printargs,		"SYS_220"	}, /* 220 */
	{ 5,	0,	printargs,		"SYS_221"	}, /* 221 */
	{ 5,	0,	printargs,		"SYS_222"	}, /* 222 */
	{ 5,	0,	printargs,		"SYS_223"	}, /* 223 */
	{ 5,	0,	printargs,		"SYS_224"	}, /* 224 */
	{ 5,	0,	printargs,		"SYS_225"	}, /* 225 */
	{ 5,	0,	printargs,		"SYS_226"	}, /* 226 */
	{ 5,	0,	printargs,		"SYS_227"	}, /* 227 */
	{ 5,	0,	printargs,		"SYS_228"	}, /* 228 */
	{ 5,	0,	printargs,		"SYS_229"	}, /* 229 */
	{ 5,	0,	printargs,		"SYS_230"	}, /* 230 */
	{ 5,	0,	printargs,		"SYS_231"	}, /* 231 */
	{ 5,	0,	printargs,		"SYS_232"	}, /* 232 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 233 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 234 */
	{ 5,	0,	sys_sigaltstack,	"sigaltstack"	}, /* 235 */
	{ 5,	0,	printargs,		"SYS_236"	}, /* 236 */
	{ 5,	0,	printargs,		"SYS_237"	}, /* 237 */
	{ 5,	0,	printargs,		"SYS_238"	}, /* 238 */
	{ 5,	0,	printargs,		"SYS_239"	}, /* 239 */
	{ 5,	0,	printargs,		"osf_sysinfo"	}, /* 240 */
	{ 5,	0,	printargs,		"SYS_241"	}, /* 241 */
	{ 5,	0,	printargs,		"SYS_242"	}, /* 242 */
	{ 5,	0,	printargs,		"SYS_243"	}, /* 243 */
	{ 2,	0,	printargs,		"osf_proplist_syscall"}, /* 244 */
	{ 5,	0,	printargs,		"SYS_245"	}, /* 245 */
	{ 5,	0,	printargs,		"SYS_246"	}, /* 246 */
	{ 5,	0,	printargs,		"SYS_247"	}, /* 247 */
	{ 5,	0,	printargs,		"SYS_248"	}, /* 248 */
	{ 5,	0,	printargs,		"SYS_249"	}, /* 249 */
	{ 2,	0,	printargs,		"osf_usleep_thread"}, /* 250 */
	{ 5,	0,	printargs,		"SYS_251"	}, /* 251 */
	{ 5,	0,	printargs,		"SYS_252"	}, /* 252 */
	{ 5,	0,	printargs,		"SYS_253"	}, /* 253 */
	{ 5,	0,	sys_sysfs,		"sysfs"		}, /* 254 */
	{ 5,	0,	printargs,		"SYS_255"	}, /* 255 */
	{ 5,	0,	printargs,		"osf_getsysinfo"}, /* 256 */
	{ 5,	0,	printargs,		"osf_setsysinfo"}, /* 257 */
	{ 5,	0,	printargs,		"SYS_258"	}, /* 258 */
	{ 5,	0,	printargs,		"SYS_259"	}, /* 259 */
	{ 5,	0,	printargs,		"SYS_260"	}, /* 260 */
	{ 5,	0,	printargs,		"SYS_261"	}, /* 261 */
	{ 5,	0,	printargs,		"SYS_262"	}, /* 262 */
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
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 300 */
	{ 3,	0,	printargs,		"sethae"	}, /* 301 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 302 */
	{ 1,	0,	sys_adjtimex,		"adjtimex32"	}, /* 303 */
	{ 1,	0,	sys_swapoff,		"swapoff"	}, /* 304 */
	{ 3,	0,	sys_getdents,		"getdents"	}, /* 305 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 306 */
	{ 4,	0,	sys_init_module,	"init_module"	}, /* 307 */
	{ 1,	0,	sys_delete_module,	"delete_module"	}, /* 308 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 309 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 310 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 311 */
	{ 2,	TP,	sys_clone,		"clone"		}, /* 312 */
	{ 1,	0,	sys_uselib,		"uselib"	}, /* 313 */
	{ 2,	0,	sys_mlock,		"mlock"		}, /* 314 */
	{ 2,	0,	sys_munlock,		"munlock"	}, /* 315 */
	{ 1,	0,	sys_mlockall,		"mlockall"	}, /* 316 */
	{ 1,	0,	sys_munlockall,		"munlockall"	}, /* 317 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 318 */
	{ 1,	0,	sys_sysctl,		"sysctl"	}, /* 319 */
	{ 0,	0,	sys_idle,		"idle"		}, /* 320 */
	{ 1,	0,	sys_umount,		"oldumount"	}, /* 321 */
	{ 1,	0,	sys_swapon,		"swapon"	}, /* 322 */
	{ 1,	0,	sys_times,		"times"		}, /* 323 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 324 */
	{ 1,	0,	sys_setfsuid,		"setfsuid"	}, /* 325 */
	{ 1,	0,	sys_setfsgid,		"setfsgid"	}, /* 326 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 327 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 328 */
	{ 2,	0,	sys_fstatfs,		"fstatfs"	}, /* 329 */
	{ 2,	0,	sys_sched_setparam,	"sched_setparam"}, /* 330 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 331 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 332 */
	{ 2,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 333 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"	}, /* 334 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 335 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 336 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 337 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 338 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 339 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 340 */
	{ 5,	0,	sys_mremap,		"mremap"	}, /* 341 */
	{ 5,	0,	printargs,		"nfsservctl"	}, /* 342 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 343 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 344 */
	{ 5,	0,	printargs,		"pciconfig_read"}, /* 345 */
	{ 5,	0,	printargs,		"pciconfig_write"}, /* 346 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 347 */
	{ 5,	0,	printargs,		"prctl"		}, /* 348 */
	{ 5,	TF,	sys_pread,		"pread"		}, /* 349 */

	{ 5,	TF,	sys_pwrite,		"pwrite"	}, /* 350 */
	{ 1,	TS,	printargs,		"rt_sigreturn"	}, /* 351 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 352 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 353 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 354 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 355 */
	{ 3,	TS,	sys_rt_sigqueueinfo,	"rt_sigqueueinfo"}, /* 356 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 357 */
	{ 5,	0,	sys_select,		"select"	}, /* 358 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 359 */
	{ 3,	0,	sys_settimeofday,	"settimeofday"	}, /* 360 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 361 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 362 */
	{ 2,	0,	sys_utimes,		"utimes"	}, /* 363 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 364 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 365 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 366 */
	{ 2,	0,	sys_getcwd,		"getcwd"	}, /* 367 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 368 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 369 */
	{ 4,	TF,	sys_sendfile,		"sendfile"	}, /* 370 */
