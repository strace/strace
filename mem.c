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
#include <linux/mman.h>
#endif
#include <sys/mman.h>

#if defined(LINUX) && defined(__i386__)
#include <asm/ldt.h>
#endif

#ifdef HAVE_LONG_LONG_OFF_T
/*
 * Ugly hacks for systems that have a long long off_t
 */
#define sys_mmap64	sys_mmap
#endif

int
sys_brk(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx", tcp->u_arg[0]);
	}
#ifdef LINUX
	return RVAL_HEX;
#else
	return 0;
#endif
}

int
sys_sbrk(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
	}
	return RVAL_HEX;
}

static struct xlat mmap_prot[] = {
	{ PROT_NONE,	"PROT_NONE",	},
	{ PROT_READ,	"PROT_READ"	},
	{ PROT_WRITE,	"PROT_WRITE"	},
	{ PROT_EXEC,	"PROT_EXEC"	},
	{ 0,		NULL		},
};

static struct xlat mmap_flags[] = {
	{ MAP_SHARED,	"MAP_SHARED"	},
	{ MAP_PRIVATE,	"MAP_PRIVATE"	},
	{ MAP_FIXED,	"MAP_FIXED"	},
#ifdef MAP_ANONYMOUS
	{ MAP_ANONYMOUS,"MAP_ANONYMOUS"	},
#endif
#ifdef MAP_RENAME
	{ MAP_RENAME,	"MAP_RENAME"	},
#endif
#ifdef MAP_NORESERVE
	{ MAP_NORESERVE,"MAP_NORESERVE"	},
#endif
	/*
	 * XXX - this was introduced in SunOS 4.x to distinguish between
	 * the old pre-4.x "mmap()", which:
	 *
	 *	only let you map devices with an "mmap" routine (e.g.,
	 *	frame buffers) in;
	 *
	 *	required you to specify the mapping address;
	 *
	 *	returned 0 on success and -1 on failure;
	 *
	 * memory and which, and the 4.x "mmap()" which:
	 *
	 *	can map plain files;
	 *
	 *	can be asked to pick where to map the file;
	 *
	 *	returns the address where it mapped the file on success
	 *	and -1 on failure.
	 *
	 * It's not actually used in source code that calls "mmap()"; the
	 * "mmap()" routine adds it for you.
	 *
	 * It'd be nice to come up with some way of eliminating it from
	 * the flags, e.g. reporting calls *without* it as "old_mmap()"
	 * and calls with it as "mmap()".
	 */
#ifdef _MAP_NEW
	{ _MAP_NEW,	"_MAP_NEW"	},
#endif
#ifdef MAP_GROWSDOWN
	{ MAP_GROWSDOWN,"MAP_GROWSDOWN"	},
#endif
#ifdef MAP_DENYWRITE
	{ MAP_DENYWRITE,"MAP_DENYWRITE"	},
#endif
#ifdef MAP_EXECUTABLE
	{ MAP_EXECUTABLE,"MAP_EXECUTABLE"},
#endif
#ifdef MAP_INHERIT
	{ MAP_INHERIT,"MAP_INHERIT"	},
#endif
#ifdef MAP_FILE
	{ MAP_FILE,"MAP_FILE"},
#endif
#ifdef MAP_LOCKED
	{ MAP_LOCKED,"MAP_LOCKED"},
#endif
	/* FreeBSD ones */
#ifdef MAP_ANON
	{ MAP_ANON,		"MAP_ANON"	},
#endif
#ifdef MAP_HASSEMAPHORE
	{ MAP_HASSEMAPHORE,	"MAP_HASSEMAPHORE"	},
#endif
#ifdef MAP_STACK
	{ MAP_STACK,		"MAP_STACK"	},
#endif
#ifdef MAP_NOSYNC
	{ MAP_NOSYNC,		"MAP_NOSYNC"	},
#endif
#ifdef MAP_NOCORE
	{ MAP_NOCORE,		"MAP_NOCORE"	},
#endif
	{ 0,		NULL		},
};

#if !HAVE_LONG_LONG_OFF_T
static
int
print_mmap(tcp,u_arg)
struct tcb *tcp;
long *u_arg;
{
	if (entering(tcp)) {
		/* addr */
		if (!u_arg[0])
			tprintf("NULL, ");
		else
			tprintf("%#lx, ", u_arg[0]);
		/* len */
		tprintf("%lu, ", u_arg[1]);
		/* prot */
		printflags(mmap_prot, u_arg[2]);
		tprintf(", ");
		/* flags */
#ifdef MAP_TYPE
		printxval(mmap_flags, u_arg[3] & MAP_TYPE, "MAP_???");
		addflags(mmap_flags, u_arg[3] & ~MAP_TYPE);
#else
		printflags(mmap_flags, u_arg[3]);
#endif
		/* fd */
		tprintf(", %ld, ", u_arg[4]);
		/* offset */
		tprintf("%#lx", u_arg[5]);
	}
	return RVAL_HEX;
}

