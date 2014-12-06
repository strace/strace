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
	if (insize == 0)
		failed = 1;
	if (!failed) {
		unsigned long capacity = 4 * size + 1;
		unsigned char *buf = (capacity < size) ? NULL : malloc(capacity);
		if (buf == NULL || /* probably a bogus size argument */
			umoven(tcp, arg, size, (char *) &buf[3 * size]) < 0) {
			failed = 1;
		}
		else {
			unsigned char *out = buf;
			unsigned char *in = &buf[3 * size];
			size_t i;
			for (i = 0; i < size; ++i) {
				if (in[i] >= ' ' && in[i] <= 0x7e)
					*out++ = in[i];
				else {
#define tohex(n) "0123456789abcdef"[n]
					*out++ = '\\';
					*out++ = 'x';
					*out++ = tohex(in[i] / 16);
					*out++ = tohex(in[i] % 16);
				}
			}
			/* Don't print terminating NUL if there is one.  */
			if (i > 0 && in[i - 1] == '\0')
				out -= 4;
			*out = '\0';
			tprintf(", \"%s\", %ld", buf, insize);
		}
		free(buf);
	}
	if (failed)
		tprintf(", 0x%lx, %ld", arg, insize);
}

int
sys_setxattr(struct tcb *tcp)
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

int
sys_fsetxattr(struct tcb *tcp)
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

int
sys_getxattr(struct tcb *tcp)
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

int
sys_fgetxattr(struct tcb *tcp)
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

int
sys_listxattr(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_flistxattr(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_removexattr(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	}
	return 0;
}

int
sys_fremovexattr(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	}
	return 0;
}
