/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

#include "defs.h"

#define _LINUX_SOCKET_H
#define _LINUX_FS_H

#define MS_RDONLY	 1	/* Mount read-only */
#define MS_NOSUID	 2	/* Ignore suid and sgid bits */
#define MS_NODEV	 4	/* Disallow access to device special files */
#define MS_NOEXEC	 8	/* Disallow program execution */
#define MS_SYNCHRONOUS	16	/* Writes are synced at once */
#define MS_REMOUNT	32	/* Alter flags of a mounted FS */
#define MS_MANDLOCK	64	/* Allow mandatory locks on an FS */
#define MS_DIRSYNC	128	/* Directory modifications are synchronous */
#define MS_NOATIME	1024	/* Do not update access times. */
#define MS_NODIRATIME	2048	/* Do not update directory access times */
#define MS_BIND		4096
#define MS_MOVE		8192
#define MS_REC		16384
#define MS_SILENT	32768
#define MS_POSIXACL	(1<<16)	/* VFS does not apply the umask */
#define MS_UNBINDABLE	(1<<17)	/* change to unbindable */
#define MS_PRIVATE	(1<<18)	/* change to private */
#define MS_SLAVE	(1<<19)	/* change to slave */
#define MS_SHARED	(1<<20)	/* change to shared */
#define MS_RELATIME	(1<<21)
#define MS_KERNMOUNT	(1<<22)
#define MS_I_VERSION	(1<<23)
#define MS_STRICTATIME	(1<<24)
#define MS_NOSEC	(1<<28)
#define MS_BORN		(1<<29)
#define MS_ACTIVE	(1<<30)
#define MS_NOUSER	(1<<31)
#define MS_MGC_VAL	0xc0ed0000	/* Magic flag number */
#define MS_MGC_MSK	0xffff0000	/* Magic flag mask */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef HAVE_LINUX_CAPABILITY_H
# include <linux/capability.h>
#endif
#ifdef HAVE_ASM_CACHECTL_H
# include <asm/cachectl.h>
#endif
#ifdef HAVE_LINUX_USTNAME_H
# include <linux/utsname.h>
#endif
#ifdef HAVE_ASM_SYSMIPS_H
# include <asm/sysmips.h>
#endif
#include <linux/sysctl.h>
#include <linux/personality.h>

static const struct xlat mount_flags[] = {
	XLAT(MS_MGC_VAL),
	XLAT(MS_RDONLY),
	XLAT(MS_NOSUID),
	XLAT(MS_NODEV),
	XLAT(MS_NOEXEC),
	XLAT(MS_SYNCHRONOUS),
	XLAT(MS_REMOUNT),
	XLAT(MS_RELATIME),
	XLAT(MS_KERNMOUNT),
	XLAT(MS_I_VERSION),
	XLAT(MS_STRICTATIME),
	XLAT(MS_NOSEC),
	XLAT(MS_BORN),
	XLAT(MS_MANDLOCK),
	XLAT(MS_NOATIME),
	XLAT(MS_NODIRATIME),
	XLAT(MS_BIND),
	XLAT(MS_MOVE),
	XLAT(MS_REC),
	XLAT(MS_SILENT),
	XLAT(MS_POSIXACL),
	XLAT(MS_UNBINDABLE),
	XLAT(MS_PRIVATE),
	XLAT(MS_SLAVE),
	XLAT(MS_SHARED),
	XLAT(MS_ACTIVE),
	XLAT(MS_NOUSER),
	XLAT_END
};

int
sys_mount(struct tcb *tcp)
{
	if (entering(tcp)) {
		int ignore_type = 0, ignore_data = 0;
		unsigned long flags = tcp->u_arg[3];

		/* Discard magic */
		if ((flags & MS_MGC_MSK) == MS_MGC_VAL)
			flags &= ~MS_MGC_MSK;

		if (flags & MS_REMOUNT)
			ignore_type = 1;
		else if (flags & (MS_BIND | MS_MOVE))
			ignore_type = ignore_data = 1;

		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");

		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		if (ignore_type && tcp->u_arg[2])
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printstr(tcp, tcp->u_arg[2], -1);
		tprints(", ");

		printflags(mount_flags, tcp->u_arg[3], "MS_???");
		tprints(", ");

		if (ignore_data && tcp->u_arg[4])
			tprintf("%#lx", tcp->u_arg[4]);
		else
			printstr(tcp, tcp->u_arg[4], -1);
	}
	return 0;
}

