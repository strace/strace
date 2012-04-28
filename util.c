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
 */

#include "defs.h"
#include <sys/user.h>
#include <sys/param.h>
#include <fcntl.h>
#if HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#if __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 1)
# include <linux/ptrace.h>
#endif

#if defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
#elif defined(HAVE_LINUX_PTRACE_H)
# undef PTRACE_SYSCALL
# ifdef HAVE_STRUCT_IA64_FPREG
#  define ia64_fpreg XXX_ia64_fpreg
# endif
# ifdef HAVE_STRUCT_PT_ALL_USER_REGS
#  define pt_all_user_regs XXX_pt_all_user_regs
# endif
# include <linux/ptrace.h>
# undef ia64_fpreg
# undef pt_all_user_regs
#endif

#if defined(SPARC64)
# undef PTRACE_GETREGS
# define PTRACE_GETREGS PTRACE_GETREGS64
# undef PTRACE_SETREGS
# define PTRACE_SETREGS PTRACE_SETREGS64
#endif

/* macros */
#ifndef MAX
# define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

int
string_to_uint(const char *str)
{
	char *error;
	long value;

	if (!*str)
		return -1;
	errno = 0;
	value = strtol(str, &error, 10);
	if (errno || *error || value < 0 || (long)(int)value != value)
		return -1;
	return (int)value;
}

int
tv_nz(struct timeval *a)
{
	return a->tv_sec || a->tv_usec;
}

int
tv_cmp(struct timeval *a, struct timeval *b)
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
tv_float(struct timeval *tv)
{
	return tv->tv_sec + tv->tv_usec/1000000.0;
}

void
tv_add(struct timeval *tv, struct timeval *a, struct timeval *b)
{
	tv->tv_sec = a->tv_sec + b->tv_sec;
	tv->tv_usec = a->tv_usec + b->tv_usec;
	if (tv->tv_usec >= 1000000) {
		tv->tv_sec++;
		tv->tv_usec -= 1000000;
	}
}

void
tv_sub(struct timeval *tv, struct timeval *a, struct timeval *b)
{
	tv->tv_sec = a->tv_sec - b->tv_sec;
	tv->tv_usec = a->tv_usec - b->tv_usec;
	if (((long) tv->tv_usec) < 0) {
		tv->tv_sec--;
		tv->tv_usec += 1000000;
	}
}

