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

#include "xlat/mount_flags.h"

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

#include "xlat/umount_flags.h"

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
#include "xlat/personality_options.h"

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

#include "xlat/syslog_action_type.h"

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
#include "xlat/cacheflush_scope.h"

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

#include "xlat/sram_alloc_flags.h"

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

#include "xlat/sysctl_root.h"
#include "xlat/sysctl_kern.h"
#include "xlat/sysctl_vm.h"
#include "xlat/sysctl_net.h"
#include "xlat/sysctl_net_core.h"
#include "xlat/sysctl_net_unix.h"
#include "xlat/sysctl_net_ipv4.h"
#include "xlat/sysctl_net_ipv4_route.h"
#include "xlat/sysctl_net_ipv4_conf.h"
#include "xlat/sysctl_net_ipv6.h"
#include "xlat/sysctl_net_ipv6_route.h"

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

#include "xlat/sysmips_operations.h"

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

#include "xlat/atomic_ops.h"

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
