#include "defs.h"

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "xlat/xattrflags.h"

static void
print_xattr_val(struct tcb *tcp,
		unsigned long addr,
		unsigned long insize,
		unsigned long size)
{
	char *buf = NULL;
	unsigned int len;

	tprints(", ");

	if (insize == 0)
		goto done;

	len = size;
	if (size != (unsigned long) len)
		goto done;

	if (!len) {
		tprintf("\"\", %ld", insize);
		return;
	}

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)))
		goto done;

	buf = malloc(len);
	if (!buf)
		goto done;

	if (umoven(tcp, addr, len, buf) < 0) {
		free(buf);
		buf = NULL;
		goto done;
	}

	/* Don't print terminating NUL if there is one. */
	if (buf[len - 1] == '\0')
		--len;

done:
	if (buf) {
		print_quoted_string(buf, len, 0);
		free(buf);
	} else {
		printaddr(addr);
	}
	tprintf(", %ld", insize);
}

SYS_FUNC(setxattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(fsetxattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

SYS_FUNC(fgetxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

static void
print_xattr_list(struct tcb *tcp, unsigned long addr, unsigned long size)
{
	if (syserror(tcp)) {
		printaddr(addr);
	} else {
		unsigned long len =
			(size < (unsigned long) tcp->u_rval) ?
				size : (unsigned long) tcp->u_rval;
		printstr(tcp, addr, len);
	}
	tprintf(", %lu", size);
}

SYS_FUNC(listxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(flistxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(removexattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	return RVAL_DECODED;
}

SYS_FUNC(fremovexattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	return RVAL_DECODED;
}
