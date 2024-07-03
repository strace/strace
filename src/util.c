/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 1999-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <limits.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include <sys/uio.h>

#include "largefile_wrappers.h"
#include "number_set.h"
#include "print_fields.h"
#include "print_utils.h"
#include "secontext.h"
#include "static_assert.h"
#include "string_to_uint.h"
#include "xlat.h"
#include "xstring.h"

const struct xlat_data *
find_xlat_val_ex(const struct xlat_data * const items, const char * const s,
		 const size_t num_items, const unsigned int flags)
{
	for (size_t i = 0; i < num_items; i++) {
		if (!(flags & FXL_CASE_SENSITIVE ? strcmp
						 : strcasecmp)(items[i].str, s))
			return items + i;
	}

	return NULL;
}

uint64_t
find_arg_val_(const char *arg, const struct xlat_data *strs, size_t strs_size,
	       uint64_t default_val, uint64_t not_found)
{
	if (!arg)
		return default_val;

	const struct xlat_data *res = find_xlat_val_ex(strs, arg, strs_size, 0);

	return  res ? res->val : not_found;
}

int
str2timescale_ex(const char *arg, int empty_dflt, int null_dflt,
		 int *width)
{
	static const struct xlat_data units[] = {
		{ 1000000000U | (0ULL << 32), "s" },
		{ 1000000U    | (3ULL << 32), "ms" },
		{ 1000U       | (6ULL << 32), "us" },
		{ 1U          | (9ULL << 32), "ns" },
	};

	if (!arg)
		return null_dflt;
	if (!arg[0])
		return empty_dflt;

	uint64_t res = find_arg_val(arg, units, null_dflt, -1ULL);

	if (width && res != -1ULL)
		*width = res >> 32;

	return res & 0xffffffff;
}

int
ts_nz(const struct timespec *a)
{
	return a->tv_sec || a->tv_nsec;
}

int
ts_cmp(const struct timespec *a, const struct timespec *b)
{
	if (a->tv_sec < b->tv_sec
	    || (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec))
		return -1;
	if (a->tv_sec > b->tv_sec
	    || (a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec))
		return 1;
	return 0;
}

double
ts_float(const struct timespec *tv)
{
	return tv->tv_sec + tv->tv_nsec/1000000000.0;
}

void
ts_add(struct timespec *tv, const struct timespec *a, const struct timespec *b)
{
	tv->tv_sec = a->tv_sec + b->tv_sec;
	tv->tv_nsec = a->tv_nsec + b->tv_nsec;
	if (tv->tv_nsec >= 1000000000) {
		tv->tv_sec++;
		tv->tv_nsec -= 1000000000;
	}
}

void
ts_sub(struct timespec *tv, const struct timespec *a, const struct timespec *b)
{
	tv->tv_sec = a->tv_sec - b->tv_sec;
	tv->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (tv->tv_nsec < 0) {
		tv->tv_sec--;
		tv->tv_nsec += 1000000000;
	}
}

void
ts_div(struct timespec *tv, const struct timespec *a, uint64_t n)
{
	long long nsec = (a->tv_sec % n * 1000000000LL + a->tv_nsec + n / 2) / n;
	tv->tv_sec = a->tv_sec / n + nsec / 1000000000;
	tv->tv_nsec = nsec % 1000000000;
}

const struct timespec *
ts_min(const struct timespec *a, const struct timespec *b)
{
	return ts_cmp(a, b) < 0 ? a : b;
}

const struct timespec *
ts_max(const struct timespec *a, const struct timespec *b)
{
	return ts_cmp(a, b) > 0 ? a : b;
}

int
parse_ts(const char *s, struct timespec *t)
{
	enum { NS_IN_S = 1000000000 };

	static const char float_accept[] =  "eE.-+0123456789";
	static const char int_accept[] = "+0123456789";

	size_t float_len = strspn(s, float_accept);
	size_t int_len = strspn(s, int_accept);
	char *endptr = NULL;
	double float_val = -1;
	long long int_val = -1;

	if (float_len > int_len) {
		errno = 0;

		float_val = strtod(s, &endptr);

		if (endptr == s || errno)
			return -1;
		if (float_val < 0)
			return -1;
	} else {
		int_val = string_to_uint_ex(s, &endptr, LLONG_MAX, "smun");

		if (int_val < 0)
			return -1;
	}

	int scale = str2timescale_sfx(endptr, NULL);
	if (scale <= 0)
		return -1;

	if (float_len > int_len) {
		t->tv_sec = float_val / (NS_IN_S / scale);
		t->tv_nsec = ((uint64_t) ((float_val -
					   (t->tv_sec * (NS_IN_S / scale)))
					  * scale)) % NS_IN_S;
	} else {
		t->tv_sec = int_val / (NS_IN_S / scale);
		t->tv_nsec = (int_val % (NS_IN_S / scale)) * scale;
	}

	return 0;
}

#define ILOG10_ITER_(val_, div_, ret_, pow_)	\
	do {					\
		if ((val_) >= (div_)) {		\
			(val_) /= (div_);	\
			(ret_) += (pow_);	\
		}				\
	} while (0)				\
	/* End of ILOG10_ITER_ */

/* Returns 0 for 0. */
static int
ilog10(uint64_t val)
{
	int ret = 0;

	ILOG10_ITER_(val, 10000000000000000ULL, ret, 16);
	ILOG10_ITER_(val, 100000000,            ret, 8);
	ILOG10_ITER_(val, 10000,                ret, 4);
	ILOG10_ITER_(val, 100,                  ret, 2);
	ILOG10_ITER_(val, 10,                   ret, 1);

	return ret;
}

void
print_ticks(uint64_t val, long freq, unsigned int precision)
{
	PRINT_VAL_U(val);
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW
	    && freq > 0 && val > 0) {
		tprintf_comment("%" PRIu64 ".%0*" PRIu64 " s",
				val / freq, precision, val % freq);
	}
}

void
print_ticks_d(int64_t val, long freq, unsigned int precision)
{
	PRINT_VAL_D(val);
	/* freq > 1 to avoid special casing for val ==  */
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW
	    && freq > 1 && val != 0) {
		tprintf_comment("%s%lld.%0*lld s",
				val < 0 ? "-" : "", llabs(val / freq),
				precision, llabs(val % freq));
	}
}

