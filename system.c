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
 *
 *	$Id$
 */

#include "defs.h"

#ifdef LINUX
#define _LINUX_SOCKET_H
#define _LINUX_FS_H

#define MS_RDONLY   1  /* Mount read-only */
#define MS_NOSUID   2  /* Ignore suid and sgid bits */
#define MS_NODEV    4  /* Disallow access to device special files */
#define MS_NOEXEC   8  /* Disallow program execution */
#define MS_SYNCHRONOUS 16  /* Writes are synced at once */
#define MS_REMOUNT 32  /* Alter flags of a mounted FS */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/syscall.h>

#ifdef SYS_personality
/* Workaround for kernel namespace pollution. */
#define _LINUX_PTRACE_H
/* Yuck yuck yuck.  We can't include linux/ptrace.h, but personality.h
   makes a declaration with struct pt_regs, which is defined there. */
struct pt_regs;
#define sys_personality kernel_sys_personality
#include <linux/personality.h>
#undef sys_personality
#endif /* SYS_personality */

#ifdef SYS_capget
#include <linux/capability.h>
#endif

#ifdef SYS_cacheflush
#include <asm/cachectl.h>
#endif

#ifdef LINUX
#include <linux/sysctl.h>
#endif

static struct xlat mount_flags[] = {
	{ MS_RDONLY,	"MS_RDONLY"	},
	{ MS_NOSUID,	"MS_NOSUID"	},
	{ MS_NODEV,	"MS_NODEV"	},
	{ MS_NOEXEC,	"MS_NOEXEC"	},
#ifdef MS_SYNCHRONOUS
	{ MS_SYNCHRONOUS,"MS_SYNCHRONOUS"},
#else
	{ MS_SYNC,	"MS_SYNC"	},
#endif
	{ MS_REMOUNT,	"MS_REMOUNT"	},
	{ 0,		NULL		},
};

int
sys_mount(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[2]);
		tprintf(", ");
		printflags(mount_flags, tcp->u_arg[3]);
		tprintf(", %#lx", tcp->u_arg[4]);
	}
	return 0;
}

int
sys_umount2(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", ");
		if (tcp->u_arg[1] & 1)
			tprintf("MNT_FORCE");
		else
			tprintf("0");
	}
	return 0;
}

static struct xlat personality_options[] = {
#ifdef PER_LINUX
	{ PER_LINUX,	"PER_LINUX"	},
#endif
#ifdef PER_LINUX_32BIT
	{ PER_LINUX_32BIT,	"PER_LINUX"	},
#endif
#ifdef PER_SVR4
	{ PER_SVR4,	"PER_SVR4"	},
#endif
#ifdef PER_SVR3
	{ PER_SVR3,	"PER_SVR3"	},
#endif
#ifdef PER_SCOSVR3
	{ PER_SCOSVR3,	"PER_SCOSVR3"	},
#endif
#ifdef PER_WYSEV386
	{ PER_WYSEV386,	"PER_WYSEV386"	},
#endif
#ifdef PER_ISCR4
	{ PER_ISCR4,	"PER_ISCR4"	},
#endif
#ifdef PER_BSD
	{ PER_BSD,	"PER_BSD"	},
#endif
#ifdef PER_XENIX
	{ PER_XENIX,	"PER_XENIX"	},
#endif
	{ 0,		NULL		},
};

int
sys_personality(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		printxval(personality_options, tcp->u_arg[0], "PER_???");
	return 0;
}

#ifdef M68K
static struct xlat cacheflush_scope[] = {
#ifdef FLUSH_SCOPE_LINE
	{ FLUSH_SCOPE_LINE,	"FLUSH_SCOPE_LINE" },
#endif
#ifdef FLUSH_SCOPE_PAGE
	{ FLUSH_SCOPE_PAGE,	"FLUSH_SCOPE_PAGE" },
#endif
#ifdef FLUSH_SCOPE_ALL
	{ FLUSH_SCOPE_ALL,	"FLUSH_SCOPE_ALL" },
#endif
	{ 0,			NULL },
};

static struct xlat cacheflush_flags[] = {
#ifdef FLUSH_CACHE_BOTH
	{ FLUSH_CACHE_BOTH,	"FLUSH_CACHE_BOTH" },
#endif
#ifdef FLUSH_CACHE_DATA
	{ FLUSH_CACHE_DATA,	"FLUSH_CACHE_DATA" },
#endif
#ifdef FLUSH_CACHE_INSN
	{ FLUSH_CACHE_INSN,	"FLUSH_CACHE_INSN" },
#endif
	{ 0,			NULL },
};

int
sys_cacheflush(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* scope */
		printxval(cacheflush_scope, tcp->u_arg[1], "FLUSH_SCOPE_???");
		tprintf(", ");
		/* flags */
		printflags(cacheflush_flags, tcp->u_arg[2]);
		/* len */
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}
#endif /* M68K */

#endif /* LINUX */

#ifdef SUNOS4

#include <sys/reboot.h>
#define NFSCLIENT
#define LOFS
#define RFS
#define PCFS
#include <sys/mount.h>
#include <sys/socket.h>
#include <nfs/export.h>
#include <rpc/types.h>
#include <rpc/auth.h>

/*ARGSUSED*/
int
sys_sync(tcp)
struct tcb *tcp;
{
	return 0;
}

static struct xlat bootflags[] = {
	{ RB_AUTOBOOT,	"RB_AUTOBOOT"	},	/* for system auto-booting itself */
	{ RB_ASKNAME,	"RB_ASKNAME"	},	/* ask for file name to reboot from */
	{ RB_SINGLE,	"RB_SINGLE"	},	/* reboot to single user only */
	{ RB_NOSYNC,	"RB_NOSYNC"	},	/* dont sync before reboot */
	{ RB_HALT,	"RB_HALT"	},	/* don't reboot, just halt */
	{ RB_INITNAME,	"RB_INITNAME"	},	/* name given for /etc/init */
	{ RB_NOBOOTRC,	"RB_NOBOOTRC"	},	/* don't run /etc/rc.boot */
	{ RB_DEBUG,	"RB_DEBUG"	},	/* being run under debugger */
	{ RB_DUMP,	"RB_DUMP"	},	/* dump system core */
	{ RB_WRITABLE,	"RB_WRITABLE"	},	/* mount root read/write */
	{ RB_STRING,	"RB_STRING"	},	/* pass boot args to prom monitor */
	{ 0,		NULL		},
};

int
sys_reboot(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		if (!printflags(bootflags, tcp->u_arg[0]))
			tprintf("RB_???");
		if (tcp->u_arg[0] & RB_STRING) {
			printstr(tcp, tcp->u_arg[1], -1);
		}
	}
	return 0;
}

int
sys_sysacct(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
	}
	return 0;
}

int
sys_swapon(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
	}
	return 0;
}

int
sys_nfs_svc(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printsock(tcp, tcp->u_arg[0]);
	}
	return 0;
}

static struct xlat mountflags[] = {
	{ M_RDONLY,	"M_RDONLY"	},
	{ M_NOSUID,	"M_NOSUID"	},
	{ M_NEWTYPE,	"M_NEWTYPE"	},
	{ M_GRPID,	"M_GRPID"	},
#ifdef	M_REMOUNT
	{ M_REMOUNT,	"M_REMOUNT"	},
#endif
#ifdef	M_NOSUB
	{ M_NOSUB,	"M_NOSUB"	},
#endif
#ifdef	M_MULTI
	{ M_MULTI,	"M_MULTI"	},
#endif
#ifdef	M_SYS5
	{ M_SYS5,	"M_SYS5"	},
#endif
	{ 0,		NULL		},
};

static struct xlat nfsflags[] = {
	{ NFSMNT_SOFT,		"NFSMNT_SOFT"		},
	{ NFSMNT_WSIZE,		"NFSMNT_WSIZE"		},
	{ NFSMNT_RSIZE,		"NFSMNT_RSIZE"		},
	{ NFSMNT_TIMEO,		"NFSMNT_TIMEO"		},
	{ NFSMNT_RETRANS,	"NFSMNT_RETRANS"	},
	{ NFSMNT_HOSTNAME,	"NFSMNT_HOSTNAME"	},
	{ NFSMNT_INT,		"NFSMNT_INT"		},
	{ NFSMNT_NOAC,		"NFSMNT_NOAC"		},
	{ NFSMNT_ACREGMIN,	"NFSMNT_ACREGMIN"	},
	{ NFSMNT_ACREGMAX,	"NFSMNT_ACREGMAX"	},
	{ NFSMNT_ACDIRMIN,	"NFSMNT_ACDIRMIN"	},
	{ NFSMNT_ACDIRMAX,	"NFSMNT_ACDIRMAX"	},
#ifdef	NFSMNT_SECURE
	{ NFSMNT_SECURE,	"NFSMNT_SECURE"		},
#endif
#ifdef	NFSMNT_NOCTO
	{ NFSMNT_NOCTO,		"NFSMNT_NOCTO"		},
#endif
#ifdef	NFSMNT_POSIX
	{ NFSMNT_POSIX,		"NFSMNT_POSIX"		},
#endif
	{ 0,			NULL			},
};

int
sys_mount(tcp)
struct tcb *tcp;
{
	char type[4];

