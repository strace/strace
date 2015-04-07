#include "defs.h"

#include <fcntl.h>

#ifdef O_LARGEFILE
# if O_LARGEFILE == 0          /* biarch platforms in 64-bit mode */
#  undef O_LARGEFILE
#  ifdef SPARC64
#   define O_LARGEFILE 0x40000
#  elif defined X86_64 || defined S390X
#   define O_LARGEFILE 0100000
#  endif
# endif
#endif

#include "xlat/open_access_modes.h"
#include "xlat/open_mode_flags.h"

#ifndef AT_FDCWD
# define AT_FDCWD                -100
#endif

/* The fd is an "int", so when decoding x86 on x86_64, we need to force sign
 * extension to get the right value.  We do this by declaring fd as int here.
 */
void
print_dirfd(struct tcb *tcp, int fd)
{
	if (fd == AT_FDCWD)
		tprints("AT_FDCWD, ");
	else {
		printfd(tcp, fd);
		tprints(", ");
	}
}

/*
 * low bits of the open(2) flags define access mode,
 * other bits are real flags.
 */
const char *
sprint_open_modes(int flags)
{
	static char outstr[(1 + ARRAY_SIZE(open_mode_flags)) * sizeof("O_LARGEFILE")];
	char *p;
	char sep;
	const char *str;
	const struct xlat *x;

	sep = ' ';
	p = stpcpy(outstr, "flags");
	str = xlookup(open_access_modes, flags & 3);
	if (str) {
		*p++ = sep;
		p = stpcpy(p, str);
		flags &= ~3;
		if (!flags)
			return outstr;
		sep = '|';
	}

	for (x = open_mode_flags; x->str; x++) {
		if ((flags & x->val) == x->val) {
			*p++ = sep;
			p = stpcpy(p, x->str);
			flags &= ~x->val;
			if (!flags)
				return outstr;
			sep = '|';
		}
	}
	/* flags is still nonzero */
	*p++ = sep;
	sprintf(p, "%#x", flags);
	return outstr;
}

void
tprint_open_modes(int flags)
{
	tprints(sprint_open_modes(flags) + sizeof("flags"));
}

static int
decode_open(struct tcb *tcp, int offset)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprints(", ");
		/* flags */
		tprint_open_modes(tcp->u_arg[offset + 1]);
		if (tcp->u_arg[offset + 1] & O_CREAT) {
			/* mode */
			tprintf(", %#lo", tcp->u_arg[offset + 2]);
		}
	}
	return RVAL_FD;
}

SYS_FUNC(open)
{
	return decode_open(tcp, 0);
}

SYS_FUNC(openat)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_open(tcp, 1);
}

SYS_FUNC(creat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return RVAL_FD;
}