void
print_clock_t(uint64_t val)
{
	static long clk_tck;
	static int frac_width;

	if (!clk_tck) {
		errno = 0;
		clk_tck = sysconf(_SC_CLK_TCK);
		if (clk_tck == -1 && errno)
			debug_func_perror_msg("sysconf(_SC_CLK_TCK)");
		if (clk_tck == 0)
			clk_tck = -1;
		if (clk_tck > 0)
			frac_width = MIN(ilog10(clk_tck), 9);
	}

	print_ticks(val, clk_tck, frac_width);
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
	int little_endian = *(char *) (void *) &endian;

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
 * Fetch 64bit argument at position arg_no and
 * return the index of the next argument.
 */
unsigned int
getllval(struct tcb *tcp, unsigned long long *val, unsigned int arg_no)
{
#if SIZEOF_KERNEL_LONG_T > 4
# ifndef current_klongsize
	if (current_klongsize < SIZEOF_KERNEL_LONG_T) {
#  if defined(AARCH64) || defined(POWERPC64) || defined(POWERPC64LE)
		/* Align arg_no to the next even number. */
		arg_no = (arg_no + 1) & 0xe;
#  endif /* AARCH64 || POWERPC64 || POWERPC64LE */
		*val = ULONG_LONG(tcp->u_arg[arg_no], tcp->u_arg[arg_no + 1]);
		arg_no += 2;
	} else
# endif /* !current_klongsize */
	{
		*val = tcp->u_arg[arg_no];
		arg_no++;
	}
#else /* SIZEOF_KERNEL_LONG_T == 4 */
# if defined __ARM_EABI__	\
  || defined LINUX_MIPSO32	\
  || defined POWERPC		\
  || defined XTENSA
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
 * Print 64bit argument at position arg_no as a signed long long
 * and return the index of the next argument.
 */
unsigned int
print_arg_lld(struct tcb *tcp, unsigned int arg_no)
{
	unsigned long long val = 0;

	arg_no = getllval(tcp, &val, arg_no);
	PRINT_VAL_D(val);
	return arg_no;
}

/*
 * Print 64bit argument at position arg_no as an unsigned long long
 * and return the index of the next argument.
 */
unsigned int
print_arg_llu(struct tcb *tcp, unsigned int arg_no)
{
	unsigned long long val = 0;

	arg_no = getllval(tcp, &val, arg_no);
	PRINT_VAL_U(val);
	return arg_no;
}

void
printaddr64(const uint64_t addr)
{
	if (!addr)
		tprint_null();
	else
		PRINT_VAL_X(addr);
}

#define DEF_PRINTNUM(name, type) \
bool									\
printnum_ ## name(struct tcb *const tcp, const kernel_ulong_t addr,	\
		  const char *const fmt)				\
{									\
	type num;							\
	if (umove_or_printaddr(tcp, addr, &num))			\
		return false;						\
	tprint_indirect_begin();					\
	tprintf_string(fmt, num);					\
	tprint_indirect_end();						\
	return true;							\
}

#define DEF_PRINTNUM_ADDR(name, type) \
bool									\
printnum_addr_ ## name(struct tcb *tcp, const kernel_ulong_t addr)	\
{									\
	type num;							\
	if (umove_or_printaddr(tcp, addr, &num))			\
		return false;						\
	tprint_indirect_begin();					\
	printaddr64(num);						\
	tprint_indirect_end();						\
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
	tprint_array_begin();						\
	tprintf_string(fmt, pair[0]);					\
	tprint_array_next();						\
	tprintf_string(fmt, pair[1]);					\
	tprint_array_end();						\
	return true;							\
}

DEF_PRINTNUM(int, int)
DEF_PRINTNUM_ADDR(int, unsigned int)
DEF_PRINTPAIR(int, int)
DEF_PRINTNUM(short, short)
DEF_PRINTNUM(int64, uint64_t)
DEF_PRINTNUM_ADDR(int64, uint64_t)
DEF_PRINTPAIR(int64, uint64_t)

bool
printnum_fd(struct tcb *const tcp, const kernel_ulong_t addr)
{
	int fd;
	if (umove_or_printaddr(tcp, addr, &fd))
		return false;
	tprint_indirect_begin();
	printfd(tcp, fd);
	tprint_indirect_end();
	return true;
}

bool
printnum_pid(struct tcb *const tcp, const kernel_ulong_t addr, enum pid_type type)
{
	int pid;
	if (umove_or_printaddr(tcp, addr, &pid))
		return false;
	tprint_indirect_begin();
	printpid(tcp, pid, type);
	tprint_indirect_end();
	return true;
}

/**
 * Prints time to a (static internal) buffer and returns pointer to it.
 * Returns NULL if the provided time specification is not correct.
 *
 * @param sec		Seconds since epoch.
 * @param part_sec	Amount of second parts since the start of a second.
 * @param max_part_sec	Maximum value of a valid part_sec.
 * @param width		1 + floor(log10(max_part_sec)).
 * @return		Pointer to a statically allocated string on success,
 *			NULL on error.
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

	if (part_sec > 0)
		pos += xsnprintf(buf + pos, sizeof(buf) - pos, ".%0*llu",
				 width, part_sec);

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

void
print_uuid(const unsigned char *uuid)
{
	const char str[] = {
		BYTE_HEX_CHARS(uuid[0]),
		BYTE_HEX_CHARS(uuid[1]),
		BYTE_HEX_CHARS(uuid[2]),
		BYTE_HEX_CHARS(uuid[3]),
		'-',
		BYTE_HEX_CHARS(uuid[4]),
		BYTE_HEX_CHARS(uuid[5]),
		'-',
		BYTE_HEX_CHARS(uuid[6]),
		BYTE_HEX_CHARS(uuid[7]),
		'-',
		BYTE_HEX_CHARS(uuid[8]),
		BYTE_HEX_CHARS(uuid[9]),
		'-',
		BYTE_HEX_CHARS(uuid[10]),
		BYTE_HEX_CHARS(uuid[11]),
		BYTE_HEX_CHARS(uuid[12]),
		BYTE_HEX_CHARS(uuid[13]),
		BYTE_HEX_CHARS(uuid[14]),
		BYTE_HEX_CHARS(uuid[15]),
		'\0'
	};

	tprints_string(str);
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

	xsprintf(path, "/proc/%u/fd/%u", get_proc_pid(tcp->pid), fd);
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

static unsigned long
get_inode_of_socket_path(const char *path)
{
	const char *str = STR_STRIP_PREFIX(path, "socket:[");
	char *end;
	size_t len;
	unsigned long r;

	if ((str != path)
	    && (len = strlen(str))
	    && (str[len - 1] == ']')
	    && (r = strtoul(str, &end, 10))
	    && (end == &str[len - 1]))
		return r;

	return 0;
}

unsigned long
getfdinode(struct tcb *tcp, int fd)
{
	char path[PATH_MAX + 1];

	if (getfdpath(tcp, fd, path, sizeof(path)) >= 0)
		return get_inode_of_socket_path(path);

	return 0;
}

static void
print_string_in_angle_brackets(const char *str)
{
	tprint_associated_info_begin();
	tprints_string(str);
	tprint_associated_info_end();
}

static bool
printsocket(struct tcb *tcp, int fd, const char *path)
{
	unsigned long inode = get_inode_of_socket_path(path);
	if (!inode)
		return false;

	const char *details = get_sockaddr_by_inode(tcp, fd, inode);
	print_string_in_angle_brackets(details ?: path);

	return true;
}

typedef bool (*scan_fdinfo_fn)(const char *value, void *data);

struct scan_fdinfo {
	const char *search_pfx;
	size_t search_pfx_len;
	scan_fdinfo_fn fn;
	void *data;
	size_t matches;
};

static size_t
scan_fdinfo_lines(pid_t pid_of_fd, int fd, struct scan_fdinfo *fdinfo_search_array,
		  size_t fdinfo_array_size)
{
	int proc_pid = 0;
	translate_pid(NULL, pid_of_fd, PT_TID, &proc_pid);
	if (!proc_pid)
		return false;

	char fdi_path[sizeof("/proc/%u/fdinfo/%u") + 2 * sizeof(int) * 3];
	xsprintf(fdi_path, "/proc/%u/fdinfo/%u", proc_pid, fd);

	FILE *f = fopen_stream(fdi_path, "r");
	if (!f)
		return false;

	char *line = NULL;
	size_t sz = 0;
	size_t matches = 0;

	while (matches < fdinfo_array_size && getline(&line, &sz, f) > 0) {
		for (size_t i = 0; i < fdinfo_array_size ; ++i) {
			if (fdinfo_search_array[i].matches)
				continue;

			scan_fdinfo_fn fn = fdinfo_search_array[i].fn;
			void *data = fdinfo_search_array[i].data;
			const char *value =
				str_strip_prefix_len(line, fdinfo_search_array[i].search_pfx,
				                     fdinfo_search_array[i].search_pfx_len);
			if (value != line && fn(value, data)) {
				++fdinfo_search_array[i].matches;
				++matches;
				break;
			}
		}
	}

	free(line);
	fclose(f);

	return matches;
}

static inline bool
scan_fdinfo(pid_t pid_of_fd, int fd, const char *search_pfx,
	    size_t search_pfx_len, scan_fdinfo_fn fn, void *data)
{
	struct scan_fdinfo fdinfo_lines[] = {
		{
			.search_pfx = search_pfx,
			.search_pfx_len = search_pfx_len,
			.fn = fn,
			.data = data
		}
	};

	return scan_fdinfo_lines(pid_of_fd, fd, fdinfo_lines,
				 ARRAY_SIZE(fdinfo_lines)) > 0;
}

static bool
is_ptmx(const struct finfo *finfo)
{
	return finfo->type == FINFO_DEV_CHR
		&& finfo->dev.major == 5
		&& finfo->dev.minor == 2;
}

static bool
set_tty_index(const char *value, void *data)
{
	struct finfo *finfo = data;

	finfo->dev.tty_index = string_to_uint_ex(value, NULL, INT_MAX, "\n");
	return true;
}

struct finfo *
get_finfo_for_dev(pid_t pid, int fd, const char *path, struct finfo *finfo)
{
	strace_stat_t st;

	finfo->path = path;
	finfo->type = FINFO_UNSET;
	finfo->deleted = false;

	if (path[0] != '/')
		return finfo;

	if (stat_file(path, &st)) {
		debug_func_perror_msg("stat(\"%s\")", path);
		return finfo;
	}

	switch (st.st_mode & S_IFMT) {
	case S_IFBLK:
		finfo->type = FINFO_DEV_BLK;
		break;
	case S_IFCHR:
		finfo->type = FINFO_DEV_CHR;
		break;
	default:
		return finfo;
	}

	finfo->dev.major = major(st.st_rdev);
	finfo->dev.minor = minor(st.st_rdev);

	if (is_ptmx(finfo)) {
		static const char prefix[] = "tty-index:\t";

		finfo->dev.tty_index = -1;
		scan_fdinfo(pid, fd, prefix, sizeof(prefix) - 1,
			    set_tty_index, finfo);
	}

	return finfo;
}

static bool
printdev(struct tcb *tcp, int fd, const char *path, const struct finfo *finfo)
{
	struct finfo finfo_buf;
	if (!finfo)
		finfo = get_finfo_for_dev(tcp->pid, fd, path, &finfo_buf);

	switch (finfo->type) {
	case FINFO_DEV_BLK:
	case FINFO_DEV_CHR:
		tprint_associated_info_begin();
		print_quoted_string_ex(finfo->path, strlen(finfo->path),
				       QUOTE_OMIT_LEADING_TRAILING_QUOTES,
				       "<>");
		tprint_associated_info_begin();
		tprintf_string("%s %u:%u",
			       (finfo->type == FINFO_DEV_BLK)? "block" : "char",
			       finfo->dev.major, finfo->dev.minor);
		if (is_ptmx(finfo) && finfo->dev.tty_index >= 0)
			tprintf_string(" @/dev/pts/%d", finfo->dev.tty_index);
		tprint_associated_info_end();
		tprint_associated_info_end();
		return true;
	default:
		break;
	}

	return false;
}

static bool
parse_fdinfo_pid(const char *value, void *data)
{
	pid_t *pid = data;
	*pid = string_to_uint_ex(value, NULL, INT_MAX, "\n");
	return true;
}

pid_t
pidfd_get_pid(pid_t pid_of_fd, int fd)
{
	static const char pid_pfx[] = "Pid:\t";
	pid_t pid = -1;

	scan_fdinfo(pid_of_fd, fd, pid_pfx, sizeof(pid_pfx) - 1,
		    parse_fdinfo_pid, &pid);
	return pid;
}

static bool
printpidfd(pid_t pid_of_fd, int fd, const char *path)
{
	static const char pidfs_prefix[] = "pidfd:";
	static const char pidfd_path[] = "anon_inode:[pidfd]";

	if (STR_STRIP_PREFIX(path, pidfs_prefix) == path
	    && strcmp(path, pidfd_path) != 0)
		return false;

	pid_t pid = pidfd_get_pid(pid_of_fd, fd);
	if (pid > 0) {
		tprint_associated_info_begin();
		tprints_string("pid:");
		/*
		 * The pid translation is not needed because
		 * the pid is in strace's namespace.
		 */
		printpid(NULL, pid, PT_TID);
		tprint_associated_info_end();
	} else {
		print_string_in_angle_brackets(path);
	}

	return true;
}