	if (entering(tcp)) {
		if (!(tcp->u_arg[2] & M_NEWTYPE) || umovestr(tcp,
				tcp->u_arg[0],  sizeof type, type) < 0) {
			tprintf("OLDTYPE:#%lx", tcp->u_arg[0]);
		} else {
			tprintf("\"%s\", ", type);
		}
		printstr(tcp, tcp->u_arg[1], -1);
		tprintf(", ");
		if (!printflags(mountflags, tcp->u_arg[2] & ~M_NEWTYPE))
			tprintf("0");
		tprintf(", ");

		if (strcmp(type, "4.2") == 0) {
			struct ufs_args a;
			if (umove(tcp, tcp->u_arg[3], &a) < 0)
				return 0;
			printstr(tcp, (int)a.fspec, -1);
		} else if (strcmp(type, "lo") == 0) {
			struct lo_args a;
			if (umove(tcp, tcp->u_arg[3], &a) < 0)
				return 0;
			printstr(tcp, (int)a.fsdir, -1);
		} else if (strcmp(type, "nfs") == 0) {
			struct nfs_args a;
			if (umove(tcp, tcp->u_arg[3], &a) < 0)
				return 0;
			tprintf("[");
			printsock(tcp, (int) a.addr);
			tprintf(", ");
			if (!printflags(nfsflags, a.flags))
				tprintf("NFSMNT_???");
			tprintf(", ws:%u,rs:%u,to:%u,re:%u,",
				a.wsize, a.rsize, a.timeo, a.retrans);
			if (a.flags & NFSMNT_HOSTNAME && a.hostname)
				printstr(tcp, (int)a.hostname, -1);
			else
				tprintf("%#lx", (unsigned long) a.hostname);
			tprintf(",reg-min:%u,max:%u,dir-min:%u,max:%u,",
				a.acregmin, a.acregmax, a.acdirmin, a.acdirmax);
			if ((a.flags & NFSMNT_SECURE) && a.netname)
				printstr(tcp, (int) a.netname, -1);
			else
				tprintf("%#lx", (unsigned long) a.netname);
			tprintf("]");
		} else if (strcmp(type, "rfs") == 0) {
			struct rfs_args a;
			struct token t;
			if (umove(tcp, tcp->u_arg[3], &a) < 0)
				return 0;
			tprintf("[");
			printstr(tcp, (int)a.rmtfs, -1);
			if (umove(tcp, (int)a.token, &t) < 0)
				return 0;
			tprintf(", %u, %s]", t.t_id, t.t_uname);
		} else if (strcmp(type, "pcfs") == 0) {
			struct pc_args a;
			if (umove(tcp, tcp->u_arg[3], &a) < 0)
				return 0;
			printstr(tcp, (int)a.fspec, -1);
		}
	}
	return 0;
}

int
sys_unmount(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
	}
	return 0;
}

int
sys_umount(tcp)
struct tcb *tcp;
{
	return sys_unmount(tcp);
}

int
sys_auditsys(tcp)
struct tcb *tcp;
{
	/* XXX - no information available */
	return printargs(tcp);
}

static struct xlat ex_auth_flags[] = {
	{ AUTH_UNIX,	"AUTH_UNIX"	},
	{ AUTH_DES,	"AUTH_DES"	},
	{ 0,		NULL		},
};

int
sys_exportfs(tcp)
struct tcb *tcp;
{
	struct export e;
	int i;

	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		if (umove(tcp, tcp->u_arg[1], &e) < 0) {
			tprintf("%#lx", tcp->u_arg[1]);
			return 0;
		}
		tprintf("{fl:%u, anon:%u, ", e.ex_flags, e.ex_anon);
		printxval(ex_auth_flags, e.ex_auth, "AUTH_???");
		tprintf(", roots:[");
		if (e.ex_auth == AUTH_UNIX) {
			for (i=0; i<e.ex_u.exunix.rootaddrs.naddrs; i++) {
				printsock(tcp,
					(int)&e.ex_u.exunix.rootaddrs.addrvec[i]);
			}
			tprintf("], writers:[");
			for (i=0; i<e.ex_writeaddrs.naddrs; i++) {
				printsock(tcp,
					(int)&e.ex_writeaddrs.addrvec[i]);
			}
			tprintf("]");
		} else {
			for (i=0; i<e.ex_u.exdes.nnames; i++) {
				printsock(tcp,
					(int)&e.ex_u.exdes.rootnames[i]);
				tprintf(", ");
			}
			tprintf("], window:%u", e.ex_u.exdes.window);
		}
		tprintf("}");
	}
	return 0;
}

static struct xlat sysconflimits[] = {
#ifdef	_SC_ARG_MAX
	{ _SC_ARG_MAX,	"_SC_ARG_MAX"	},	/* space for argv & envp */
#endif
#ifdef	_SC_CHILD_MAX
	{ _SC_CHILD_MAX,	"_SC_CHILD_MAX"	},	/* maximum children per process??? */
#endif
#ifdef	_SC_CLK_TCK
	{ _SC_CLK_TCK,	"_SC_CLK_TCK"	},	/* clock ticks/sec */
#endif
#ifdef	_SC_NGROUPS_MAX
	{ _SC_NGROUPS_MAX,	"_SC_NGROUPS_MAX"	},	/* number of groups if multple supp. */
#endif
#ifdef	_SC_OPEN_MAX
	{ _SC_OPEN_MAX,	"_SC_OPEN_MAX"	},	/* max open files per process */
#endif
#ifdef	_SC_JOB_CONTROL
	{ _SC_JOB_CONTROL,	"_SC_JOB_CONTROL"	},	/* do we have job control */
#endif
#ifdef	_SC_SAVED_IDS
	{ _SC_SAVED_IDS,	"_SC_SAVED_IDS"	},	/* do we have saved uid/gids */
#endif
#ifdef	_SC_VERSION
	{ _SC_VERSION,	"_SC_VERSION"	},	/* POSIX version supported */
#endif
	{ 0,		NULL		},
};

static struct xlat pathconflimits[] = {
#ifdef	_PC_LINK_MAX
	{ _PC_LINK_MAX,	"_PC_LINK_MAX"	},	/* max links to file/dir */
#endif
#ifdef	_PC_MAX_CANON
	{ _PC_MAX_CANON,	"_PC_MAX_CANON"	},	/* max line length */
#endif
#ifdef	_PC_MAX_INPUT
	{ _PC_MAX_INPUT,	"_PC_MAX_INPUT"	},	/* max "packet" to a tty device */
#endif
#ifdef	_PC_NAME_MAX
	{ _PC_NAME_MAX,	"_PC_NAME_MAX"	},	/* max pathname component length */
#endif
#ifdef	_PC_PATH_MAX
	{ _PC_PATH_MAX,	"_PC_PATH_MAX"	},	/* max pathname length */
#endif
#ifdef	_PC_PIPE_BUF
	{ _PC_PIPE_BUF,	"_PC_PIPE_BUF"	},	/* size of a pipe */
#endif
#ifdef	_PC_CHOWN_RESTRICTED
	{ _PC_CHOWN_RESTRICTED,	"_PC_CHOWN_RESTRICTED"	},	/* can we give away files */
#endif
#ifdef	_PC_NO_TRUNC
	{ _PC_NO_TRUNC,	"_PC_NO_TRUNC"	},	/* trunc or error on >NAME_MAX */
#endif
#ifdef	_PC_VDISABLE
	{ _PC_VDISABLE,	"_PC_VDISABLE"	},	/* best char to shut off tty c_cc */
#endif
	{ 0,		NULL		},
};

int
sys_sysconf(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(sysconflimits, tcp->u_arg[0], "_SC_???");
	}
	return 0;
}

int
sys_pathconf(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", ");
		printxval(pathconflimits, tcp->u_arg[1], "_SC_???");
	}
	return 0;
}

int
sys_fpathconf(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		printxval(pathconflimits, tcp->u_arg[1], "_SC_???");
	}
	return 0;
}

#endif /* SUNOS4 */

#ifdef SVR4

#ifdef HAVE_SYS_SYSCONFIG_H
#include <sys/sysconfig.h>
#endif /* HAVE_SYS_SYSCONFIG_H */

#include <sys/mount.h>
#include <sys/systeminfo.h>
#include <sys/utsname.h>

