/*
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2013-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SYS_socket_subcall
# error SYS_socket_subcall is not defined
#endif

#define IS	TRACE_INDIRECT_SUBCALL

[SYS_socket_subcall +  1] = { 3,	IS|TN,	SEN(socket),		"socket"		},
[SYS_socket_subcall +  2] = { 3,	IS|TN,	SEN(bind),		"bind"			},
[SYS_socket_subcall +  3] = { 3,	IS|TN,	SEN(connect),		"connect"		},
[SYS_socket_subcall +  4] = { 2,	IS|TN,	SEN(listen),		"listen"		},
[SYS_socket_subcall +  5] = { 3,	IS|TN,	SEN(accept),		"accept"		},
[SYS_socket_subcall +  6] = { 3,	IS|TN,	SEN(getsockname),	"getsockname"		},
[SYS_socket_subcall +  7] = { 3,	IS|TN,	SEN(getpeername),	"getpeername"		},
[SYS_socket_subcall +  8] = { 4,	IS|TN,	SEN(socketpair),	"socketpair"		},
[SYS_socket_subcall +  9] = { 4,	IS|TN,	SEN(send),		"send"			},
[SYS_socket_subcall + 10] = { 4,	IS|TN,	SEN(recv),		"recv"			},
[SYS_socket_subcall + 11] = { 6,	IS|TN,	SEN(sendto),		"sendto"		},
[SYS_socket_subcall + 12] = { 6,	IS|TN,	SEN(recvfrom),		"recvfrom"		},
[SYS_socket_subcall + 13] = { 2,	IS|TN,	SEN(shutdown),		"shutdown"		},
[SYS_socket_subcall + 14] = { 5,	IS|TN,	SEN(setsockopt),	"setsockopt"		},
[SYS_socket_subcall + 15] = { 5,	IS|TN,	SEN(getsockopt),	"getsockopt"		},
[SYS_socket_subcall + 16] = { 3,	IS|TN,	SEN(sendmsg),		"sendmsg"		},
[SYS_socket_subcall + 17] = { 3,	IS|TN,	SEN(recvmsg),		"recvmsg"		},
[SYS_socket_subcall + 18] = { 4,	IS|TN,	SEN(accept4),		"accept4"		},
[SYS_socket_subcall + 19] = { 5,	IS|TN,	SEN(recvmmsg),		"recvmmsg"		},
[SYS_socket_subcall + 20] = { 4,	IS|TN,	SEN(sendmmsg),		"sendmmsg"		},

#define SYS_socket_nsubcalls	21
#define SYS_ipc_subcall	((SYS_socket_subcall) + (SYS_socket_nsubcalls))

[SYS_ipc_subcall +  1] = { 4,	IS|TI,		SEN(semop),		"semop"			},
[SYS_ipc_subcall +  2] = { 3,	IS|TI,		SEN(semget),		"semget"		},
[SYS_ipc_subcall +  3] = { 4,	IS|TI,		SEN(semctl),		"semctl"		},
[SYS_ipc_subcall +  4] = { 5,	IS|TI,		SEN(semtimedop),	"semtimedop"		},
[SYS_ipc_subcall + 11] = { 4,	IS|TI,		SEN(msgsnd),		"msgsnd"		},
[SYS_ipc_subcall + 12] = { 5,	IS|TI,		SEN(msgrcv),		"msgrcv"		},
[SYS_ipc_subcall + 13] = { 2,	IS|TI,		SEN(msgget),		"msgget"		},
[SYS_ipc_subcall + 14] = { 4,	IS|TI,		SEN(msgctl),		"msgctl"		},
[SYS_ipc_subcall + 21] = { 4,	IS|TI|TM|SI,	SEN(shmat),		"shmat"			},
[SYS_ipc_subcall + 22] = { 4,	IS|TI|TM|SI,	SEN(shmdt),		"shmdt"			},
[SYS_ipc_subcall + 23] = { 3,	IS|TI,		SEN(shmget),		"shmget"		},
[SYS_ipc_subcall + 24] = { 4,	IS|TI,		SEN(shmctl),		"shmctl"		},

#define SYS_ipc_nsubcalls	25

#undef IS