static bool
print_fdinfo_sigmask(const char *value, void *data)
{
#ifdef WORDS_BIGENDIAN
	unsigned int pos_xor_mask = current_wordsize - 1;
#else
	unsigned int pos_xor_mask = 0;
#endif
	size_t sigset_size = strlen(value) / 2;
	uint8_t *sigmask = xmalloc(sigset_size);

	for (size_t i = 0; i < sigset_size; ++i) {
		uint8_t byte;
		if (sscanf(value + i * 2, "%02hhx", &byte) != 1) {
			free(sigmask);
			return false;
		}
		sigmask[(sigset_size - 1 - i) ^ pos_xor_mask] = byte;
	}

	tprint_associated_info_begin();
	tprints_string(sprintsigmask_n("signalfd:", sigmask, sigset_size));
	tprint_associated_info_end();

	free(sigmask);
	return true;
}

static bool
printsignalfd(pid_t pid_of_fd, int fd, const char *path)
{
	static const char signalfd_path[] = "anon_inode:[signalfd]";
	static const char sigmask_pfx[] = "sigmask:\t";

	if (strcmp(path, signalfd_path))
		return false;

	return scan_fdinfo(pid_of_fd, fd, sigmask_pfx, sizeof(sigmask_pfx) - 1,
			   print_fdinfo_sigmask, NULL);
}

