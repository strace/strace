/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef STRACE_UID_SIZE
# if STRACE_UID_SIZE != 16
#  error invalid STRACE_UID_SIZE
# endif

# define SIZEIFY(x)		SIZEIFY_(x, STRACE_UID_SIZE)
# define SIZEIFY_(x, size)	SIZEIFY__(x, size)
# define SIZEIFY__(x, size)	x ## size

# define printuid	SIZEIFY(printuid)
# define sys_chown	SIZEIFY(sys_chown)
# define sys_fchown	SIZEIFY(sys_fchown)
# define sys_getgroups	SIZEIFY(sys_getgroups)
# define sys_getresuid	SIZEIFY(sys_getresuid)
# define sys_getuid	SIZEIFY(sys_getuid)
# define sys_setfsuid	SIZEIFY(sys_setfsuid)
# define sys_setgroups	SIZEIFY(sys_setgroups)
# define sys_setresuid	SIZEIFY(sys_setresuid)
# define sys_setreuid	SIZEIFY(sys_setreuid)
# define sys_setuid	SIZEIFY(sys_setuid)
#endif /* STRACE_UID_SIZE */

#include "defs.h"

#ifdef STRACE_UID_SIZE
# if !HAVE_ARCH_UID16_SYSCALLS
#  undef STRACE_UID_SIZE
# endif
#else
# define STRACE_UID_SIZE 32
#endif

#ifdef STRACE_UID_SIZE

# undef uid_t
# define uid_t		uid_t_(STRACE_UID_SIZE)
# define uid_t_(size)	uid_t__(size)
# define uid_t__(size)	uint ## size ## _t

SYS_FUNC(getuid)
{
	return RVAL_DECODED;
}

SYS_FUNC(setfsuid)
{
	printuid("", tcp->u_arg[0]);

	return RVAL_DECODED;
}

SYS_FUNC(setuid)
{
	printuid("", tcp->u_arg[0]);

	return RVAL_DECODED;
}

static void
get_print_uid(struct tcb *const tcp, const char *const prefix,
	      const kernel_ulong_t addr)
{
	uid_t uid;

	tprints(prefix);
	if (!umove_or_printaddr(tcp, addr, &uid)) {
		printuid("[", uid);
		tprints("]");
	}
}

SYS_FUNC(getresuid)
{
	if (entering(tcp))
		return 0;

	get_print_uid(tcp, "", tcp->u_arg[0]);
	get_print_uid(tcp, ", ", tcp->u_arg[1]);
	get_print_uid(tcp, ", ", tcp->u_arg[2]);

	return 0;
}

SYS_FUNC(setreuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setresuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(chown)
{
	printpath(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(fchown)
{
	printfd(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

void
printuid(const char *text, const unsigned int uid)
{
	if ((uid_t) -1U == (uid_t) uid)
		tprintf("%s-1", text);
	else
		tprintf("%s%u", text, (uid_t) uid);
}

static bool
print_gid(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	printuid("", (*(uid_t *) elem_buf));

	return true;
}

static void
print_groups(struct tcb *const tcp, const unsigned int len,
	     const kernel_ulong_t addr)
{
	static unsigned long ngroups_max;
	if (!ngroups_max)
		ngroups_max = sysconf(_SC_NGROUPS_MAX);

	if (len > ngroups_max) {
		printaddr(addr);
		return;
	}

	uid_t gid;
	print_array(tcp, addr, len, &gid, sizeof(gid),
		    tfetch_mem, print_gid, 0);
}

SYS_FUNC(setgroups)
{
	const int len = tcp->u_arg[0];

	tprintf("%d, ", len);
	print_groups(tcp, len, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(getgroups)
{
	if (entering(tcp))
		tprintf("%d, ", (int) tcp->u_arg[0]);
	else
		print_groups(tcp, tcp->u_rval, tcp->u_arg[1]);
	return 0;
}

#endif /* STRACE_UID_SIZE */
