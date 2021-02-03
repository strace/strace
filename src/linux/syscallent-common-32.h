/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef BASE_NR
# define BASE_NR 0
#endif
[BASE_NR + 403] = { 2,	TCL,		SEN(clock_gettime64),		"clock_gettime64"	},
[BASE_NR + 404] = { 2,	TCL,		SEN(clock_settime64),		"clock_settime64"	},
[BASE_NR + 405] = { 2,	TCL,		SEN(clock_adjtime64),		"clock_adjtime64"	},
[BASE_NR + 406] = { 2,	TCL,		SEN(clock_getres_time64),	"clock_getres_time64"	},
[BASE_NR + 407] = { 4,	0,		SEN(clock_nanosleep_time64),	"clock_nanosleep_time64"},
[BASE_NR + 408] = { 2,	0,		SEN(timer_gettime64),		"timer_gettime64"	},
[BASE_NR + 409] = { 4,	0,		SEN(timer_settime64),		"timer_settime64"	},
[BASE_NR + 410] = { 2,	TD,		SEN(timerfd_gettime64),		"timerfd_gettime64"	},
[BASE_NR + 411] = { 4,	TD,		SEN(timerfd_settime64),		"timerfd_settime64"	},
[BASE_NR + 412] = { 4,	TD|TF,		SEN(utimensat_time64),		"utimensat_time64"	},
[BASE_NR + 413] = { 6,	TD,		SEN(pselect6_time64),		"pselect6_time64"	},
[BASE_NR + 414] = { 5,	TD,		SEN(ppoll_time64),		"ppoll_time64"		},
[BASE_NR + 416] = { 6,	0,		SEN(io_pgetevents_time64),	"io_pgetevents_time64"	},
[BASE_NR + 417] = { 5,	TN,		SEN(recvmmsg_time64),		"recvmmsg_time64"	},
[BASE_NR + 418] = { 5,	TD,		SEN(mq_timedsend_time64),	"mq_timedsend_time64"	},
[BASE_NR + 419] = { 5,	TD,		SEN(mq_timedreceive_time64),	"mq_timedreceive_time64"},
[BASE_NR + 420] = { 4,	TI,		SEN(semtimedop_time64),		"semtimedop_time64"	},
[BASE_NR + 421] = { 4,	TS,		SEN(rt_sigtimedwait_time64),	"rt_sigtimedwait_time64"},
[BASE_NR + 422] = { 6,	0,		SEN(futex_time64),		"futex_time64"		},
[BASE_NR + 423] = { 2,	0,		SEN(sched_rr_get_interval_time64),	"sched_rr_get_interval_time64"	},