static bool
parse_fdinfo_efd_semaphore(const char *value, void *data)
{
	int *efd_semaphore = data;
	*efd_semaphore = string_to_uint_ex(value, NULL, INT_MAX, "\n");
	return true;
}

static bool
parse_fdinfo_efd_id(const char *value, void *data)
{
	int *efd_id = data;
	*efd_id = string_to_uint_ex(value, NULL, INT_MAX, "\n");
	return true;
}

static bool
parse_fdinfo_efd_counter(const char *value, void *data)
{
	char *ptr = (char *) value;

	ptr += strspn(ptr, " \t");
	ptr[strcspn(ptr, "\n")] = '\0';

	if (*ptr == '\0')
		ptr = NULL;

	*(char **) data = xstrdup(ptr);
	return true;
}

static bool
printeventfd(pid_t pid_of_fd, int fd, const char *path)
{
	static const char eventfd_path[] = "anon_inode:[eventfd]";
	/* Linux kernel commit v3.8-rc1~74^2~8 */
	static const char efd_counter_pfx[] = "eventfd-count:";
	/* Linux kernel commit v5.2-rc1~62^2~38 */
	static const char efd_id_pfx[] = "eventfd-id:";
	/* Linux kernel commit v6.5-rc1~246^2~5 */
	static const char efd_semaphore_pfx[] = "eventfd-semaphore:";

	if (strcmp(path, eventfd_path))
		return false;

	char *efd_counter = NULL;
	int efd_id = -1;
	int efd_semaphore = -1;

	struct scan_fdinfo fdinfo_lines[] = {
		{
			.search_pfx = efd_counter_pfx,
			.search_pfx_len = sizeof(efd_counter_pfx) - 1,
			.fn = parse_fdinfo_efd_counter,
			.data = &efd_counter
		},
		{
			.search_pfx = efd_id_pfx,
			.search_pfx_len = sizeof(efd_id_pfx) - 1,
			.fn = parse_fdinfo_efd_id,
			.data = &efd_id
		},
		{
			.search_pfx = efd_semaphore_pfx,
			.search_pfx_len = sizeof(efd_semaphore_pfx) - 1,
			.fn = parse_fdinfo_efd_semaphore,
			.data = &efd_semaphore
		}
	};

	scan_fdinfo_lines(pid_of_fd, fd, fdinfo_lines, ARRAY_SIZE(fdinfo_lines));

	if (efd_counter) {
		tprint_associated_info_begin();
		tprint_struct_begin();
		tprints_field_name("eventfd-count");
		if (efd_counter[0] != '0')
			tprints_string("0x");
		tprints_string(efd_counter);
		free(efd_counter);

		if (efd_id != -1) {
			tprint_struct_next();
			tprints_field_name("eventfd-id");
			PRINT_VAL_U(efd_id);

			if (efd_semaphore != -1) {
				tprint_struct_next();
				tprints_field_name("eventfd-semaphore");
				PRINT_VAL_U(efd_semaphore);
			}
		}

		tprint_struct_end();
		tprint_associated_info_end();
	} else
		print_string_in_angle_brackets(path);

	return true;
}

static void
print_quoted_string_in_angle_brackets(const char *str, const bool deleted)
{
	tprint_associated_info_begin();
	print_quoted_string_ex(str, strlen(str),
			       QUOTE_OMIT_LEADING_TRAILING_QUOTES, "<>");
	tprint_associated_info_end();

	if (deleted)
		tprints_string("(deleted)");
}

void
printfd_pid_with_finfo(struct tcb *tcp, pid_t pid, int fd, const struct finfo *finfo)
{
	PRINT_VAL_D(fd);

	char patha[PATH_MAX + 1];
	bool deleted;
	if (pid > 0 && !number_set_array_is_empty(decode_fd_set, 0)
	    && (finfo || (getfdpath_pid(pid, fd, patha, sizeof(patha), &deleted) >= 0))) {
		const char *path = finfo? finfo->path: patha;
		if (is_number_in_set(DECODE_FD_SOCKET, decode_fd_set) &&
		    printsocket(tcp, fd, path))
			goto printed;
		if (is_number_in_set(DECODE_FD_DEV, decode_fd_set) &&
		    printdev(tcp, fd, path, finfo))
			goto printed;
		if (is_number_in_set(DECODE_FD_EVENTFD, decode_fd_set) &&
		    printeventfd(pid, fd, path))
			goto printed;
		if (is_number_in_set(DECODE_FD_PIDFD, decode_fd_set) &&
		    printpidfd(pid, fd, path))
			goto printed;
		if (is_number_in_set(DECODE_FD_SIGNALFD, decode_fd_set) &&
		    printsignalfd(pid, fd, path))
			goto printed;
		if (is_number_in_set(DECODE_FD_PATH, decode_fd_set))
			print_quoted_string_in_angle_brackets(path,
							      finfo? finfo->deleted: deleted);
printed:	;
	}

	selinux_printfdcon(pid, fd);
}

