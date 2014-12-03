#include "defs.h"

#define _LINUX_SOCKET_H
#define _LINUX_FS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef HAVE_LINUX_CAPABILITY_H
# include <linux/capability.h>
#endif

#ifdef SYS_capget

#include "xlat/capabilities.h"

#ifndef _LINUX_CAPABILITY_VERSION_1
# define _LINUX_CAPABILITY_VERSION_1 0x19980330
#endif
#ifndef _LINUX_CAPABILITY_VERSION_2
# define _LINUX_CAPABILITY_VERSION_2 0x20071026
#endif
#ifndef _LINUX_CAPABILITY_VERSION_3
# define _LINUX_CAPABILITY_VERSION_3 0x20080522
#endif

#include "xlat/cap_version.h"

static void
print_cap_header(struct tcb *tcp, unsigned long addr)
{
	union { cap_user_header_t p; long *a; char *c; } arg;
	long a[sizeof(*arg.p) / sizeof(long) + 1];
	arg.a = a;

	if (!addr)
		tprints("NULL");
	else if (!verbose(tcp) ||
		 umoven(tcp, addr, sizeof(*arg.p), arg.c) < 0)
		tprintf("%#lx", addr);
	else {
		tprints("{");
		printxval(cap_version, arg.p->version,
			  "_LINUX_CAPABILITY_VERSION_???");
		tprintf(", %d}", arg.p->pid);
	}
}

static void
print_cap_data(struct tcb *tcp, unsigned long addr)
{
	union { cap_user_data_t p; long *a; char *c; } arg;
	long a[sizeof(*arg.p) / sizeof(long) + 1];
	arg.a = a;

	if (!addr)
		tprints("NULL");
	else if (!verbose(tcp) ||
		 (exiting(tcp) && syserror(tcp)) ||
		 umoven(tcp, addr, sizeof(*arg.p), arg.c) < 0)
		tprintf("%#lx", addr);
	else {
		tprints("{");
		printflags(capabilities, arg.p->effective, "CAP_???");
		tprints(", ");
		printflags(capabilities, arg.p->permitted, "CAP_???");
		tprints(", ");
		printflags(capabilities, arg.p->inheritable, "CAP_???");
		tprints("}");
	}
}

int
sys_capget(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_cap_header(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_cap_data(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_capset(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_cap_header(tcp, tcp->u_arg[0]);
		tprints(", ");
		print_cap_data(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#else

int sys_capget(struct tcb *tcp)
{
	return printargs(tcp);
}

int sys_capset(struct tcb *tcp)
{
	return printargs(tcp);
}

#endif