#ifdef LINUX
int sys_old_mmap(tcp)
struct tcb *tcp;
{
    long u_arg[6];

#if	defined(IA64)
    int i, v;
    /*
     *  IA64 processes never call this routine, they only use the
     *  new `sys_mmap' interface.  This code converts the integer
     *  arguments that the IA32 process pushed onto the stack into
     *  longs.
     *
     *  Note that addresses with bit 31 set will be sign extended.
     *  Fortunately, those addresses are not currently being generated
     *  for IA32 processes so it's not a problem.
     */
    for (i = 0; i < 6; i++)
	if (umove(tcp, tcp->u_arg[0] + (i * sizeof(int)), &v) == -1)
		return 0;
	else
		u_arg[i] = v;
#else	// defined(IA64)
    if (umoven(tcp, tcp->u_arg[0], sizeof u_arg, (char *) u_arg) == -1)
	    return 0;
#endif	// defined(IA64)
    return print_mmap(tcp, u_arg);
   
}
#endif

int
sys_mmap(tcp)
struct tcb *tcp;
{
    return print_mmap(tcp, tcp->u_arg);
}
#endif /* !HAVE_LONG_LONG_OFF_T */

#if _LFS64_LARGEFILE || HAVE_LONG_LONG_OFF_T
int
sys_mmap64(tcp)
struct tcb *tcp;
{
#ifdef linux
#ifdef ALPHA
	long *u_arg = tcp->u_arg;
#else /* !ALPHA */
	long u_arg[7];
#endif /* !ALPHA */
#else /* !linux */
	long *u_arg = tcp->u_arg;
#endif /* !linux */

	if (entering(tcp)) {
#ifdef linux
#ifndef ALPHA
		if (umoven(tcp, tcp->u_arg[0], sizeof u_arg,
				(char *) u_arg) == -1)
			return 0;
#endif /* ALPHA */
#endif /* linux */
		ALIGN64 (tcp, 5);	/* FreeBSD wierdies */

		/* addr */
		tprintf("%#lx, ", u_arg[0]);
		/* len */
		tprintf("%lu, ", u_arg[1]);
		/* prot */
		printflags(mmap_prot, u_arg[2]);
		tprintf(", ");
		/* flags */
#ifdef MAP_TYPE
		printxval(mmap_flags, u_arg[3] & MAP_TYPE, "MAP_???");
		addflags(mmap_flags, u_arg[3] & ~MAP_TYPE);
#else
		printflags(mmap_flags, u_arg[3]);
#endif
		/* fd */
		tprintf(", %ld, ", u_arg[4]);
		/* offset */
		tprintf("%#llx", LONG_LONG(u_arg[5], u_arg[6]));
	}
	return RVAL_HEX;
}
#endif

 
int
sys_munmap(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu",
			tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_mprotect(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ",
			tcp->u_arg[0], tcp->u_arg[1]);
		if (!printflags(mmap_prot, tcp->u_arg[2]))
			tprintf("PROT_???");
	}
	return 0;
}

#ifdef LINUX

static struct xlat mremap_flags[] = {
	{ MREMAP_MAYMOVE,	"MREMAP_MAYMOVE"	},
	{ 0,			NULL			}
};

int
sys_mremap(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, %lu, ", tcp->u_arg[0], tcp->u_arg[1],
			tcp->u_arg[2]);
		printflags(mremap_flags, tcp->u_arg[3]);
	}
	return RVAL_HEX;
}

static struct xlat madvise_flags[] = {
#ifdef MADV_NORMAL
	{ MADV_NORMAL,		"MADV_NORMAL" },
#endif
#ifdef MADZV_RANDOM
	{ MADV_RANDOM,		"MADV_RANDOM" },
#endif
#ifdef MADV_SEQUENTIAL
	{ MADV_SEQUENTIAL,	"MADV_SEQUENTIAL" },
#endif
#ifdef MADV_WILLNEED
	{ MADV_WILLNEED,	"MADV_WILLNEED" },
#endif
#ifdef MADV_DONTNED
	{ MADV_DONTNEED,	"MADV_DONTNEED" },
#endif
	{ 0,			NULL },
};


