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
#include <stdarg.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include <sys/uio.h>
#include <asm/unistd.h>

#include "scno.h"
#include "regs.h"
#include "ptrace.h"

int
string_to_uint_ex(const char *const str, char **const endptr,
		  const unsigned int max_val, const char *const accepted_ending)
{
	char *end;
	long val;

	if (!*str)
		return -1;

	errno = 0;
	val = strtol(str, &end, 10);

	if (str == end || val < 0 || (unsigned long) val > max_val
	    || (val == LONG_MAX && errno == ERANGE))
		return -1;

	if (*end && (!accepted_ending || !strchr(accepted_ending, *end)))
		return -1;

	if (endptr)
		*endptr = end;

	return (int) val;
}

int
string_to_uint(const char *const str)
{
	return string_to_uint_upto(str, INT_MAX);
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
xlookup(const struct xlat *xlat, const uint64_t val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

static int
xlat_bsearch_compare(const void *a, const void *b)
{
	const uint64_t val1 = *(const uint64_t *) a;
	const uint64_t val2 = ((const struct xlat *) b)->val;
	return (val1 > val2) ? 1 : (val1 < val2) ? -1 : 0;
}

const char *
xlat_search(const struct xlat *xlat, const size_t nmemb, const uint64_t val)
{
	const struct xlat *e =
		bsearch((const void*) &val,
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
	int little_endian = * (char *) (void *) &endian;

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

/**
 * Print entry in struct xlat table, if there.
 *
 * @param val  Value to search a literal representation for.
 * @param dflt String (abbreviated in comment syntax) which should be emitted
 *             if no appropriate xlat value has been found.
 * @param xlat (And the following arguments) Pointers to arrays of xlat values.
 *             The last argument should be NULL.
 * @return     1 if appropriate xlat value has been found, 0 otherwise.
 */
int
printxvals(const uint64_t val, const char *dflt, const struct xlat *xlat, ...)
{
	va_list args;

	va_start(args, xlat);
	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		const char *str = xlookup(xlat, val);

		if (str) {
			tprints(str);
			va_end(args);
			return 1;
		}
	}
	/* No hits -- print raw # instead. */
	tprintf("%#" PRIx64, val);
	tprints_comment(dflt);

	va_end(args);

	return 0;
}

/**
 * Print entry in sorted struct xlat table, if it is there.
 *
 * @param xlat      Pointer to an array of xlat values (not terminated with
 *                  XLAT_END).
 * @param xlat_size Number of xlat elements present in array (usually ARRAY_SIZE
 *                  if array is declared in the unit's scope and not
 *                  terminated with XLAT_END).
 * @param val       Value to search literal representation for.
 * @param dflt      String (abbreviated in comment syntax) which should be
 *                  emitted if no appropriate xlat value has been found.
 * @return          1 if appropriate xlat value has been found, 0
 *                  otherwise.
 */
int
printxval_searchn(const struct xlat *xlat, size_t xlat_size, uint64_t val,
	const char *dflt)
{
	const char *s = xlat_search(xlat, xlat_size, val);

	if (s) {
		tprints(s);
		return 1;
	}

	tprintf("%#" PRIx64, val);
	tprints_comment(dflt);

	return 0;
}

/*
 * Fetch 64bit argument at position arg_no and
 * return the index of the next argument.
 */
int
getllval(struct tcb *tcp, unsigned long long *val, int arg_no)
{
#if SIZEOF_KERNEL_LONG_T > 4
# ifndef current_klongsize
	if (current_klongsize < SIZEOF_KERNEL_LONG_T) {
#  if defined(AARCH64) || defined(POWERPC64)
		/* Align arg_no to the next even number. */
		arg_no = (arg_no + 1) & 0xe;
#  endif /* AARCH64 || POWERPC64 */
		*val = ULONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
		arg_no += 2;
	} else
# endif /* !current_klongsize */
	{
		*val = tcp->u_arg[arg_no];
		arg_no++;
	}
#else /* SIZEOF_KERNEL_LONG_T == 4 */
# if defined __ARM_EABI__ || \
     defined LINUX_MIPSO32 || \
     defined POWERPC || \
     defined XTENSA
	/* Align arg_no to the next even number. */
	arg_no = (arg_no + 1) & 0xe;
# elif defined SH
	/*
	 * The SH4 ABI does allow long longs in odd-numbered registers, but
	 * does not allow them to be split between registers and memory - and
	 * there are only four argument registers for normal functions.  As a
	 * result, pread, for example, takes an extra padding argument before
	 * the offset.  This was changed late in the 2.4 series (around 2.4.20).
	 */
	if (arg_no == 3)
		arg_no++;
# endif /* __ARM_EABI__ || LINUX_MIPSO32 || POWERPC || XTENSA || SH */
	*val = ULONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
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
 */
void
addflags(const struct xlat *xlat, uint64_t flags)
{
	for (; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			tprintf("|%s", xlat->str);
			flags &= ~xlat->val;
		}
	}
	if (flags) {
		tprintf("|%#" PRIx64, flags);
	}
}

/*
 * Interpret `xlat' as an array of flags.
 * Print to static string the entries whose bits are on in `flags'
 * Return static string.
 */
const char *
sprintflags(const char *prefix, const struct xlat *xlat, uint64_t flags)
{
	static char outstr[1024];
	char *outptr;
	int found = 0;

	outptr = stpcpy(outstr, prefix);

	if (flags == 0 && xlat->val == 0 && xlat->str) {
		strcpy(outptr, xlat->str);
		return outstr;
	}

	for (; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
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
		outptr += sprintf(outptr, "%#" PRIx64, flags);
	}

	return outstr;
}

int
printflags64(const struct xlat *xlat, uint64_t flags, const char *dflt)
{
	int n;
	const char *sep;

	if (flags == 0 && xlat->val == 0 && xlat->str) {
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
			tprintf("%s%#" PRIx64, sep, flags);
			n++;
		}
	} else {
		if (flags) {
			tprintf("%#" PRIx64, flags);
			tprints_comment(dflt);
		} else {
			if (dflt)
				tprints("0");
		}
	}

	return n;
}

void
printaddr(const kernel_ulong_t addr)
{
	if (!addr)
		tprints("NULL");
	else
		tprintf("%#" PRI_klx, addr);
}

#define DEF_PRINTNUM(name, type) \
bool									\
printnum_ ## name(struct tcb *const tcp, const kernel_ulong_t addr,	\
		  const char *const fmt)				\
{									\
	type num;							\
	if (umove_or_printaddr(tcp, addr, &num))			\
		return false;						\
	tprints("[");							\
	tprintf(fmt, num);						\
	tprints("]");							\
	return true;							\
}

#define DEF_PRINTNUM_ADDR(name, type) \
bool									\
printnum_addr_ ## name(struct tcb *tcp, const kernel_ulong_t addr)	\
{									\
	type num;							\
	if (umove_or_printaddr(tcp, addr, &num))			\
		return false;						\
	tprints("[");							\
	printaddr(num);							\
	tprints("]");							\
	return true;							\
}

#define DEF_PRINTPAIR(name, type) \
bool									\
printpair_ ## name(struct tcb *const tcp, const kernel_ulong_t addr,	\
		   const char *const fmt)				\
{									\
	type pair[2];							\
	if (umove_or_printaddr(tcp, addr, &pair))			\
		return false;						\
	tprints("[");							\
	tprintf(fmt, pair[0]);						\
	tprints(", ");							\
	tprintf(fmt, pair[1]);						\
	tprints("]");							\
	return true;							\
}