void
tv_div(struct timeval *tv, struct timeval *a, int n)
{
	tv->tv_usec = (a->tv_sec % n * 1000000 + a->tv_usec + n / 2) / n;
	tv->tv_sec = a->tv_sec / n + tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

void
tv_mul(struct timeval *tv, struct timeval *a, int n)
{
	tv->tv_usec = a->tv_usec * n;
	tv->tv_sec = a->tv_sec * n + tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

const char *
xlookup(const struct xlat *xlat, int val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

#if !defined HAVE_STPCPY
char *
stpcpy(char *dst, const char *src)
{
	while ((*dst = *src++) != '\0')
		dst++;
	return dst;
}
#endif

/*
 * Print entry in struct xlat table, if there.
 */
void
printxval(const struct xlat *xlat, int val, const char *dflt)
{
	const char *str = xlookup(xlat, val);

	if (str)
		tprints(str);
	else
		tprintf("%#x /* %s */", val, dflt);
}

#if HAVE_LONG_LONG
/*
 * Print 64bit argument at position llarg and return the index of the next
 * argument.
 */
int
printllval(struct tcb *tcp, const char *format, int llarg)
{
# if defined(X86_64) || defined(POWERPC64)
	if (current_personality == 0) {
		tprintf(format, tcp->u_arg[llarg]);
		llarg++;
	} else {
#  ifdef POWERPC64
		/* Align 64bit argument to 64bit boundary.  */
		llarg = (llarg + 1) & 0x1e;
#  endif
		tprintf(format, LONG_LONG(tcp->u_arg[llarg], tcp->u_arg[llarg + 1]));
		llarg += 2;
	}
# elif defined IA64 || defined ALPHA
	tprintf(format, tcp->u_arg[llarg]);
	llarg++;
# elif defined LINUX_MIPSN32 || defined X32
	tprintf(format, tcp->ext_arg[llarg]);
	llarg++;
# else
	tprintf(format, LONG_LONG(tcp->u_arg[llarg], tcp->u_arg[llarg + 1]));
	llarg += 2;
# endif
	return llarg;
}
#endif

/*
 * Interpret `xlat' as an array of flags
 * print the entries whose bits are on in `flags'
 * return # of flags printed.
 */
void
addflags(const struct xlat *xlat, int flags)
{
	for (; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			tprintf("|%s", xlat->str);
			flags &= ~xlat->val;
		}
	}
	if (flags) {
		tprintf("|%#x", flags);
	}
}

/*
 * Interpret `xlat' as an array of flags.
 * Print to static string the entries whose bits are on in `flags'
 * Return static string.
 */
const char *
sprintflags(const char *prefix, const struct xlat *xlat, int flags)
{
	static char outstr[1024];
	char *outptr;
	int found = 0;

	outptr = stpcpy(outstr, prefix);

	for (; xlat->str; xlat++) {
		if ((flags & xlat->val) == xlat->val) {
			if (found)
				*outptr++ = '|';
			outptr = stpcpy(outptr, xlat->str);
			found = 1;
			flags &= ~xlat->val;
			if (!flags)
				break;
		}
	}
	if (flags) {
		if (found)
			*outptr++ = '|';
		outptr += sprintf(outptr, "%#x", flags);
	}

	return outstr;
}

int
printflags(const struct xlat *xlat, int flags, const char *dflt)
{
	int n;
	const char *sep;

	if (flags == 0 && xlat->val == 0) {
		tprints(xlat->str);
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

	if (n) {
		if (flags) {
			tprintf("%s%#x", sep, flags);
			n++;
		}
	} else {
		if (flags) {
			tprintf("%#x", flags);
			if (dflt)
				tprintf(" /* %s */", dflt);
		} else {
			if (dflt)
				tprints("0");
		}
	}

	return n;
}

void
printnum(struct tcb *tcp, long addr, const char *fmt)
{
	long num;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (umove(tcp, addr, &num) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	tprints("[");
	tprintf(fmt, num);
	tprints("]");
}

void
printnum_int(struct tcb *tcp, long addr, const char *fmt)
{
	int num;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (umove(tcp, addr, &num) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	tprints("[");
	tprintf(fmt, num);
	tprints("]");
}

void
printfd(struct tcb *tcp, int fd)
{
	const char *p;

	if (show_fd_path && (p = getfdpath(tcp, fd)))
		tprintf("%d<%s>", fd, p);
	else
		tprintf("%d", fd);
}

void
printuid(const char *text, unsigned long uid)
{
	tprintf((uid == -1) ? "%s%ld" : "%s%lu", text, uid);
}

/*
 * Quote string `instr' of length `size'
 * Write up to (3 + `size' * 4) bytes to `outstr' buffer.
 * If `len' is -1, treat `instr' as a NUL-terminated string
 * and quote at most (`size' - 1) bytes.
 *
 * Returns 0 if len == -1 and NUL was seen, 1 otherwise.
 * Note that if len >= 0, always returns 1.
 */
int
string_quote(const char *instr, char *outstr, long len, int size)
{
	const unsigned char *ustr = (const unsigned char *) instr;
	char *s = outstr;
	int usehex, c, i, eol;

	eol = 0x100; /* this can never match a char */
	if (len == -1) {
		size--;
		eol = '\0';
	}

	usehex = 0;
	if (xflag > 1)
		usehex = 1;
	else if (xflag) {
		/* Check for presence of symbol which require
		   to hex-quote the whole string. */
		for (i = 0; i < size; ++i) {
			c = ustr[i];
			/* Check for NUL-terminated string. */
			if (c == eol)
				break;
			if (!isprint(c) && !isspace(c)) {
				usehex = 1;
				break;
			}
		}
	}

	*s++ = '\"';

	if (usehex) {
		/* Hex-quote the whole string. */
		for (i = 0; i < size; ++i) {
			c = ustr[i];
			/* Check for NUL-terminated string. */
			if (c == eol)
				goto asciz_ended;
			*s++ = '\\';
			*s++ = 'x';
			*s++ = "0123456789abcdef"[c >> 4];
			*s++ = "0123456789abcdef"[c & 0xf];
		}
	} else {
		for (i = 0; i < size; ++i) {
			c = ustr[i];
			/* Check for NUL-terminated string. */
			if (c == eol)
				goto asciz_ended;
			switch (c) {
				case '\"': case '\\':
					*s++ = '\\';
					*s++ = c;
					break;
				case '\f':
					*s++ = '\\';
					*s++ = 'f';
					break;
				case '\n':
					*s++ = '\\';
					*s++ = 'n';
					break;
				case '\r':
					*s++ = '\\';
					*s++ = 'r';
					break;
				case '\t':
					*s++ = '\\';
					*s++ = 't';
					break;
				case '\v':
					*s++ = '\\';
					*s++ = 'v';
					break;
				default:
					if (isprint(c))
						*s++ = c;
					else {
						/* Print \octal */
						*s++ = '\\';
						if (i + 1 < size
						    && ustr[i + 1] >= '0'
						    && ustr[i + 1] <= '9'
						) {
							/* Print \ooo */
							*s++ = '0' + (c >> 6);
							*s++ = '0' + ((c >> 3) & 0x7);
						} else {
							/* Print \[[o]o]o */
							if ((c >> 3) != 0) {
								if ((c >> 6) != 0)
									*s++ = '0' + (c >> 6);
								*s++ = '0' + ((c >> 3) & 0x7);
							}
						}
						*s++ = '0' + (c & 0x7);
					}
					break;
			}
		}
	}

	*s++ = '\"';
	*s = '\0';

	/* Return zero if we printed entire ASCIZ string (didn't truncate it) */
	if (len == -1 && ustr[i] == '\0') {
		/* We didn't see NUL yet (otherwise we'd jump to 'asciz_ended')
		 * but next char is NUL.
		 */
		return 0;
	}

	return 1;

 asciz_ended:
	*s++ = '\"';
	*s = '\0';
	/* Return zero: we printed entire ASCIZ string (didn't truncate it) */
	return 0;
}

/*
 * Print path string specified by address `addr' and length `n'.
 * If path length exceeds `n', append `...' to the output.
 */
void
printpathn(struct tcb *tcp, long addr, int n)
{
	char path[MAXPATHLEN + 1];
	int nul_seen;

	if (!addr) {
		tprints("NULL");
		return;
	}

	/* Cap path length to the path buffer size */
	if (n > sizeof path - 1)
		n = sizeof path - 1;

	/* Fetch one byte more to find out whether path length > n. */
	nul_seen = umovestr(tcp, addr, n + 1, path);
	if (nul_seen < 0)
		tprintf("%#lx", addr);
	else {
		char *outstr;

		path[n] = '\0';
		n++;
		outstr = alloca(4 * n); /* 4*(n-1) + 3 for quotes and NUL */
		string_quote(path, outstr, -1, n);
		tprints(outstr);
		if (!nul_seen)
			tprints("...");
	}
}

void
printpath(struct tcb *tcp, long addr)
{
	/* Size must correspond to char path[] size in printpathn */
	printpathn(tcp, addr, MAXPATHLEN);
}

/*
 * Print string specified by address `addr' and length `len'.
 * If `len' < 0, treat the string as a NUL-terminated string.
 * If string length exceeds `max_strlen', append `...' to the output.
 */
void
printstr(struct tcb *tcp, long addr, long len)
{
	static char *str = NULL;
	static char *outstr;
	int size;
	int ellipsis;

	if (!addr) {
		tprints("NULL");
		return;
	}
	/* Allocate static buffers if they are not allocated yet. */
	if (!str) {
		unsigned int outstr_size = 4 * max_strlen + /*for quotes and NUL:*/ 3;

		if (outstr_size / 4 != max_strlen)
			die_out_of_memory();
		str = malloc(max_strlen + 1);
		if (!str)
			die_out_of_memory();
		outstr = malloc(outstr_size);
		if (!outstr)
			die_out_of_memory();
	}

	if (len == -1) {
		/*
		 * Treat as a NUL-terminated string: fetch one byte more
		 * because string_quote() quotes one byte less.
		 */
		size = max_strlen + 1;
		if (umovestr(tcp, addr, size, str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
	}
	else {
		size = max_strlen;
		if (size > (unsigned long)len)
			size = (unsigned long)len;
		if (umoven(tcp, addr, size, str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
	}

	/* If string_quote didn't see NUL and (it was supposed to be ASCIZ str
	 * or we were requested to print more than -s NUM chars)...
	 */
	ellipsis = (string_quote(str, outstr, len, size) &&
			(len < 0 || len > max_strlen));

	tprints(outstr);
	if (ellipsis)
		tprints("...");
}

#if HAVE_SYS_UIO_H
void
dumpiov(struct tcb *tcp, int len, long addr)
{
#if SUPPORTED_PERSONALITIES > 1
	union {
		struct { u_int32_t base; u_int32_t len; } *iov32;
		struct { u_int64_t base; u_int64_t len; } *iov64;
	} iovu;
#define iov iovu.iov64
#define sizeof_iov \
	(current_wordsize == 4 ? sizeof(*iovu.iov32) : sizeof(*iovu.iov64))
#define iov_iov_base(i) \
	(current_wordsize == 4 ? (uint64_t) iovu.iov32[i].base : iovu.iov64[i].base)
#define iov_iov_len(i) \
	(current_wordsize == 4 ? (uint64_t) iovu.iov32[i].len : iovu.iov64[i].len)
#else
	struct iovec *iov;
#define sizeof_iov sizeof(*iov)
#define iov_iov_base(i) iov[i].iov_base
#define iov_iov_len(i) iov[i].iov_len
#endif
	int i;
	unsigned size;

	size = sizeof_iov * len;
	/* Assuming no sane program has millions of iovs */
	if ((unsigned)len > 1024*1024 /* insane or negative size? */
	    || (iov = malloc(size)) == NULL) {
		fprintf(stderr, "Out of memory\n");
		return;
	}
	if (umoven(tcp, addr, size, (char *) iov) >= 0) {
		for (i = 0; i < len; i++) {
			/* include the buffer number to make it easy to
			 * match up the trace with the source */
			tprintf(" * %lu bytes in buffer %d\n",
				(unsigned long)iov_iov_len(i), i);
			dumpstr(tcp, (long) iov_iov_base(i),
				iov_iov_len(i));
		}
	}
	free(iov);
#undef sizeof_iov
#undef iov_iov_base
#undef iov_iov_len
#undef iov
}
#endif

void
dumpstr(struct tcb *tcp, long addr, int len)
{
	static int strsize = -1;
	static unsigned char *str;
	char *s;
	int i, j;

	if (strsize < len) {
		free(str);
		str = malloc(len);
		if (!str) {
			strsize = -1;
			fprintf(stderr, "Out of memory\n");
			return;
		}
		strsize = len;
	}

	if (umoven(tcp, addr, len, (char *) str) < 0)
		return;

	for (i = 0; i < len; i += 16) {
		char outstr[80];

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

#ifdef HAVE_PROCESS_VM_READV
/* C library supports this, but the kernel might not. */
static bool process_vm_readv_not_supported = 0;
#else

/* Need to do this since process_vm_readv() is not yet available in libc.
 * When libc is be updated, only "static bool process_vm_readv_not_supported"
 * line should remain.
 */
#if !defined(__NR_process_vm_readv)
# if defined(I386)
#  define __NR_process_vm_readv  347
# elif defined(X86_64)
#  define __NR_process_vm_readv  310
# elif defined(POWERPC)
#  define __NR_process_vm_readv  351
# endif
#endif

#if defined(__NR_process_vm_readv)
static bool process_vm_readv_not_supported = 0;
static ssize_t process_vm_readv(pid_t pid,
		 const struct iovec *lvec,
		 unsigned long liovcnt,
		 const struct iovec *rvec,
		 unsigned long riovcnt,
		 unsigned long flags)
{
	return syscall(__NR_process_vm_readv, (long)pid, lvec, liovcnt, rvec, riovcnt, flags);
}
#else
static bool process_vm_readv_not_supported = 1;
# define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

#endif /* end of hack */

#define PAGMASK	(~(PAGSIZ - 1))
/*
 * move `len' bytes of data from process `pid'
 * at address `addr' to our space at `laddr'
 */
int
umoven(struct tcb *tcp, long addr, int len, char *laddr)
{
	int pid = tcp->pid;
	int n, m;
	int started;
	union {
		long val;
		char x[sizeof(long)];
	} u;

#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize < sizeof(addr))
		addr &= (1ul << 8 * current_wordsize) - 1;
#endif

	if (!process_vm_readv_not_supported) {
		struct iovec local[1], remote[1];
		int r;

		local[0].iov_base = laddr;
		remote[0].iov_base = (void*)addr;
		local[0].iov_len = remote[0].iov_len = len;
		r = process_vm_readv(pid,
				local, 1,
				remote, 1,
				/*flags:*/ 0
		);
		if (r < 0) {
			if (errno == ENOSYS)
				process_vm_readv_not_supported = 1;
			else if (errno != EINVAL) /* EINVAL is seen if process is gone */
				/* strange... */
				perror("process_vm_readv");
			goto vm_readv_didnt_work;
		}
		return r;
	}
 vm_readv_didnt_work:

	started = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		if (errno) {
			/* But if not started, we had a bogus address. */
			if (addr != 0 && errno != EIO && errno != ESRCH)
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%lx", pid, addr);
			return -1;
		}
		started = 1;
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
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
			if (addr != 0 && errno != EIO && errno != ESRCH)
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%lx", pid, addr);
			return -1;
		}
		started = 1;
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		addr += sizeof(long), laddr += m, len -= m;
	}

	return 0;
}

/*
 * Like `umove' but make the additional effort of looking
 * for a terminating zero byte.
 *
 * Returns < 0 on error, > 0 if NUL was seen,
 * (TODO if useful: return count of bytes including NUL),
 * else 0 if len bytes were read but no NUL byte seen.
 *
 * Note: there is no guarantee we won't overwrite some bytes
 * in laddr[] _after_ terminating NUL (but, of course,
 * we never write past laddr[len-1]).
 */
int
umovestr(struct tcb *tcp, long addr, int len, char *laddr)
{
	int started;
	int pid = tcp->pid;
	int i, n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;

#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize < sizeof(addr))
		addr &= (1ul << 8 * current_wordsize) - 1;
#endif

	if (!process_vm_readv_not_supported) {
		struct iovec local[1], remote[1];

		local[0].iov_base = laddr;
		remote[0].iov_base = (void*)addr;

		while (len > 0) {
			int end_in_page;
			int r;
			int chunk_len;

			/* Don't read kilobytes: most strings are short */
			chunk_len = len;
			if (chunk_len > 256)
				chunk_len = 256;
			/* Don't cross pages. I guess otherwise we can get EFAULT
			 * and fail to notice that terminating NUL lies
			 * in the existing (first) page.
			 * (I hope there aren't arches with pages < 4K)
			 */
			end_in_page = ((addr + chunk_len) & 4095);
			r = chunk_len - end_in_page;
			if (r > 0) /* if chunk_len > end_in_page */
				chunk_len = r; /* chunk_len -= end_in_page */

			local[0].iov_len = remote[0].iov_len = chunk_len;
			r = process_vm_readv(pid,
					local, 1,
					remote, 1,
					/*flags:*/ 0
			);
			if (r < 0) {
				if (errno == ENOSYS)
					process_vm_readv_not_supported = 1;
				else if (errno != EINVAL) /* EINVAL is seen if process is gone */
					/* strange... */
					perror("process_vm_readv");
				goto vm_readv_didnt_work;
			}
			if (memchr(local[0].iov_base, '\0', r))
				return 1;
			local[0].iov_base += r;
			remote[0].iov_base += r;
			len -= r;
		}
		return 0;
	}
 vm_readv_didnt_work:

	started = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *)addr, 0);
		if (errno) {
			if (addr != 0 && errno != EIO && errno != ESRCH)
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%lx", pid, addr);
			return -1;
		}
		started = 1;
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return 1;
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
			if (addr != 0 && errno != EIO && errno != ESRCH)
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%lx", pid, addr);
			return -1;
		}
		started = 1;
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		for (i = 0; i < sizeof(long); i++)
			if (u.x[i] == '\0')
				return 1;
		addr += sizeof(long), laddr += m, len -= m;
	}
	return 0;
}

int
upeek(struct tcb *tcp, long off, long *res)
{
	long val;

	errno = 0;
	val = ptrace(PTRACE_PEEKUSER, tcp->pid, (char *) off, 0);
	if (val == -1 && errno) {
		if (errno != ESRCH) {
			perror_msg("upeek: PTRACE_PEEKUSER pid:%d @0x%lx)", tcp->pid, off);
		}
		return -1;
	}
	*res = val;
	return 0;
}

void
printcall(struct tcb *tcp)
{
#define PRINTBADPC tprintf(sizeof(long) == 4 ? "[????????] " : \
			   sizeof(long) == 8 ? "[????????????????] " : \
			   NULL /* crash */)

#if defined(I386)
	long eip;

	if (upeek(tcp, 4*EIP, &eip) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", eip);
#elif defined(S390) || defined(S390X)
	long psw;
	if (upeek(tcp, PT_PSWADDR, &psw) < 0) {
		PRINTBADPC;
		return;
	}
# ifdef S390
	tprintf("[%08lx] ", psw);
# elif S390X
	tprintf("[%16lx] ", psw);
# endif

#elif defined(X86_64) || defined(X32)
	long rip;

	if (upeek(tcp, 8*RIP, &rip) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%16lx] ", rip);
#elif defined(IA64)
	long ip;

	if (upeek(tcp, PT_B0, &ip) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", ip);
#elif defined(POWERPC)
	long pc;

	if (upeek(tcp, sizeof(unsigned long)*PT_NIP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
# ifdef POWERPC64
	tprintf("[%016lx] ", pc);
# else
	tprintf("[%08lx] ", pc);
# endif
#elif defined(M68K)
	long pc;

	if (upeek(tcp, 4*PT_PC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(ALPHA)
	long pc;

	if (upeek(tcp, REG_PC, &pc) < 0) {
		tprints("[????????????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SPARC) || defined(SPARC64)
	struct pt_regs regs;
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
		PRINTBADPC;
		return;
	}
# if defined(SPARC64)
	tprintf("[%08lx] ", regs.tpc);
# else
	tprintf("[%08lx] ", regs.pc);
# endif
#elif defined(HPPA)
	long pc;

	if (upeek(tcp, PT_IAOQ0, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(MIPS)
	long pc;

	if (upeek(tcp, REG_EPC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SH)
	long pc;

	if (upeek(tcp, 4*REG_PC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SH64)
	long pc;

	if (upeek(tcp, REG_PC, &pc) < 0) {
		tprints("[????????????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(ARM)
	long pc;

	if (upeek(tcp, 4*15, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(AVR32)
	long pc;

	if (upeek(tcp, REG_PC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(BFIN)
	long pc;

	if (upeek(tcp, PT_PC, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(CRISV10)
	long pc;

	if (upeek(tcp, 4*PT_IRP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(CRISV32)
	long pc;

	if (upeek(tcp, 4*PT_ERP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#endif /* architecture */
}

/*
 * These #if's are huge, please indent them correctly.
 * It's easy to get confused otherwise.
 */

#include "syscall.h"

#ifndef CLONE_PTRACE
# define CLONE_PTRACE    0x00002000
#endif
#ifndef CLONE_VFORK
# define CLONE_VFORK     0x00004000
#endif
#ifndef CLONE_VM
# define CLONE_VM        0x00000100
#endif

static int
change_syscall(struct tcb *tcp, int new)
{
#if defined(I386)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(ORIG_EAX * 4), new) < 0)
		return -1;
	return 0;
#elif defined(X86_64) || defined(X32)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(ORIG_RAX * 8), new) < 0)
		return -1;
	return 0;
#elif defined(POWERPC)
	if (ptrace(PTRACE_POKEUSER, tcp->pid,
		   (char*)(sizeof(unsigned long)*PT_R0), new) < 0)
		return -1;
	return 0;
#elif defined(S390) || defined(S390X)
	/* s390 linux after 2.4.7 has a hook in entry.S to allow this */
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_GPR2), new) < 0)
		return -1;
	return 0;
#elif defined(M68K)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(4*PT_ORIG_D0), new) < 0)
		return -1;
	return 0;
#elif defined(SPARC) || defined(SPARC64)
	struct pt_regs regs;
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char*)&regs, 0) < 0)
		return -1;
	regs.u_regs[U_REG_G1] = new;
	if (ptrace(PTRACE_SETREGS, tcp->pid, (char*)&regs, 0) < 0)
		return -1;
	return 0;
#elif defined(MIPS)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_V0), new) < 0)
		return -1;
	return 0;
#elif defined(ALPHA)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_A3), new) < 0)
		return -1;
	return 0;
#elif defined(AVR32)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_R8), new) < 0)
		return -1;
	return 0;
#elif defined(BFIN)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_P0), new) < 0)
		return -1;
	return 0;
#elif defined(IA64)
	if (ia32) {
		switch (new) {
		case 2:
			break;	/* x86 SYS_fork */
		case SYS_clone:
			new = 120;
			break;
		default:
			fprintf(stderr, "%s: unexpected syscall %d\n",
				__FUNCTION__, new);
			return -1;
		}
		if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_R1), new) < 0)
			return -1;
	} else if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_R15), new) < 0)
		return -1;
	return 0;
#elif defined(HPPA)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_GR20), new) < 0)
		return -1;
	return 0;
#elif defined(SH)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(4*(REG_REG0+3)), new) < 0)
		return -1;
	return 0;
#elif defined(SH64)
	/* Top half of reg encodes the no. of args n as 0x1n.
	   Assume 0 args as kernel never actually checks... */
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_SYSCALL),
				0x100000 | new) < 0)
		return -1;
	return 0;
#elif defined(CRISV10) || defined(CRISV32)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(4*PT_R9), new) < 0)
		return -1;
	return 0;
#elif defined(ARM)
	/* Some kernels support this, some (pre-2.6.16 or so) don't.  */
# ifndef PTRACE_SET_SYSCALL
#  define PTRACE_SET_SYSCALL 23
# endif
	if (ptrace(PTRACE_SET_SYSCALL, tcp->pid, 0, new & 0xffff) != 0)
		return -1;
	return 0;
#elif defined(TILE)
	if (ptrace(PTRACE_POKEUSER, tcp->pid,
		   (char*)PTREGS_OFFSET_REG(0),
		   new) != 0)
		return -1;
	return 0;
#elif defined(MICROBLAZE)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_GPR(0)), new) < 0)
		return -1;
	return 0;
#else
#warning Do not know how to handle change_syscall for this architecture
#endif /* architecture */
	return -1;
}

#ifdef IA64

typedef unsigned long *arg_setup_state;

static int
arg_setup(struct tcb *tcp, arg_setup_state *state)
{
	unsigned long cfm, sof, sol;
	long bsp;

	if (ia32) {
		/* Satisfy a false GCC warning.  */
		*state = NULL;
		return 0;
	}

	if (upeek(tcp, PT_AR_BSP, &bsp) < 0)
		return -1;
	if (upeek(tcp, PT_CFM, (long *) &cfm) < 0)
		return -1;

	sof = (cfm >> 0) & 0x7f;
	sol = (cfm >> 7) & 0x7f;
	bsp = (long) ia64_rse_skip_regs((unsigned long *) bsp, -sof + sol);

	*state = (unsigned long *) bsp;
	return 0;
}

# define arg_finish_change(tcp, state)	0

static int
get_arg0(struct tcb *tcp, arg_setup_state *state, long *valp)
{
	int ret;

	if (ia32)
		ret = upeek(tcp, PT_R11, valp);
	else
		ret = umoven(tcp,
			      (unsigned long) ia64_rse_skip_regs(*state, 0),
			      sizeof(long), (void *) valp);
	return ret;
}

static int
get_arg1(struct tcb *tcp, arg_setup_state *state, long *valp)
{
	int ret;

	if (ia32)
		ret = upeek(tcp, PT_R9, valp);
	else
		ret = umoven(tcp,
			      (unsigned long) ia64_rse_skip_regs(*state, 1),
			      sizeof(long), (void *) valp);
	return ret;
}

static int
set_arg0(struct tcb *tcp, arg_setup_state *state, long val)
{
	int req = PTRACE_POKEDATA;
	void *ap;

	if (ia32) {
		ap = (void *) (intptr_t) PT_R11;	 /* r11 == EBX */
		req = PTRACE_POKEUSER;
	} else
		ap = ia64_rse_skip_regs(*state, 0);
	errno = 0;
	ptrace(req, tcp->pid, ap, val);
	return errno ? -1 : 0;
}

static int
set_arg1(struct tcb *tcp, arg_setup_state *state, long val)
{
	int req = PTRACE_POKEDATA;
	void *ap;

	if (ia32) {
		ap = (void *) (intptr_t) PT_R9;		/* r9 == ECX */
		req = PTRACE_POKEUSER;
	} else
		ap = ia64_rse_skip_regs(*state, 1);
	errno = 0;
	ptrace(req, tcp->pid, ap, val);
	return errno ? -1 : 0;
}

/* ia64 does not return the input arguments from functions (and syscalls)
   according to ia64 RSE (Register Stack Engine) behavior.  */

# define restore_arg0(tcp, state, val) ((void) (state), 0)
# define restore_arg1(tcp, state, val) ((void) (state), 0)

#elif defined(SPARC) || defined(SPARC64)

typedef struct pt_regs arg_setup_state;

# define arg_setup(tcp, state) \
    (ptrace(PTRACE_GETREGS, (tcp)->pid, (char *) (state), 0))
# define arg_finish_change(tcp, state) \
    (ptrace(PTRACE_SETREGS, (tcp)->pid, (char *) (state), 0))

# define get_arg0(tcp, state, valp) (*(valp) = (state)->u_regs[U_REG_O0], 0)
# define get_arg1(tcp, state, valp) (*(valp) = (state)->u_regs[U_REG_O1], 0)
# define set_arg0(tcp, state, val)  ((state)->u_regs[U_REG_O0] = (val), 0)
# define set_arg1(tcp, state, val)  ((state)->u_regs[U_REG_O1] = (val), 0)
# define restore_arg0(tcp, state, val) 0

#else /* other architectures */

# if defined S390 || defined S390X
/* Note: this is only true for the `clone' system call, which handles
   arguments specially.  We could as well say that its first two arguments
   are swapped relative to other architectures, but that would just be
   another #ifdef in the calls.  */
#  define arg0_offset	PT_GPR3
#  define arg1_offset	PT_ORIGGPR2
#  define restore_arg0(tcp, state, val) ((void) (state), 0)
#  define restore_arg1(tcp, state, val) ((void) (state), 0)
#  define arg0_index	1
#  define arg1_index	0
# elif defined(ALPHA) || defined(MIPS)
#  define arg0_offset	REG_A0
#  define arg1_offset	(REG_A0+1)
# elif defined(AVR32)
#  define arg0_offset	(REG_R12)
#  define arg1_offset	(REG_R11)
# elif defined(POWERPC)
#  define arg0_offset	(sizeof(unsigned long)*PT_R3)
#  define arg1_offset	(sizeof(unsigned long)*PT_R4)
#  define restore_arg0(tcp, state, val) ((void) (state), 0)
# elif defined(HPPA)
#  define arg0_offset	PT_GR26
#  define arg1_offset	(PT_GR26-4)
# elif defined(X86_64) || defined(X32)
#  define arg0_offset	((long)(8*(current_personality ? RBX : RDI)))
#  define arg1_offset	((long)(8*(current_personality ? RCX : RSI)))
# elif defined(SH)
#  define arg0_offset	(4*(REG_REG0+4))
#  define arg1_offset	(4*(REG_REG0+5))
# elif defined(SH64)
   /* ABI defines arg0 & 1 in r2 & r3 */
#  define arg0_offset	(REG_OFFSET+16)
#  define arg1_offset	(REG_OFFSET+24)
#  define restore_arg0(tcp, state, val) 0
# elif defined CRISV10 || defined CRISV32
#  define arg0_offset	(4*PT_R11)
#  define arg1_offset	(4*PT_ORIG_R10)
#  define restore_arg0(tcp, state, val) 0
#  define restore_arg1(tcp, state, val) 0
#  define arg0_index	1
#  define arg1_index	0
# else
#  define arg0_offset	0
#  define arg1_offset	4
#  if defined ARM
#   define restore_arg0(tcp, state, val) 0
#  endif
# endif

typedef int arg_setup_state;

# define arg_setup(tcp, state)         (0)
# define arg_finish_change(tcp, state) 0
# define get_arg0(tcp, cookie, valp)   (upeek((tcp), arg0_offset, (valp)))
# define get_arg1(tcp, cookie, valp)   (upeek((tcp), arg1_offset, (valp)))

static int
set_arg0(struct tcb *tcp, void *cookie, long val)
{
	return ptrace(PTRACE_POKEUSER, tcp->pid, (char*)arg0_offset, val);
}

static int
set_arg1(struct tcb *tcp, void *cookie, long val)
{
	return ptrace(PTRACE_POKEUSER, tcp->pid, (char*)arg1_offset, val);
}

#endif /* architectures */

#ifndef restore_arg0
# define restore_arg0(tcp, state, val) set_arg0((tcp), (state), (val))
#endif
#ifndef restore_arg1
# define restore_arg1(tcp, state, val) set_arg1((tcp), (state), (val))
#endif

#ifndef arg0_index
# define arg0_index 0
# define arg1_index 1
#endif

int
setbpt(struct tcb *tcp)
{
	static int clone_scno[SUPPORTED_PERSONALITIES] = { SYS_clone };
	arg_setup_state state;

	if (tcp->flags & TCB_BPTSET) {
		fprintf(stderr, "PANIC: TCB already set in pid %u\n", tcp->pid);
		return -1;
	}

	/*
	 * It's a silly kludge to initialize this with a search at runtime.
	 * But it's better than maintaining another magic thing in the
	 * godforsaken tables.
	 */
	if (clone_scno[current_personality] == 0) {
		int i;
		for (i = 0; i < nsyscalls; ++i)
			if (sysent[i].sys_func == sys_clone) {
				clone_scno[current_personality] = i;
				break;
			}
	}

	if (sysent[tcp->scno].sys_func == sys_fork ||
	    sysent[tcp->scno].sys_func == sys_vfork) {
		if (arg_setup(tcp, &state) < 0
		    || get_arg0(tcp, &state, &tcp->inst[0]) < 0
		    || get_arg1(tcp, &state, &tcp->inst[1]) < 0
		    || change_syscall(tcp, clone_scno[current_personality]) < 0
		    || set_arg0(tcp, &state, CLONE_PTRACE|SIGCHLD) < 0
		    || set_arg1(tcp, &state, 0) < 0
		    || arg_finish_change(tcp, &state) < 0)
			return -1;
		tcp->u_arg[arg0_index] = CLONE_PTRACE|SIGCHLD;
		tcp->u_arg[arg1_index] = 0;
		tcp->flags |= TCB_BPTSET;
		return 0;
	}

	if (sysent[tcp->scno].sys_func == sys_clone) {
		/* ia64 calls directly `clone (CLONE_VFORK | CLONE_VM)'
		   contrary to x86 vfork above.  Even on x86 we turn the
		   vfork semantics into plain fork - each application must not
		   depend on the vfork specifics according to POSIX.  We would
		   hang waiting for the parent resume otherwise.  We need to
		   clear also CLONE_VM but only in the CLONE_VFORK case as
		   otherwise we would break pthread_create.  */

		long new_arg0 = (tcp->u_arg[arg0_index] | CLONE_PTRACE);
		if (new_arg0 & CLONE_VFORK)
			new_arg0 &= ~(unsigned long)(CLONE_VFORK | CLONE_VM);
		if (arg_setup(tcp, &state) < 0
		 || set_arg0(tcp, &state, new_arg0) < 0
		 || arg_finish_change(tcp, &state) < 0)
			return -1;
		tcp->flags |= TCB_BPTSET;
		tcp->inst[0] = tcp->u_arg[arg0_index];
		tcp->inst[1] = tcp->u_arg[arg1_index];
		return 0;
	}

	fprintf(stderr, "PANIC: setbpt for syscall %ld on %u???\n",
		tcp->scno, tcp->pid);
	return -1;
}

int
clearbpt(struct tcb *tcp)
{
	arg_setup_state state;
	if (arg_setup(tcp, &state) < 0
	    || restore_arg0(tcp, &state, tcp->inst[0]) < 0
	    || restore_arg1(tcp, &state, tcp->inst[1]) < 0
	    || arg_finish_change(tcp, &state))
		if (errno != ESRCH)
			return -1;
	tcp->flags &= ~TCB_BPTSET;
	return 0;
}
