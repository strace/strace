#ifdef STRACE_UID_SIZE
# if STRACE_UID_SIZE != 16
#  error invalid STRACE_UID_SIZE
# endif

# define SIZEIFY(x)		SIZEIFY_(x,STRACE_UID_SIZE)
# define SIZEIFY_(x,size)	SIZEIFY__(x,size)
# define SIZEIFY__(x,size)	x ## size

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
# if !NEED_UID16_PARSERS
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
	if (exiting(tcp))
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

SYS_FUNC(setfsuid)
{
	if (entering(tcp))
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	else
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

SYS_FUNC(setuid)
{
	if (entering(tcp)) {
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	}
	return 0;
}

static void
get_print_uid(struct tcb *tcp, const char *prefix, const long addr)
{
	uid_t uid;

	if (umove(tcp, addr, &uid) < 0)
		tprintf("%s%#lx", prefix, addr);
	else
		tprintf("%s[%u]", prefix, uid);
}

SYS_FUNC(getresuid)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		} else {
			get_print_uid(tcp, "", tcp->u_arg[0]);
			get_print_uid(tcp, ", ", tcp->u_arg[1]);
			get_print_uid(tcp, ", ", tcp->u_arg[2]);
		}
	}
	return 0;
}

SYS_FUNC(setreuid)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(setresuid)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(chown)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(fchown)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

void
printuid(const char *text, const unsigned int uid)
{
	if ((unsigned int) -1 == uid || (uid_t) -1 == uid)
		tprintf("%s-1", text);
	else
		tprintf("%s%u", text, uid);
}

SYS_FUNC(setgroups)
{
	if (entering(tcp)) {
		unsigned long len, size, start, cur, end, abbrev_end;
		uid_t gid;
		int failed = 0;

		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%u", (unsigned int) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(getgroups)
{
	unsigned long len;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
	} else {
		unsigned long size, start, cur, end, abbrev_end;
		uid_t gid;
		int failed = 0;

		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		len = tcp->u_rval;
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || tcp->u_arg[0] == 0 ||
		    size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%u", (unsigned int) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

#endif /* STRACE_UID_SIZE */
