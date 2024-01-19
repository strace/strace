/*
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * There is nothing to do here if <asm-generic/fcntl.h> is already included,
 * so use the same guard as <asm-generic/fcntl.h> does.
 */
#ifndef _ASM_GENERIC_FCNTL_H

# undef f_owner_ex
# define f_owner_ex kernel_f_owner_ex
# undef flock
# define flock kernel_flock
# undef flock64
# define flock64 kernel_flock64

/*
 * sed -En 's/^#define[[:space:]]+([^[:space:]]+)[[:space:]].*$/# undef \1/p' \
 *	include/uapi/asm-generic/fcntl.h
 */
# undef O_ACCMODE
# undef O_RDONLY
# undef O_WRONLY
# undef O_RDWR
# undef O_CREAT
# undef O_EXCL
# undef O_NOCTTY
# undef O_TRUNC
# undef O_APPEND
# undef O_NONBLOCK
# undef O_DSYNC
# undef FASYNC
# undef O_DIRECT
# undef O_LARGEFILE
# undef O_DIRECTORY
# undef O_NOFOLLOW
# undef O_NOATIME
# undef O_CLOEXEC
# undef __O_SYNC
# undef O_SYNC
# undef O_PATH
# undef __O_TMPFILE
# undef O_TMPFILE
# undef O_TMPFILE_MASK
# undef O_NDELAY
# undef F_DUPFD
# undef F_GETFD
# undef F_SETFD
# undef F_GETFL
# undef F_SETFL
# undef F_GETLK
# undef F_SETLK
# undef F_SETLKW
# undef F_SETOWN
# undef F_GETOWN
# undef F_SETSIG
# undef F_GETSIG
# undef F_GETLK64
# undef F_SETLK64
# undef F_SETLKW64
# undef F_SETOWN_EX
# undef F_GETOWN_EX
# undef F_GETOWNER_UIDS
# undef F_OFD_GETLK
# undef F_OFD_SETLK
# undef F_OFD_SETLKW
# undef F_OWNER_TID
# undef F_OWNER_PID
# undef F_OWNER_PGRP
# undef FD_CLOEXEC
# undef F_RDLCK
# undef F_WRLCK
# undef F_UNLCK
# undef F_EXLCK
# undef F_SHLCK
# undef LOCK_SH
# undef LOCK_EX
# undef LOCK_NB
# undef LOCK_UN
# undef LOCK_MAND
# undef LOCK_READ
# undef LOCK_WRITE
# undef LOCK_RW
# undef F_LINUX_SPECIFIC_BASE

# include <asm/fcntl.h>

# undef f_owner_ex
# undef flock
# undef flock64

#endif /* !_ASM_GENERIC_FCNTL_H */

/*
 * On sparc32 O_NDELAY is erroneously defined to (0x0004 | O_NONBLOCK).
 * On many architectures O_NDELAY is defined to O_NONBLOCK.
 * Both cases are wrong and have to be fixed.
 */
#undef O_NDELAY
