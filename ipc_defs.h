/*
 * Copyright (c) 2003 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2003-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_IPC_DEFS_H
# define STRACE_IPC_DEFS_H

# ifdef HAVE_SYS_IPC_H
#  include <sys/ipc.h>
# elif defined HAVE_LINUX_IPC_H
#  include <linux/ipc.h>
/* While glibc uses __key, the kernel uses key. */
#  define __key key
# endif

# if !defined IPC_64
#  define IPC_64 0x100
# endif

# define PRINTCTL(flagset, arg, dflt)				\
	do {							\
		if ((arg) & IPC_64) {				\
			print_xlat(IPC_64);			\
			tprints("|");				\
		}						\
		printxval((flagset), (arg) & ~IPC_64, dflt);	\
	} while (0)

#endif /* !STRACE_IPC_DEFS_H */