#define MNT_FORCE	0x00000001	/* Attempt to forcibily umount */
#define MNT_DETACH	0x00000002	/* Just detach from the tree */
#define MNT_EXPIRE	0x00000004	/* Mark for expiry */

static const struct xlat umount_flags[] = {
	XLAT(MNT_FORCE),
	XLAT(MNT_DETACH),
	XLAT(MNT_EXPIRE),
	XLAT_END
};

int
sys_umount2(struct tcb *tcp)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprints(", ");
		printflags(umount_flags, tcp->u_arg[1], "MNT_???");
	}
	return 0;
}

/* These are not macros, but enums.  We just copy the values by hand
   from Linux 2.6.9 here.  */
static const struct xlat personality_options[] = {
	XLAT(PER_LINUX),
	XLAT(PER_LINUX_32BIT),
	XLAT(PER_LINUX_FDPIC),
	XLAT(PER_SVR4),
	XLAT(PER_SVR3),
	XLAT(PER_SCOSVR3),
	XLAT(PER_OSR5),
	XLAT(PER_WYSEV386),
	XLAT(PER_ISCR4),
	XLAT(PER_BSD),
	XLAT(PER_SUNOS),
	XLAT(PER_XENIX),
	XLAT(PER_LINUX32),
	XLAT(PER_LINUX32_3GB),
	XLAT(PER_IRIX32),
	XLAT(PER_IRIXN32),
	XLAT(PER_IRIX64),
	XLAT(PER_RISCOS),
	XLAT(PER_SOLARIS),
	XLAT(PER_UW7),
	XLAT(PER_OSF4),
	XLAT(PER_HPUX),
	XLAT_END
};

int
sys_personality(struct tcb *tcp)
{
	if (entering(tcp))
		printxval(personality_options, tcp->u_arg[0], "PER_???");
	return 0;
}

enum {
	SYSLOG_ACTION_CLOSE = 0,
	SYSLOG_ACTION_OPEN,
	SYSLOG_ACTION_READ,
	SYSLOG_ACTION_READ_ALL,
	SYSLOG_ACTION_READ_CLEAR,
	SYSLOG_ACTION_CLEAR,
	SYSLOG_ACTION_CONSOLE_OFF,
	SYSLOG_ACTION_CONSOLE_ON,
	SYSLOG_ACTION_CONSOLE_LEVEL,
	SYSLOG_ACTION_SIZE_UNREAD,
	SYSLOG_ACTION_SIZE_BUFFER
};

static const struct xlat syslog_action_type[] = {
	XLAT(SYSLOG_ACTION_CLOSE),
	XLAT(SYSLOG_ACTION_OPEN),
	XLAT(SYSLOG_ACTION_READ),
	XLAT(SYSLOG_ACTION_READ_ALL),
	XLAT(SYSLOG_ACTION_READ_CLEAR),
	XLAT(SYSLOG_ACTION_CLEAR),
	XLAT(SYSLOG_ACTION_CONSOLE_OFF),
	XLAT(SYSLOG_ACTION_CONSOLE_ON),
	XLAT(SYSLOG_ACTION_CONSOLE_LEVEL),
	XLAT(SYSLOG_ACTION_SIZE_UNREAD),
	XLAT(SYSLOG_ACTION_SIZE_BUFFER),
	XLAT_END
};

int
sys_syslog(struct tcb *tcp)
{
	int type = tcp->u_arg[0];

	if (entering(tcp)) {
		/* type */
		printxval(syslog_action_type, type, "SYSLOG_ACTION_???");
		tprints(", ");
	}

	switch (type) {
		case SYSLOG_ACTION_READ:
		case SYSLOG_ACTION_READ_ALL:
		case SYSLOG_ACTION_READ_CLEAR:
			if (entering(tcp))
				return 0;
			break;
		default:
			if (entering(tcp)) {
				tprintf("%#lx, %lu",
					tcp->u_arg[1], tcp->u_arg[2]);
			}
			return 0;
	}

	/* bufp */
	if (syserror(tcp))
		tprintf("%#lx", tcp->u_arg[1]);
	else
		printstr(tcp, tcp->u_arg[1], tcp->u_rval);
	/* len */
	tprintf(", %d", (int) tcp->u_arg[2]);

	return 0;
}

