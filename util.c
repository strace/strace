/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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

#include <sys/user.h>
#include <sys/param.h>
#include <fcntl.h>
#ifdef SUNOS4
#include <machine/reg.h>
#include <a.out.h>
#include <link.h>
#endif /* SUNOS4 */

#if defined(linux)
#include <linux/ptrace.h>
#endif 

#ifdef HAVE_SYS_REG_H
#include <sys/reg.h>
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
#endif

#ifdef HAVE_SYS_PTRACE_H
#include <sys/ptrace.h>
#endif

#ifdef SUNOS4_KERNEL_ARCH_KLUDGE
#include <sys/utsname.h>
#endif /* SUNOS4_KERNEL_ARCH_KLUDGE */

#if defined(LINUX) && defined(SPARC)

#include <asm/reg.h>

#if !defined(__GLIBC__)

#include <linux/unistd.h>

#define _hack_syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,\
          type5,arg5,syscall) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
      long __res; \
\
__asm__ volatile ("or %%g0, %1, %%o0\n\t" \
                  "or %%g0, %2, %%o1\n\t" \
                  "or %%g0, %3, %%o2\n\t" \
                  "or %%g0, %4, %%o3\n\t" \
                  "or %%g0, %5, %%o4\n\t" \
                  "or %%g0, %6, %%g1\n\t" \
                  "t 0x10\n\t" \
                  "bcc 1f\n\t" \
                  "or %%g0, %%o0, %0\n\t" \
                  "sub %%g0, %%o0, %0\n\t" \
                  "1:\n\t" \
                  : "=r" (__res) \
                  : "0" ((long)(arg1)),"1" ((long)(arg2)), \
                    "2" ((long)(arg3)),"3" ((long)(arg4)),"4" ((long)(arg5)), \
                    "i" (__NR_##syscall)  \
                  : "g1", "o0", "o1", "o2", "o3", "o4"); \
if (__res>=0) \
        return (type) __res; \
errno = -__res; \
return -1; \
}

static _hack_syscall5(int,_ptrace,int,__request,int,__pid,int,__addr,int,__data,int,__addr2,ptrace)

#define _ptrace

#endif

#endif

/* macros */
#ifndef MAX
#define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

void
tv_tv(tv, a, b)
struct timeval *tv;
int a;
int b;
{
	tv->tv_sec = a;
	tv->tv_usec = b;
}

int
tv_nz(a)
struct timeval *a;
{
	return a->tv_sec || a->tv_usec;
}

int
tv_cmp(a, b)
struct timeval *a, *b;
{
	if (a->tv_sec < b->tv_sec
	    || (a->tv_sec == b->tv_sec && a->tv_usec < b->tv_usec))
		return -1;
	if (a->tv_sec > b->tv_sec
	    || (a->tv_sec == b->tv_sec && a->tv_usec > b->tv_usec))
		return 1;
	return 0;
}

double
tv_float(tv)
struct timeval *tv;
{
	return tv->tv_sec + tv->tv_usec/1000000.0;
}

void
tv_add(tv, a, b)
struct timeval *tv, *a, *b;
{
	tv->tv_sec = a->tv_sec + b->tv_sec;
	tv->tv_usec = a->tv_usec + b->tv_usec;
	if (tv->tv_usec > 1000000) {
		tv->tv_sec++;
		tv->tv_usec -= 1000000;
	}
}

void
tv_sub(tv, a, b)
struct timeval *tv, *a, *b;
{
	tv->tv_sec = a->tv_sec - b->tv_sec;
	tv->tv_usec = a->tv_usec - b->tv_usec;
	if (((long) tv->tv_usec) < 0) {
		tv->tv_sec--;
		tv->tv_usec += 1000000;
	}
}