int
sys_madvise(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printflags(madvise_flags, tcp->u_arg[2]);
	}
	return 0;
}


static struct xlat mlockall_flags[] = {
#ifdef MCL_CURRENT
	{ MCL_CURRENT,	"MCL_CURRENT" },
#endif
#ifdef MCL_FUTURE
	{ MCL_FUTURE,	"MCL_FUTURE" },
#endif
	{ 0,		NULL}
};

int
sys_mlockall(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printflags(mlockall_flags, tcp->u_arg[0]);
	}
	return 0;
}


#endif /* LINUX */

#ifdef MS_ASYNC

static struct xlat mctl_sync[] = {
	{ MS_ASYNC,	"MS_ASYNC"	},
	{ MS_INVALIDATE,"MS_INVALIDATE"	},
#ifdef MS_SYNC
	{ MS_SYNC,	"MS_SYNC"	},
#endif
	{ 0,		NULL		},
};

int
sys_msync(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* flags */
		if (!printflags(mctl_sync, tcp->u_arg[2]))
			tprintf("MS_???");
	}
	return 0;
}

#endif /* MS_ASYNC */

#ifdef MC_SYNC

static struct xlat mctl_funcs[] = {
	{ MC_LOCK,	"MC_LOCK"	},
	{ MC_LOCKAS,	"MC_LOCKAS"	},
	{ MC_SYNC,	"MC_SYNC"	},
	{ MC_UNLOCK,	"MC_UNLOCK"	},
	{ MC_UNLOCKAS,	"MC_UNLOCKAS"	},
	{ 0,		NULL		},
};

static struct xlat mctl_lockas[] = {
	{ MCL_CURRENT,	"MCL_CURRENT"	},
	{ MCL_FUTURE,	"MCL_FUTURE"	},
	{ 0,		NULL		},
};

int
sys_mctl(tcp)
struct tcb *tcp;
{
	int arg, function;

	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* function */
		function = tcp->u_arg[2];
		if (!printflags(mctl_funcs, function))
			tprintf("MC_???");
		/* arg */
		arg = tcp->u_arg[3];
		tprintf(", ");
		switch (function) {
		case MC_SYNC:
			if (!printflags(mctl_sync, arg))
				tprintf("MS_???");
			break;
		case MC_LOCKAS:
			if (!printflags(mctl_lockas, arg))
				tprintf("MCL_???");
			break;
		default:
			tprintf("%#x", arg);
			break;
		}
	}
	return 0;
}

#endif /* MC_SYNC */

int
sys_mincore(tcp)
struct tcb *tcp;
{
	int i, len;
	char *vec = NULL;

	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		len = tcp->u_arg[1];
		if (syserror(tcp) || tcp->u_arg[2] == 0 ||
			(vec = malloc((u_int)len)) == NULL ||
			umoven(tcp, tcp->u_arg[2], len, vec) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else {
			tprintf("[");
			for (i = 0; i < len; i++) {
				if (abbrev(tcp) && i >= max_strlen) {
					tprintf("...");
					break;
				}
				tprintf((vec[i] & 1) ? "1" : "0");
			}
			tprintf("]");
		}
		if (vec)
			free(vec);
	}
	return 0;
}

int
sys_getpagesize(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_HEX;
	return 0;
}

#if defined(LINUX) && defined(__i386__)
int
sys_modify_ldt(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		struct modify_ldt_ldt_s copy;
		tprintf("%ld", tcp->u_arg[0]);
		if (tcp->u_arg[1] == 0
				|| tcp->u_arg[2] != sizeof (struct modify_ldt_ldt_s)
				|| umove(tcp, tcp->u_arg[1], &copy) == -1)
			tprintf(", %lx", tcp->u_arg[1]);
		else {
			tprintf(", {entry_number:%d, ", copy.entry_number);
			if (!verbose(tcp))
				tprintf("...}");
			else {
				tprintf("base_addr:%#08lx, "
						"limit:%d, "
						"seg_32bit:%d, "
						"contents:%d, "
						"read_exec_only:%d, "
						"limit_in_pages:%d, "
						"seg_not_present:%d, "
						"useable:%d}",
						copy.base_addr,
						copy.limit,
						copy.seg_32bit,
						copy.contents,
						copy.read_exec_only,
						copy.limit_in_pages,
						copy.seg_not_present,
						copy.useable);
			}
		}
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif /* LINUX && __i386__ */