#ifdef M68K
static const struct xlat cacheflush_scope[] = {
#ifdef FLUSH_SCOPE_LINE
	XLAT(FLUSH_SCOPE_LINE),
#endif
#ifdef FLUSH_SCOPE_PAGE
	XLAT(FLUSH_SCOPE_PAGE),
#endif
#ifdef FLUSH_SCOPE_ALL
	XLAT(FLUSH_SCOPE_ALL),
#endif
	XLAT_END
};

static const struct xlat cacheflush_flags[] = {
#ifdef FLUSH_CACHE_BOTH
	XLAT(FLUSH_CACHE_BOTH),
#endif
#ifdef FLUSH_CACHE_DATA
	XLAT(FLUSH_CACHE_DATA),
#endif
#ifdef FLUSH_CACHE_INSN
	XLAT(FLUSH_CACHE_INSN),
#endif
	XLAT_END
};

int
sys_cacheflush(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* scope */
		printxval(cacheflush_scope, tcp->u_arg[1], "FLUSH_SCOPE_???");
		tprints(", ");
		/* flags */
		printflags(cacheflush_flags, tcp->u_arg[2], "FLUSH_CACHE_???");
		/* len */
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}
#endif /* M68K */

#ifdef BFIN

#include <bfin_sram.h>

static const struct xlat sram_alloc_flags[] = {
	XLAT(L1_INST_SRAM),
	XLAT(L1_DATA_A_SRAM),
	XLAT(L1_DATA_B_SRAM),
	XLAT(L1_DATA_SRAM),
	XLAT(L2_SRAM),
	XLAT_END
};

int
sys_sram_alloc(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* size */
		tprintf("%lu, ", tcp->u_arg[0]);
		/* flags */
		printflags(sram_alloc_flags, tcp->u_arg[1], "???_SRAM");
	}
	return 1;
}

#include <asm/cachectl.h>

static const struct xlat cacheflush_flags[] = {
	XLAT(ICACHE),
	XLAT(DCACHE),
	XLAT(BCACHE),
	XLAT_END
};

int
sys_cacheflush(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* start addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* length */
		tprintf("%ld, ", tcp->u_arg[1]);
		/* flags */
		printxval(cacheflush_flags, tcp->u_arg[1], "?CACHE");
	}
	return 0;
}

#endif

#ifdef SH
static const struct xlat cacheflush_flags[] = {
#ifdef CACHEFLUSH_D_INVAL
	XLAT(CACHEFLUSH_D_INVAL),
#endif
#ifdef CACHEFLUSH_D_WB
	XLAT(CACHEFLUSH_D_WB),
#endif
#ifdef CACHEFLUSH_D_PURGE
	XLAT(CACHEFLUSH_D_PURGE),
#endif
#ifdef CACHEFLUSH_I
	XLAT(CACHEFLUSH_I),
#endif
	XLAT_END
};

int
sys_cacheflush(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* len */
		tprintf("%lu, ", tcp->u_arg[1]);
		/* flags */
		printflags(cacheflush_flags, tcp->u_arg[2], "CACHEFLUSH_???");
	}
	return 0;
}
#endif /* SH */

#ifdef SYS_capget

