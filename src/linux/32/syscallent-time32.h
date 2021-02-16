/*
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

[  4] = { 5,	0,		SEN(io_getevents_time32),	"io_getevents"		},
[ 72] = { 6,	TD,		SEN(pselect6_time32),		"pselect6"		},
[ 73] = { 5,	TD,		SEN(ppoll_time32),		"ppoll"			},
[ 86] = { 4,	TD,		SEN(timerfd_settime32),		"timerfd_settime"	},
[ 87] = { 2,	TD,		SEN(timerfd_gettime32),		"timerfd_gettime"	},
[ 88] = { 4,	TD|TF,		SEN(utimensat_time32),		"utimensat"		},
[ 98] = { 6,	0,		SEN(futex_time32),		"futex"			},
[101] = { 2,	0,		SEN(nanosleep_time32),		"nanosleep"		},
[108] = { 2,	0,		SEN(timer_gettime32),		"timer_gettime"		},
[110] = { 4,	0,		SEN(timer_settime32),		"timer_settime"		},
[112] = { 2,	TCL,		SEN(clock_settime32),		"clock_settime"		},
[113] = { 2,	TCL,		SEN(clock_gettime32),		"clock_gettime"		},
[114] = { 2,	TCL,		SEN(clock_getres_time32),	"clock_getres"		},
[115] = { 4,	0,		SEN(clock_nanosleep_time32),	"clock_nanosleep"	},
[127] = { 2,	0,		SEN(sched_rr_get_interval_time32),"sched_rr_get_interval"},
[137] = { 4,	TS,		SEN(rt_sigtimedwait_time32),	"rt_sigtimedwait"	},
[169] = { 2,	TCL,		SEN(gettimeofday),		"gettimeofday"		},
[170] = { 2,	TCL,		SEN(settimeofday),		"settimeofday"		},
[171] = { 1,	TCL,		SEN(adjtimex32),		"adjtimex"		},
[182] = { 5,	TD,		SEN(mq_timedsend_time32),	"mq_timedsend"		},
[183] = { 5,	TD,		SEN(mq_timedreceive_time32),	"mq_timedreceive"	},
[192] = { 4,	TI,		SEN(semtimedop_time32),		"semtimedop"		},
[243] = { 5,	TN,		SEN(recvmmsg_time32),		"recvmmsg"		},
[260] = { 4,	TP,		SEN(wait4),			"wait4"			},
[266] = { 2,	TCL,		SEN(clock_adjtime32),		"clock_adjtime"		},
[292] = { 6,	0,		SEN(io_pgetevents_time32),	"io_pgetevents"		},