void
tv_div(tv, a, n)
struct timeval *tv, *a;
int n;
{
	tv->tv_usec = (a->tv_sec % n * 1000000 + a->tv_usec + n / 2) / n;
	tv->tv_sec = a->tv_sec / n + tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

void
tv_mul(tv, a, n)
struct timeval *tv, *a;
int n;
{
	tv->tv_usec = a->tv_usec * n;
	tv->tv_sec = a->tv_sec * n + a->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

char *
xlookup(xlat, val)
struct xlat *xlat;
int val;
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

/*
 * Print entry in struct xlat table, if there.
 */
void
printxval(xlat, val, dflt)
struct xlat *xlat;
int val;
char *dflt;
{
	char *str = xlookup(xlat, val);

	if (str)
		tprintf("%s", str);
	else
		tprintf("%#x /* %s */", val, dflt);
}

/*
 * Interpret `xlat' as an array of flags
 * print the entries whose bits are on in `flags'
 * return # of flags printed.
 */
int
addflags(xlat, flags)
struct xlat *xlat;
int flags;
{
	int n;

	for (n = 0; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			tprintf("|%s", xlat->str);
			flags &= ~xlat->val;
			n++;
		}
	}
	if (flags) {
		tprintf("|%#x", flags);
		n++;
	}
	return n;
}

int
printflags(xlat, flags)
struct xlat *xlat;
int flags;
{
	int n;
	char *sep;

	if (flags == 0 && xlat->val == 0) {
		tprintf("%s", xlat->str);
		return 1;
	}

	sep = "";
	for (n = 0; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			tprintf("%s%s", sep, xlat->str);
			flags &= ~xlat->val;
			sep = "|";
			n++;
		}
	}
	if (flags) {
		tprintf("%s%#x", sep, flags);
		n++;
	}
	return n;
}

