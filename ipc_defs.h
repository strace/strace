/*
 * Copyright (c) 2003 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2003-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_IPC_DEFS_H
# define STRACE_IPC_DEFS_H

# ifdef HAVE_LINUX_IPC_H
#  if defined MPERS_IS_m32
#   if defined ARCH_M32_SIZEOF_STRUCT_MSQID64_DS && \
       defined M32_SIZEOF_STRUCT_MSQID64_DS && \
       ARCH_M32_SIZEOF_STRUCT_MSQID64_DS != M32_SIZEOF_STRUCT_MSQID64_DS
#    undef HAVE_LINUX_IPC_H
#   endif
#  elif defined MPERS_IS_mx32
#   if defined ARCH_MX32_SIZEOF_STRUCT_MSQID64_DS && \
       defined MX32_SIZEOF_STRUCT_MSQID64_DS && \
       ARCH_MX32_SIZEOF_STRUCT_MSQID64_DS != MX32_SIZEOF_STRUCT_MSQID64_DS
#    undef HAVE_LINUX_IPC_H
#   endif
#  else /* !IN_MPERS */
#   if defined ARCH_SIZEOF_STRUCT_MSQID64_DS && \
       defined SIZEOF_STRUCT_MSQID64_DS && \
       ARCH_SIZEOF_STRUCT_MSQID64_DS != SIZEOF_STRUCT_MSQID64_DS
#    undef HAVE_LINUX_IPC_H
#   endif
#  endif
# endif /* HAVE_LINUX_IPC_H */

# if defined HAVE_LINUX_IPC_H
#  include <linux/ipc.h>
#  define MSG_H_PROVIDER "linux/msg.h"
#  define SEM_H_PROVIDER "linux/sem.h"
#  define SHM_H_PROVIDER "linux/shm.h"
#  define NAME_OF_STRUCT_MSQID_DS msqid64_ds
#  define NAME_OF_STRUCT_SEMID_DS semid64_ds
#  define NAME_OF_STRUCT_SHMID_DS shmid64_ds
#  define NAME_OF_STRUCT_SHMINFO shminfo64
#  define NAME_OF_STRUCT_IPC_PERM_KEY key
# elif defined HAVE_SYS_IPC_H
#  include <sys/ipc.h>
#  define MSG_H_PROVIDER "sys/msg.h"
#  define SEM_H_PROVIDER "sys/sem.h"
#  define SHM_H_PROVIDER "sys/shm.h"
#  define NAME_OF_STRUCT_MSQID_DS msqid_ds
#  define NAME_OF_STRUCT_SEMID_DS semid_ds
#  define NAME_OF_STRUCT_SHMID_DS shmid_ds
#  define NAME_OF_STRUCT_SHMINFO shminfo
#  define NAME_OF_STRUCT_IPC_PERM_KEY __key
# else
#  error Neither <sys/ipc.h> nor <linux/ipc.h> is available
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