static const struct xlat capabilities[] = {
	{ 1<<CAP_CHOWN,		"CAP_CHOWN"	},
	{ 1<<CAP_DAC_OVERRIDE,	"CAP_DAC_OVERRIDE"},
	{ 1<<CAP_DAC_READ_SEARCH,"CAP_DAC_READ_SEARCH"},
	{ 1<<CAP_FOWNER,	"CAP_FOWNER"	},
	{ 1<<CAP_FSETID,	"CAP_FSETID"	},
	{ 1<<CAP_KILL,		"CAP_KILL"	},
	{ 1<<CAP_SETGID,	"CAP_SETGID"	},
	{ 1<<CAP_SETUID,	"CAP_SETUID"	},
	{ 1<<CAP_SETPCAP,	"CAP_SETPCAP"	},
	{ 1<<CAP_LINUX_IMMUTABLE,"CAP_LINUX_IMMUTABLE"},
	{ 1<<CAP_NET_BIND_SERVICE,"CAP_NET_BIND_SERVICE"},
	{ 1<<CAP_NET_BROADCAST,	"CAP_NET_BROADCAST"},
	{ 1<<CAP_NET_ADMIN,	"CAP_NET_ADMIN"	},
	{ 1<<CAP_NET_RAW,	"CAP_NET_RAW"	},
	{ 1<<CAP_IPC_LOCK,	"CAP_IPC_LOCK"	},
	{ 1<<CAP_IPC_OWNER,	"CAP_IPC_OWNER"	},
	{ 1<<CAP_SYS_MODULE,	"CAP_SYS_MODULE"},
	{ 1<<CAP_SYS_RAWIO,	"CAP_SYS_RAWIO"	},
	{ 1<<CAP_SYS_CHROOT,	"CAP_SYS_CHROOT"},
	{ 1<<CAP_SYS_PTRACE,	"CAP_SYS_PTRACE"},
	{ 1<<CAP_SYS_PACCT,	"CAP_SYS_PACCT"	},
	{ 1<<CAP_SYS_ADMIN,	"CAP_SYS_ADMIN"	},
	{ 1<<CAP_SYS_BOOT,	"CAP_SYS_BOOT"	},
	{ 1<<CAP_SYS_NICE,	"CAP_SYS_NICE"	},
	{ 1<<CAP_SYS_RESOURCE,	"CAP_SYS_RESOURCE"},
	{ 1<<CAP_SYS_TIME,	"CAP_SYS_TIME"	},
	{ 1<<CAP_SYS_TTY_CONFIG,"CAP_SYS_TTY_CONFIG"},
#ifdef CAP_MKNOD
	{ 1<<CAP_MKNOD,		"CAP_MKNOD"	},
#endif
#ifdef CAP_LEASE
	{ 1<<CAP_LEASE,		"CAP_LEASE"	},
#endif
#ifdef CAP_AUDIT_WRITE
	{ 1<<CAP_AUDIT_WRITE,	"CAP_AUDIT_WRITE"},
#endif
#ifdef CAP_AUDIT_CONTROL
	{ 1<<CAP_AUDIT_CONTROL,	"CAP_AUDIT_CONTROL"},
#endif
#ifdef CAP_SETFCAP
	{ 1<<CAP_SETFCAP,	"CAP_SETFCAP"	},
#endif
	XLAT_END
};

#ifndef _LINUX_CAPABILITY_VERSION_1
# define _LINUX_CAPABILITY_VERSION_1 0x19980330
#endif
#ifndef _LINUX_CAPABILITY_VERSION_2
# define _LINUX_CAPABILITY_VERSION_2 0x20071026
#endif
#ifndef _LINUX_CAPABILITY_VERSION_3
# define _LINUX_CAPABILITY_VERSION_3 0x20080522
#endif

static const struct xlat cap_version[] = {
	XLAT(_LINUX_CAPABILITY_VERSION_1),
	XLAT(_LINUX_CAPABILITY_VERSION_2),
	XLAT(_LINUX_CAPABILITY_VERSION_3),
	XLAT_END
};

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

/* Linux 2.6.18+ headers removed CTL_PROC enum.  */
# define CTL_PROC 4
# define CTL_CPU 10		/* older headers lack */
static const struct xlat sysctl_root[] = {
	XLAT(CTL_KERN),
	XLAT(CTL_VM),
	XLAT(CTL_NET),
	XLAT(CTL_PROC),
	XLAT(CTL_FS),
	XLAT(CTL_DEBUG),
	XLAT(CTL_DEV),
	XLAT(CTL_BUS),
	XLAT(CTL_ABI),
	XLAT(CTL_CPU),
	XLAT_END
};

static const struct xlat sysctl_kern[] = {
	XLAT(KERN_OSTYPE),
	XLAT(KERN_OSRELEASE),
	XLAT(KERN_OSREV),
	XLAT(KERN_VERSION),
	XLAT(KERN_SECUREMASK),
	XLAT(KERN_PROF),
	XLAT(KERN_NODENAME),
	XLAT(KERN_DOMAINNAME),
#ifdef KERN_SECURELVL
	XLAT(KERN_SECURELVL),
#endif
	XLAT(KERN_PANIC),
#ifdef KERN_REALROOTDEV
	XLAT(KERN_REALROOTDEV),
#endif
#ifdef KERN_JAVA_INTERPRETER
	XLAT(KERN_JAVA_INTERPRETER),
#endif
#ifdef KERN_JAVA_APPLETVIEWER
	XLAT(KERN_JAVA_APPLETVIEWER),
#endif
	XLAT(KERN_SPARC_REBOOT),
	XLAT(KERN_CTLALTDEL),
	XLAT(KERN_PRINTK),
	XLAT(KERN_NAMETRANS),
	XLAT(KERN_PPC_HTABRECLAIM),
	XLAT(KERN_PPC_ZEROPAGED),
	XLAT(KERN_PPC_POWERSAVE_NAP),
	XLAT(KERN_MODPROBE),
	XLAT(KERN_SG_BIG_BUFF),
	XLAT(KERN_ACCT),
	XLAT(KERN_PPC_L2CR),
	XLAT(KERN_RTSIGNR),
	XLAT(KERN_RTSIGMAX),
	XLAT(KERN_SHMMAX),
	XLAT(KERN_MSGMAX),
	XLAT(KERN_MSGMNB),
	XLAT(KERN_MSGPOOL),
	XLAT_END
};

