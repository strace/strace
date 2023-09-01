/*
 * Copyright (c) 2019-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef BASE_NR
# define BASE_NR 0
#endif
[BASE_NR + 424] = { 4,	TD|TS|TP,	SEN(pidfd_send_signal),		"pidfd_send_signal"	},
[BASE_NR + 425] = { 2,	TD,		SEN(io_uring_setup),		"io_uring_setup"	},
[BASE_NR + 426] = { 6,	TD|TS,		SEN(io_uring_enter),		"io_uring_enter"	},
[BASE_NR + 427] = { 4,	TD|TM,		SEN(io_uring_register),		"io_uring_register"	},
[BASE_NR + 428] = { 3,	TD|TF,		SEN(open_tree),			"open_tree"		},
[BASE_NR + 429] = { 5,	TD|TF,		SEN(move_mount),		"move_mount"		},
[BASE_NR + 430] = { 2,	TD,		SEN(fsopen),			"fsopen"		},
[BASE_NR + 431] = { 5,	TD|TF,		SEN(fsconfig),			"fsconfig"		},
[BASE_NR + 432] = { 3,	TD,		SEN(fsmount),			"fsmount"		},
[BASE_NR + 433] = { 3,	TD|TF,		SEN(fspick),			"fspick"		},
[BASE_NR + 434] = { 2,	TD,		SEN(pidfd_open),		"pidfd_open"		},
[BASE_NR + 435] = { 2,	TP,		SEN(clone3),			"clone3"		},
[BASE_NR + 436] = { 3,	0,		SEN(close_range),		"close_range"		},
[BASE_NR + 437] = { 4,	TD|TF,		SEN(openat2),			"openat2"		},
[BASE_NR + 438] = { 3,	TD,		SEN(pidfd_getfd),		"pidfd_getfd"		},
[BASE_NR + 439] = { 4,	TD|TF,		SEN(faccessat2),		"faccessat2"		},
[BASE_NR + 440] = { 5,	TD,		SEN(process_madvise),		"process_madvise"	},
[BASE_NR + 441] = { 6,	TD,		SEN(epoll_pwait2),		"epoll_pwait2"		},
[BASE_NR + 442] = { 5,	TD|TF,		SEN(mount_setattr),		"mount_setattr"		},
[BASE_NR + 443] = { 4,	TD,		SEN(quotactl_fd),		"quotactl_fd"		},
[BASE_NR + 444] = { 3,	TD,		SEN(landlock_create_ruleset),	"landlock_create_ruleset"	},
[BASE_NR + 445] = { 4,	TD,		SEN(landlock_add_rule),		"landlock_add_rule"		},
[BASE_NR + 446] = { 2,	TD,		SEN(landlock_restrict_self),	"landlock_restrict_self"	},
[BASE_NR + 447] = { 1,	TD,		SEN(memfd_secret),		"memfd_secret"			},
[BASE_NR + 448] = { 2,	TD,		SEN(process_mrelease),		"process_mrelease"	},
[BASE_NR + 449] = { 5,	0,		SEN(futex_waitv),		"futex_waitv"	},
[BASE_NR + 450] = { 4,	TM,		SEN(set_mempolicy_home_node),	"set_mempolicy_home_node"	},
[BASE_NR + 451] = { 4,	TD,		SEN(cachestat),			"cachestat"	},
[BASE_NR + 452] = { 4,	TD|TF,		SEN(fchmodat2),			"fchmodat2"	},