static struct xlat sysconfig_options[] = {
#ifdef _CONFIG_NGROUPS
	{ _CONFIG_NGROUPS,		"_CONFIG_NGROUPS"		},
#endif
#ifdef _CONFIG_CHILD_MAX
	{ _CONFIG_CHILD_MAX,		"_CONFIG_CHILD_MAX"		},
#endif
#ifdef _CONFIG_OPEN_FILES
	{ _CONFIG_OPEN_FILES,		"_CONFIG_OPEN_FILES"		},
#endif
#ifdef _CONFIG_POSIX_VER
	{ _CONFIG_POSIX_VER,		"_CONFIG_POSIX_VER"		},
#endif
#ifdef _CONFIG_PAGESIZE
	{ _CONFIG_PAGESIZE,		"_CONFIG_PAGESIZE"		},
#endif
#ifdef _CONFIG_CLK_TCK
	{ _CONFIG_CLK_TCK,		"_CONFIG_CLK_TCK"		},
#endif
#ifdef _CONFIG_XOPEN_VER
	{ _CONFIG_XOPEN_VER,		"_CONFIG_XOPEN_VER"		},
#endif
#ifdef _CONFIG_PROF_TCK
	{ _CONFIG_PROF_TCK,		"_CONFIG_PROF_TCK"		},
#endif
#ifdef _CONFIG_NPROC_CONF
	{ _CONFIG_NPROC_CONF,		"_CONFIG_NPROC_CONF"		},
#endif
#ifdef _CONFIG_NPROC_ONLN
	{ _CONFIG_NPROC_ONLN,		"_CONFIG_NPROC_ONLN"		},
#endif
#ifdef _CONFIG_AIO_LISTIO_MAX
	{ _CONFIG_AIO_LISTIO_MAX,	"_CONFIG_AIO_LISTIO_MAX"	},
#endif
#ifdef _CONFIG_AIO_MAX
	{ _CONFIG_AIO_MAX,		"_CONFIG_AIO_MAX"		},
#endif
#ifdef _CONFIG_AIO_PRIO_DELTA_MAX
	{ _CONFIG_AIO_PRIO_DELTA_MAX,	"_CONFIG_AIO_PRIO_DELTA_MAX"	},
#endif
#ifdef _CONFIG_CONFIG_DELAYTIMER_MAX
	{ _CONFIG_DELAYTIMER_MAX,	"_CONFIG_DELAYTIMER_MAX"	},
#endif
#ifdef _CONFIG_MQ_OPEN_MAX
	{ _CONFIG_MQ_OPEN_MAX,		"_CONFIG_MQ_OPEN_MAX"		},
#endif
#ifdef _CONFIG_MQ_PRIO_MAX
	{ _CONFIG_MQ_PRIO_MAX,		"_CONFIG_MQ_PRIO_MAX"		},
#endif
#ifdef _CONFIG_RTSIG_MAX
	{ _CONFIG_RTSIG_MAX,		"_CONFIG_RTSIG_MAX"		},
#endif
#ifdef _CONFIG_SEM_NSEMS_MAX
	{ _CONFIG_SEM_NSEMS_MAX,	"_CONFIG_SEM_NSEMS_MAX"		},
#endif
#ifdef _CONFIG_SEM_VALUE_MAX
	{ _CONFIG_SEM_VALUE_MAX,	"_CONFIG_SEM_VALUE_MAX"		},
#endif
#ifdef _CONFIG_SIGQUEUE_MAX
	{ _CONFIG_SIGQUEUE_MAX,		"_CONFIG_SIGQUEUE_MAX"		},
#endif
#ifdef _CONFIG_SIGRT_MIN
	{ _CONFIG_SIGRT_MIN,		"_CONFIG_SIGRT_MIN"		},
#endif
#ifdef _CONFIG_SIGRT_MAX
	{ _CONFIG_SIGRT_MAX,		"_CONFIG_SIGRT_MAX"		},
#endif
#ifdef _CONFIG_TIMER_MAX
	{ _CONFIG_TIMER_MAX,		"_CONFIG_TIMER_MAX"		},
#endif
#ifdef _CONFIG_CONFIG_PHYS_PAGES
	{ _CONFIG_PHYS_PAGES,		"_CONFIG_PHYS_PAGES"		},
#endif
#ifdef _CONFIG_AVPHYS_PAGES
	{ _CONFIG_AVPHYS_PAGES,		"_CONFIG_AVPHYS_PAGES"		},
#endif
	{ 0,				NULL				},
};

int
sys_sysconfig(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		printxval(sysconfig_options, tcp->u_arg[0], "_CONFIG_???");
	return 0;
}

static struct xlat sysinfo_options[] = {
	{ SI_SYSNAME,		"SI_SYSNAME"		},
	{ SI_HOSTNAME,		"SI_HOSTNAME"		},
	{ SI_RELEASE,		"SI_RELEASE"		},
	{ SI_VERSION,		"SI_VERSION"		},
	{ SI_MACHINE,		"SI_MACHINE"		},
	{ SI_ARCHITECTURE,	"SI_ARCHITECTURE"	},
	{ SI_HW_SERIAL,		"SI_HW_SERIAL"		},
	{ SI_HW_PROVIDER,	"SI_HW_PROVIDER"	},
	{ SI_SRPC_DOMAIN,	"SI_SRPC_DOMAIN"	},
#ifdef SI_SET_HOSTNAME
	{ SI_SET_HOSTNAME,	"SI_SET_HOSTNAME"	},
#endif
#ifdef SI_SET_SRPC_DOMAIN
	{ SI_SET_SRPC_DOMAIN,	"SI_SET_SRPC_DOMAIN"	},
#endif
#ifdef SI_SET_KERB_REALM
	{ SI_SET_KERB_REALM,	"SI_SET_KERB_REALM"	},
#endif
#ifdef 	SI_KERB_REALM
	{ SI_KERB_REALM,	"SI_KERB_REALM"		},
#endif
	{ 0,			NULL			},
};