void
printnum(tcp, addr, fmt)
struct tcb *tcp;
long addr;
char *fmt;
{
	int num;

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (umove(tcp, addr, &num) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	tprintf("[");
	tprintf(fmt, num);
	tprintf("]");
}

static char path[MAXPATHLEN + 1];

void
string_quote(str)
char *str;
{
	char buf[2 * MAXPATHLEN + 1];
	char *s;

	if (!strpbrk(str, "\"\'\\")) {
		tprintf("\"%s\"", str);
		return;
	}
	for (s = buf; *str; str++) {
		switch (*str) {
		case '\"': case '\'': case '\\':
			*s++ = '\\'; *s++ = *str; break;
		default:
			*s++ = *str; break;
		}
	}
	*s = '\0';
	tprintf("\"%s\"", buf);
}

void
printpath(tcp, addr)
struct tcb *tcp;
long addr;
{
	if (umovestr(tcp, addr, MAXPATHLEN, path) < 0)
		tprintf("%#lx", addr);
	else
		string_quote(path);
	return;
}

void
printpathn(tcp, addr, n)
struct tcb *tcp;
long addr;
int n;
{
	if (umovestr(tcp, addr, n, path) < 0)
		tprintf("%#lx", addr);
	else {
		path[n] = '\0';
		string_quote(path);
	}
}

void
printstr(tcp, addr, len)
struct tcb *tcp;
long addr;
int len;
{
	static unsigned char *str = NULL;
	static char *outstr;
	int i, n, c, usehex;
	char *s, *outend;

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (!str) {
		if ((str = malloc(max_strlen)) == NULL
		    || (outstr = malloc(2*max_strlen)) == NULL) {
			fprintf(stderr, "printstr: no memory\n");
			tprintf("%#lx", addr);
			return;
		}
	}
	outend = outstr + max_strlen * 2 - 10;
	if (len < 0) {
		n = max_strlen;
		if (umovestr(tcp, addr, n, (char *) str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
	}
	else {
		n = MIN(len, max_strlen);
		if (umoven(tcp, addr, n, (char *) str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
	}

	usehex = 0;
	if (xflag > 1)
		usehex = 1;
	else if (xflag) {
		for (i = 0; i < n; i++) {
			c = str[i];
			if (len < 0 && c == '\0')
				break;
			if (!isprint(c) && !isspace(c)) {
				usehex = 1;
				break;
			}
		}
	}

	s = outstr;
	*s++ = '\"';

	if (usehex) {
		for (i = 0; i < n; i++) {
			c = str[i];
			if (len < 0 && c == '\0')
				break;
			sprintf(s, "\\x%02x", c);
			s += 4;
			if (s > outend)
				break;
		}
	}
	else {
		for (i = 0; i < n; i++) {
			c = str[i];
			if (len < 0 && c == '\0')
				break;
			switch (c) {
			case '\"': case '\'': case '\\':
				*s++ = '\\'; *s++ = c; break;
			case '\f':
				*s++ = '\\'; *s++ = 'f'; break;
			case '\n':
				*s++ = '\\'; *s++ = 'n'; break;
			case '\r':
				*s++ = '\\'; *s++ = 'r'; break;
			case '\t':
				*s++ = '\\'; *s++ = 't'; break;
			case '\v':
				*s++ = '\\'; *s++ = 'v'; break;
			default:
				if (isprint(c))
					*s++ = c;
				else if (i < n - 1 && isdigit(str[i + 1])) {
					sprintf(s, "\\%03o", c);
					s += 4;
				}
				else {
					sprintf(s, "\\%o", c);
					s += strlen(s);
				}
				break;
			}
			if (s > outend)
				break;
		}
	}

	*s++ = '\"';
	if (i < len || (len < 0 && (i == n || s > outend))) {
		*s++ = '.'; *s++ = '.'; *s++ = '.';
	}
	*s = '\0';
	tprintf("%s", outstr);
}

void
dumpstr(tcp, addr, len)
struct tcb *tcp;
long addr;
int len;
{
	static int strsize = -1;
	static unsigned char *str;
	static char outstr[80];
	char *s;
	int i, j;

	if (strsize < len) {
		if (str)
			free(str);
		if ((str = malloc(len)) == NULL) {
			fprintf(stderr, "dump: no memory\n");
			return;
		}
		strsize = len;
	}

	if (umoven(tcp, addr, len, (char *) str) < 0)
		return;

	for (i = 0; i < len; i += 16) {
		s = outstr;
		sprintf(s, " | %05x ", i);
		s += 9;
		for (j = 0; j < 16; j++) {
			if (j == 8)
				*s++ = ' ';
			if (i + j < len) {
				sprintf(s, " %02x", str[i + j]);
				s += 3;
			}
			else {
				*s++ = ' '; *s++ = ' '; *s++ = ' ';
			}
		}
		*s++ = ' '; *s++ = ' ';
		for (j = 0; j < 16; j++) {
			if (j == 8)
				*s++ = ' ';
			if (i + j < len) {
				if (isprint(str[i + j]))
					*s++ = str[i + j];
				else
					*s++ = '.';
			}
			else
				*s++ = ' ';
		}
		tprintf("%s |\n", outstr);
	}
}

#define PAGMASK	(~(PAGSIZ - 1))
/*
 * move `len' bytes of data from process `pid'
 * at address `addr' to our space at `laddr'
 */
int
umoven(tcp, addr, len, laddr)
struct tcb *tcp;
long addr;
int len;
char *laddr;
{

#ifdef LINUX
	int pid = tcp->pid;
	int n, m;
	int started = 0;
	union {
		long val;
		char x[sizeof(long)];
	} u;

	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		if (errno) {
			if (started && (errno==EPERM || errno==EIO)) {
				/* Ran into 'end of memory' - stupid "printpath" */
				return 0;
			}
			/* But if not started, we had a bogus address. */
			perror("ptrace: umoven");
			return -1;
		}
		started = 1;
		memcpy(laddr, &u.x[n], m = MIN(sizeof(long) - n, len));
		addr += sizeof(long), laddr += m, len -= m;
	}
	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		if (errno) {
			if (started && (errno==EPERM || errno==EIO)) {
				/* Ran into 'end of memory' - stupid "printpath" */
				return 0;
			}
			perror("ptrace: umoven");
			return -1;
		}
		started = 1;
		memcpy(laddr, u.x, m = MIN(sizeof(long), len));
		addr += sizeof(long), laddr += m, len -= m;
	}
#endif /* LINUX */

#ifdef SUNOS4
	int pid = tcp->pid;
#if 0
	int n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;

	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		if (errno) {
			perror("umoven");
			return -1;
		}
		memcpy(laddr, &u.x[n], m = MIN(sizeof(long) - n, len));
		addr += sizeof(long), laddr += m, len -= m;
	}
	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		if (errno) {
			perror("umoven");
			return -1;
		}
		memcpy(laddr, u.x, m = MIN(sizeof(long), len));
		addr += sizeof(long), laddr += m, len -= m;
	}
#else /* !oldway */
	int n;

	while (len) {
		n = MIN(len, PAGSIZ);
		n = MIN(n, ((addr + PAGSIZ) & PAGMASK) - addr);
		if (ptrace(PTRACE_READDATA, pid,
			   (char *) addr, len, laddr) < 0) {
			perror("umoven: ptrace(PTRACE_READDATA, ...)");
			abort();
			return -1;
		}
		len -= n;
		addr += n;
		laddr += n;
	}
#endif /* !oldway */
#endif /* SUNOS4 */

#ifdef SVR4
#ifdef HAVE_MP_PROCFS
	if (pread(tcp->pfd_as, laddr, len, addr) == -1)
		return -1;
#else
/*
 * We would like to use pread preferentially for speed
 * but even though SGI has it in their library, it no longer works.
 */
#ifdef MIPS
#undef HAVE_PREAD
#endif
#ifdef HAVE_PREAD
	if (pread(tcp->pfd, laddr, len, addr) == -1)
		return -1;
#else /* !HAVE_PREAD */
	lseek(tcp->pfd, addr, SEEK_SET);
	if (read(tcp->pfd, laddr, len) == -1)
		return -1;
#endif /* !HAVE_PREAD */
#endif /* HAVE_MP_PROCFS */
#endif /* SVR4 */

	return 0;
}

/*
 * like `umove' but make the additional effort of looking
 * for a terminating zero byte.
 */
int
umovestr(tcp, addr, len, laddr)
struct tcb *tcp;
long addr;
int len;
char *laddr;
{
#ifdef SVR4
	return umoven(tcp, addr, len, laddr);
#else /* !SVR4 */
	int started = 0;
	int pid = tcp->pid;
	int i, n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;

	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *)addr, 0);
		if (errno) {
			if (started && (errno==EPERM || errno==EIO)) {
				/* Ran into 'end of memory' - stupid "printpath" */
				return 0;
			}
			perror("umovestr");
			return -1;
		}
		started = 1;
		memcpy(laddr, &u.x[n], m = MIN(sizeof(long)-n,len));
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return 0;
		addr += sizeof(long), laddr += m, len -= m;
	}
	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *)addr, 0);
		if (errno) {
			if (started && (errno==EPERM || errno==EIO)) {
				/* Ran into 'end of memory' - stupid "printpath" */
				return 0;
			}
			perror("umovestr");
			return -1;
		}
		started = 1;
		memcpy(laddr, u.x, m = MIN(sizeof(long), len));
		for (i = 0; i < sizeof(long); i++)
			if (u.x[i] == '\0')
				return 0;

		addr += sizeof(long), laddr += m, len -= m;
	}
	return 0;
