#include "defs.h"

enum {
	CAP_CHOWN,
	CAP_DAC_OVERRIDE,
	CAP_DAC_READ_SEARCH,
	CAP_FOWNER,
	CAP_FSETID,
	CAP_KILL,
	CAP_SETGID,
	CAP_SETUID,
	CAP_SETPCAP,
	CAP_LINUX_IMMUTABLE,
	CAP_NET_BIND_SERVICE,
	CAP_NET_BROADCAST,
	CAP_NET_ADMIN,
	CAP_NET_RAW,
	CAP_IPC_LOCK,
	CAP_IPC_OWNER,
	CAP_SYS_MODULE,
	CAP_SYS_RAWIO,
	CAP_SYS_CHROOT,
	CAP_SYS_PTRACE,
	CAP_SYS_PACCT,
	CAP_SYS_ADMIN,
	CAP_SYS_BOOT,
	CAP_SYS_NICE,
	CAP_SYS_RESOURCE,
	CAP_SYS_TIME,
	CAP_SYS_TTY_CONFIG,
	CAP_MKNOD,
	CAP_LEASE,
	CAP_AUDIT_WRITE,
	CAP_AUDIT_CONTROL,
	CAP_SETFCAP
};

#include "xlat/capabilities.h"

enum {
	_LINUX_CAPABILITY_VERSION_1 = 0x19980330,
	_LINUX_CAPABILITY_VERSION_2 = 0x20071026,
	_LINUX_CAPABILITY_VERSION_3 = 0x20080522
};

#include "xlat/cap_version.h"

typedef struct user_cap_header_struct {
	uint32_t version;
	int pid;
} *cap_user_header_t;

typedef struct user_cap_data_struct {
	uint32_t effective;
	uint32_t permitted;
	uint32_t inheritable;
} *cap_user_data_t;

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