static const struct xlat sysctl_vm[] = {
#ifdef VM_SWAPCTL
	XLAT(VM_SWAPCTL),
#endif
#ifdef VM_UNUSED1
	XLAT(VM_UNUSED1),
#endif
#ifdef VM_SWAPOUT
	XLAT(VM_SWAPOUT),
#endif
#ifdef VM_UNUSED2
	XLAT(VM_UNUSED2),
#endif
#ifdef VM_FREEPG
	XLAT(VM_FREEPG),
#endif
#ifdef VM_UNUSED3
	XLAT(VM_UNUSED3),
#endif
#ifdef VM_BDFLUSH
	XLAT(VM_BDFLUSH),
#endif
#ifdef VM_UNUSED4
	XLAT(VM_UNUSED4),
#endif
	XLAT(VM_OVERCOMMIT_MEMORY),
#ifdef VM_BUFFERMEM
	XLAT(VM_BUFFERMEM),
#endif
#ifdef VM_UNUSED5
	XLAT(VM_UNUSED5),
#endif
#ifdef VM_PAGECACHE
	XLAT(VM_PAGECACHE),
#endif
#ifdef VM_UNUSED7
	XLAT(VM_UNUSED7),
#endif
#ifdef VM_PAGERDAEMON
	XLAT(VM_PAGERDAEMON),
#endif
#ifdef VM_UNUSED8
	XLAT(VM_UNUSED8),
#endif
#ifdef VM_PGT_CACHE
	XLAT(VM_PGT_CACHE),
#endif
#ifdef VM_UNUSED9
	XLAT(VM_UNUSED9),
#endif
	XLAT(VM_PAGE_CLUSTER),
	XLAT_END
};

static const struct xlat sysctl_net[] = {
	XLAT(NET_CORE),
	XLAT(NET_ETHER),
	XLAT(NET_802),
	XLAT(NET_UNIX),
	XLAT(NET_IPV4),
	XLAT(NET_IPX),
	XLAT(NET_ATALK),
	XLAT(NET_NETROM),
	XLAT(NET_AX25),
	XLAT(NET_BRIDGE),
	XLAT(NET_ROSE),
	XLAT(NET_IPV6),
	XLAT(NET_X25),
	XLAT(NET_TR),
	XLAT(NET_DECNET),
	XLAT_END
};

static const struct xlat sysctl_net_core[] = {
	XLAT(NET_CORE_WMEM_MAX),
	XLAT(NET_CORE_RMEM_MAX),
	XLAT(NET_CORE_WMEM_DEFAULT),
	XLAT(NET_CORE_RMEM_DEFAULT),
	XLAT(NET_CORE_MAX_BACKLOG),
	XLAT(NET_CORE_FASTROUTE),
	XLAT(NET_CORE_MSG_COST),
	XLAT(NET_CORE_MSG_BURST),
	XLAT(NET_CORE_OPTMEM_MAX),
	XLAT_END
};

static const struct xlat sysctl_net_unix[] = {
	XLAT(NET_UNIX_DESTROY_DELAY),
	XLAT(NET_UNIX_DELETE_DELAY),
	XLAT_END
};