#endif /* !SVR4 */
}

#ifdef LINUX
#ifndef SPARC
#define PTRACE_WRITETEXT	101
#define PTRACE_WRITEDATA	102
#endif /* !SPARC */
#endif /* LINUX */

#ifdef SUNOS4

static int
uload(cmd, pid, addr, len, laddr)
int cmd;
int pid;
long addr;
int len;
char *laddr;
{
#if 0
	int n;

	while (len) {
		n = MIN(len, PAGSIZ);
		n = MIN(n, ((addr + PAGSIZ) & PAGMASK) - addr);
		if (ptrace(cmd, pid, (char *)addr, n, laddr) < 0) {
			perror("uload: ptrace(PTRACE_WRITE, ...)");
			return -1;
		}
		len -= n;
		addr += n;
		laddr += n;
	}
#else
	int peek, poke;
	int n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;

	if (cmd == PTRACE_WRITETEXT) {
		peek = PTRACE_PEEKTEXT;
		poke = PTRACE_POKETEXT;
	}
	else {
		peek = PTRACE_PEEKDATA;
		poke = PTRACE_POKEDATA;
	}
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long);
		errno = 0;
		u.val = ptrace(peek, pid, (char *) addr, 0);
		if (errno) {
			perror("uload: POKE");
			return -1;
		}
		memcpy(&u.x[n], laddr, m = MIN(sizeof(long) - n, len));
		if (ptrace(poke, pid, (char *)addr, u.val) < 0) {
			perror("uload: POKE");
			return -1;
		}
		addr += sizeof(long), laddr += m, len -= m;
	}
	while (len) {
		if (len < sizeof(long))
			u.val = ptrace(peek, pid, (char *) addr, 0);
		memcpy(u.x, laddr, m = MIN(sizeof(long), len));
		if (ptrace(poke, pid, (char *) addr, u.val) < 0) {
			perror("uload: POKE");
			return -1;
		}
		addr += sizeof(long), laddr += m, len -= m;
	}
#endif
	return 0;
}

int
tload(pid, addr, len, laddr)
int pid;
int addr, len;
char *laddr;
{
	return uload(PTRACE_WRITETEXT, pid, addr, len, laddr);
}

int
dload(pid, addr, len, laddr)
int pid;
int addr;
int len;
char *laddr;
{
	return uload(PTRACE_WRITEDATA, pid, addr, len, laddr);
}

#endif /* SUNOS4 */

#ifndef SVR4

