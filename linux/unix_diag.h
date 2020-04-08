/*
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_UNIX_DIAG_H
# define STRACE_LINUX_UNIX_DIAG_H

struct unix_diag_req {
	uint8_t	 sdiag_family;
	uint8_t	 sdiag_protocol;
	uint16_t pad;
	uint32_t udiag_states;
	uint32_t udiag_ino;
	uint32_t udiag_show;
	uint32_t udiag_cookie[2];
};

# define UDIAG_SHOW_NAME		0x01
# define UDIAG_SHOW_VFS		0x02
# define UDIAG_SHOW_PEER		0x04
# define UDIAG_SHOW_ICONS	0x08
# define UDIAG_SHOW_RQLEN	0x10
# define UDIAG_SHOW_MEMINFO	0x20
# define UDIAG_SHOW_UID		0x40

struct unix_diag_msg {
	uint8_t	 udiag_family;
	uint8_t	 udiag_type;
	uint8_t	 udiag_state;
	uint8_t	 pad;
	uint32_t udiag_ino;
	uint32_t udiag_cookie[2];
};

enum {
	UNIX_DIAG_NAME,
	UNIX_DIAG_VFS,
	UNIX_DIAG_PEER,
	UNIX_DIAG_ICONS,
	UNIX_DIAG_RQLEN,
	UNIX_DIAG_MEMINFO,
	UNIX_DIAG_SHUTDOWN,
	UNIX_DIAG_UID,
	UNIX_DIAG_FIRST_UNUSED
};

struct unix_diag_vfs {
	uint32_t udiag_vfs_ino;
	uint32_t udiag_vfs_dev;
};

struct unix_diag_rqlen {
	uint32_t udiag_rqueue;
	uint32_t udiag_wqueue;
};

#endif /* !STRACE_LINUX_UNIX_DIAG_H */
