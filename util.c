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
#include <sys/param.h>
#include <fcntl.h>
#if HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include <sys/uio.h>

#include "regs.h"
#include "ptrace.h"

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
tv_nz(const struct timeval *a)
{
	return a->tv_sec || a->tv_usec;
}

int
tv_cmp(const struct timeval *a, const struct timeval *b)
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
tv_float(const struct timeval *tv)
{
	return tv->tv_sec + tv->tv_usec/1000000.0;
}

void
tv_add(struct timeval *tv, const struct timeval *a, const struct timeval *b)
{
	tv->tv_sec = a->tv_sec + b->tv_sec;
	tv->tv_usec = a->tv_usec + b->tv_usec;
	if (tv->tv_usec >= 1000000) {
		tv->tv_sec++;
		tv->tv_usec -= 1000000;
	}
}

void
tv_sub(struct timeval *tv, const struct timeval *a, const struct timeval *b)
{
	tv->tv_sec = a->tv_sec - b->tv_sec;
	tv->tv_usec = a->tv_usec - b->tv_usec;
	if (((long) tv->tv_usec) < 0) {
		tv->tv_sec--;
		tv->tv_usec += 1000000;
	}
}

void
tv_div(struct timeval *tv, const struct timeval *a, int n)
{
	tv->tv_usec = (a->tv_sec % n * 1000000 + a->tv_usec + n / 2) / n;
	tv->tv_sec = a->tv_sec / n + tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

void
tv_mul(struct timeval *tv, const struct timeval *a, int n)
{
	tv->tv_usec = a->tv_usec * n;
	tv->tv_sec = a->tv_sec * n + tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
}

const char *
xlookup(const struct xlat *xlat, const unsigned int val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

static int
xlat_bsearch_compare(const void *a, const void *b)
{
	const unsigned int val1 = (const unsigned long) a;
	const unsigned int val2 = ((const struct xlat *) b)->val;
	return (val1 > val2) ? 1 : (val1 < val2) ? -1 : 0;
}

const char *
xlat_search(const struct xlat *xlat, const size_t nmemb, const unsigned int val)
{
	const struct xlat *e =
		bsearch((const void*) (const unsigned long) val,
			xlat, nmemb, sizeof(*xlat), xlat_bsearch_compare);

	return e ? e->str : NULL;
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

/* Find a next bit which is set.
 * Starts testing at cur_bit.
 * Returns -1 if no more bits are set.
 *
 * We never touch bytes we don't need to.
 * On big-endian, array is assumed to consist of
 * current_wordsize wide words: for example, is current_wordsize is 4,
 * the bytes are walked in 3,2,1,0, 7,6,5,4, 11,10,9,8 ... sequence.
 * On little-endian machines, word size is immaterial.
 */
int
next_set_bit(const void *bit_array, unsigned cur_bit, unsigned size_bits)
{
	const unsigned endian = 1;
	int little_endian = *(char*)&endian;

	const uint8_t *array = bit_array;
	unsigned pos = cur_bit / 8;
	unsigned pos_xor_mask = little_endian ? 0 : current_wordsize-1;

	for (;;) {
		uint8_t bitmask;
		uint8_t cur_byte;

		if (cur_bit >= size_bits)
			return -1;
		cur_byte = array[pos ^ pos_xor_mask];
		if (cur_byte == 0) {
			cur_bit = (cur_bit + 8) & (-8);
			pos++;
			continue;
		}
		bitmask = 1 << (cur_bit & 7);
		for (;;) {
			if (cur_byte & bitmask)
				return cur_bit;
			cur_bit++;
			if (cur_bit >= size_bits)
				return -1;
			bitmask <<= 1;
			/* This check *can't be* optimized out: */
			if (bitmask == 0)
				break;
		}
		pos++;
	}
}
/*
 * Print entry in struct xlat table, if there.
 */
void
printxval(const struct xlat *xlat, const unsigned int val, const char *dflt)
{
	const char *str = xlookup(xlat, val);

	if (str)
		tprints(str);
	else
		tprintf("%#x /* %s */", val, dflt);
}

/*
 * Fetch 64bit argument at position arg_no and
 * return the index of the next argument.
 */
int
getllval(struct tcb *tcp, unsigned long long *val, int arg_no)
{
#if SIZEOF_LONG > 4 && SIZEOF_LONG == SIZEOF_LONG_LONG
# if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize > 4) {
# endif
		*val = tcp->u_arg[arg_no];
		arg_no++;
# if SUPPORTED_PERSONALITIES > 1
	} else {
#  if defined(AARCH64) || defined(POWERPC64)
		/* Align arg_no to the next even number. */
		arg_no = (arg_no + 1) & 0xe;
#  endif /* AARCH64 || POWERPC64 */
		*val = LONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
		arg_no += 2;
	}
# endif /* SUPPORTED_PERSONALITIES > 1 */
#elif SIZEOF_LONG > 4
#  error Unsupported configuration: SIZEOF_LONG > 4 && SIZEOF_LONG_LONG > SIZEOF_LONG
#elif defined LINUX_MIPSN32
	*val = tcp->ext_arg[arg_no];
	arg_no++;
#elif defined X32
	if (current_personality == 0) {
		*val = tcp->ext_arg[arg_no];
		arg_no++;
	} else {
		*val = LONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
		arg_no += 2;
	}
#else
# if defined __ARM_EABI__ || \
     defined LINUX_MIPSO32 || \
     defined POWERPC || \
     defined XTENSA
	/* Align arg_no to the next even number. */
	arg_no = (arg_no + 1) & 0xe;
# endif
	*val = LONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
	arg_no += 2;
#endif

	return arg_no;
}

/*
 * Print 64bit argument at position arg_no and
 * return the index of the next argument.
 */
int
printllval(struct tcb *tcp, const char *format, int arg_no)
{
	unsigned long long val = 0;

	arg_no = getllval(tcp, &val, arg_no);
	tprintf(format, val);
	return arg_no;
}

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
printnum_long(struct tcb *tcp, long addr, const char *fmt)
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

const char *
sprinttime(time_t t)
{
	struct tm *tmp;
	static char buf[sizeof(int) * 3 * 6];

	if (t == 0) {
		strcpy(buf, "0");
		return buf;
	}
	tmp = localtime(&t);
	if (tmp)
		snprintf(buf, sizeof buf, "%02d/%02d/%02d-%02d:%02d:%02d",
			tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
			tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	else
		snprintf(buf, sizeof buf, "%lu", (unsigned long) t);

	return buf;
}

static char *
getfdproto(struct tcb *tcp, int fd, char *buf, unsigned bufsize)
{
#if HAVE_SYS_XATTR_H
	ssize_t r;
	char path[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];

	if (fd < 0)
		return NULL;

	sprintf(path, "/proc/%u/fd/%u", tcp->pid, fd);
	r = getxattr(path, "system.sockprotoname", buf, bufsize - 1);
	if (r <= 0)
		return NULL;
	else {
		/*
		 * This is a protection for the case when the kernel
		 * side does not append a null byte to the buffer.
		 */
		buf[r] = '\0';
		return buf;
	}
#else
	return NULL;
#endif
}

void
printfd(struct tcb *tcp, int fd)
{
	char path[PATH_MAX + 1];
	if (show_fd_path && getfdpath(tcp, fd, path, sizeof(path)) >= 0) {
		static const char socket_prefix[] = "socket:[";
		const size_t socket_prefix_len = sizeof(socket_prefix) - 1;
		const size_t path_len = strlen(path);

		tprintf("%d<", fd);
		if (show_fd_path > 1 &&
		    strncmp(path, socket_prefix, socket_prefix_len) == 0 &&
		    path[path_len - 1] == ']') {
			unsigned long inodenr;
#define PROTO_NAME_LEN 32
			char proto_buf[PROTO_NAME_LEN];
			const char *proto =
				getfdproto(tcp, fd, proto_buf, PROTO_NAME_LEN);
			inodenr = strtoul(path + socket_prefix_len, NULL, 10);
			if (!print_sockaddr_by_inode(inodenr, proto)) {
				if (proto)
					tprintf("%s:[%lu]", proto, inodenr);
				else
					tprints(path);
			}
		} else {
			print_quoted_string(path, path_len,
					    QUOTE_OMIT_LEADING_TRAILING_QUOTES);
		}
		tprints(">");
	} else
		tprintf("%d", fd);
}

/*
 * Quote string `instr' of length `size'
 * Write up to (3 + `size' * 4) bytes to `outstr' buffer.
 *
 * If QUOTE_0_TERMINATED `style' flag is set,
 * treat `instr' as a NUL-terminated string,
 * checking up to (`size' + 1) bytes of `instr'.
 *
 * If QUOTE_OMIT_LEADING_TRAILING_QUOTES `style' flag is set,
 * do not add leading and trailing quoting symbols.
 *
 * Returns 0 if QUOTE_0_TERMINATED is set and NUL was seen, 1 otherwise.
 * Note that if QUOTE_0_TERMINATED is not set, always returns 1.
 */
static int
string_quote(const char *instr, char *outstr, const unsigned int size,
	     const unsigned int style)
{
	const unsigned char *ustr = (const unsigned char *) instr;
	char *s = outstr;
	unsigned int i;
	int usehex, c, eol;

	if (style & QUOTE_0_TERMINATED)
		eol = '\0';
	else
		eol = 0x100; /* this can never match a char */

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

			/* Force hex unless c is printable or whitespace */
			if (c > 0x7e) {
				usehex = 1;
				break;
			}
			/* In ASCII isspace is only these chars: "\t\n\v\f\r".
			 * They happen to have ASCII codes 9,10,11,12,13.
			 */
			if (c < ' ' && (unsigned)(c - 9) >= 5) {
				usehex = 1;
				break;
			}
		}
	}

	if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
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
					if (c >= ' ' && c <= 0x7e)
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

	if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
		*s++ = '\"';
	*s = '\0';

	/* Return zero if we printed entire ASCIZ string (didn't truncate it) */
	if (style & QUOTE_0_TERMINATED && ustr[i] == '\0') {
		/* We didn't see NUL yet (otherwise we'd jump to 'asciz_ended')
		 * but next char is NUL.
		 */
		return 0;
	}

	return 1;

 asciz_ended:
	if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
		*s++ = '\"';
	*s = '\0';
	/* Return zero: we printed entire ASCIZ string (didn't truncate it) */
	return 0;
}

#ifndef ALLOCA_CUTOFF
# define ALLOCA_CUTOFF	4032
#endif
#define use_alloca(n) ((n) <= ALLOCA_CUTOFF)

/*
 * Quote string `str' of length `size' and print the result.
 *
 * If QUOTE_0_TERMINATED `style' flag is set,
 * treat `str' as a NUL-terminated string and
 * quote at most (`size' - 1) bytes.
 *
 * If QUOTE_OMIT_LEADING_TRAILING_QUOTES `style' flag is set,
 * do not add leading and trailing quoting symbols.
 *
 * Returns 0 if QUOTE_0_TERMINATED is set and NUL was seen, 1 otherwise.
 * Note that if QUOTE_0_TERMINATED is not set, always returns 1.
 */
int
print_quoted_string(const char *str, unsigned int size,
		    const unsigned int style)
{
	char *buf;
	char *outstr;
	unsigned int alloc_size;
	int rc;

	if (size && style & QUOTE_0_TERMINATED)
		--size;

	alloc_size = 4 * size;
	if (alloc_size / 4 != size) {
		error_msg("Out of memory");
		tprints("???");
		return -1;
	}
	alloc_size += 1 + (style & QUOTE_OMIT_LEADING_TRAILING_QUOTES ? 0 : 2);

	if (use_alloca(alloc_size)) {
		outstr = alloca(alloc_size);
		buf = NULL;
	} else {
		outstr = buf = malloc(alloc_size);
		if (!buf) {
			error_msg("Out of memory");
			tprints("???");
			return -1;
		}
	}

	rc = string_quote(str, outstr, size, style);
	tprints(outstr);

	free(buf);
	return rc;
}

/*
 * Print path string specified by address `addr' and length `n'.
 * If path length exceeds `n', append `...' to the output.
 */
void
printpathn(struct tcb *tcp, long addr, unsigned int n)
{
	char path[PATH_MAX + 1];
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
		path[n++] = '\0';
		print_quoted_string(path, n, QUOTE_0_TERMINATED);
		if (!nul_seen)
			tprints("...");
	}
}