int
upeek(pid, off, res)
int pid;
long off;
long *res;
{
	long val;

#ifdef SUNOS4_KERNEL_ARCH_KLUDGE
	{
		static int is_sun4m = -1;
		struct utsname name;

		/* Round up the usual suspects. */
		if (is_sun4m == -1) {
			if (uname(&name) < 0) {
				perror("upeek: uname?");
				exit(1);
			}
			is_sun4m = strcmp(name.machine, "sun4m") == 0;
			if (is_sun4m) {
				extern struct xlat struct_user_offsets[];
				struct xlat *x;

				for (x = struct_user_offsets; x->str; x++)
					x->val += 1024;
			}
		}
		if (is_sun4m)
			off += 1024;
	}
#endif /* SUNOS4_KERNEL_ARCH_KLUDGE */
	errno = 0;
	val = ptrace(PTRACE_PEEKUSER, pid, (char *) off, 0);
	if (val == -1 && errno) {
		perror("upeek: ptrace(PTRACE_PEEKUSER, ... )");
		return -1;
	}
	*res = val;
	return 0;
}

#endif /* !SVR4 */

long
getpc(tcp)
struct tcb *tcp;
{

#ifdef LINUX
	long pc;

#if defined(I386)
	if (upeek(tcp->pid, 4*EIP, &pc) < 0)
		return -1;
#elif defined(ARM)
	if (upeek(tcp->pid, 4*15, &pc) < 0)
		return -1;
#elif defined(POWERPC)
	if (upeek(tcp->pid, 4*PT_NIP, &pc) < 0)
		return -1;
#elif defined(M68k)
	if (upeek(tcp->pid, 4*PT_PC, &pc) < 0)
		return -1;
#elif defined(ALPHA)
	if (upeek(tcp->pid, REG_PC, &pc) < 0)
		return -1;
#elif defined(MIPS)
 	if (upeek(tcp->pid, REG_EPC, &pc) < 0)
 		return -1;
#elif defined(SPARC)
	struct regs regs;
	if (ptrace(PTRACE_GETREGS,tcp->pid,(char *)&regs,0) < 0)
		return -1;
	pc = regs.r_pc;
#elif defined(S390)
	if(upeek(tcp->pid,PT_PSWADDR,&pc) < 0)
		return -1;

#else
	return pc;
#endif /* LINUX */

#ifdef SUNOS4
	/*
	 * Return current program counter for `pid'
	 * Assumes PC is never 0xffffffff
	 */
	struct regs regs;

	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *) &regs, 0) < 0) {
		perror("getpc: ptrace(PTRACE_GETREGS, ...)");
		return -1;
	}
	return regs.r_pc;
#endif /* SUNOS4 */

#ifdef SVR4
	/* XXX */
	return 0;
#endif /* SVR4 */

}

void
printcall(tcp)
struct tcb *tcp;
{

#ifdef LINUX
#ifdef I386
	long eip;

	if (upeek(tcp->pid, 4*EIP, &eip) < 0) {
		tprintf("[????????] ");
		return;
	}
	tprintf("[%08lx] ", eip);
#else /* !I386K */
#ifdef POWERPC
	long pc;

	if (upeek(tcp->pid, 4*PT_NIP, &pc) < 0) {
		tprintf ("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#else /* !POWERPC */
#ifdef M68K
	long pc;

	if (upeek(tcp->pid, 4*PT_PC, &pc) < 0) {
		tprintf ("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#else /* !M68K */
#ifdef ALPHA
	long pc;

	if (upeek(tcp->pid, REG_PC, &pc) < 0) {
		tprintf ("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#else /* !ALPHA */
#ifdef SPARC
	struct regs regs;
	if (ptrace(PTRACE_GETREGS,tcp->pid,(char *)&regs,0) < 0) {
		tprintf("[????????] ");
		return;
	}
	tprintf("[%08lx] ", regs.r_pc);
#endif /* SPARC */
#endif /* ALPHA */
#endif /* !M68K */
#endif /* !POWERPC */
#endif /* !I386 */
#endif /* LINUX */

#ifdef SUNOS4
	struct regs regs;

	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *) &regs, 0) < 0) {
		perror("printcall: ptrace(PTRACE_GETREGS, ...)");
		tprintf("[????????] ");
		return;
	}
	tprintf("[%08x] ", regs.r_o7);
#endif /* SUNOS4 */

#ifdef SVR4
	/* XXX */
	tprintf("[????????] ");
#endif

}

#ifndef SVR4

int
setbpt(tcp)
struct tcb *tcp;
{

#ifdef LINUX
#ifdef SPARC
	/* We simply use the SunOS breakpoint code. */

	struct regs regs;
#define LOOPA	0x30800000	/* ba,a	0 */

	if (tcp->flags & TCB_BPTSET) {
		fprintf(stderr, "PANIC: TCB already set in pid %u\n", tcp->pid);
		return -1;
	}
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
		perror("setbpt: ptrace(PTRACE_GETREGS, ...)");
		return -1;
	}
	tcp->baddr = regs.r_o7 + 8;
	errno = 0;
	tcp->inst[0] = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)tcp->baddr, 0);
	if(errno) {
		perror("setbpt: ptrace(PTRACE_PEEKTEXT, ...)");
		return -1;
	}

