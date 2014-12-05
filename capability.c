#include "defs.h"

/* these constants are the same as in <linux/capability.h> */
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

/* these constants are CAP_TO_INDEX'ed constants from <linux/capability.h> */
enum {
	CAP_MAC_OVERRIDE,
	CAP_MAC_ADMIN,
	CAP_SYSLOG,
	CAP_WAKE_ALARM,
	CAP_BLOCK_SUSPEND,
	CAP_AUDIT_READ
};

#include "xlat/capabilities1.h"

/* these constants are the same as in <linux/capability.h> */
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

static cap_user_header_t
get_cap_header(struct tcb *tcp, unsigned long addr)
{
	static struct user_cap_header_struct header;

	if (!addr || !verbose(tcp))
		return NULL;

	if (umove(tcp, addr, &header) < 0)
		return NULL;

	return &header;
}

static void
print_cap_header(struct tcb *tcp, unsigned long addr, cap_user_header_t h)
{
	if (!addr) {
		tprints("NULL");
		return;
	}

	if (!h) {
		tprintf("%#lx", addr);
		return;
	}

	tprints("{");
	printxval(cap_version, h->version,
		  "_LINUX_CAPABILITY_VERSION_???");
	tprintf(", %d}", h->pid);
}

static void
print_cap_bits(const uint32_t lo, const uint32_t hi)
{
	if (lo || !hi)
		printflags(capabilities, lo, "CAP_???");

	if (hi) {
		if (lo)
			tprints("|");
		printflags(capabilities1, hi, "CAP_???");
	}
}

static void
print_cap_data(struct tcb *tcp, unsigned long addr, const cap_user_header_t h)
{
	struct user_cap_data_struct data[2];
	unsigned int len;

	if (!addr) {
		tprints("NULL");
		return;
	}

	if (!h || !verbose(tcp) ||
	    (exiting(tcp) && syserror(tcp))) {
		tprintf("%#lx", addr);
		return;
	}

	if (_LINUX_CAPABILITY_VERSION_2 == h->version ||
	    _LINUX_CAPABILITY_VERSION_3 == h->version)
		len = 2;
	else
		len = 1;

	if (umoven(tcp, addr, len * sizeof(data[0]), (char *) data) < 0) {
		tprintf("%#lx", addr);
		return;
	}

	tprints("{");
	print_cap_bits(data[0].effective, len > 1 ? data[1].effective : 0);
	tprints(", ");
	print_cap_bits(data[0].permitted, len > 1 ? data[1].permitted : 0);
	tprints(", ");
	print_cap_bits(data[0].inheritable, len > 1 ? data[1].inheritable : 0);
	tprints("}");
}

int
sys_capget(struct tcb *tcp)
{
	cap_user_header_t h;

	if (entering(tcp)) {
		h = get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_header(tcp, tcp->u_arg[0], h);
		tprints(", ");
	} else {
		h = syserror(tcp) ? NULL : get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_data(tcp, tcp->u_arg[1], h);
	}
	return 0;
}

int
sys_capset(struct tcb *tcp)
{
	if (entering(tcp)) {
		cap_user_header_t h = get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_header(tcp, tcp->u_arg[0], h);
		tprints(", ");
		print_cap_data(tcp, tcp->u_arg[1], h);
	}
	return 0;
}