DEF_PRINTNUM(int, int)
DEF_PRINTNUM_ADDR(int, unsigned int)
DEF_PRINTPAIR(int, int)
DEF_PRINTNUM(short, short)
DEF_PRINTNUM(int64, uint64_t)
DEF_PRINTNUM_ADDR(int64, uint64_t)
DEF_PRINTPAIR(int64, uint64_t)

#ifndef current_wordsize
bool
printnum_long_int(struct tcb *const tcp, const kernel_ulong_t addr,
		  const char *const fmt_long, const char *const fmt_int)
{
	if (current_wordsize > sizeof(int)) {
		return printnum_int64(tcp, addr, fmt_long);
	} else {
		return printnum_int(tcp, addr, fmt_int);
	}
}

bool
printnum_addr_long_int(struct tcb *tcp, const kernel_ulong_t addr)
{
	if (current_wordsize > sizeof(int)) {
		return printnum_addr_int64(tcp, addr);
	} else {
		return printnum_addr_int(tcp, addr);
	}
}
#endif /* !current_wordsize */

#ifndef current_klongsize
bool
printnum_addr_klong_int(struct tcb *tcp, const kernel_ulong_t addr)
{
	if (current_klongsize > sizeof(int)) {
		return printnum_addr_int64(tcp, addr);
	} else {
		return printnum_addr_int(tcp, addr);
	}
}
#endif /* !current_klongsize */