int
sys_sysinfo(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(sysinfo_options, tcp->u_arg[0], "SI_???");
		tprintf(", ");
	}
	else {
		/* Technically some calls write values.  So what. */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printpath(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

#ifdef MIPS

#include <sys/syssgi.h>

static struct xlat syssgi_options[] = {
	{ SGI_SYSID,		"SGI_SYSID"		},
#ifdef SGI_RDUBLK
	{ SGI_RDUBLK,		"SGI_RDUBLK"		},
#endif
	{ SGI_TUNE,		"SGI_TUNE"		},
	{ SGI_IDBG,		"SGI_IDBG"		},
	{ SGI_INVENT,		"SGI_INVENT"		},
	{ SGI_RDNAME,		"SGI_RDNAME"		},
	{ SGI_SETLED,		"SGI_SETLED"		},
	{ SGI_SETNVRAM,		"SGI_SETNVRAM"		},
	{ SGI_GETNVRAM,		"SGI_GETNVRAM"		},
	{ SGI_QUERY_FTIMER,	"SGI_QUERY_FTIMER"	},
	{ SGI_QUERY_CYCLECNTR,	"SGI_QUERY_CYCLECNTR"	},
	{ SGI_PROCSZ,		"SGI_PROCSZ"		},
	{ SGI_SIGACTION,	"SGI_SIGACTION"		},
	{ SGI_SIGPENDING,	"SGI_SIGPENDING"	},
	{ SGI_SIGPROCMASK,	"SGI_SIGPROCMASK"	},
	{ SGI_SIGSUSPEND,	"SGI_SIGSUSPEND"	},
	{ SGI_SETSID,		"SGI_SETSID"		},
	{ SGI_SETPGID,		"SGI_SETPGID"		},
	{ SGI_SYSCONF,		"SGI_SYSCONF"		},
	{ SGI_WAIT4,		"SGI_WAIT4"		},
	{ SGI_PATHCONF,		"SGI_PATHCONF"		},
	{ SGI_READB,		"SGI_READB"		},
	{ SGI_WRITEB,		"SGI_WRITEB"		},
	{ SGI_SETGROUPS,	"SGI_SETGROUPS"		},
	{ SGI_GETGROUPS,	"SGI_GETGROUPS"		},
	{ SGI_SETTIMEOFDAY,	"SGI_SETTIMEOFDAY"	},
	{ SGI_SETTIMETRIM,	"SGI_SETTIMETRIM"	},
	{ SGI_GETTIMETRIM,	"SGI_GETTIMETRIM"	},
	{ SGI_SPROFIL,		"SGI_SPROFIL"		},
	{ SGI_RUSAGE,		"SGI_RUSAGE"		},
	{ SGI_SIGSTACK,		"SGI_SIGSTACK"		},
	{ SGI_SIGSTATUS,	"SGI_SIGSTATUS"		},
	{ SGI_NETPROC,		"SGI_NETPROC"		},
	{ SGI_SIGALTSTACK,	"SGI_SIGALTSTACK"	},
	{ SGI_BDFLUSHCNT,	"SGI_BDFLUSHCNT"	},
	{ SGI_SSYNC,		"SGI_SSYNC"		},
	{ SGI_NFSCNVT,		"SGI_NFSCNVT"		},
	{ SGI_GETPGID,		"SGI_GETPGID"		},
	{ SGI_GETSID,		"SGI_GETSID"		},
	{ SGI_IOPROBE,		"SGI_IOPROBE"		},
	{ SGI_CONFIG,		"SGI_CONFIG"		},
	{ SGI_ELFMAP,		"SGI_ELFMAP"		},
	{ SGI_MCONFIG,		"SGI_MCONFIG"		},
	{ SGI_GETPLABEL,	"SGI_GETPLABEL"		},
	{ SGI_SETPLABEL,	"SGI_SETPLABEL"		},
	{ SGI_GETLABEL,		"SGI_GETLABEL"		},
	{ SGI_SETLABEL,		"SGI_SETLABEL"		},
	{ SGI_SATREAD,		"SGI_SATREAD"		},
	{ SGI_SATWRITE,		"SGI_SATWRITE"		},
	{ SGI_SATCTL,		"SGI_SATCTL"		},
	{ SGI_LOADATTR,		"SGI_LOADATTR"		},
	{ SGI_UNLOADATTR,	"SGI_UNLOADATTR"	},
#ifdef SGI_RECVLMSG
	{ SGI_RECVLMSG,		"SGI_RECVLMSG"		},
#endif
	{ SGI_PLANGMOUNT,	"SGI_PLANGMOUNT"	},
	{ SGI_GETPSOACL,	"SGI_GETPSOACL"		},
	{ SGI_SETPSOACL,	"SGI_SETPSOACL"		},
#ifdef SGI_EAG_GET_ATTR
	{ SGI_EAG_GET_ATTR,	"SGI_EAG_GET_ATTR"	},
#endif
#ifdef SGI_EAG_SET_ATTR
	{ SGI_EAG_SET_ATTR,	"SGI_EAG_SET_ATTR"	},
#endif
#ifdef SGI_EAG_GET_PROCATTR
	{ SGI_EAG_GET_PROCATTR,	"SGI_EAG_GET_PROCATTR"	},
#endif
#ifdef SGI_EAG_SET_PROCATTR
	{ SGI_EAG_SET_PROCATTR,	"SGI_EAG_SET_PROCATTR"	},
#endif
#ifdef SGI_FREVOKE
	{ SGI_FREVOKE,		"SGI_FREVOKE"		},
#endif
#ifdef SGI_SBE_GET_INFO
	{ SGI_SBE_GET_INFO,	"SGI_SBE_GET_INFO"	},
#endif
#ifdef SGI_SBE_CLR_INFO
	{ SGI_SBE_CLR_INFO,	"SGI_SBE_CLR_INFO"	},
#endif
	{ SGI_RMI_FIXECC,	"SGI_RMI_FIXECC"	},
	{ SGI_R4K_CERRS,	"SGI_R4K_CERRS"		},
	{ SGI_GET_EVCONF,	"SGI_GET_EVCONF"	},
	{ SGI_MPCWAROFF,	"SGI_MPCWAROFF"		},
	{ SGI_SET_AUTOPWRON,	"SGI_SET_AUTOPWRON"	},
	{ SGI_SPIPE,		"SGI_SPIPE"		},
	{ SGI_SYMTAB,		"SGI_SYMTAB"		},
#ifdef SGI_SET_FPDEBUG
	{ SGI_SET_FPDEBUG,	"SGI_SET_FPDEBUG"	},
#endif
#ifdef SGI_SET_FP_PRECISE
	{ SGI_SET_FP_PRECISE,	"SGI_SET_FP_PRECISE"	},
#endif
	{ SGI_TOSSTSAVE,	"SGI_TOSSTSAVE"		},
	{ SGI_FDHI,		"SGI_FDHI"		},
#ifdef SGI_SET_CONFIG_SMM
	{ SGI_SET_CONFIG_SMM,	"SGI_SET_CONFIG_SMM"	},
#endif
#ifdef SGI_SET_FP_PRESERVE
	{ SGI_SET_FP_PRESERVE,	"SGI_SET_FP_PRESERVE"	},
#endif
	{ SGI_MINRSS,		"SGI_MINRSS"		},
#ifdef SGI_GRIO
	{ SGI_GRIO,		"SGI_GRIO"		},
#endif
#ifdef SGI_XLV_SET_TAB
	{ SGI_XLV_SET_TAB,	"SGI_XLV_SET_TAB"	},
#endif
#ifdef SGI_XLV_GET_TAB
	{ SGI_XLV_GET_TAB,	"SGI_XLV_GET_TAB"	},
#endif
#ifdef SGI_GET_FP_PRECISE
	{ SGI_GET_FP_PRECISE,	"SGI_GET_FP_PRECISE"	},
#endif
#ifdef SGI_GET_CONFIG_SMM	
	{ SGI_GET_CONFIG_SMM,	"SGI_GET_CONFIG_SMM"	},
#endif
#ifdef SGI_FP_IMPRECISE_SUPP	
	{ SGI_FP_IMPRECISE_SUPP,"SGI_FP_IMPRECISE_SUPP"	},
#endif
#ifdef SGI_CONFIG_NSMM_SUPP	
	{ SGI_CONFIG_NSMM_SUPP,	"SGI_CONFIG_NSMM_SUPP"	},
#endif
#ifdef SGI_RT_TSTAMP_CREATE    
	{ SGI_RT_TSTAMP_CREATE,	"SGI_RT_TSTAMP_CREATE"	},
#endif
#ifdef SGI_RT_TSTAMP_DELETE    
	{ SGI_RT_TSTAMP_DELETE,	"SGI_RT_TSTAMP_DELETE"	},
#endif
#ifdef SGI_RT_TSTAMP_START     
	{ SGI_RT_TSTAMP_START,	"SGI_RT_TSTAMP_START"	},
#endif
#ifdef SGI_RT_TSTAMP_STOP      
	{ SGI_RT_TSTAMP_STOP,	"SGI_RT_TSTAMP_STOP"	},
#endif
#ifdef SGI_RT_TSTAMP_ADDR      
	{ SGI_RT_TSTAMP_ADDR,	"SGI_RT_TSTAMP_ADDR"	},
#endif
#ifdef SGI_RT_TSTAMP_MASK      
	{ SGI_RT_TSTAMP_MASK,	"SGI_RT_TSTAMP_MASK"	},
#endif
#ifdef SGI_RT_TSTAMP_EOB_MODE  
	{ SGI_RT_TSTAMP_EOB_MODE,"SGI_RT_TSTAMP_EOB_MODE"},
#endif
#ifdef SGI_USE_FP_BCOPY	
	{ SGI_USE_FP_BCOPY,	"SGI_USE_FP_BCOPY"	},
#endif
#ifdef SGI_GET_UST		
	{ SGI_GET_UST,		"SGI_GET_UST"		},
#endif
#ifdef SGI_SPECULATIVE_EXEC	
	{ SGI_SPECULATIVE_EXEC,	"SGI_SPECULATIVE_EXEC"	},
#endif
#ifdef SGI_XLV_NEXT_RQST	
	{ SGI_XLV_NEXT_RQST,	"SGI_XLV_NEXT_RQST"	},
#endif
#ifdef SGI_XLV_ATTR_CURSOR	
	{ SGI_XLV_ATTR_CURSOR,	"SGI_XLV_ATTR_CURSOR"	},
#endif
#ifdef SGI_XLV_ATTR_GET	
	{ SGI_XLV_ATTR_GET,	"SGI_XLV_ATTR_GET"	},
#endif
#ifdef SGI_XLV_ATTR_SET	
	{ SGI_XLV_ATTR_SET,	"SGI_XLV_ATTR_SET"	},
#endif
#ifdef SGI_BTOOLSIZE
	{ SGI_BTOOLSIZE,	"SGI_BTOOLSIZE"		},
#endif
#ifdef SGI_BTOOLGET		
	{ SGI_BTOOLGET,		"SGI_BTOOLGET"		},
#endif
#ifdef SGI_BTOOLREINIT		
	{ SGI_BTOOLREINIT,	"SGI_BTOOLREINIT"	},
#endif
#ifdef SGI_CREATE_UUID		
	{ SGI_CREATE_UUID,	"SGI_CREATE_UUID"	},
#endif
#ifdef SGI_NOFPE		
	{ SGI_NOFPE,		"SGI_NOFPE"		},
#endif
#ifdef SGI_OLD_SOFTFP		
	{ SGI_OLD_SOFTFP,	"SGI_OLD_SOFTFP"	},
#endif
#ifdef SGI_FS_INUMBERS		
	{ SGI_FS_INUMBERS,	"SGI_FS_INUMBERS"	},
#endif
#ifdef SGI_FS_BULKSTAT		
	{ SGI_FS_BULKSTAT,	"SGI_FS_BULKSTAT"	},
#endif
#ifdef SGI_RT_TSTAMP_WAIT	
	{ SGI_RT_TSTAMP_WAIT,	"SGI_RT_TSTAMP_WAIT"	},
#endif
#ifdef SGI_RT_TSTAMP_UPDATE    
	{ SGI_RT_TSTAMP_UPDATE,	"SGI_RT_TSTAMP_UPDATE"	},
#endif
#ifdef SGI_PATH_TO_HANDLE	
	{ SGI_PATH_TO_HANDLE,	"SGI_PATH_TO_HANDLE"	},
#endif
#ifdef SGI_PATH_TO_FSHANDLE	
	{ SGI_PATH_TO_FSHANDLE,	"SGI_PATH_TO_FSHANDLE"	},
#endif
#ifdef SGI_FD_TO_HANDLE	
	{ SGI_FD_TO_HANDLE,	"SGI_FD_TO_HANDLE"	},
#endif
#ifdef SGI_OPEN_BY_HANDLE	
	{ SGI_OPEN_BY_HANDLE,	"SGI_OPEN_BY_HANDLE"	},
#endif
#ifdef SGI_READLINK_BY_HANDLE	
	{ SGI_READLINK_BY_HANDLE,"SGI_READLINK_BY_HANDLE"},
#endif
#ifdef SGI_READ_DANGID		
	{ SGI_READ_DANGID,	"SGI_READ_DANGID"	},
#endif
#ifdef SGI_CONST		
	{ SGI_CONST,		"SGI_CONST"		},
#endif
#ifdef SGI_XFS_FSOPERATIONS	
	{ SGI_XFS_FSOPERATIONS,	"SGI_XFS_FSOPERATIONS"	},
#endif
#ifdef SGI_SETASH		
	{ SGI_SETASH,		"SGI_SETASH"		},
#endif
#ifdef SGI_GETASH		
	{ SGI_GETASH,		"SGI_GETASH"		},
#endif
#ifdef SGI_SETPRID		
	{ SGI_SETPRID,		"SGI_SETPRID"		},
#endif
#ifdef SGI_GETPRID		
	{ SGI_GETPRID,		"SGI_GETPRID"		},
#endif
#ifdef SGI_SETSPINFO		
	{ SGI_SETSPINFO,	"SGI_SETSPINFO"		},
#endif
#ifdef SGI_GETSPINFO		
	{ SGI_GETSPINFO,	"SGI_GETSPINFO"		},
#endif
#ifdef SGI_SHAREII		
	{ SGI_SHAREII,		"SGI_SHAREII"		},
#endif
#ifdef SGI_NEWARRAYSESS	
	{ SGI_NEWARRAYSESS,	"SGI_NEWARRAYSESS"	},
#endif
#ifdef SGI_GETDFLTPRID		
	{ SGI_GETDFLTPRID,	"SGI_GETDFLTPRID"	},
#endif
#ifdef SGI_SET_DISMISSED_EXC_CNT 
	{ SGI_SET_DISMISSED_EXC_CNT,"SGI_SET_DISMISSED_EXC_CNT"	},
#endif
#ifdef SGI_GET_DISMISSED_EXC_CNT 
	{ SGI_GET_DISMISSED_EXC_CNT,"SGI_GET_DISMISSED_EXC_CNT"	},
#endif
#ifdef SGI_CYCLECNTR_SIZE	
	{ SGI_CYCLECNTR_SIZE,	"SGI_CYCLECNTR_SIZE"	},
#endif
#ifdef SGI_QUERY_FASTTIMER	
	{ SGI_QUERY_FASTTIMER,	"SGI_QUERY_FASTTIMER"	},
#endif
#ifdef SGI_PIDSINASH		
	{ SGI_PIDSINASH,	"SGI_PIDSINASH"		},
#endif
#ifdef SGI_ULI			
	{ SGI_ULI,		"SGI_ULI"		},
#endif
#ifdef SGI_LPG_SHMGET          
	{ SGI_LPG_SHMGET,	"SGI_LPG_SHMGET"	},
#endif
#ifdef SGI_LPG_MAP             
	{ SGI_LPG_MAP,		"SGI_LPG_MAP"		},
#endif
#ifdef SGI_CACHEFS_SYS		
	{ SGI_CACHEFS_SYS,	"SGI_CACHEFS_SYS"	},
#endif
#ifdef SGI_NFSNOTIFY		
	{ SGI_NFSNOTIFY,	"SGI_NFSNOTIFY"		},
#endif
#ifdef SGI_LOCKDSYS		
	{ SGI_LOCKDSYS,		"SGI_LOCKDSYS"		},
#endif
#ifdef SGI_EVENTCTR            
	{ SGI_EVENTCTR,		"SGI_EVENTCTR"		},
#endif
#ifdef SGI_GETPRUSAGE          
	{ SGI_GETPRUSAGE,	"SGI_GETPRUSAGE"	},
#endif
#ifdef SGI_PROCMASK_LOCATION	
	{ SGI_PROCMASK_LOCATION,"SGI_PROCMASK_LOCATION"	},
#endif
#ifdef SGI_UNUSED		
	{ SGI_UNUSED,		"SGI_UNUSED"		},
#endif
#ifdef SGI_CKPT_SYS		
	{ SGI_CKPT_SYS,		"SGI_CKPT_SYS"		},
#endif
#ifdef SGI_CKPT_SYS		
	{ SGI_CKPT_SYS,		"SGI_CKPT_SYS"		},
#endif
#ifdef SGI_GETGRPPID		
	{ SGI_GETGRPPID,	"SGI_GETGRPPID"		},
#endif
#ifdef SGI_GETSESPID		
	{ SGI_GETSESPID,	"SGI_GETSESPID"		},
#endif
#ifdef SGI_ENUMASHS		
	{ SGI_ENUMASHS,		"SGI_ENUMASHS"		},
#endif
#ifdef SGI_SETASMACHID		
	{ SGI_SETASMACHID,	"SGI_SETASMACHID"	},
#endif
#ifdef SGI_GETASMACHID		
	{ SGI_GETASMACHID,	"SGI_GETASMACHID"	},
#endif
#ifdef SGI_GETARSESS		
	{ SGI_GETARSESS,	"SGI_GETARSESS"		},
#endif
#ifdef SGI_JOINARRAYSESS	
	{ SGI_JOINARRAYSESS,	"SGI_JOINARRAYSESS"	},
#endif
#ifdef SGI_SPROC_KILL		
	{ SGI_SPROC_KILL,	"SGI_SPROC_KILL"	},
#endif
#ifdef SGI_DBA_CONFIG		
	{ SGI_DBA_CONFIG,	"SGI_DBA_CONFIG"	},
#endif
#ifdef SGI_RELEASE_NAME	
	{ SGI_RELEASE_NAME,	"SGI_RELEASE_NAME"	},
#endif
#ifdef SGI_SYNCH_CACHE_HANDLER 
	{ SGI_SYNCH_CACHE_HANDLER,"SGI_SYNCH_CACHE_HANDLER"},
#endif
#ifdef SGI_SWASH_INIT		
	{ SGI_SWASH_INIT,	"SGI_SWASH_INIT"	},
#endif
#ifdef SGI_NUMA_MIGR_PAGE	
	{ SGI_NUMA_MIGR_PAGE,	"SGI_NUMA_MIGR_PAGE"	},
#endif
#ifdef SGI_NUMA_MIGR_PAGE_ALT	
	{ SGI_NUMA_MIGR_PAGE_ALT,"SGI_NUMA_MIGR_PAGE_ALT"},
#endif
#ifdef SGI_KAIO_USERINIT	
	{ SGI_KAIO_USERINIT,	"SGI_KAIO_USERINIT"	},
#endif
#ifdef SGI_KAIO_READ		
	{ SGI_KAIO_READ,	"SGI_KAIO_READ"		},
#endif
#ifdef SGI_KAIO_WRITE		
	{ SGI_KAIO_WRITE,	"SGI_KAIO_WRITE"	},
#endif
#ifdef SGI_KAIO_SUSPEND	
	{ SGI_KAIO_SUSPEND,	"SGI_KAIO_SUSPEND"	},
#endif
#ifdef SGI_KAIO_STATS		
	{ SGI_KAIO_STATS,	"SGI_KAIO_STATS"	},
#endif
#ifdef SGI_INITIAL_PT_SPROC	
	{ SGI_INITIAL_PT_SPROC,	"SGI_INITIAL_PT_SPROC"	},
#endif
	{ 0,			NULL			},
};

int
sys_syssgi(tcp)
struct tcb *tcp;
{
	int i;

	if (entering(tcp)) {
		printxval(syssgi_options, tcp->u_arg[0], "SGI_???");
		switch (tcp->u_arg[0]) {
		default:
			for (i = 1; i < tcp->u_nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
		}
	}
	return 0;
}

#include <sys/types.h>
#include <rpc/rpc.h>
struct cred;
struct uio;
#include <sys/fsid.h>
#include <sys/vnode.h>
#include <sys/fs/nfs.h>
#include <sys/fs/nfs_clnt.h>

static struct xlat mount_flags[] = {
	{ MS_RDONLY,	"MS_RDONLY"	},
	{ MS_FSS,	"MS_FSS"	},
	{ MS_DATA,	"MS_DATA"	},
	{ MS_NOSUID,	"MS_NOSUID"	},
	{ MS_REMOUNT,	"MS_REMOUNT"	},
	{ MS_NOTRUNC,	"MS_NOTRUNC"	},
	{ MS_GRPID,	"MS_GRPID"	},
	{ MS_NODEV,	"MS_NODEV"	},
	{ MS_BEFORE,	"MS_BEFORE"	},
	{ MS_AFTER,	"MS_AFTER"	},
	{ 0,		NULL		},
};

static struct xlat nfs_flags[] = {
	{ NFSMNT_SOFT,		"NFSMNT_SOFT"		},
	{ NFSMNT_WSIZE,		"NFSMNT_WSIZE"		},
	{ NFSMNT_RSIZE,		"NFSMNT_RSIZE"		},
	{ NFSMNT_TIMEO,		"NFSMNT_TIMEO"		},
	{ NFSMNT_RETRANS,	"NFSMNT_RETRANS"	},
	{ NFSMNT_HOSTNAME,	"NFSMNT_HOSTNAME"	},
#ifdef NFSMNT_NOINT	/* IRIX 6 */
	{ NFSMNT_NOINT,		"NFSMNT_NOINT"		},
#endif
#ifdef NFSMNT_INT	/* IRIX 5 */
	{ NFSMNT_INT,		"NFSMNT_INT"		},
#endif
	{ NFSMNT_NOAC,		"NFSMNT_NOAC"		},
	{ NFSMNT_ACREGMIN,	"NFSMNT_ACREGMIN"	},
	{ NFSMNT_ACREGMAX,	"NFSMNT_ACREGMAX"	},
	{ NFSMNT_ACDIRMIN,	"NFSMNT_ACDIRMIN"	},
	{ NFSMNT_ACDIRMAX,	"NFSMNT_ACDIRMAX"	},
	{ NFSMNT_PRIVATE,	"NFSMNT_PRIVATE"	},
	{ NFSMNT_SYMTTL,	"NFSMNT_SYMTTL"		},
	{ NFSMNT_LOOPBACK,	"NFSMNT_LOOPBACK"	},
	{ NFSMNT_BASETYPE,	"NFSMNT_BASETYPE"	},
	{ NFSMNT_NAMEMAX,	"NFSMNT_NAMEMAX"	},
#ifdef NFSMNT_SHORTUID	/* IRIX 6 */
	{ NFSMNT_SHORTUID,	"NFSMNT_SHORTUID"	},
#endif
#ifdef NFSMNT_ASYNCNLM	/* IRIX 6 */
	{ NFSMNT_ASYNCNLM,	"NFSMNT_ASYNCNLM"	},
#endif
	{ 0,			NULL			},
};

int
sys_mount(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ");
		printflags(mount_flags, tcp->u_arg[2]);
		if (tcp->u_arg[2] & (MS_FSS | MS_DATA)) {
			tprintf(", ");
			tprintf("%ld", tcp->u_arg[3]);
		}
		if (tcp->u_arg[2] & MS_DATA) {
			int nfs_type = sysfs(GETFSIND, FSID_NFS);

			tprintf(", ");
			if (tcp->u_arg[3] == nfs_type) {
				struct nfs_args args;
				if (umove(tcp, tcp->u_arg[4], &args) < 0)
					tprintf("%#lx", tcp->u_arg[4]);
				else {
					tprintf("addr=");
					printsock(tcp, (int) args.addr);
					tprintf(", flags=");
					if (!printflags(nfs_flags, args.flags))
						tprintf("NFSMNT_???");
					tprintf(", hostname=");
					printstr(tcp, (int) args.hostname, -1);
					tprintf(", ...}");
				}
			}
			else
				tprintf("%#lx", tcp->u_arg[4]);
			tprintf(", %ld", tcp->u_arg[5]);
		}
	}
	return 0;
}

#else /* !MIPS */

#if UNIXWARE

#include <sys/types.h>
#include <sys/fstyp.h>
#include <sys/mount.h>
#include <sys/xti.h>

#define NFSCLIENT	1
#include <nfs/mount.h>

#include <sys/fs/vx_ioctl.h>

static struct xlat mount_flags[] = {
	{ MS_RDONLY,	"MS_RDONLY"	},
	{ MS_FSS,	"MS_FSS"	},
	{ MS_DATA,	"MS_DATA"	},
	{ MS_HADBAD,	"MS_HADBAD"	},
	{ MS_NOSUID,	"MS_NOSUID"	},
	{ MS_REMOUNT,	"MS_REMOUNT"	},
	{ MS_NOTRUNC,	"MS_NOTRUNC"	},
	{ MS_SOFTMNT,	"MS_SOFTMNT"	},
	{ MS_SYSSPACE,	"MS_SYSSPACE"	},
	{ 0,		NULL		},
};

#ifdef VX_MS_MASK
static struct xlat vxfs_flags[] = {
	{ VX_MS_NOLOG,		"VX_MS_NOLOG"		},
	{ VX_MS_BLKCLEAR,	"VX_MS_BLKCLEAR"	},
	{ VX_MS_SNAPSHOT,	"VX_MS_SNAPSHOT"	},
	{ VX_MS_NODATAINLOG,	"VX_MS_NODATAINLOG"	},
	{ VX_MS_DELAYLOG,	"VX_MS_DELAYLOG"	},
	{ VX_MS_TMPLOG,		"VX_MS_TMPLOG"		},
	{ VX_MS_FILESET,	"VX_MS_FILESET"		},

	{ VX_MS_CACHE_DIRECT,	"VX_MS_CACHE_DIRECT"	},
	{ VX_MS_CACHE_DSYNC,	"VX_MS_CACHE_DSYNC"	},
	{ VX_MS_CACHE_CLOSESYNC,"VX_MS_CACHE_CLOSESYNC"	},
	{ VX_MS_CACHE_TMPCACHE,	"VX_MS_CACHE_TMPCACHE"	},

	{ VX_MS_OSYNC_DIRECT,	"VX_MS_OSYNC_DIRECT"	},
	{ VX_MS_OSYNC_DSYNC,	"VX_MS_OSYNC_DSYNC"	},
	{ VX_MS_OSYNC_CLOSESYNC,"VX_MS_OSYNC_CLOSESYNC"	},
	{ VX_MS_OSYNC_DELAY,	"VX_MS_OSYNC_DELAY"	},
	{ 0,			NULL,			},
};
#endif

static struct xlat nfs_flags[] = {
	{ NFSMNT_SOFT,		"NFSMNT_SOFT"		},
	{ NFSMNT_WSIZE,		"NFSMNT_WSIZE"		},
	{ NFSMNT_RSIZE,		"NFSMNT_RSIZE"		},
	{ NFSMNT_TIMEO,		"NFSMNT_TIMEO"		},
	{ NFSMNT_RETRANS,	"NFSMNT_RETRANS"	},
	{ NFSMNT_HOSTNAME,	"NFSMNT_HOSTNAME"	},
	{ NFSMNT_INT,		"NFSMNT_INT"		},
	{ NFSMNT_NOAC,		"NFSMNT_NOAC"		},
	{ NFSMNT_ACREGMIN,	"NFSMNT_ACREGMIN"	},
	{ NFSMNT_ACREGMAX,	"NFSMNT_ACREGMAX"	},
	{ NFSMNT_ACDIRMIN,	"NFSMNT_ACDIRMIN"	},
	{ NFSMNT_ACDIRMAX,	"NFSMNT_ACDIRMAX"	},
	{ NFSMNT_SECURE,	"NFSMNT_SECURE"		},
	{ NFSMNT_NOCTO,		"NFSMNT_NOCTO"		},
	{ NFSMNT_GRPID,		"NFSMNT_GRPID"		},
	{ NFSMNT_RPCTIMESYNC,	"NFSMNT_RPCTIMESYNC"	},
	{ NFSMNT_LWPSMAX,	"NFSMNT_LWPSMAX"	},
	{ 0,			NULL			},
};

int
sys_mount(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		char fstyp [FSTYPSZ];
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ");
		printflags(mount_flags, tcp->u_arg[2]);
		/* The doc sez that the file system type is given as a
		   fsindex, and we should use sysfs to work out the name.
		   This appears to be untrue for UW.  Maybe it's untrue
		   for all SVR4's? */
		if (tcp->u_arg[2] & (MS_FSS | MS_DATA)) {
			if (umovestr(tcp, tcp->u_arg[3], FSTYPSZ, fstyp) < 0) {
				*fstyp = 0;
				tprintf(", %ld", tcp->u_arg[3]);
			}
			else
				tprintf(", \"%s\"", fstyp);
		}
		if (tcp->u_arg[2] & MS_DATA) {
			tprintf(", ");
#ifdef VX_MS_MASK
			/* On UW7 they don't give us the defines and structs
			   we need to see what is going on.  Bummer. */
			if (strcmp (fstyp, "vxfs") == 0) {
				struct vx_mountargs5 args;
				if (umove(tcp, tcp->u_arg[4], &args) < 0)
					tprintf("%#lx", tcp->u_arg[4]);
				else {
					tprintf("{ flags=");
					if (!printflags(vxfs_flags, args.mflags))
						tprintf("0x%08x", args.mflags);
					if (args.mflags & VX_MS_SNAPSHOT) {
						tprintf (", snapof=");
						printstr (tcp,
							  (long) args.primaryspec, 
							  -1);
						if (args.snapsize > 0)
							tprintf (", snapsize=%ld", args.snapsize);
					}
					tprintf(" }");
				}
			}
			else
#endif
			if (strcmp (fstyp, "specfs") == 0) {
				tprintf ("dev=");
				printstr (tcp, tcp->u_arg[4], -1);
			}
			else
			if (strcmp (fstyp, "nfs") == 0) {
				struct nfs_args args;
				if (umove(tcp, tcp->u_arg[4], &args) < 0)
					tprintf("%#lx", tcp->u_arg[4]);
				else {
					struct netbuf addr;
					tprintf("{ addr=");
					if (umove (tcp, (int) args.addr, &addr) < 0) {
						tprintf ("%#lx", (long) args.addr);
					}
					else {
						printsock(tcp, (int) addr.buf, addr.len);
					}
					tprintf(", flags=");
					if (!printflags(nfs_flags, args.flags))
						tprintf("NFSMNT_???");
					tprintf(", hostname=");
					printstr(tcp, (int) args.hostname, -1);
					tprintf(", ...}");
				}
			}
			else
				tprintf("%#lx", tcp->u_arg[4]);
			tprintf(", %ld", tcp->u_arg[5]);
		}
	}
	return 0;
}

#else /* !UNIXWARE */

int
sys_mount(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ...");
	}
	return 0;
}
#endif /* !UNIXWARE */

