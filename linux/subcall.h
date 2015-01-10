#ifndef SYS_socket_subcall
# error SYS_socket_subcall is not defined
#endif

#define IS	TRACE_INDIRECT_SUBCALL

[SYS_socket_subcall +  0] = { 6,	0,		printargs,			"socket_subcall"	},
[SYS_socket_subcall +  1] = { 3,	IS|TN,		sys_socket,			"socket"		},
[SYS_socket_subcall +  2] = { 3,	IS|TN,		sys_bind,			"bind"			},
[SYS_socket_subcall +  3] = { 3,	IS|TN,		sys_connect,			"connect"		},
[SYS_socket_subcall +  4] = { 2,	IS|TN,		sys_listen,			"listen"		},
[SYS_socket_subcall +  5] = { 3,	IS|TN,		sys_accept,			"accept"		},
[SYS_socket_subcall +  6] = { 3,	IS|TN,		sys_getsockname,		"getsockname"		},
[SYS_socket_subcall +  7] = { 3,	IS|TN,		sys_getpeername,		"getpeername"		},
[SYS_socket_subcall +  8] = { 4,	IS|TN,		sys_socketpair,			"socketpair"		},
[SYS_socket_subcall +  9] = { 4,	IS|TN,		sys_send,			"send"			},
[SYS_socket_subcall + 10] = { 4,	IS|TN,		sys_recv,			"recv"			},
[SYS_socket_subcall + 11] = { 6,	IS|TN,		sys_sendto,			"sendto"		},
[SYS_socket_subcall + 12] = { 6,	IS|TN,		sys_recvfrom,			"recvfrom"		},
[SYS_socket_subcall + 13] = { 2,	IS|TN,		sys_shutdown,			"shutdown"		},
[SYS_socket_subcall + 14] = { 5,	IS|TN,		sys_setsockopt,			"setsockopt"		},
[SYS_socket_subcall + 15] = { 5,	IS|TN,		sys_getsockopt,			"getsockopt"		},
[SYS_socket_subcall + 16] = { 3,	IS|TN,		sys_sendmsg,			"sendmsg"		},
[SYS_socket_subcall + 17] = { 3,	IS|TN,		sys_recvmsg,			"recvmsg"		},
[SYS_socket_subcall + 18] = { 4,	IS|TN,		sys_accept4,			"accept4"		},
[SYS_socket_subcall + 19] = { 5,	IS|TN,		sys_recvmmsg,			"recvmmsg"		},

#define SYS_socket_nsubcalls	20
#define SYS_ipc_subcall	((SYS_socket_subcall) + (SYS_socket_nsubcalls))

[SYS_socket_subcall + 20] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 21] = { 4,	IS|TI,		sys_semop,			"semop"			},
[SYS_socket_subcall + 22] = { 3,	IS|TI,		sys_semget,			"semget"		},
[SYS_socket_subcall + 23] = { 4,	IS|TI,		sys_semctl,			"semctl"		},
[SYS_socket_subcall + 24] = { 5,	IS|TI,		sys_semtimedop,			"semtimedop"		},
[SYS_socket_subcall + 25] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 26] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 27] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 28] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 29] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 30] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 31] = { 4,	IS|TI,		sys_msgsnd,			"msgsnd"		},
[SYS_socket_subcall + 32] = { 4,	IS|TI,		sys_msgrcv,			"msgrcv"		},
[SYS_socket_subcall + 33] = { 2,	IS|TI,		sys_msgget,			"msgget"		},
[SYS_socket_subcall + 34] = { 4,	IS|TI,		sys_msgctl,			"msgctl"		},
[SYS_socket_subcall + 35] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 36] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 37] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 38] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 39] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 40] = { 6,	0,		printargs,			"ipc_subcall"		},
[SYS_socket_subcall + 41] = { 4,	IS|TI|TM|SI,	sys_shmat,			"shmat"			},
[SYS_socket_subcall + 42] = { 4,	IS|TI|TM|SI,	sys_shmdt,			"shmdt"			},
[SYS_socket_subcall + 43] = { 3,	IS|TI,		sys_shmget,			"shmget"		},
[SYS_socket_subcall + 44] = { 4,	IS|TI,		sys_shmctl,			"shmctl"		},

#define SYS_ipc_nsubcalls	25

#undef IS
