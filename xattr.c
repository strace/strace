#include "defs.h"

#ifdef HAVE_LINUX_XATTR_H
# include <linux/xattr.h>
#else
# define XATTR_CREATE 1
# define XATTR_REPLACE 2
#endif

#include "xlat/xattrflags.h"

static void
print_xattr_val(struct tcb *tcp, int failed,
		unsigned long arg,
		unsigned long insize,
		unsigned long size)
{
	char *buf;
	unsigned int len;

	if (insize == 0)
		goto failed;

	len = size;
	if (size != (unsigned long) len)
		goto failed;

	if (!len) {
		tprintf(", \"\", %ld", insize);
		return;
	}

	buf = malloc(len);
	if (!buf)
		goto failed;

	if (umoven(tcp, arg, len, buf) < 0) {
		free(buf);
		goto failed;
	}

	/* Don't print terminating NUL if there is one. */
	if (buf[len - 1] == '\0')
		--len;

	tprints(", ");
	print_quoted_string(buf, len, 0);
	tprintf(", %ld", insize);

	free(buf);
	return;

failed:
	tprintf(", 0x%lx, %ld", arg, insize);
}

SYS_FUNC(setxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
		print_xattr_val(tcp, 0, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
		tprints(", ");
		printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	}
	return 0;
}

SYS_FUNC(fsetxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
		print_xattr_val(tcp, 0, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
		tprints(", ");
		printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	}
	return 0;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	} else {
		print_xattr_val(tcp, syserror(tcp), tcp->u_arg[2], tcp->u_arg[3],
				tcp->u_rval);
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
		print_xattr_val(tcp, syserror(tcp), tcp->u_arg[2], tcp->u_arg[3],
				tcp->u_rval);
	}
	return 0;
}

static void
print_xattr_list(struct tcb *tcp, unsigned long addr, unsigned long size)
{
	if (syserror(tcp)) {
		tprintf("%#lx", addr);
	} else {
		if (!addr) {
			tprints("NULL");
		} else {
			unsigned long len =
				(size < (unsigned long) tcp->u_rval) ?
					size : (unsigned long) tcp->u_rval;
			printstr(tcp, addr, len);
		}
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
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	}
	return 0;
}

SYS_FUNC(fremovexattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	}
	return 0;
}