/**
 * Prints time to a (static internal) buffer and returns pointer to it.
 *
 * @param sec		Seconds since epoch.
 * @param part_sec	Amount of second parts since the start of a second.
 * @param max_part_sec	Maximum value of a valid part_sec.
 * @param width		1 + floor(log10(max_part_sec)).
 */
static const char *
sprinttime_ex(const long long sec, const unsigned long long part_sec,
	      const unsigned int max_part_sec, const int width)
{
	static char buf[sizeof(int) * 3 * 6 + sizeof(part_sec) * 3
			+ sizeof("+0000")];

	if ((sec == 0 && part_sec == 0) || part_sec > max_part_sec)
		return NULL;

	time_t t = (time_t) sec;
	struct tm *tmp = (sec == t) ? localtime(&t) : NULL;
	if (!tmp)
		return NULL;

	size_t pos = strftime(buf, sizeof(buf), "%FT%T", tmp);
	if (!pos)
		return NULL;

	if (part_sec > 0) {
		int ret = snprintf(buf + pos, sizeof(buf) - pos, ".%0*llu",
				   width, part_sec);

		if (ret < 0 || (size_t) ret >= sizeof(buf) - pos)
			return NULL;

		pos += ret;
	}

	return strftime(buf + pos, sizeof(buf) - pos, "%z", tmp) ? buf : NULL;
}

const char *
sprinttime(long long sec)
{
	return sprinttime_ex(sec, 0, 0, 0);
}

const char *
sprinttime_usec(long long sec, unsigned long long usec)
{
	return sprinttime_ex(sec, usec, 999999, 6);
}

const char *
sprinttime_nsec(long long sec, unsigned long long nsec)
{
	return sprinttime_ex(sec, nsec, 999999999, 9);
}