	/*
	 * XXX - BRUTAL MODE ON
	 * We cannot set a real BPT in the child, since it will not be
	 * traced at the moment it will reach the trap and would probably
	 * die with a core dump.
	 * Thus, we are force our way in by taking out two instructions
	 * and insert an eternal loop instead, in expectance of the SIGSTOP
	 * generated by out PTRACE_ATTACH.
	 * Of cause, if we evaporate ourselves in the middle of all this...
	 */
	errno = 0;
	ptrace(PTRACE_POKETEXT, tcp->pid, (char *) tcp->baddr, LOOPA);
	if(errno) {
		perror("setbpt: ptrace(PTRACE_POKETEXT, ...)");
		return -1;
	}
	tcp->flags |= TCB_BPTSET;

#else /* !SPARC */

#if defined (I386)
#define LOOP	0x0000feeb
#elif defined (M68K)
#define LOOP	0x60fe0000
#elif defined (ALPHA)
#define LOOP	0xc3ffffff
#elif defined (POWERPC)
#define LOOP	0x0000feeb
#elif defined(ARM)
#define LOOP	-1		/* almost certainly wrong, jws */
#elif defined(MIPS)
#define LOOP	0x1000ffff
#elif defined(S390)
#define LOOP	0xa7f40000	/* BRC 15,0 */
#else
#error unknown architecture
#endif

	if (tcp->flags & TCB_BPTSET) {
		fprintf(stderr, "PANIC: bpt already set in pid %u\n", tcp->pid);
		return -1;
	}
#if defined (I386)
	if (upeek(tcp->pid, 4*EIP, &tcp->baddr) < 0)
		return -1;
#elif defined (M68K)
	if (upeek(tcp->pid, 4*PT_PC, &tcp->baddr) < 0)
	  return -1;
#elif defined (ALPHA)
	return -1;
#elif defined (ARM)
	return -1;
#elif defined (MIPS)
	return -1;		/* FIXME: I do not know what i do - Flo */
#elif defined (POWERPC)
	if (upeek(tcp->pid, 4*PT_NIP, &tcp->baddr) < 0)
		return -1;
#elif defined(S390)
	if (upeek(tcp->pid,PT_PSWADDR, &tcp->baddr) < 0)
		return -1;
#else
#error unknown architecture
#endif
	if (debug)
		fprintf(stderr, "[%d] setting bpt at %lx\n", tcp->pid, tcp->baddr);
	tcp->inst[0] = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *) tcp->baddr, 0);
	if (errno) {
		perror("setbpt: ptrace(PTRACE_PEEKTEXT, ...)");
		return -1;
	}
	ptrace(PTRACE_POKETEXT, tcp->pid, (char *) tcp->baddr, LOOP);
	if (errno) {
		perror("setbpt: ptrace(PTRACE_POKETEXT, ...)");
		return -1;
	}
	tcp->flags |= TCB_BPTSET;

#endif /* SPARC */
#endif /* LINUX */

#ifdef SUNOS4
#ifdef SPARC	/* This code is slightly sparc specific */

	struct regs regs;
#define BPT	0x91d02001	/* ta	1 */
#define LOOP	0x10800000	/* ba	0 */
#define LOOPA	0x30800000	/* ba,a	0 */
#define NOP	0x01000000
#if LOOPA
	static int loopdeloop[1] = {LOOPA};
#else
	static int loopdeloop[2] = {LOOP, NOP};
#endif

	if (tcp->flags & TCB_BPTSET) {
		fprintf(stderr, "PANIC: TCB already set in pid %u\n", tcp->pid);
		return -1;
	}
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
		perror("setbpt: ptrace(PTRACE_GETREGS, ...)");
		return -1;
	}
	tcp->baddr = regs.r_o7 + 8;
	if (ptrace(PTRACE_READTEXT, tcp->pid, (char *)tcp->baddr,
				sizeof tcp->inst, (char *)tcp->inst) < 0) {
		perror("setbpt: ptrace(PTRACE_READTEXT, ...)");
		return -1;
	}

	/*
	 * XXX - BRUTAL MODE ON
	 * We cannot set a real BPT in the child, since it will not be
	 * traced at the moment it will reach the trap and would probably
	 * die with a core dump.
	 * Thus, we are force our way in by taking out two instructions
	 * and insert an eternal loop in stead, in expectance of the SIGSTOP
	 * generated by out PTRACE_ATTACH.
	 * Of cause, if we evaporate ourselves in the middle of all this...
	 */
	if (ptrace(PTRACE_WRITETEXT, tcp->pid, (char *) tcp->baddr,
			sizeof loopdeloop, (char *) loopdeloop) < 0) {
		perror("setbpt: ptrace(PTRACE_WRITETEXT, ...)");
		return -1;
	}
	tcp->flags |= TCB_BPTSET;