#endif /* !MIPS */

#endif /* SVR4 */

#ifdef SYS_capget
int
sys_capget(tcp)
struct tcb *tcp;
{
	cap_user_header_t       arg0;
	cap_user_data_t         arg1;

	if(!entering(tcp)) {
		arg0 = (cap_user_header_t)tcp->u_arg[0];
		arg1 = (cap_user_data_t)tcp->u_arg[1];
		tprintf("{%lx, %d}, ", (unsigned long)arg0->version, arg0->pid);
		tprintf("{%lx, %lx, %lx}", (unsigned long)arg1->effective,
			(unsigned long)arg1->permitted, (unsigned long)arg1->inheritable);
	}
	return 0;
}

int
sys_capset(tcp)
struct tcb *tcp;
{
	cap_user_header_t       arg0;
	cap_user_data_t         arg1;

   	if(entering(tcp)) {
		arg0 = (cap_user_header_t)tcp->u_arg[0];
		arg1 = (cap_user_data_t)tcp->u_arg[1];
		tprintf("{%lx, %d}, ", (unsigned long)arg0->version, arg0->pid);
		tprintf("{%lx, %lx, %lx}", (unsigned long)arg1->effective,
			(unsigned long)arg1->permitted, (unsigned long)arg1->inheritable);
	}
	return 0;
}