static const struct xlat sysctl_net_ipv4[] = {
	XLAT(NET_IPV4_FORWARD),
	XLAT(NET_IPV4_DYNADDR),
	XLAT(NET_IPV4_CONF),
	XLAT(NET_IPV4_NEIGH),
	XLAT(NET_IPV4_ROUTE),
	XLAT(NET_IPV4_FIB_HASH),
	XLAT(NET_IPV4_TCP_TIMESTAMPS),
	XLAT(NET_IPV4_TCP_WINDOW_SCALING),
	XLAT(NET_IPV4_TCP_SACK),
	XLAT(NET_IPV4_TCP_RETRANS_COLLAPSE),
	XLAT(NET_IPV4_DEFAULT_TTL),
	XLAT(NET_IPV4_AUTOCONFIG),
	XLAT(NET_IPV4_NO_PMTU_DISC),
	XLAT(NET_IPV4_TCP_SYN_RETRIES),
	XLAT(NET_IPV4_IPFRAG_HIGH_THRESH),
	XLAT(NET_IPV4_IPFRAG_LOW_THRESH),
	XLAT(NET_IPV4_IPFRAG_TIME),
	XLAT(NET_IPV4_TCP_MAX_KA_PROBES),
	XLAT(NET_IPV4_TCP_KEEPALIVE_TIME),
	XLAT(NET_IPV4_TCP_KEEPALIVE_PROBES),
	XLAT(NET_IPV4_TCP_RETRIES1),
	XLAT(NET_IPV4_TCP_RETRIES2),
	XLAT(NET_IPV4_TCP_FIN_TIMEOUT),
	XLAT(NET_IPV4_IP_MASQ_DEBUG),
	XLAT(NET_TCP_SYNCOOKIES),
	XLAT(NET_TCP_STDURG),
	XLAT(NET_TCP_RFC1337),
	XLAT(NET_TCP_SYN_TAILDROP),
	XLAT(NET_TCP_MAX_SYN_BACKLOG),
	XLAT(NET_IPV4_LOCAL_PORT_RANGE),
	XLAT(NET_IPV4_ICMP_ECHO_IGNORE_ALL),
	XLAT(NET_IPV4_ICMP_ECHO_IGNORE_BROADCASTS),
	XLAT(NET_IPV4_ICMP_SOURCEQUENCH_RATE),
	XLAT(NET_IPV4_ICMP_DESTUNREACH_RATE),
	XLAT(NET_IPV4_ICMP_TIMEEXCEED_RATE),
	XLAT(NET_IPV4_ICMP_PARAMPROB_RATE),
	XLAT(NET_IPV4_ICMP_ECHOREPLY_RATE),
	XLAT(NET_IPV4_ICMP_IGNORE_BOGUS_ERROR_RESPONSES),
	XLAT(NET_IPV4_IGMP_MAX_MEMBERSHIPS),
	XLAT_END
};

static const struct xlat sysctl_net_ipv4_route[] = {
	XLAT(NET_IPV4_ROUTE_FLUSH),
	XLAT(NET_IPV4_ROUTE_MIN_DELAY),
	XLAT(NET_IPV4_ROUTE_MAX_DELAY),
	XLAT(NET_IPV4_ROUTE_GC_THRESH),
	XLAT(NET_IPV4_ROUTE_MAX_SIZE),
	XLAT(NET_IPV4_ROUTE_GC_MIN_INTERVAL),
	XLAT(NET_IPV4_ROUTE_GC_TIMEOUT),
	XLAT(NET_IPV4_ROUTE_GC_INTERVAL),
	XLAT(NET_IPV4_ROUTE_REDIRECT_LOAD),
	XLAT(NET_IPV4_ROUTE_REDIRECT_NUMBER),
	XLAT(NET_IPV4_ROUTE_REDIRECT_SILENCE),
	XLAT(NET_IPV4_ROUTE_ERROR_COST),
	XLAT(NET_IPV4_ROUTE_ERROR_BURST),
	XLAT(NET_IPV4_ROUTE_GC_ELASTICITY),
	XLAT_END
};

static const struct xlat sysctl_net_ipv4_conf[] = {
	XLAT(NET_IPV4_CONF_FORWARDING),
	XLAT(NET_IPV4_CONF_MC_FORWARDING),
	XLAT(NET_IPV4_CONF_PROXY_ARP),
	XLAT(NET_IPV4_CONF_ACCEPT_REDIRECTS),
	XLAT(NET_IPV4_CONF_SECURE_REDIRECTS),
	XLAT(NET_IPV4_CONF_SEND_REDIRECTS),
	XLAT(NET_IPV4_CONF_SHARED_MEDIA),
	XLAT(NET_IPV4_CONF_RP_FILTER),
	XLAT(NET_IPV4_CONF_ACCEPT_SOURCE_ROUTE),
	XLAT(NET_IPV4_CONF_BOOTP_RELAY),
	XLAT(NET_IPV4_CONF_LOG_MARTIANS),
	XLAT_END
};

static const struct xlat sysctl_net_ipv6[] = {
	XLAT(NET_IPV6_CONF),
	XLAT(NET_IPV6_NEIGH),
	XLAT(NET_IPV6_ROUTE),
	XLAT_END
};