void
printfd_pid_tracee_ns(struct tcb *tcp, pid_t pid, int fd)
{
	int strace_pid = translate_pid(tcp, pid, PT_TGID, NULL);
	printfd_pid(tcp, strace_pid, fd);
}

const char *
pid_to_str(pid_t pid)
{
	if (!pid)
		return "self";

	static char buf[sizeof("-2147483648")];
	xsprintf(buf, "%d", pid);
	return buf;
}

size_t
proc_status_get_id_list(int proc_pid, int *id_buf, size_t id_buf_size,
			const char *str, size_t str_size)
{
	size_t n = 0;

	if (!str_size)
		str_size = strlen(str);

	char status_path[PATH_MAX + 1];
	xsprintf(status_path, "/proc/%s/status", pid_to_str(proc_pid));
	FILE *f = fopen_stream(status_path, "r");
	if (!f)
		return 0;

	char *line = NULL;
	size_t linesize = 0;
	char *p = NULL;

	while (getline(&line, &linesize, f) > 0) {
		if (strncmp(line, str, str_size) == 0) {
			p = line + str_size;
			break;
		}
	}

	while (p) {
		errno = 0;
		long id = strtol(p, NULL, 10);

		if (id < 0 || id > INT_MAX || errno) {
			debug_func_perror_msg("converting \"%s\" to int", p);
			break;
		}

		if (id_buf && n < id_buf_size)
			id_buf[n] = (int) id;

		n++;
		strsep(&p, "\t");
	}

	free(line);
	fclose(f);

	return n;
}

/*
 * Quote string `instr' of length `size'
 * Write up to (3 + `size' * 4) bytes to `outstr' buffer.
 *
 * `escape_chars' specifies characters (in addition to characters with
 * codes 0..31, 127..255, single and double quotes) that should be escaped.
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
	     const unsigned int style, const char *escape_chars)
{
	const unsigned char *ustr = (const unsigned char *) instr;
	char *s = outstr;
	unsigned int i;
	int usehex, c, eol;
	bool printable;
	enum xflag_opts xstyle = style & QUOTE_OVERWRITE_HEXSTR
					? ((style & QUOTE_HEXSTR_MASK)
					   >> QUOTE_HEXSTR_SHIFT)
					: xflag;

	if (style & QUOTE_0_TERMINATED)
		eol = '\0';
	else
		eol = 0x100; /* this can never match a char */

	usehex = 0;
	if (xstyle == HEXSTR_ALL) {
		usehex = 1;
	} else if (xstyle == HEXSTR_NON_ASCII) {
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

	if (style & QUOTE_EMIT_COMMENT)
		s = stpcpy(s, " /* ");
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
			s = sprint_byte_hex(s, c);
		}

		goto string_ended;
	}

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
			printable = is_print(c);

			if (printable && escape_chars)
				printable = !strchr(escape_chars, c);

			if (printable) {
				*s++ = c;
			} else {
				if (xstyle == HEXSTR_NON_ASCII_CHARS) {
					/* Print he\x */
					*s++ = '\\';
					*s++ = 'x';
					s = sprint_byte_hex(s, c);
				} else {
					/* Print \octal */
					*s++ = '\\';
					s = sprint_byte_oct(s, c,
							i + 1 < size
							&& ustr[i + 1] >= '0'
							&& ustr[i + 1] <= '7');
				}
			}
		}
	}

 string_ended:
	if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
		*s++ = '\"';
	if (style & QUOTE_EMIT_COMMENT)
		s = stpcpy(s, " */");
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
	if (style & QUOTE_EMIT_COMMENT)
		s = stpcpy(s, " */");
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
print_quoted_string_ex(const char *str, unsigned int size,
		       const unsigned int style, const char *escape_chars)
{
	char *buf;
	char *outstr;
	unsigned int alloc_size;
	int rc;

	if (size && style & QUOTE_0_TERMINATED)
		--size;

	alloc_size = 4 * size;
	if (alloc_size / 4 != size) {
		error_func_msg("requested %u bytes exceeds %u bytes limit",
			       size, -1U / 4);
		tprint_unavailable();
		return -1;
	}
	alloc_size += 1 + (style & QUOTE_OMIT_LEADING_TRAILING_QUOTES ? 0 : 2) +
		(style & QUOTE_EMIT_COMMENT ? 7 : 0);

	if (use_alloca(alloc_size)) {
		outstr = alloca(alloc_size);
		buf = NULL;
	} else {
		outstr = buf = malloc(alloc_size);
		if (!buf) {
			error_func_msg("memory exhausted when tried to allocate"
				       " %u bytes", alloc_size);
			tprint_unavailable();
			return -1;
		}
	}

	rc = string_quote(str, outstr, size, style, escape_chars);
	tprints_string(outstr);

	if (((style & (QUOTE_0_TERMINATED | QUOTE_EXPECT_TRAILING_0))
	     == (QUOTE_0_TERMINATED | QUOTE_EXPECT_TRAILING_0)) && rc) {
		tprint_more_data_follows();
	}

	free(buf);
	return rc;
}

inline int
print_quoted_string(const char *str, unsigned int size,
		    const unsigned int style)
{
	return print_quoted_string_ex(str, size, style, NULL);
}

/*
 * Quote a NUL-terminated string `str' of length up to `size' - 1
 * and print the result.
 *
 * Returns 0 if NUL was seen, 1 otherwise.
 */
int
print_quoted_cstring(const char *str, unsigned int size)
{
	int unterminated =
		print_quoted_string(str, size, QUOTE_0_TERMINATED);

	if (unterminated)
		tprint_more_data_follows();

	return unterminated;
}

/*
 * Print path string specified by address `addr' and length `n',
 * including the terminating NUL.
 * If path length exceeds `n - 1', append `...' to the output.
 *
 * Returns the result of umovestr.
 */