#endif /* SPARC */
#endif /* SUNOS4 */

	return 0;
}

int
clearbpt(tcp)
struct tcb *tcp;
{

#ifdef LINUX
#ifdef I386
	long eip;
#else /* !I386 */
#ifdef POWERPC
	long pc;
#else /* !POWERPC */
#ifdef M68K
	long pc;
#else /* !M68K */
#ifdef ALPHA
	long pc;
#endif /* ALPHA */
#endif /* !M68K */
#endif /* !POWERPC */
#endif /* !I386 */

#ifdef SPARC
	/* Again, we borrow the SunOS breakpoint code. */
	if (!(tcp->flags & TCB_BPTSET)) {
		fprintf(stderr, "PANIC: TCB not set in pid %u\n", tcp->pid);
		return -1;
	}
	errno = 0;
	ptrace(PTRACE_POKETEXT, tcp->pid, (char *) tcp->baddr, tcp->inst[0]);
	if(errno) {
		perror("clearbtp: ptrace(PTRACE_POKETEXT, ...)");
		return -1;
	}
	tcp->flags &= ~TCB_BPTSET;
#else /* !SPARC */

	if (debug)
		fprintf(stderr, "[%d] clearing bpt\n", tcp->pid);
	if (!(tcp->flags & TCB_BPTSET)) {
		fprintf(stderr, "PANIC: TCB not set in pid %u\n", tcp->pid);
		return -1;
	}
	errno = 0;
	ptrace(PTRACE_POKETEXT, tcp->pid, (char *) tcp->baddr, tcp->inst[0]);
	if (errno) {
		perror("clearbtp: ptrace(PTRACE_POKETEXT, ...)");
		return -1;
	}
	tcp->flags &= ~TCB_BPTSET;

#ifdef I386
	if (upeek(tcp->pid, 4*EIP, &eip) < 0)
		return -1;
	if (eip != tcp->baddr) {
		/* The breakpoint has not been reached yet.  */
		if (debug)
			fprintf(stderr,
				"NOTE: PC not at bpt (pc %#lx baddr %#lx)\n",
					eip, tcp->baddr);
		return 0;
	}
#else /* !I386 */
#ifdef POWERPC
	if (upeek(tcp->pid, 4*PT_NIP, &pc) < 0)
		return -1;
	if (pc != tcp->baddr) {
		/* The breakpoint has not been reached yet.  */
		if (debug)
			fprintf(stderr, "NOTE: PC not at bpt (pc %#lx baddr %#lx)\n",
				pc, tcp->baddr);
		return 0;
	}
#else /* !POWERPC */
#ifdef M68K
	if (upeek(tcp->pid, 4*PT_PC, &pc) < 0)
		return -1;
	if (pc != tcp->baddr) {
		/* The breakpoint has not been reached yet.  */
		if (debug)
			fprintf(stderr, "NOTE: PC not at bpt (pc %#lx baddr %#lx)\n",
				pc, tcp->baddr);
		return 0;
	}
#else /* !M68K */
#ifdef ALPHA
	if (upeek(tcp->pid, REG_PC, &pc) < 0)
		return -1;
	if (pc != tcp->baddr) {
		/* The breakpoint has not been reached yet.  */
		if (debug)
			fprintf(stderr, "NOTE: PC not at bpt (pc %#lx baddr %#lx)\n",
				pc, tcp->baddr);
		return 0;
	}
#endif /* ALPHA */
#endif /* !M68K */
#endif /* !POWERPC */
#endif /* !I386 */
#endif /* !SPARC */
#endif /* LINUX */

#ifdef SUNOS4
#ifdef SPARC

#if !LOOPA
	struct regs regs;
#endif