enum sock_proto
getfdproto(struct tcb *tcp, int fd)
{
#ifdef HAVE_SYS_XATTR_H
	size_t bufsize = 256;
	char buf[bufsize];
	ssize_t r;
	char path[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];

	if (fd < 0)
		return SOCK_PROTO_UNKNOWN;

	sprintf(path, "/proc/%u/fd/%u", tcp->pid, fd);
	r = getxattr(path, "system.sockprotoname", buf, bufsize - 1);
	if (r <= 0)
		return SOCK_PROTO_UNKNOWN;
	else {
		/*
		 * This is a protection for the case when the kernel
		 * side does not append a null byte to the buffer.
		 */
		buf[r] = '\0';

		return get_proto_by_name(buf);
	}
#else
	return SOCK_PROTO_UNKNOWN;
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
			unsigned long inode =
				strtoul(path + socket_prefix_len, NULL, 10);

			if (!print_sockaddr_by_inode_cached(inode)) {
				const enum sock_proto proto =
					getfdproto(tcp, fd);
				if (!print_sockaddr_by_inode(inode, proto))
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
int
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
	if ((xflag > 1) || (style & QUOTE_FORCE_HEX)) {
		usehex = 1;
	} else if (xflag) {
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
			if ((i == (size - 1)) &&
			    (style & QUOTE_OMIT_TRAILING_0) && (c == '\0'))
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
printpathn(struct tcb *const tcp, const kernel_ulong_t addr, unsigned int n)
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
		printaddr(addr);
	else {
		path[n++] = '\0';
		print_quoted_string(path, n, QUOTE_0_TERMINATED);
		if (!nul_seen)
			tprints("...");
	}
}

void
printpath(struct tcb *const tcp, const kernel_ulong_t addr)
{
	/* Size must correspond to char path[] size in printpathn */
	printpathn(tcp, addr, PATH_MAX);
}

/*
 * Print string specified by address `addr' and length `len'.
 * If `user_style' has QUOTE_0_TERMINATED bit set, treat the string
 * as a NUL-terminated string.
 * Pass `user_style' on to `string_quote'.
 * Append `...' to the output if either the string length exceeds `max_strlen',
 * or QUOTE_0_TERMINATED bit is set and the string length exceeds `len'.
 */
void
printstr_ex(struct tcb *const tcp, const kernel_ulong_t addr,
	    const kernel_ulong_t len, const unsigned int user_style)
{
	static char *str = NULL;
	static char *outstr;
	unsigned int size;
	unsigned int style = user_style;
	int rc;
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
		str = xmalloc(max_strlen + 1);
		outstr = xmalloc(outstr_size);
	}

	/* Fetch one byte more because string_quote may look one byte ahead. */
	size = max_strlen + 1;

	if (size > len)
		size = len;
	if (style & QUOTE_0_TERMINATED)
		rc = umovestr(tcp, addr, size, str);
	else
		rc = umoven(tcp, addr, size, str);

	if (rc < 0) {
		printaddr(addr);
		return;
	}

	if (size > max_strlen)
		size = max_strlen;
	else
		str[size] = '\xff';

	/* If string_quote didn't see NUL and (it was supposed to be ASCIZ str
	 * or we were requested to print more than -s NUM chars)...
	 */
	ellipsis = string_quote(str, outstr, size, style)
		   && len
		   && ((style & QUOTE_0_TERMINATED)
		       || len > max_strlen);

	tprints(outstr);
	if (ellipsis)
		tprints("...");
}

void
dumpiov_upto(struct tcb *const tcp, const int len, const kernel_ulong_t addr,
	     kernel_ulong_t data_size)
{
#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	union {
		struct { uint32_t base; uint32_t len; } *iov32;
		struct { uint64_t base; uint64_t len; } *iov64;
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
#define iov_iov_base(i) ptr_to_kulong(iov[i].iov_base)
#define iov_iov_len(i) iov[i].iov_len
#endif
	int i;
	unsigned size;

	size = sizeof_iov * len;
	/* Assuming no sane program has millions of iovs */
	if ((unsigned)len > 1024*1024 /* insane or negative size? */
	    || (iov = malloc(size)) == NULL) {
		error_msg("Out of memory");
		return;
	}
	if (umoven(tcp, addr, size, iov) >= 0) {
		for (i = 0; i < len; i++) {
			kernel_ulong_t iov_len = iov_iov_len(i);
			if (iov_len > data_size)
				iov_len = data_size;
			if (!iov_len)
				break;
			data_size -= iov_len;
			/* include the buffer number to make it easy to
			 * match up the trace with the source */
			tprintf(" * %" PRI_klu " bytes in buffer %d\n", iov_len, i);
			dumpstr(tcp, iov_iov_base(i), iov_len);
		}
	}
	free(iov);
#undef sizeof_iov
#undef iov_iov_base
#undef iov_iov_len
#undef iov
}

void
dumpstr(struct tcb *const tcp, const kernel_ulong_t addr, const int len)
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
			error_msg("Out of memory");
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

static bool process_vm_readv_not_supported = 0;
#ifndef HAVE_PROCESS_VM_READV
/*
 * Need to do this since process_vm_readv() is not yet available in libc.
 * When libc is be updated, only "static bool process_vm_readv_not_supported"
 * line should remain.
 */
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
# define process_vm_readv strace_process_vm_readv
#endif /* !HAVE_PROCESS_VM_READV */

static ssize_t
vm_read_mem(const pid_t pid, void *const laddr,
	    const kernel_ulong_t raddr, const size_t len)
{
	const unsigned long truncated_raddr = raddr;

	if (raddr != (kernel_ulong_t) truncated_raddr) {
		errno = EIO;
		return -1;
	}

	const struct iovec local = {
		.iov_base = laddr,
		.iov_len = len
	};
	const struct iovec remote = {
		.iov_base = (void *) truncated_raddr,
		.iov_len = len
	};

	return process_vm_readv(pid, &local, 1, &remote, 1, 0);
}

/*
 * move `len' bytes of data from process `pid'
 * at address `addr' to our space at `our_addr'
 */
int
umoven(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len,
       void *const our_addr)
{
	char *laddr = our_addr;
	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		long val;
		char x[sizeof(long)];
	} u;

#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	if (current_wordsize < sizeof(addr)
	    && (addr & (~ (kernel_ulong_t) -1U))) {
		return -1;
	}