int
printpathn(struct tcb *const tcp, const kernel_ulong_t addr, unsigned int n)
{
	char path[PATH_MAX];
	int nul_seen;

	if (!addr) {
		tprint_null();
		return -1;
	}

	/* Cap path size to the path buffer size */
	if (n > sizeof(path))
		n = sizeof(path);

	/*
	 * Fetch including the terminating NUL to find out
	 * whether path length >= n.
	 */
	nul_seen = umovestr(tcp, addr, n, path);
	if (nul_seen < 0)
		printaddr(addr);
	else {
		print_quoted_cstring(path, (unsigned int) nul_seen ?: n);

		if (nul_seen)
			selinux_printfilecon(tcp, path);
	}

	return nul_seen;
}

int
printpath(struct tcb *const tcp, const kernel_ulong_t addr)
{
	/* Size must correspond to char path[] size in printpathn */
	return printpathn(tcp, addr, PATH_MAX);
}

/*
 * Print string specified by address `addr' and length `len'.
 * If `user_style' has QUOTE_0_TERMINATED bit set, treat the string
 * as a NUL-terminated string.
 * Pass `user_style' on to `string_quote'.
 * Append `...' to the output if either the string length exceeds `max_strlen',
 * or QUOTE_0_TERMINATED bit is set and the string length exceeds `len'.
 *
 * Returns the result of umovestr if style has QUOTE_0_TERMINATED,
 * or the result of umoven otherwise.
 */
int
printstr_ex(struct tcb *const tcp, const kernel_ulong_t addr,
	    const kernel_ulong_t len, const unsigned int user_style)
{
	static char *str;
	static char *outstr;

	unsigned int size;
	unsigned int style = user_style;
	int rc;
	int ellipsis;

	if (!addr) {
		tprint_null();
		return -1;
	}
	/* Allocate static buffers if they are not allocated yet. */
	if (!str) {
		const unsigned int outstr_size =
			4 * max_strlen + /* for quotes and NUL */ 3;
		/*
		 * We can assume that outstr_size / 4 == max_strlen
		 * since we have a guarantee that max_strlen <= -1U / 4.
		 */

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
		return rc;
	}

	if (size > max_strlen)
		size = max_strlen;
	else
		str[size] = '\xff';

	if ((style & (QUOTE_0_TERMINATED | QUOTE_EXPECT_TRAILING_0))
	    == (QUOTE_0_TERMINATED | QUOTE_EXPECT_TRAILING_0)
	    && size == len && size) {
		--size;
	}

	/* If string_quote didn't see NUL and (it was supposed to be ASCIZ str
	 * or we were requested to print more than -s NUM chars)...
	 */
	ellipsis = string_quote(str, outstr, size, style, NULL)
		   && len
		   && ((style & (QUOTE_0_TERMINATED | QUOTE_EXPECT_TRAILING_0))
		       || len > max_strlen);

	tprints_string(outstr);
	if (ellipsis)
		tprint_more_data_follows();

	return rc;
}