	if (!(tcp->flags & TCB_BPTSET)) {
		fprintf(stderr, "PANIC: TCB not set in pid %u\n", tcp->pid);
		return -1;
	}
	if (ptrace(PTRACE_WRITETEXT, tcp->pid, (char *) tcp->baddr,
				sizeof tcp->inst, (char *) tcp->inst) < 0) {
		perror("clearbtp: ptrace(PTRACE_WRITETEXT, ...)");
		return -1;
	}
	tcp->flags &= ~TCB_BPTSET;

#if !LOOPA
	/*
	 * Since we don't have a single instruction breakpoint, we may have
	 * to adjust the program counter after removing the our `breakpoint'.
	 */
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
		perror("clearbpt: ptrace(PTRACE_GETREGS, ...)");
		return -1;
	}
	if ((regs.r_pc < tcp->baddr) ||
				(regs.r_pc > tcp->baddr + 4)) {
		/* The breakpoint has not been reached yet */
		if (debug)
			fprintf(stderr,
				"NOTE: PC not at bpt (pc %#x baddr %#x)\n",
					regs.r_pc, tcp->parent->baddr);
		return 0;
	}
	if (regs.r_pc != tcp->baddr)
		if (debug)
			fprintf(stderr, "NOTE: PC adjusted (%#x -> %#x\n",
				regs.r_pc, tcp->baddr);

	regs.r_pc = tcp->baddr;
	if (ptrace(PTRACE_SETREGS, tcp->pid, (char *)&regs, 0) < 0) {
		perror("clearbpt: ptrace(PTRACE_SETREGS, ...)");
		return -1;
	}
#endif /* LOOPA */
#endif /* SPARC */
#endif /* SUNOS4 */

	return 0;
}

#endif /* !SVR4 */

#ifdef SUNOS4

static int
getex(pid, hdr)
int pid;
struct exec *hdr;
{
	int n;

	for (n = 0; n < sizeof *hdr; n += 4) {
		long res;
		if (upeek(pid, uoff(u_exdata) + n, &res) < 0)
			return -1;
		memcpy(((char *) hdr) + n, &res, 4);
	}
	if (debug) {
		fprintf(stderr, "[struct exec: magic: %o version %u Mach %o\n",
			hdr->a_magic, hdr->a_toolversion, hdr->a_machtype);
		fprintf(stderr, "Text %lu Data %lu Bss %lu Syms %lu Entry %#lx]\n",
			hdr->a_text, hdr->a_data, hdr->a_bss, hdr->a_syms, hdr->a_entry);
	}
	return 0;
}

int
fixvfork(tcp)
struct tcb *tcp;
{
	int pid = tcp->pid;
	/*
	 * Change `vfork' in a freshly exec'ed dynamically linked
	 * executable's (internal) symbol table to plain old `fork'
	 */

	struct exec hdr;
	struct link_dynamic dyn;
	struct link_dynamic_2 ld;
	char *strtab, *cp;

	if (getex(pid, &hdr) < 0)
		return -1;
	if (!hdr.a_dynamic)
		return -1;

	if (umove(tcp, (int) N_DATADDR(hdr), &dyn) < 0) {
		fprintf(stderr, "Cannot read DYNAMIC\n");
		return -1;
	}
	if (umove(tcp, (int) dyn.ld_un.ld_2, &ld) < 0) {
		fprintf(stderr, "Cannot read link_dynamic_2\n");
		return -1;
	}
	if ((strtab = malloc((unsigned)ld.ld_symb_size)) == NULL) {
		fprintf(stderr, "fixvfork: out of memory\n");
		return -1;
	}
	if (umoven(tcp, (int)ld.ld_symbols+(int)N_TXTADDR(hdr),
					(int)ld.ld_symb_size, strtab) < 0)
		goto err;

#if 0
	for (cp = strtab; cp < strtab + ld.ld_symb_size; ) {
		fprintf(stderr, "[symbol: %s]\n", cp);
		cp += strlen(cp)+1;
	}
	return 0;
#endif
	for (cp = strtab; cp < strtab + ld.ld_symb_size; ) {
		if (strcmp(cp, "_vfork") == 0) {
			if (debug)
				fprintf(stderr, "fixvfork: FOUND _vfork\n");
			strcpy(cp, "_fork");
			break;
		}
		cp += strlen(cp)+1;
	}
	if (cp < strtab + ld.ld_symb_size)
		/*
		 * Write entire symbol table back to avoid
		 * memory alignment bugs in ptrace
		 */
		if (tload(pid, (int)ld.ld_symbols+(int)N_TXTADDR(hdr),
					(int)ld.ld_symb_size, strtab) < 0)
			goto err;

	free(strtab);
	return 0;

err:
	free(strtab);
	return -1;
}

#endif /* SUNOS4 */