static const struct xlat sysctl_net_ipv6_route[] = {
	XLAT(NET_IPV6_ROUTE_FLUSH),
	XLAT(NET_IPV6_ROUTE_GC_THRESH),
	XLAT(NET_IPV6_ROUTE_MAX_SIZE),
	XLAT(NET_IPV6_ROUTE_GC_MIN_INTERVAL),
	XLAT(NET_IPV6_ROUTE_GC_TIMEOUT),
	XLAT(NET_IPV6_ROUTE_GC_INTERVAL),
	XLAT(NET_IPV6_ROUTE_GC_ELASTICITY),
	XLAT_END
};

int
sys_sysctl(struct tcb *tcp)
{
	struct __sysctl_args info;
	int *name;
	unsigned long size;

	if (umove(tcp, tcp->u_arg[0], &info) < 0)
		return printargs(tcp);

	size = sizeof(int) * (unsigned long) info.nlen;
	name = (size / sizeof(int) != info.nlen) ? NULL : malloc(size);
	if (name == NULL ||
	    umoven(tcp, (unsigned long) info.name, size, (char *) name) < 0) {
		free(name);
		if (entering(tcp))
			tprintf("{%p, %d, %p, %p, %p, %lu}",
				info.name, info.nlen, info.oldval,
				info.oldlenp, info.newval, (unsigned long)info.newlen);
		return 0;
	}

	if (entering(tcp)) {
		int cnt = 0, max_cnt;

		tprints("{{");

		if (info.nlen == 0)
			goto out;
		printxval(sysctl_root, name[0], "CTL_???");
		++cnt;

		if (info.nlen == 1)
			goto out;
		switch (name[0]) {
		case CTL_KERN:
			tprints(", ");
			printxval(sysctl_kern, name[1], "KERN_???");
			++cnt;
			break;
		case CTL_VM:
			tprints(", ");
			printxval(sysctl_vm, name[1], "VM_???");
			++cnt;
			break;
		case CTL_NET:
			tprints(", ");
			printxval(sysctl_net, name[1], "NET_???");
			++cnt;

			if (info.nlen == 2)
				goto out;
			switch (name[1]) {
			case NET_CORE:
				tprints(", ");
				printxval(sysctl_net_core, name[2],
					  "NET_CORE_???");
				break;
			case NET_UNIX:
				tprints(", ");
				printxval(sysctl_net_unix, name[2],
					  "NET_UNIX_???");
				break;
			case NET_IPV4:
				tprints(", ");
				printxval(sysctl_net_ipv4, name[2],
					  "NET_IPV4_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV4_ROUTE:
					tprints(", ");
					printxval(sysctl_net_ipv4_route,
						  name[3],
						  "NET_IPV4_ROUTE_???");
					break;
				case NET_IPV4_CONF:
					tprints(", ");
					printxval(sysctl_net_ipv4_conf,
						  name[3],
						  "NET_IPV4_CONF_???");
					break;
				default:
					goto out;
				}
				break;
			case NET_IPV6:
				tprints(", ");
				printxval(sysctl_net_ipv6, name[2],
					  "NET_IPV6_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV6_ROUTE:
					tprints(", ");
					printxval(sysctl_net_ipv6_route,
						  name[3],
						  "NET_IPV6_ROUTE_???");
					break;
				default:
					goto out;
				}
				break;
			default:
				goto out;
			}
			break;
		default:
			goto out;
		}
	out:
		max_cnt = info.nlen;
		if (abbrev(tcp) && max_cnt > max_strlen)
			max_cnt = max_strlen;
		while (cnt < max_cnt)
			tprintf(", %x", name[cnt++]);
		if (cnt < info.nlen)
			tprints(", ...");
		tprintf("}, %d, ", info.nlen);
	} else {
		size_t oldlen = 0;
		if (info.oldval == NULL) {
			tprints("NULL");
		} else if (umove(tcp, (long)info.oldlenp, &oldlen) >= 0
			   && info.nlen >= 2
			   && ((name[0] == CTL_KERN
				&& (name[1] == KERN_OSRELEASE
				    || name[1] == KERN_OSTYPE
#ifdef KERN_JAVA_INTERPRETER
				    || name[1] == KERN_JAVA_INTERPRETER
#endif
#ifdef KERN_JAVA_APPLETVIEWER
				    || name[1] == KERN_JAVA_APPLETVIEWER
#endif
					)))) {
			printpath(tcp, (size_t)info.oldval);
		} else {
			tprintf("%p", info.oldval);
		}
		tprintf(", %lu, ", (unsigned long)oldlen);
		if (info.newval == NULL)
			tprints("NULL");
		else if (syserror(tcp))
			tprintf("%p", info.newval);
		else
			printpath(tcp, (size_t)info.newval);
		tprintf(", %lu", (unsigned long)info.newlen);
	}

	free(name);
	return 0;
}