#else

int sys_capget(tcp)
struct tcb *tcp;
{
	return printargs(tcp);
}

int sys_capset(tcp)
struct tcb *tcp;
{
	return printargs(tcp);
}

#endif

#ifdef LINUX
static struct xlat sysctl_root[] = {
	{ CTL_KERN, "CTL_KERN" },
	{ CTL_VM, "CTL_VM" },
	{ CTL_NET, "CTL_NET" },
	{ CTL_PROC, "CTL_PROC" },
	{ CTL_FS, "CTL_FS" },
	{ CTL_DEBUG, "CTL_DEBUG" },
	{ CTL_DEV, "CTL_DEV" },
	{ 0, NULL }
};

static struct xlat sysctl_kern[] = {
	{ KERN_OSTYPE, "KERN_OSTYPE" },
	{ KERN_OSRELEASE, "KERN_OSRELEASE" },
	{ KERN_OSREV, "KERN_OSREV" },
	{ KERN_VERSION, "KERN_VERSION" },
	{ KERN_SECUREMASK, "KERN_SECUREMASK" },
	{ KERN_PROF, "KERN_PROF" },
	{ KERN_NODENAME, "KERN_NODENAME" },
	{ KERN_DOMAINNAME, "KERN_DOMAINNAME" },
#ifdef KERN_SECURELVL
	{ KERN_SECURELVL, "KERN_SECURELVL" },
#endif
	{ KERN_PANIC, "KERN_PANIC" },
#ifdef KERN_REALROOTDEV
	{ KERN_REALROOTDEV, "KERN_REALROOTDEV" },
#endif
#ifdef KERN_JAVA_INTERPRETER
	{ KERN_JAVA_INTERPRETER, "KERN_JAVA_INTERPRETER" },
#endif
#ifdef KERN_JAVA_APPLETVIEWER
	{ KERN_JAVA_APPLETVIEWER, "KERN_JAVA_APPLETVIEWER" },
#endif
	{ KERN_SPARC_REBOOT, "KERN_SPARC_REBOOT" },
	{ KERN_CTLALTDEL, "KERN_CTLALTDEL" },
	{ KERN_PRINTK, "KERN_PRINTK" },
	{ KERN_NAMETRANS, "KERN_NAMETRANS" },
	{ KERN_PPC_HTABRECLAIM, "KERN_PPC_HTABRECLAIM" },
	{ KERN_PPC_ZEROPAGED, "KERN_PPC_ZEROPAGED" },
	{ KERN_PPC_POWERSAVE_NAP, "KERN_PPC_POWERSAVE_NAP" },
	{ KERN_MODPROBE, "KERN_MODPROBE" },
	{ KERN_SG_BIG_BUFF, "KERN_SG_BIG_BUFF" },
	{ KERN_ACCT, "KERN_ACCT" },
	{ KERN_PPC_L2CR, "KERN_PPC_L2CR" },
	{ KERN_RTSIGNR, "KERN_RTSIGNR" },
	{ KERN_RTSIGMAX, "KERN_RTSIGMAX" },
	{ KERN_SHMMAX, "KERN_SHMMAX" },
	{ KERN_MSGMAX, "KERN_MSGMAX" },
	{ KERN_MSGMNB, "KERN_MSGMNB" },
	{ KERN_MSGPOOL, "KERN_MSGPOOL" },
	{ 0, NULL }
};

