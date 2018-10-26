/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef STRACE_UID_SIZE
# if STRACE_UID_SIZE != 16
#  error invalid STRACE_UID_SIZE
# endif

# define SIZEIFY(x)		SIZEIFY_(x, STRACE_UID_SIZE)
# define SIZEIFY_(x, size)	SIZEIFY__(x, size)
# define SIZEIFY__(x, size)	x ## size

# define printuid	SIZEIFY(printuid)
# define printgid	SIZEIFY(printgid)
# define sys_chown	SIZEIFY(sys_chown)
# define sys_fchown	SIZEIFY(sys_fchown)
# define sys_getgroups	SIZEIFY(sys_getgroups)
# define sys_getresuid	SIZEIFY(sys_getresuid)
# define sys_getresgid	SIZEIFY(sys_getresgid)
# define sys_getuid	SIZEIFY(sys_getuid)
# define sys_getgid	SIZEIFY(sys_getgid)
# define sys_setfsuid	SIZEIFY(sys_setfsuid)
# define sys_setfsgid	SIZEIFY(sys_setfsgid)
# define sys_setgroups	SIZEIFY(sys_setgroups)
# define sys_setresuid	SIZEIFY(sys_setresuid)
# define sys_setresgid	SIZEIFY(sys_setresgid)
# define sys_setreuid	SIZEIFY(sys_setreuid)
# define sys_setregid	SIZEIFY(sys_setregid)
# define sys_setuid	SIZEIFY(sys_setuid)
# define sys_setgid	SIZEIFY(sys_setgid)
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

# undef gid_t
# define gid_t		gid_t_(STRACE_UID_SIZE)
# define gid_t_(size)	gid_t__(size)
# define gid_t__(size)	uint ## size ## _t

# include <grp.h>
# include <pwd.h>
# include <sys/types.h>

enum id_type {
	IDT_UID,
	IDT_GID,

	IDT_COUNT
};

SYS_FUNC(getuid)
{
	return RVAL_DECODED | RVAL_UID;
}

SYS_FUNC(getgid)
{
	return RVAL_DECODED | RVAL_GID;
}

SYS_FUNC(setfsuid)
{
	printuid("", tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_UID;
}

SYS_FUNC(setfsgid)
{
	printgid("", tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_GID;
}

SYS_FUNC(setuid)
{
	printuid("", tcp->u_arg[0]);

	return RVAL_DECODED;
}

SYS_FUNC(setgid)
{
	printgid("", tcp->u_arg[0]);

	return RVAL_DECODED;
}

static void
get_print_id(struct tcb *const tcp, const char *const prefix,
	      const kernel_ulong_t addr, enum id_type idt)
{
	uid_t id;

	tprints(prefix);
	if (!umove_or_printaddr(tcp, addr, &id)) {
		(idt == IDT_UID ? printuid : printgid)("[", id);
		tprints("]");
	}
}

SYS_FUNC(getresuid)
{
	if (entering(tcp))
		return 0;

	get_print_id(tcp, "", tcp->u_arg[0], IDT_UID);
	get_print_id(tcp, ", ", tcp->u_arg[1], IDT_UID);
	get_print_id(tcp, ", ", tcp->u_arg[2], IDT_UID);

	return 0;
}

SYS_FUNC(getresgid)
{
	if (entering(tcp))
		return 0;

	get_print_id(tcp, "", tcp->u_arg[0], IDT_GID);
	get_print_id(tcp, ", ", tcp->u_arg[1], IDT_GID);
	get_print_id(tcp, ", ", tcp->u_arg[2], IDT_GID);

	return 0;
}

SYS_FUNC(setreuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setregid)
{
	printgid("", tcp->u_arg[0]);
	printgid(", ", tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setresuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(setresgid)
{
	printgid("", tcp->u_arg[0]);
	printgid(", ", tcp->u_arg[1]);
	printgid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(chown)
{
	printpath(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printgid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(fchown)
{
	printfd(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printgid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

#define ID_CACHE_SIZE 32

static void
print_id(const char *prefix, const uid_t id, enum id_type t)
{
	static const uint32_t mask = 0x80000000;

	if ((uid_t) -1U == (uid_t) id) {
		tprintf("%s-1", prefix);
		return;
	}

	tprintf("%s%u", prefix, (uid_t) id);

	if (!show_fd_path)
		return;

	static struct cache_ent {
		uint32_t  id;
		uint32_t  str_len;
		char     *str;
	} id_cache[IDT_COUNT * ID_CACHE_SIZE] = { { 0 } };

	struct cache_ent *cache = id_cache + t * ID_CACHE_SIZE;
	const size_t idx = (id ^ mask) % ID_CACHE_SIZE;
	const char *str = NULL;

	if ((cache[idx].id ^ mask) == id) {
		str = cache[idx].str;
		goto print;
	}

	switch (t) {
	case IDT_UID: {
		/* NB: not thread-safe */
		struct passwd *pwd = getpwuid((uid_t) id);
		if (!pwd)
			break;

		str = pwd->pw_name;
		break;
	}
	case IDT_GID: {
		/* NB: not thread-safe */
		struct group *gr = getgrgid((gid_t) id);
		if (!gr)
			break;

		str = gr->gr_name;
		break;
	}
	default:
		break;
	}

	free(cache[idx].str);
	cache[idx].id = id ^ mask;
	cache[idx].str = str ? xstrdup(str) : NULL;
	cache[idx].str_len = str ? strlen(str) : 0;

print:
	if (str) {
		tprints("<");
		print_quoted_string_ex(cache[idx].str, cache[idx].str_len,
			       QUOTE_OMIT_LEADING_TRAILING_QUOTES,
			       "<>");
		tprints(">");
	}
}

void
printuid(const char *prefix, const unsigned int uid)
{
	print_id(prefix, uid, IDT_UID);
}

void
printgid(const char *prefix, const unsigned int gid)
{
	print_id(prefix, gid, IDT_GID);
}

static bool
print_gid(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	printgid("", (*(uid_t *) elem_buf));

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