void
printpath(struct tcb *tcp, long addr)
{
	/* Size must correspond to char path[] size in printpathn */
	printpathn(tcp, addr, PATH_MAX);
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
	unsigned int size;
	unsigned int style;
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

	size = max_strlen;
	if (len == -1) {
		/*
		 * Treat as a NUL-terminated string: fetch one byte more
		 * because string_quote may look one byte ahead.
		 */
		if (umovestr(tcp, addr, size + 1, str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		style = QUOTE_0_TERMINATED;
	}
	else {
		if (size > (unsigned long)len)
			size = (unsigned long)len;
		if (umoven(tcp, addr, size, str) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		style = 0;
	}

	/* If string_quote didn't see NUL and (it was supposed to be ASCIZ str
	 * or we were requested to print more than -s NUM chars)...
	 */
	ellipsis = (string_quote(str, outstr, size, style) &&
			(len < 0 || (unsigned long) len > max_strlen));

	tprints(outstr);
	if (ellipsis)
		tprints("...");
}

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
	if (umoven(tcp, addr, size, iov) >= 0) {
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

void
dumpstr(struct tcb *tcp, long addr, int len)
{
	static int strsize = -1;
	static unsigned char *str;

	char outbuf[
		(
			(sizeof(
			"xx xx xx xx xx xx xx xx  xx xx xx xx xx xx xx xx  "
			"1234567890123456") + /*in case I'm off by few:*/ 4)
		/*align to 8 to make memset easier:*/ + 7) & -8
	];
	const unsigned char *src;
	int i;

	memset(outbuf, ' ', sizeof(outbuf));

	if (strsize < len + 16) {
		free(str);
		str = malloc(len + 16);
		if (!str) {
			strsize = -1;
			fprintf(stderr, "Out of memory\n");
			return;
		}
		strsize = len + 16;
	}

	if (umoven(tcp, addr, len, str) < 0)
		return;

	/* Space-pad to 16 bytes */
	i = len;
	while (i & 0xf)
		str[i++] = ' ';

	i = 0;
	src = str;
	while (i < len) {
		char *dst = outbuf;
		/* Hex dump */
		do {
			if (i < len) {
				*dst++ = "0123456789abcdef"[*src >> 4];
				*dst++ = "0123456789abcdef"[*src & 0xf];
			}
			else {
				*dst++ = ' ';
				*dst++ = ' ';
			}
			dst++; /* space is there by memset */
			i++;
			if ((i & 7) == 0)
				dst++; /* space is there by memset */
			src++;
		} while (i & 0xf);
		/* ASCII dump */
		i -= 16;
		src -= 16;
		do {
			if (*src >= ' ' && *src < 0x7f)
				*dst++ = *src;
			else
				*dst++ = '.';
			src++;
		} while (++i & 0xf);
		*dst = '\0';
		tprintf(" | %05x  %s |\n", i - 16, outbuf);
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
/* Have to avoid duplicating with the C library headers. */
static ssize_t strace_process_vm_readv(pid_t pid,
		 const struct iovec *lvec,
		 unsigned long liovcnt,
		 const struct iovec *rvec,
		 unsigned long riovcnt,
		 unsigned long flags)
{
	return syscall(__NR_process_vm_readv, (long)pid, lvec, liovcnt, rvec, riovcnt, flags);
}
#define process_vm_readv strace_process_vm_readv
#else
static bool process_vm_readv_not_supported = 1;
# define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

#endif /* end of hack */

static ssize_t
vm_read_mem(pid_t pid, void *laddr, long raddr, size_t len)
{
	const struct iovec local = {
		.iov_base = laddr,
		.iov_len = len
	};
	const struct iovec remote = {
		.iov_base = (void *) raddr,
		.iov_len = len
	};

	return process_vm_readv(pid, &local, 1, &remote, 1, 0);
}

/*
 * move `len' bytes of data from process `pid'
 * at address `addr' to our space at `our_addr'
 */
int
umoven(struct tcb *tcp, long addr, unsigned int len, void *our_addr)
{
	char *laddr = our_addr;
	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		long val;
		char x[sizeof(long)];
	} u;

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize < sizeof(addr))
		addr &= (1ul << 8 * current_wordsize) - 1;
#endif

	if (!process_vm_readv_not_supported) {
		int r = vm_read_mem(pid, laddr, addr, len);
		if ((unsigned int) r == len)
			return 0;
		if (r >= 0) {
			error_msg("umoven: short read (%u < %u) @0x%lx",
				  (unsigned int) r, len, addr);
			return -1;
		}
		switch (errno) {
			case ENOSYS:
				process_vm_readv_not_supported = 1;
				break;
			case EPERM:
				/* operation not permitted, try PTRACE_PEEKDATA */
				break;
			case ESRCH:
				/* the process is gone */
				return -1;
			case EFAULT: case EIO:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("process_vm_readv");
				return -1;
		}
	}

	nread = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr & (sizeof(long) - 1);	/* residue */
		addr &= -sizeof(long);		/* aligned address */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%lx",
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}
	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *) addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umoven: short read (%u < %u) @0x%lx",
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%lx",
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
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
umovestr(struct tcb *tcp, long addr, unsigned int len, char *laddr)
{
#if SIZEOF_LONG == 4
	const unsigned long x01010101 = 0x01010101ul;
	const unsigned long x80808080 = 0x80808080ul;
#elif SIZEOF_LONG == 8
	const unsigned long x01010101 = 0x0101010101010101ul;
	const unsigned long x80808080 = 0x8080808080808080ul;
#else
# error SIZEOF_LONG > 8
#endif

	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		unsigned long val;
		char x[sizeof(long)];
	} u;

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize < sizeof(addr))
		addr &= (1ul << 8 * current_wordsize) - 1;
#endif

	nread = 0;
	if (!process_vm_readv_not_supported) {
		const size_t page_size = get_pagesize();
		const size_t page_mask = page_size - 1;

		while (len > 0) {
			unsigned int chunk_len;
			unsigned int end_in_page;

			/*
			 * Don't cross pages, otherwise we can get EFAULT
			 * and fail to notice that terminating NUL lies
			 * in the existing (first) page.
			 */
			chunk_len = len > page_size ? page_size : len;
			end_in_page = (addr + chunk_len) & page_mask;
			if (chunk_len > end_in_page) /* crosses to the next page */
				chunk_len -= end_in_page;

			int r = vm_read_mem(pid, laddr, addr, chunk_len);
			if (r > 0) {
				if (memchr(laddr, '\0', r))
					return 1;
				addr += r;
				laddr += r;
				nread += r;
				len -= r;
				continue;
			}
			switch (errno) {
				case ENOSYS:
					process_vm_readv_not_supported = 1;
					goto vm_readv_didnt_work;
				case ESRCH:
					/* the process is gone */
					return -1;
				case EPERM:
					/* operation not permitted, try PTRACE_PEEKDATA */
					if (!nread)
						goto vm_readv_didnt_work;
					/* fall through */
				case EFAULT: case EIO:
					/* address space is inaccessible */
					if (nread) {
						perror_msg("umovestr: short read (%d < %d) @0x%lx",
							   nread, nread + len, addr - nread);
					}
					return -1;
				default:
					/* all the rest is strange and should be reported */
					perror_msg("process_vm_readv");
					return -1;
			}
		}
		return 0;
	}
 vm_readv_didnt_work:

	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr & (sizeof(long) - 1);	/* residue */
		addr &= -sizeof(long);		/* aligned address */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *)addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%lx",
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return 1;
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}

	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, (char *)addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umovestr: short read (%d < %d) @0x%lx",
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%lx",
					   pid, addr);
				return -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		/* "If a NUL char exists in this word" */
		if ((u.val - x01010101) & ~u.val & x80808080)
			return 1;
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}
	return 0;
}

int
upeek(int pid, long off, long *res)
{
	long val;

	errno = 0;
	val = ptrace(PTRACE_PEEKUSER, (pid_t)pid, (char *) off, 0);
	if (val == -1 && errno) {
		if (errno != ESRCH) {
			perror_msg("upeek: PTRACE_PEEKUSER pid:%d @0x%lx)", pid, off);
		}
		return -1;
	}
	*res = val;
	return 0;
}