bool
print_nonzero_bytes(struct tcb *const tcp,
		    void (*const prefix_fun)(void),
		    const kernel_ulong_t start_addr,
		    const unsigned int start_offs,
		    const unsigned int total_len,
		    const unsigned int style)
{
	if (start_offs >= total_len)
		return false;

	const kernel_ulong_t addr = start_addr + start_offs;
	const unsigned int len = total_len - start_offs;
	const unsigned int size = MIN(len, max_strlen);

	char *str = malloc(len);

	if (!str) {
		error_func_msg("memory exhausted when tried to allocate"
                               " %u bytes", len);
		prefix_fun();
		tprint_unavailable();
		return true;
	}

	bool ret = true;

	if (umoven(tcp, addr, len, str)) {
		prefix_fun();
		tprint_unavailable();
	} else if (is_filled(str, 0, len)) {
		ret = false;
	} else {
		prefix_fun();
		tprintf_string("/* bytes %u..%u */ ", start_offs, total_len - 1);

		print_quoted_string(str, size, style);

		if (size < len)
			tprint_more_data_follows();
	}

	free(str);
	return ret;
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
# define iov iovu.iov64
# define sizeof_iov \
	(current_wordsize == 4 ? (unsigned int) sizeof(*iovu.iov32)	\
			       : (unsigned int) sizeof(*iovu.iov64))
# define iov_iov_base(i) \
	(current_wordsize == 4 ? (uint64_t) iovu.iov32[i].base : iovu.iov64[i].base)
# define iov_iov_len(i) \
	(current_wordsize == 4 ? (uint64_t) iovu.iov32[i].len : iovu.iov64[i].len)
#else
	struct iovec *iov;
# define sizeof_iov ((unsigned int) sizeof(*iov))
# define iov_iov_base(i) ptr_to_kulong(iov[i].iov_base)
# define iov_iov_len(i) iov[i].iov_len
#endif
	unsigned int size = sizeof_iov * len;
	if (size / sizeof_iov != (unsigned int) len) {
		error_func_msg("requested %u iovec elements exceeds"
			       " %u iovec limit", len, -1U / sizeof_iov);
		return;
	}

	iov = malloc(size);
	if (!iov) {
		error_func_msg("memory exhausted when tried to allocate"
			       " %u bytes", size);
		return;
	}
	if (umoven(tcp, addr, size, iov) >= 0) {
		for (int i = 0; i < len; ++i) {
			kernel_ulong_t iov_len = iov_iov_len(i);
			if (iov_len > data_size)
				iov_len = data_size;
			if (!iov_len)
				break;
			data_size -= iov_len;
			/* include the buffer number to make it easy to
			 * match up the trace with the source */
			tprintf_string(" * %" PRI_klu " bytes in buffer %d\n",
				       iov_len, i);
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
dumpstr(struct tcb *const tcp, const kernel_ulong_t addr,
	const kernel_ulong_t len)
{
	/* xx xx xx xx xx xx xx xx  xx xx xx xx xx xx xx xx  1234567890123456 */
	enum {
		HEX_BIT = 4,

		DUMPSTR_GROUP_BYTES = 8,
		DUMPSTR_GROUPS = 2,
		DUMPSTR_WIDTH_BYTES = DUMPSTR_GROUP_BYTES * DUMPSTR_GROUPS,

		/** Width of formatted dump in characters.  */
		DUMPSTR_WIDTH_CHARS = DUMPSTR_WIDTH_BYTES +
			sizeof("xx") * DUMPSTR_WIDTH_BYTES + DUMPSTR_GROUPS,

		DUMPSTR_GROUP_MASK = DUMPSTR_GROUP_BYTES - 1,
		DUMPSTR_BYTES_MASK = DUMPSTR_WIDTH_BYTES - 1,

		/** Minimal width of the offset field in the output.  */
		DUMPSTR_OFFS_MIN_CHARS = 5,

		/** Arbitrarily chosen internal dumpstr buffer limit.  */
		DUMPSTR_BUF_MAXSZ = 1 << 16,
	};

	static_assert(!(DUMPSTR_BUF_MAXSZ % DUMPSTR_WIDTH_BYTES),
		      "Maximum internal buffer size should be divisible "
		      "by amount of bytes dumped per line");
	static_assert(!(DUMPSTR_GROUP_BYTES & DUMPSTR_GROUP_MASK),
		      "DUMPSTR_GROUP_BYTES is not power of 2");
	static_assert(!(DUMPSTR_WIDTH_BYTES & DUMPSTR_BYTES_MASK),
		      "DUMPSTR_WIDTH_BYTES is not power of 2");

	if (len > len + DUMPSTR_WIDTH_BYTES || addr + len < addr) {
		debug_func_msg("len %" PRI_klu " at addr %#" PRI_klx
			       " is too big, skipped", len, addr);
		return;
	}

	static kernel_ulong_t strsize;
	static unsigned char *str;

	const kernel_ulong_t alloc_size =
		MIN(ROUNDUP(len, DUMPSTR_WIDTH_BYTES), DUMPSTR_BUF_MAXSZ);

	if (strsize < alloc_size) {
		free(str);
		str = malloc(alloc_size);
		if (!str) {
			strsize = 0;
			error_func_msg("memory exhausted when tried to allocate"
				       " %" PRI_klu " bytes", alloc_size);
			return;
		}
		strsize = alloc_size;
	}

	/**
	 * Characters needed in order to print the offset field. We calculate
	 * it this way in order to avoid ilog2_64 call most of the time.
	 */
	const int offs_chars = len > (1 << (DUMPSTR_OFFS_MIN_CHARS * HEX_BIT))
		? 1 + ilog2_klong(len - 1) / HEX_BIT : DUMPSTR_OFFS_MIN_CHARS;
	kernel_ulong_t i = 0;
	const unsigned char *src;

	while (i < len) {
		/*
		 * It is important to overwrite all the byte values, as we
		 * re-use the buffer in order to avoid its re-initialisation.
		 */
		static char outbuf[] = {
			[0 ... DUMPSTR_WIDTH_CHARS - 1] = ' ',
			'\0'
		};
		char *dst = outbuf;

		/* Fetching data from tracee.  */
		if (!i || (i % DUMPSTR_BUF_MAXSZ) == 0) {
			kernel_ulong_t fetch_size = MIN(len - i, alloc_size);

			if (umoven(tcp, addr + i, fetch_size, str) < 0) {
				/*
				 * Don't silently abort if we have printed
				 * something already.
				 */
				if (i)
					tprintf_string(" | <Cannot fetch %" PRI_klu
						       " byte%s from pid %d"
						       " @%#" PRI_klx ">\n",
						       fetch_size,
						       fetch_size == 1 ? "" : "s",
						       tcp->pid, addr + i);
				return;
			}
			src = str;
		}

		/* hex dump */
		do {
			if (i < len) {
				dst = sprint_byte_hex(dst, *src);
			} else {
				*dst++ = ' ';
				*dst++ = ' ';
			}
			dst++; /* space is there */
			i++;
			if ((i & DUMPSTR_GROUP_MASK) == 0)
				dst++; /* space is there */
			src++;
		} while (i & DUMPSTR_BYTES_MASK);

		/* ASCII dump */
		i -= DUMPSTR_WIDTH_BYTES;
		src -= DUMPSTR_WIDTH_BYTES;
		do {
			if (i < len) {
				if (is_print(*src))
					*dst++ = *src;
				else
					*dst++ = '.';
			} else {
				*dst++ = ' ';
			}
			src++;
		} while (++i & DUMPSTR_BYTES_MASK);

		tprintf_string(" | %0*" PRI_klx "  %s |\n",
			       offs_chars, i - DUMPSTR_WIDTH_BYTES, outbuf);
	}
}

bool
tfetch_mem64(struct tcb *const tcp, const uint64_t addr,
	     const unsigned int len, void *const our_addr)
{
	return addr && verbose(tcp) &&
	       (entering(tcp) || !syserror(tcp)) &&
	       !umoven(tcp, addr, len, our_addr);
}

bool
tfetch_mem64_ignore_syserror(struct tcb *const tcp, const uint64_t addr,
			     const unsigned int len, void *const our_addr)
{
	return addr && verbose(tcp) &&
	       !umoven(tcp, addr, len, our_addr);
}

int
umoven_or_printaddr64(struct tcb *const tcp, const uint64_t addr,
		      const unsigned int len, void *const our_addr)
{
	if (tfetch_mem64(tcp, addr, len, our_addr))
		return 0;
	printaddr64(addr);
	return -1;
}

int
umoven_or_printaddr64_ignore_syserror(struct tcb *const tcp,
				      const uint64_t addr,
				      const unsigned int len,
				      void *const our_addr)
{
	if (tfetch_mem64_ignore_syserror(tcp, addr, len, our_addr))
		return 0;
	printaddr64(addr);
	return -1;
}

int
umoven_to_uint64_or_printaddr64(struct tcb *const tcp, const uint64_t addr,
				unsigned int len, uint64_t *const our_addr)
{
	union {
		uint64_t val;
		uint8_t  bytes[sizeof(uint64_t)];
	} data = { .val = 0 };
	const size_t offs = is_bigendian ? sizeof(data) - len : 0;

	if (len <= sizeof(data) &&
	    tfetch_mem64_ignore_syserror(tcp, addr, len, data.bytes + offs)) {
		*our_addr = data.val;
		return 0;
	}
	printaddr64(addr);
	return -1;
}

bool
print_int_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
		       void *data)
{
	switch (elem_size) {
	case sizeof(int8_t):  PRINT_VAL_D(*(int8_t *) elem_buf);  break;
	case sizeof(int16_t): PRINT_VAL_D(*(int16_t *) elem_buf); break;
	case sizeof(int32_t): PRINT_VAL_D(*(int32_t *) elem_buf); break;
	case sizeof(int64_t): PRINT_VAL_D(*(int64_t *) elem_buf); break;
	default:
		error_func_msg("Unexpected elem_size: %zu", elem_size);
		return false;
	}

	return true;
}

bool
print_uint_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	switch (elem_size) {
	case sizeof(uint8_t):  PRINT_VAL_U(*(uint8_t *) elem_buf);  break;
	case sizeof(uint16_t): PRINT_VAL_U(*(uint16_t *) elem_buf); break;
	case sizeof(uint32_t): PRINT_VAL_U(*(uint32_t *) elem_buf); break;
	case sizeof(uint64_t): PRINT_VAL_U(*(uint64_t *) elem_buf); break;
	default:
		error_func_msg("Unexpected elem_size: %zu", elem_size);
		return false;
	}

	return true;
}

bool
print_xint_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	switch (elem_size) {
	case sizeof(uint8_t):  PRINT_VAL_X(*(uint8_t *) elem_buf);  break;
	case sizeof(uint16_t): PRINT_VAL_X(*(uint16_t *) elem_buf); break;
	case sizeof(uint32_t): PRINT_VAL_X(*(uint32_t *) elem_buf); break;
	case sizeof(uint64_t): PRINT_VAL_X(*(uint64_t *) elem_buf); break;
	default:
		error_func_msg("Unexpected elem_size: %zu", elem_size);
		return false;
	}

	return true;
}