static struct xlat sysctl_vm[] = {
	{ VM_SWAPCTL, "VM_SWAPCTL" },
	{ VM_SWAPOUT, "VM_SWAPOUT" },
	{ VM_FREEPG, "VM_FREEPG" },
	{ VM_BDFLUSH, "VM_BDFLUSH" },
	{ VM_OVERCOMMIT_MEMORY, "VM_OVERCOMMIT_MEMORY" },
	{ VM_BUFFERMEM, "VM_BUFFERMEM" },
	{ VM_PAGECACHE, "VM_PAGECACHE" },
	{ VM_PAGERDAEMON, "VM_PAGERDAEMON" },
	{ VM_PGT_CACHE, "VM_PGT_CACHE" },
	{ VM_PAGE_CLUSTER, "VM_PAGE_CLUSTER" },
	{ 0, NULL },
};

static struct xlat sysctl_net[] = {
	{ NET_CORE, "NET_CORE" },
	{ NET_ETHER, "NET_ETHER" },
	{ NET_802, "NET_802" },
	{ NET_UNIX, "NET_UNIX" },
	{ NET_IPV4, "NET_IPV4" },
	{ NET_IPX, "NET_IPX" },
	{ NET_ATALK, "NET_ATALK" },
	{ NET_NETROM, "NET_NETROM" },
	{ NET_AX25, "NET_AX25" },
	{ NET_BRIDGE, "NET_BRIDGE" },
	{ NET_ROSE, "NET_ROSE" },
	{ NET_IPV6, "NET_IPV6" },
	{ NET_X25, "NET_X25" },
	{ NET_TR, "NET_TR" },
	{ NET_DECNET, "NET_DECNET" },
	{ 0, NULL }
};

static struct xlat sysctl_net_core[] = {
	{ NET_CORE_WMEM_MAX, "NET_CORE_WMEM_MAX" },
	{ NET_CORE_RMEM_MAX, "NET_CORE_RMEM_MAX" },
	{ NET_CORE_WMEM_DEFAULT, "NET_CORE_WMEM_DEFAULT" },
	{ NET_CORE_RMEM_DEFAULT, "NET_CORE_RMEM_DEFAULT" },
	{ NET_CORE_MAX_BACKLOG, "NET_CORE_MAX_BACKLOG" },
	{ NET_CORE_FASTROUTE, "NET_CORE_FASTROUTE" },
	{ NET_CORE_MSG_COST, "NET_CORE_MSG_COST" },
	{ NET_CORE_MSG_BURST, "NET_CORE_MSG_BURST" },
	{ NET_CORE_OPTMEM_MAX, "NET_CORE_OPTMEM_MAX" },
	{ 0, NULL }
};

static struct xlat sysctl_net_unix[] = {
	{ NET_UNIX_DESTROY_DELAY, "NET_UNIX_DESTROY_DELAY" },
	{ NET_UNIX_DELETE_DELAY, "NET_UNIX_DELETE_DELAY" },
	{ 0, NULL }
};

static struct xlat sysctl_net_ipv4[] = {
	{ NET_IPV4_FORWARD, "NET_IPV4_FORWARD" },
	{ NET_IPV4_DYNADDR, "NET_IPV4_DYNADDR" },
	{ NET_IPV4_CONF, "NET_IPV4_CONF" },
	{ NET_IPV4_NEIGH, "NET_IPV4_NEIGH" },
	{ NET_IPV4_ROUTE, "NET_IPV4_ROUTE" },
	{ NET_IPV4_FIB_HASH, "NET_IPV4_FIB_HASH" },
	{ NET_IPV4_TCP_TIMESTAMPS, "NET_IPV4_TCP_TIMESTAMPS" },
	{ NET_IPV4_TCP_WINDOW_SCALING, "NET_IPV4_TCP_WINDOW_SCALING" },
	{ NET_IPV4_TCP_SACK, "NET_IPV4_TCP_SACK" },
	{ NET_IPV4_TCP_RETRANS_COLLAPSE, "NET_IPV4_TCP_RETRANS_COLLAPSE" },
	{ NET_IPV4_DEFAULT_TTL, "NET_IPV4_DEFAULT_TTL" },
	{ NET_IPV4_AUTOCONFIG, "NET_IPV4_AUTOCONFIG" },
	{ NET_IPV4_NO_PMTU_DISC, "NET_IPV4_NO_PMTU_DISC" },
	{ NET_IPV4_TCP_SYN_RETRIES, "NET_IPV4_TCP_SYN_RETRIES" },
	{ NET_IPV4_IPFRAG_HIGH_THRESH, "NET_IPV4_IPFRAG_HIGH_THRESH" },
	{ NET_IPV4_IPFRAG_LOW_THRESH, "NET_IPV4_IPFRAG_LOW_THRESH" },
	{ NET_IPV4_IPFRAG_TIME, "NET_IPV4_IPFRAG_TIME" },
	{ NET_IPV4_TCP_MAX_KA_PROBES, "NET_IPV4_TCP_MAX_KA_PROBES" },
	{ NET_IPV4_TCP_KEEPALIVE_TIME, "NET_IPV4_TCP_KEEPALIVE_TIME" },
	{ NET_IPV4_TCP_KEEPALIVE_PROBES, "NET_IPV4_TCP_KEEPALIVE_PROBES" },
	{ NET_IPV4_TCP_RETRIES1, "NET_IPV4_TCP_RETRIES1" },
	{ NET_IPV4_TCP_RETRIES2, "NET_IPV4_TCP_RETRIES2" },
	{ NET_IPV4_TCP_FIN_TIMEOUT, "NET_IPV4_TCP_FIN_TIMEOUT" },
	{ NET_IPV4_IP_MASQ_DEBUG, "NET_IPV4_IP_MASQ_DEBUG" },
	{ NET_TCP_SYNCOOKIES, "NET_TCP_SYNCOOKIES" },
	{ NET_TCP_STDURG, "NET_TCP_STDURG" },
	{ NET_TCP_RFC1337, "NET_TCP_RFC1337" },
	{ NET_TCP_SYN_TAILDROP, "NET_TCP_SYN_TAILDROP" },
	{ NET_TCP_MAX_SYN_BACKLOG, "NET_TCP_MAX_SYN_BACKLOG" },
	{ NET_IPV4_LOCAL_PORT_RANGE, "NET_IPV4_LOCAL_PORT_RANGE" },
	{ NET_IPV4_ICMP_ECHO_IGNORE_ALL, "NET_IPV4_ICMP_ECHO_IGNORE_ALL" },
	{ NET_IPV4_ICMP_ECHO_IGNORE_BROADCASTS, "NET_IPV4_ICMP_ECHO_IGNORE_BROADCASTS" },
	{ NET_IPV4_ICMP_SOURCEQUENCH_RATE, "NET_IPV4_ICMP_SOURCEQUENCH_RATE" },
	{ NET_IPV4_ICMP_DESTUNREACH_RATE, "NET_IPV4_ICMP_DESTUNREACH_RATE" },
	{ NET_IPV4_ICMP_TIMEEXCEED_RATE, "NET_IPV4_ICMP_TIMEEXCEED_RATE" },
	{ NET_IPV4_ICMP_PARAMPROB_RATE, "NET_IPV4_ICMP_PARAMPROB_RATE" },
	{ NET_IPV4_ICMP_ECHOREPLY_RATE, "NET_IPV4_ICMP_ECHOREPLY_RATE" },
	{ NET_IPV4_ICMP_IGNORE_BOGUS_ERROR_RESPONSES, "NET_IPV4_ICMP_IGNORE_BOGUS_ERROR_RESPONSES" },
	{ NET_IPV4_IGMP_MAX_MEMBERSHIPS, "NET_IPV4_IGMP_MAX_MEMBERSHIPS" },
	{  0, NULL }
};