#ifdef MIPS

#ifndef __NEW_UTS_LEN
#define __NEW_UTS_LEN 64
#endif

static const struct xlat sysmips_operations[] = {
	XLAT(SETNAME),
	XLAT(FLUSH_CACHE),
	XLAT(MIPS_FIXADE),
	XLAT(MIPS_RDNVRAM),
	XLAT(MIPS_ATOMIC_SET),
	XLAT_END
};

int sys_sysmips(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(sysmips_operations, tcp->u_arg[0], "???");
		if (!verbose(tcp)) {
			tprintf("%ld, %ld, %ld", tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
		} else if (tcp->u_arg[0] == SETNAME) {
			char nodename[__NEW_UTS_LEN + 1];
			if (umovestr(tcp, tcp->u_arg[1], (__NEW_UTS_LEN + 1), nodename) < 0)
				tprintf(", %#lx", tcp->u_arg[1]);
			else
				tprintf(", \"%.*s\"", (int)(__NEW_UTS_LEN + 1), nodename);
		} else if (tcp->u_arg[0] == MIPS_ATOMIC_SET) {
			tprintf(", %#lx, 0x%lx", tcp->u_arg[1], tcp->u_arg[2]);
		} else if (tcp->u_arg[0] == MIPS_FIXADE) {
			tprintf(", 0x%lx", tcp->u_arg[1]);
		} else {
			tprintf("%ld, %ld, %ld", tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
		}
	}

	return 0;
}

#endif /* MIPS */

#ifdef OR1K
#define OR1K_ATOMIC_SWAP        1
#define OR1K_ATOMIC_CMPXCHG     2
#define OR1K_ATOMIC_XCHG        3
#define OR1K_ATOMIC_ADD         4
#define OR1K_ATOMIC_DECPOS      5
#define OR1K_ATOMIC_AND         6
#define OR1K_ATOMIC_OR          7
#define OR1K_ATOMIC_UMAX        8
#define OR1K_ATOMIC_UMIN        9

static const struct xlat atomic_ops[] = {
	{ OR1K_ATOMIC_SWAP,		"SWAP"		},
	{ OR1K_ATOMIC_CMPXCHG,		"CMPXCHG"	},
	{ OR1K_ATOMIC_XCHG,		"XCHG"		},
	{ OR1K_ATOMIC_ADD,		"ADD"		},
	{ OR1K_ATOMIC_DECPOS,		"DECPOS"	},
	{ OR1K_ATOMIC_AND,		"AND"		},
	{ OR1K_ATOMIC_OR,		"OR"		},
	{ OR1K_ATOMIC_UMAX,		"UMAX"		},
	{ OR1K_ATOMIC_UMIN,		"UMIN"		},
	XLAT_END
};

int sys_or1k_atomic(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(atomic_ops, tcp->u_arg[0], "???");
		switch(tcp->u_arg[0]) {
		case OR1K_ATOMIC_SWAP:
			tprintf(", 0x%lx, 0x%lx", tcp->u_arg[1], tcp->u_arg[2]);
			break;
		case OR1K_ATOMIC_CMPXCHG:
			tprintf(", 0x%lx, %#lx, %#lx", tcp->u_arg[1], tcp->u_arg[2],
				tcp->u_arg[3]);
			break;

		case OR1K_ATOMIC_XCHG:
		case OR1K_ATOMIC_ADD:
		case OR1K_ATOMIC_AND:
		case OR1K_ATOMIC_OR:
		case OR1K_ATOMIC_UMAX:
		case OR1K_ATOMIC_UMIN:
			tprintf(", 0x%lx, %#lx", tcp->u_arg[1], tcp->u_arg[2]);
			break;

		case OR1K_ATOMIC_DECPOS:
			tprintf(", 0x%lx", tcp->u_arg[1]);
			break;

		default:
			break;
		}
	}

	return RVAL_HEX;
}

#endif /* OR1K */