bool
print_fd_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
		      void *data)
{
	printfd(tcp, *(int *) elem_buf);
	return true;
}

/*
 * Iteratively fetch and print up to nmemb elements of elem_size size
 * from the array that starts at tracee's address start_addr.
 *
 * Array elements are being fetched to the address specified by elem_buf.
 *
 * The fetcher callback function specified by tfetch_mem_func should follow
 * the same semantics as tfetch_mem function.
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
 * - start_addr, if the first tfetch_mem_func invocation returned false;
 * - elements of the array, delimited by ", ", with the array itself
 *   enclosed with [] brackets.
 *
 * If abbrev(tcp) is true, then
 * - the maximum number of elements printed equals to max_strlen;
 * - "..." is printed instead of max_strlen+1 element
 *   and no more iterations will be made.
 *
 * This function returns true only if tfetch_mem_func has returned true
 * at least once.
 */
bool
print_array_ex(struct tcb *const tcp,
	       const kernel_ulong_t start_addr,
	       const size_t nmemb,
	       void *elem_buf,
	       const size_t elem_size,
	       tfetch_mem_fn tfetch_mem_func,
	       print_fn print_func,
	       void *const opaque_data,
	       unsigned int flags,
	       const struct xlat *index_xlat,
	       const char *index_dflt)
{
	if (!start_addr) {
		tprint_null();
		return false;
	}

	if (!nmemb) {
		tprint_array_begin();
		if (flags & PAF_ARRAY_TRUNCATED)
			tprint_more_data_follows();
		tprint_array_end();
		return false;
	}

	const size_t size = nmemb * elem_size;
	const kernel_ulong_t end_addr = start_addr + size;

	if (end_addr <= start_addr || size / elem_size != nmemb) {
		if (tfetch_mem_func)
			printaddr(start_addr);
		else
			tprint_unavailable();
		return false;
	}

	const kernel_ulong_t abbrev_end =
		sequence_truncation_needed(tcp, nmemb) ?
			start_addr + elem_size * max_strlen : end_addr;
	kernel_ulong_t cur;
	kernel_ulong_t idx = 0;
	enum xlat_style xlat_style = flags & XLAT_STYLE_MASK;
	bool truncated = false;

	for (cur = start_addr; cur < end_addr; cur += elem_size, idx++) {
		if (cur != start_addr)
			tprint_array_next();

		if (tfetch_mem_func) {
			if (!tfetch_mem_func(tcp, cur, elem_size, elem_buf)) {
				if (cur == start_addr)
					printaddr(cur);
				else {
					tprint_more_data_follows();
					printaddr_comment(cur);
					truncated = true;
				}
				break;
			}
		} else {
			elem_buf = (void *) (uintptr_t) cur;
		}

		if (cur == start_addr)
			tprint_array_begin();

		if (cur >= abbrev_end) {
			tprint_more_data_follows();
			cur = end_addr;
			truncated = true;
			break;
		}

		if (flags & PAF_PRINT_INDICES) {
			tprint_array_index_begin();

			if (!index_xlat) {
				print_xlat_ex(idx, NULL, xlat_style);
			} else {
				printxval_ex(index_xlat, idx, index_dflt,
					     xlat_style);
			}

			tprint_array_index_equal();
		}

		bool break_needed =
			!print_func(tcp, elem_buf, elem_size, opaque_data);

		if (flags & PAF_PRINT_INDICES)
			tprint_array_index_end();

		if (break_needed) {
			cur = end_addr;
			break;
		}
	}

	if ((cur != start_addr) || !tfetch_mem_func) {
		if ((flags & PAF_ARRAY_TRUNCATED) && !truncated) {
			if (cur != start_addr)
				tprint_array_next();

			tprint_more_data_follows();
		}

		tprint_array_end();
	}

	return cur >= end_addr;
}

int
printargs(struct tcb *tcp)
{
	const unsigned int n = n_args(tcp);
	for (unsigned int i = 0; i < n; ++i) {
		if (i)
			tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[i]);
	}
	return RVAL_DECODED;
}

int
printargs_u(struct tcb *tcp)
{
	const unsigned int n = n_args(tcp);
	for (unsigned int i = 0; i < n; ++i) {
		if (i)
			tprint_arg_next();
		PRINT_VAL_U((unsigned int) tcp->u_arg[i]);
	}
	return RVAL_DECODED;
}

int
printargs_d(struct tcb *tcp)
{
	const unsigned int n = n_args(tcp);
	for (unsigned int i = 0; i < n; ++i) {
		if (i)
			tprint_arg_next();
		PRINT_VAL_D((int) tcp->u_arg[i]);
	}
	return RVAL_DECODED;
}

/* Print abnormal high bits of a kernel_ulong_t value. */
void
print_abnormal_hi(const kernel_ulong_t val)
{
	if (current_klongsize > 4) {
		const unsigned int hi = (unsigned int) ((uint64_t) val >> 32);
		if (hi) {
			tprint_shift_begin();
			PRINT_VAL_X(hi);
			tprint_shift();
			PRINT_VAL_U(32);
			tprint_shift_end();
			tprint_flags_or();
		}
	}
}

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

bool
sequence_truncation_needed(const struct tcb *tcp, unsigned int len)
{
	return abbrev(tcp) && len > max_strlen;
}