static struct xlat sysctl_net_ipv4_route[] = {
	{ NET_IPV4_ROUTE_FLUSH, "NET_IPV4_ROUTE_FLUSH" },
	{ NET_IPV4_ROUTE_MIN_DELAY, "NET_IPV4_ROUTE_MIN_DELAY" },
	{ NET_IPV4_ROUTE_MAX_DELAY, "NET_IPV4_ROUTE_MAX_DELAY" },
	{ NET_IPV4_ROUTE_GC_THRESH, "NET_IPV4_ROUTE_GC_THRESH" },
	{ NET_IPV4_ROUTE_MAX_SIZE, "NET_IPV4_ROUTE_MAX_SIZE" },
	{ NET_IPV4_ROUTE_GC_MIN_INTERVAL, "NET_IPV4_ROUTE_GC_MIN_INTERVAL" },
	{ NET_IPV4_ROUTE_GC_TIMEOUT, "NET_IPV4_ROUTE_GC_TIMEOUT" },
	{ NET_IPV4_ROUTE_GC_INTERVAL, "NET_IPV4_ROUTE_GC_INTERVAL" },
	{ NET_IPV4_ROUTE_REDIRECT_LOAD, "NET_IPV4_ROUTE_REDIRECT_LOAD" },
	{ NET_IPV4_ROUTE_REDIRECT_NUMBER, "NET_IPV4_ROUTE_REDIRECT_NUMBER" },
	{ NET_IPV4_ROUTE_REDIRECT_SILENCE, "NET_IPV4_ROUTE_REDIRECT_SILENCE" },
	{ NET_IPV4_ROUTE_ERROR_COST, "NET_IPV4_ROUTE_ERROR_COST" },
	{ NET_IPV4_ROUTE_ERROR_BURST, "NET_IPV4_ROUTE_ERROR_BURST" },
	{ NET_IPV4_ROUTE_GC_ELASTICITY, "NET_IPV4_ROUTE_GC_ELASTICITY" },
	{ 0, NULL }
};

static struct xlat sysctl_net_ipv4_conf[] = {
	{ NET_IPV4_CONF_FORWARDING, "NET_IPV4_CONF_FORWARDING" },
	{ NET_IPV4_CONF_MC_FORWARDING, "NET_IPV4_CONF_MC_FORWARDING" },
	{ NET_IPV4_CONF_PROXY_ARP, "NET_IPV4_CONF_PROXY_ARP" },
	{ NET_IPV4_CONF_ACCEPT_REDIRECTS, "NET_IPV4_CONF_ACCEPT_REDIRECTS" },
	{ NET_IPV4_CONF_SECURE_REDIRECTS, "NET_IPV4_CONF_SECURE_REDIRECTS" },
	{ NET_IPV4_CONF_SEND_REDIRECTS, "NET_IPV4_CONF_SEND_REDIRECTS" },
	{ NET_IPV4_CONF_SHARED_MEDIA, "NET_IPV4_CONF_SHARED_MEDIA" },
	{ NET_IPV4_CONF_RP_FILTER, "NET_IPV4_CONF_RP_FILTER" },
	{ NET_IPV4_CONF_ACCEPT_SOURCE_ROUTE, "NET_IPV4_CONF_ACCEPT_SOURCE_ROUTE" },
	{ NET_IPV4_CONF_BOOTP_RELAY, "NET_IPV4_CONF_BOOTP_RELAY" },
	{ NET_IPV4_CONF_LOG_MARTIANS, "NET_IPV4_CONF_LOG_MARTIANS" },
	{ 0, NULL }
};

static struct xlat sysctl_net_ipv6[] = {
	{ NET_IPV6_CONF, "NET_IPV6_CONF" },
	{ NET_IPV6_NEIGH, "NET_IPV6_NEIGH" },
	{ NET_IPV6_ROUTE, "NET_IPV6_ROUTE" },
	{ 0, NULL }
};

static struct xlat sysctl_net_ipv6_route[] = {
	{ NET_IPV6_ROUTE_FLUSH, "NET_IPV6_ROUTE_FLUSH" },
	{ NET_IPV6_ROUTE_GC_THRESH, "NET_IPV6_ROUTE_GC_THRESH" },
	{ NET_IPV6_ROUTE_MAX_SIZE, "NET_IPV6_ROUTE_MAX_SIZE" },
	{ NET_IPV6_ROUTE_GC_MIN_INTERVAL, "NET_IPV6_ROUTE_GC_MIN_INTERVAL" },
	{ NET_IPV6_ROUTE_GC_TIMEOUT, "NET_IPV6_ROUTE_GC_TIMEOUT" },
	{ NET_IPV6_ROUTE_GC_INTERVAL, "NET_IPV6_ROUTE_GC_INTERVAL" },
	{ NET_IPV6_ROUTE_GC_ELASTICITY, "NET_IPV6_ROUTE_GC_ELASTICITY" },
	{ 0, NULL }
};

int
sys_sysctl(tcp)
struct tcb *tcp;
{
	struct __sysctl_args info;
	int *name;
	umove (tcp, tcp->u_arg[0], &info);

	name = alloca (sizeof (int) * info.nlen);
	umoven(tcp, (size_t) info.name, sizeof (int) * info.nlen, (char *) name);

	if (entering(tcp)) {
		int cnt = 0;

		tprintf("{{");

		if (info.nlen == 0)
			goto out;
		printxval(sysctl_root, name[0], "CTL_???");
		++cnt;

		if (info.nlen == 1)
			goto out;
		switch (name[0]) {
		case CTL_KERN:
			tprintf(", ");
			printxval(sysctl_kern, name[1], "KERN_???");
			++cnt;
			break;
		case CTL_VM:
			tprintf(", ");
			printxval(sysctl_vm, name[1], "VM_???");
			++cnt;
			break;
		case CTL_NET:
			tprintf(", ");
			printxval(sysctl_net, name[1], "NET_???");
			++cnt;

			if (info.nlen == 2)
				goto out;
			switch (name[1]) {
			case NET_CORE:
				tprintf(", ");
				printxval(sysctl_net_core, name[2],
					  "NET_CORE_???");
				break;
			case NET_UNIX:
				tprintf(", ");
				printxval(sysctl_net_unix, name[2],
					  "NET_UNIX_???");
				break;
			case NET_IPV4:
				tprintf(", ");
				printxval(sysctl_net_ipv4, name[2],
					  "NET_IPV4_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV4_ROUTE:
					tprintf(", ");
					printxval(sysctl_net_ipv4_route,
						  name[3],
						  "NET_IPV4_ROUTE_???");
					break;
				case NET_IPV4_CONF:
					tprintf(", ");
					printxval(sysctl_net_ipv4_conf,
						  name[3],
						  "NET_IPV4_CONF_???");
					break;
				default:
					goto out;
				}
				break;
			case NET_IPV6:
				tprintf(", ");
				printxval(sysctl_net_ipv6, name[2],
					  "NET_IPV6_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV6_ROUTE:
					tprintf(", ");
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
		while (cnt < info.nlen)
			tprintf(", %x", name[cnt++]);
		tprintf("}, %d, ", info.nlen);
	} else {
		size_t oldlen;
		umove(tcp, (size_t)info.oldlenp, &oldlen);
		if (info.nlen >= 2
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
			tprintf(", %Zu, ", oldlen);
			if (info.newval == 0)
				tprintf("NULL");
			else if (syserror(tcp))
				tprintf("%p", info.newval);
			else
				printpath(tcp, (size_t)info.newval);
			tprintf(", %Zd", info.newlen);
		} else {
			tprintf("%p, %Zd, %p, %Zd", info.oldval, oldlen,
				info.newval, info.newlen);
		}
		tprintf("}");
	}
	return 0;
}
#else
int sys_sysctl(tcp)
struct tcb *tcp;
{
	return printargs(tcp);
}
#endif