#endif

	if (!process_vm_readv_not_supported) {
		int r = vm_read_mem(pid, laddr, addr, len);
		if ((unsigned int) r == len)
			return 0;
		if (r >= 0) {
			error_msg("umoven: short read (%u < %u) @0x%" PRI_klx,
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
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
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
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
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
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umoven: short read (%u < %u) @0x%" PRI_klx,
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
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

int
umoven_or_printaddr(struct tcb *const tcp, const kernel_ulong_t addr,
		    const unsigned int len, void *const our_addr)
{
	if (!addr || !verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    umoven(tcp, addr, len, our_addr) < 0) {
		printaddr(addr);
		return -1;
	}
	return 0;
}

int
umoven_or_printaddr_ignore_syserror(struct tcb *const tcp,
				    const kernel_ulong_t addr,
				    const unsigned int len,
				    void *const our_addr)
{
	if (!addr || !verbose(tcp) || umoven(tcp, addr, len, our_addr) < 0) {
		printaddr(addr);
		return -1;
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
umovestr(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len, char *laddr)
{
	const unsigned long x01010101 = (unsigned long) 0x0101010101010101ULL;
	const unsigned long x80808080 = (unsigned long) 0x8080808080808080ULL;

	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		unsigned long val;
		char x[sizeof(long)];
	} u;

#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	if (current_wordsize < sizeof(addr)
	    && (addr & (~ (kernel_ulong_t) -1U))) {
		return -1;
	}
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
						perror_msg("umovestr: short read (%d < %d) @0x%" PRI_klx,
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
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
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
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
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
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umovestr: short read (%d < %d) @0x%" PRI_klx,
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
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

/*
 * Iteratively fetch and print up to nmemb elements of elem_size size
 * from the array that starts at tracee's address start_addr.
 *
 * Array elements are being fetched to the address specified by elem_buf.
 *
 * The fetcher callback function specified by umoven_func should follow
 * the same semantics as umoven_or_printaddr function.
 *
 * The printer callback function specified by print_func is expected
 * to print something; if it returns false, no more iterations will be made.
 *
 * The pointer specified by opaque_data is passed to each invocation
 * of print_func callback function.
 *
 * This function prints:
 * - "NULL", if start_addr is NULL;
 * - "[]", if nmemb is 0;
 * - start_addr, if nmemb * elem_size overflows or wraps around;
 * - nothing, if the first element cannot be fetched
 *   (if umoven_func returns non-zero), but it is assumed that
 *   umoven_func has printed the address it failed to fetch data from;
 * - elements of the array, delimited by ", ", with the array itself
 *   enclosed with [] brackets.
 *
 * If abbrev(tcp) is true, then
 * - the maximum number of elements printed equals to max_strlen;
 * - "..." is printed instead of max_strlen+1 element
 *   and no more iterations will be made.
 *
 * This function returns true only if
 * - umoven_func has been called at least once AND
 * - umoven_func has not returned false.
 */
bool
print_array(struct tcb *const tcp,
	    const kernel_ulong_t start_addr,
	    const size_t nmemb,
	    void *const elem_buf,
	    const size_t elem_size,
	    int (*const umoven_func)(struct tcb *,
				     kernel_ulong_t,
				     unsigned int,
				     void *),
	    bool (*const print_func)(struct tcb *,
				     void *elem_buf,
				     size_t elem_size,
				     void *opaque_data),
	    void *const opaque_data)
{
	if (!start_addr) {
		tprints("NULL");
		return false;
	}

	if (!nmemb) {
		tprints("[]");
		return false;
	}

	const size_t size = nmemb * elem_size;
	const kernel_ulong_t end_addr = start_addr + size;

	if (end_addr <= start_addr || size / elem_size != nmemb) {
		printaddr(start_addr);
		return false;
	}

	const kernel_ulong_t abbrev_end =
		(abbrev(tcp) && max_strlen < nmemb) ?
			start_addr + elem_size * max_strlen : end_addr;
	kernel_ulong_t cur;

	for (cur = start_addr; cur < end_addr; cur += elem_size) {
		if (cur != start_addr)
			tprints(", ");

		if (umoven_func(tcp, cur, elem_size, elem_buf))
			break;

		if (cur == start_addr)
			tprints("[");

		if (cur >= abbrev_end) {
			tprints("...");
			cur = end_addr;
			break;
		}

		if (!print_func(tcp, elem_buf, elem_size, opaque_data)) {
			cur = end_addr;
			break;
		}
	}
	if (cur != start_addr)
		tprints("]");

	return cur >= end_addr;
}

int
printargs(struct tcb *tcp)
{
	const int n = tcp->s_ent->nargs;
	int i;
	for (i = 0; i < n; ++i)
		tprintf("%s%#" PRI_klx, i ? ", " : "", tcp->u_arg[i]);
	return RVAL_DECODED;
}

int
printargs_u(struct tcb *tcp)
{
	const int n = tcp->s_ent->nargs;
	int i;
	for (i = 0; i < n; ++i)
		tprintf("%s%u", i ? ", " : "",
			(unsigned int) tcp->u_arg[i]);
	return RVAL_DECODED;
}

int
printargs_d(struct tcb *tcp)
{
	const int n = tcp->s_ent->nargs;
	int i;
	for (i = 0; i < n; ++i)
		tprintf("%s%d", i ? ", " : "",
			(int) tcp->u_arg[i]);
	return RVAL_DECODED;
}

/* Print abnormal high bits of a kernel_ulong_t value. */
void
print_abnormal_hi(const kernel_ulong_t val)
{
	if (current_klongsize > 4) {
		const unsigned int hi = (unsigned int) ((uint64_t) val >> 32);
		if (hi)
			tprintf("%#x<<32|", hi);
	}
}

#if defined _LARGEFILE64_SOURCE && defined HAVE_OPEN64
# define open_file open64
#else
# define open_file open
#endif

int
read_int_from_file(const char *const fname, int *const pvalue)
{
	const int fd = open_file(fname, O_RDONLY);
	if (fd < 0)
		return -1;

	long lval;
	char buf[sizeof(lval) * 3];
	int n = read(fd, buf, sizeof(buf) - 1);
	int saved_errno = errno;
	close(fd);

	if (n < 0) {
		errno = saved_errno;
		return -1;
	}

	buf[n] = '\0';
	char *endptr = 0;
	errno = 0;
	lval = strtol(buf, &endptr, 10);
	if (!endptr || (*endptr && '\n' != *endptr)
#if INT_MAX < LONG_MAX
	    || lval > INT_MAX || lval < INT_MIN
#endif
	    || ERANGE == errno) {
		if (!errno)
			errno = EINVAL;
		return -1;
	}

	*pvalue = (int) lval;
	return 0;
}
