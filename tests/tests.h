/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_H
# define STRACE_TESTS_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# ifdef TESTS_SIZEOF_KERNEL_LONG_T
#  undef SIZEOF_KERNEL_LONG_T
#  define SIZEOF_KERNEL_LONG_T TESTS_SIZEOF_KERNEL_LONG_T
# endif

# ifdef TESTS_SIZEOF_LONG
#  undef SIZEOF_LONG
#  define SIZEOF_LONG TESTS_SIZEOF_LONG
# endif

# include <stdbool.h>
# include <stdint.h>
# include <sys/types.h>
# include "kernel_types.h"
# include "kernel_old_timespec.h"
# include "gcc_compat.h"
# include "macros.h"

/* Tests of "strace -v" are expected to define VERBOSE to 1. */
# ifndef VERBOSE
#  define VERBOSE 0
# endif

/* xlat verbosity defaults */
# ifndef XLAT_RAW
#  define XLAT_RAW 0
# endif
# ifndef XLAT_VERBOSE
#  define XLAT_VERBOSE 0
# endif



# if XLAT_RAW
#  define XLAT_KNOWN(val_, str_) STRINGIFY_VAL(val_)
#  define XLAT_UNKNOWN(val_, dflt_) STRINGIFY_VAL(val_)

#  define XLAT_KNOWN_FMT(val_, str_) val_
#  define XLAT_UNKNOWN_FMT(val_, dflt_) val_

#  define XLAT_FMT "%#x"
#  define XLAT_FMT_D "%d"
#  define XLAT_FMT_U "%u"
#  define XLAT_FMT_L "%#lx"
#  define XLAT_FMT_LL "%#llx"
#  define XLAT_ARGS(a_) (a_)
#  define XLAT_ARGS_U(a_) (unsigned int) (a_)
#  define XLAT_SEL(v_, s_) v_

#  define ABBR(...)
#  define RAW(...) __VA_ARGS__
#  define VERB(...)
#  define NABBR(...) __VA_ARGS__
#  define NRAW(...)
#  define NVERB(...) __VA_ARGS__
# elif XLAT_VERBOSE
#  define XLAT_KNOWN(val_, str_) STRINGIFY_VAL(val_) " /* " str_ " */"
#  define XLAT_UNKNOWN(val_, dflt_) STRINGIFY_VAL(val_) " /* " dflt_ " */"

#  define XLAT_KNOWN_FMT(val_, str_) val_ " /* " str_ " */"
#  define XLAT_UNKNOWN_FMT(val_, dflt_) val_ " /* " dflt_ " */"

#  define XLAT_FMT "%#x /* %s */"
#  define XLAT_FMT_D "%d /* %s */"
#  define XLAT_FMT_U "%u /* %s */"
#  define XLAT_FMT_L "%#lx /* %s */"
#  define XLAT_FMT_LL "%#llx /* %s */"
#  define XLAT_ARGS(a_) a_, #a_
#  define XLAT_ARGS_U(a_) (unsigned int) (a_), #a_
#  define XLAT_SEL(v_, s_) v_, s_

#  define ABBR(...)
#  define RAW(...)
#  define VERB(...) __VA_ARGS__
#  define NABBR(...) __VA_ARGS__
#  define NRAW(...) __VA_ARGS__
#  define NVERB(...)
# else /* !XLAT_RAW && !XLAT_VERBOSE */
#  define XLAT_KNOWN(val_, str_) str_
#  define XLAT_UNKNOWN(val_, dflt_) STRINGIFY_VAL(val_) " /* " dflt_ " */"

#  define XLAT_KNOWN_FMT(val_, str_) str_
#  define XLAT_UNKNOWN_FMT(val_, dflt_) val_ " /* " dflt_ " */"

#  define XLAT_FMT "%s"
#  define XLAT_FMT_D "%s"
#  define XLAT_FMT_U "%s"
#  define XLAT_FMT_L "%s"
#  define XLAT_FMT_LL "%s"
#  define XLAT_ARGS(a_) #a_
#  define XLAT_ARGS_U(a_) #a_
#  define XLAT_SEL(v_, s_) s_

#  define ABBR(...) __VA_ARGS__
#  define RAW(...)
#  define VERB(...)
#  define NABBR(...)
#  define NRAW(...) __VA_ARGS__
#  define NVERB(...) __VA_ARGS__
# endif /* XLAT_RAW, XLAT_VERBOSE */

# define XLAT_STR(v_) sprintxlat(#v_, v_, NULL)

# define ARG_XLAT_KNOWN(val_, str_) val_, XLAT_KNOWN(val_, str_)
# define ARG_XLAT_UNKNOWN(val_, str_) val_, XLAT_UNKNOWN(val_, str_)

# define ENUM_KNOWN(val_, enum_) enum_, XLAT_KNOWN(val_, #enum_)

# ifndef DEFAULT_STRLEN
/* Default maximum # of bytes printed in printstr et al. */
#  define DEFAULT_STRLEN 32
# endif

struct strval8 {
	uint8_t val;
	const char *str;
};

struct strval16 {
	uint16_t val;
	const char *str;
};

struct strval32 {
	uint32_t val;
	const char *str;
};

struct strval64 {
	uint64_t val;
	const char *str;
};

/* Cached sysconf(_SC_PAGESIZE). */
size_t get_page_size(void);

/* The size of kernel's sigset_t. */
unsigned int get_sigset_size(void);

/* Print message and strerror(errno) to stderr, then exit(1). */
void perror_msg_and_fail(const char *, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
/* Print message to stderr, then exit(1). */
void error_msg_and_fail(const char *, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
/* Print message to stderr, then exit(77). */
void error_msg_and_skip(const char *, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
/* Print message and strerror(errno) to stderr, then exit(77). */
void perror_msg_and_skip(const char *, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;

# ifndef perror_msg_and_fail
#  define perror_msg_and_fail(fmt_, ...) \
	perror_msg_and_fail("%s:%d: " fmt_, __FILE__, __LINE__, ##__VA_ARGS__)
# endif
# ifndef error_msg_and_fail
#  define error_msg_and_fail(fmt_, ...) \
	error_msg_and_fail("%s:%d: " fmt_, __FILE__, __LINE__, ##__VA_ARGS__)
# endif

/* Stat the specified file and skip the test if the stat call failed. */
void skip_if_unavailable(const char *);

/*
 * Obtain a file descriptor corresponding to the specified directory name,
 * die on failure.
 */
int get_dir_fd(const char *dir_path);

/*
 * Obtain a path corresponding to the specified file descriptor,
 * die on failure.
 */
char *get_fd_path(int fd) ATTRIBUTE_MALLOC;

/*
 * Create the specified directory and chdir into it,
 * die on chdir failure.
 */
void create_and_enter_subdir(const char *subdir);

/*
 * Leave from the directory entered by create_and_enter_subdir,
 * remove that directory, die on failure.
 */
void leave_and_remove_subdir(void);

/*
 * Obtain an exclusive lock on dirname(path_name)/lock_name file
 * using open and flock.
 */
int lock_file_by_dirname(const char *path_name, const char *lock_name);

/*
 * Allocate memory that ends on the page boundary.
 * Pages allocated by this call are preceded by an unmapped page
 * and followed also by an unmapped page.
 */
void *tail_alloc(const size_t)
	ATTRIBUTE_MALLOC;
/* Allocate memory using tail_alloc, then memcpy. */
void *tail_memdup(const void *, const size_t)
	ATTRIBUTE_MALLOC;

# define midtail_alloc(after_, before_) \
	((void *) ((char *) tail_alloc(((before_) + (after_))) + (before_)))

/*
 * Allocate an object of the specified type at the end
 * of a mapped memory region.
 * Assign its address to the specified constant pointer.
 */
# define TAIL_ALLOC_OBJECT_CONST_PTR(type_name, type_ptr)	\
	type_name *const type_ptr = tail_alloc(sizeof(*type_ptr))

/*
 * Allocate an array of the specified type at the end
 * of a mapped memory region.
 * Assign its address to the specified constant pointer.
 */
# define TAIL_ALLOC_OBJECT_CONST_ARR(type_name, type_ptr, cnt)	\
	type_name *const type_ptr = tail_alloc(sizeof(*type_ptr) * (cnt))

/*
 * Allocate an object of the specified type at the end
 * of a mapped memory region.
 * Assign its address to the specified variable pointer.
 */
# define TAIL_ALLOC_OBJECT_VAR_PTR(type_name, type_ptr)		\
	type_name *type_ptr = tail_alloc(sizeof(*type_ptr))

/*
 * Allocate an array of the specified type at the end
 * of a mapped memory region.
 * Assign its address to the specified variable pointer.
 */
# define TAIL_ALLOC_OBJECT_VAR_ARR(type_name, type_ptr, cnt)	\
	type_name *type_ptr = tail_alloc(sizeof(*type_ptr) * (cnt))

/**
 * Fill memory (pointed by ptr, having size bytes) with different bytes (with
 * values starting with start and resetting every period) in order to catch
 * sign, byte order and/or alignment errors.
 */
void fill_memory_ex(void *ptr, size_t size, unsigned char start,
		    unsigned int period);
/** Shortcut for fill_memory_ex(ptr, size, 0x80, 0x80) */
void fill_memory(void *ptr, size_t size);
/** Variant of fill_memory_ex for arrays of 16-bit (2-byte) values. */
void fill_memory16_ex(void *ptr, size_t size, uint16_t start,
		      unsigned int period);
/** Shortcut for fill_memory16_ex(ptr, size, 0x80c0, 0x8000) */
void fill_memory16(void *ptr, size_t size);
/** Variant of fill_memory_ex for arrays of 32-bit (4-byte) values. */
void fill_memory32_ex(void *ptr, size_t size, uint32_t start,
		      unsigned int period);
/** Shortcut for fill_memory32_ex(ptr, size, 0x80a0c0e0, 0x80000000) */
void fill_memory32(void *ptr, size_t size);
/** Variant of fill_memory_ex for arrays of 64-bit (8-byte) values. */
void fill_memory64_ex(void *ptr, size_t size, uint64_t start, uint64_t period);
/**
 * Shortcut for
 * fill_memory64_ex(ptr, size, 0x8090a0b0c0d0e0f0, 0x8000000000000000)
 */
void fill_memory64(void *ptr, size_t size);


/* Close stdin, move stdout to a non-standard descriptor, and print. */
void tprintf(const char *, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2));

/* Make a hexdump copy of C string */
const char *hexdump_strdup(const char *);

/* Make a hexdump copy of memory */
const char *hexdump_memdup(const char *, size_t);

/* Make a hexquoted copy of a string */
const char *hexquote_strndup(const char *, size_t);

/* Return inode number of socket descriptor. */
unsigned long inode_of_sockfd(int);

/* Print string in a quoted form with optional escape characters. */
void print_quoted_string_ex(const char *, bool quote, const char *escape_str);

/* Print string in a quoted form. */
void print_quoted_string(const char *);

/*
 * Print a NUL-terminated string `str' of length up to `size' - 1
 * in a quoted form.
 */
void print_quoted_cstring(const char *str, size_t size);

/*
 * Print a NUL-terminated string `str' of length up to `size'
 * in a quoted form.
 */
void print_quoted_stringn(const char *str, size_t size);

/* Print memory in a quoted form with optional escape characters. */
void print_quoted_memory_ex(const void *, size_t, bool quote,
			    const char *escape_chars);

/* Print memory in a quoted form. */
void print_quoted_memory(const void *, size_t);

/* Print memory in a hexquoted form. */
void print_quoted_hex(const void *, size_t);

/* Print time_t and nanoseconds in symbolic format. */
void print_time_t_nsec(time_t, unsigned long long, int);

/* Print time_t and microseconds in symbolic format. */
void print_time_t_usec(time_t, unsigned long long, int);

/* Put a formatted clock_t string representation into a string. */
const char *clock_t_str(uint64_t val, char *str, size_t str_size);

/* Read an int from the file. */
int read_int_from_file(const char *, int *);

/* Check whether given uid matches kernel overflowuid. */
void check_overflowuid(const int);

/* Check whether given gid matches kernel overflowgid. */
void check_overflowgid(const int);

/* Translate errno to its name. */
const char *errno2name(void);

/* Translate signal number to its name. */
const char *signal2name(int);

/* Print return code and, in case return code is -1, errno information. */
const char *sprintrc(long rc);
/* sprintrc variant suitable for usage as part of grep pattern. */
const char *sprintrc_grep(long rc);

struct xlat;

/* Print flags in symbolic form according to xlat table. */
int printflags(const struct xlat *, const unsigned long long, const char *);

/* Print constant in symbolic form according to xlat table. */
int printxval_abbrev(const struct xlat *, const unsigned long long,
		     const char *);
int printxval_raw(const struct xlat *, const unsigned long long, const char *);
int printxval_verbose(const struct xlat *, const unsigned long long,
		      const char *);

/* Print constant in symbolic form according to xlat table. */
const char *sprintxlat_abbrev(const char *, const unsigned long long,
			   const char *);
const char *sprintxlat_raw(const char *, const unsigned long long,
			   const char *);
const char *sprintxlat_verbose(const char *, const unsigned long long,
			       const char *);

/* Print constant in symbolic form according to xlat table. */
const char *sprintxval_abbrev(const struct xlat *, const unsigned long long,
			      const char *);
const char *sprintxval_raw(const struct xlat *, const unsigned long long,
			   const char *);
const char *sprintxval_verbose(const struct xlat *, const unsigned long long,
			       const char *);

# if XLAT_RAW
#  define printxval  printxval_raw
#  define sprintxlat sprintxlat_raw
#  define sprintxval sprintxval_raw
# elif XLAT_VERBOSE
#  define printxval  printxval_verbose
#  define sprintxlat sprintxlat_verbose
#  define sprintxval sprintxval_verbose
# else
#  define printxval  printxval_abbrev
#  define sprintxlat sprintxlat_abbrev
#  define sprintxval sprintxval_abbrev
# endif

/* Invoke a socket syscall, either directly or via __NR_socketcall. */
int socketcall(const int nr, const int call,
	       long a1, long a2, long a3, long a4, long a5);

/* Invoke a prctl syscall with very specific arguments for use as a marker.  */
long prctl_marker(void);

/* Call chdir and print strace output depending on flags. */
void test_status_chdir(const char *dir, bool print_success, bool print_fail);

/* Wrappers for recvmmsg and sendmmsg syscalls. */
struct mmsghdr;
int recv_mmsg(int, struct mmsghdr *, unsigned int, unsigned int, kernel_old_timespec_t *);
int send_mmsg(int, struct mmsghdr *, unsigned int, unsigned int);

/* Create a netlink socket. */
int create_nl_socket_ext(int proto, const char *name);
# define create_nl_socket(proto)	create_nl_socket_ext((proto), #proto)

/* Create a temporary file in the current directory. */
int create_tmpfile(unsigned int flags);

/* Create a pipe with maximized descriptor numbers. */
void pipe_maxfd(int pipefd[2]);

/* if_nametoindex("lo") */
unsigned int ifindex_lo(void);

# ifdef HAVE_IF_INDEXTONAME
#  define IFINDEX_LO_STR "if_nametoindex(\"lo\")"
# else
#  define IFINDEX_LO_STR "1"
# endif

# define F8ILL_KULONG_SUPPORTED	(sizeof(void *) < sizeof(kernel_ulong_t))
# define F8ILL_KULONG_MASK	((kernel_ulong_t) 0xffffffff00000000ULL)

/*
 * For 64-bit kernel_ulong_t and 32-bit pointer,
 * return a kernel_ulong_t value by filling higher bits.
 * For other architectures, return the original pointer.
 */
static inline kernel_ulong_t
f8ill_ptr_to_kulong(const void *const ptr)
{
	const unsigned long uptr = (unsigned long) ptr;
	return F8ILL_KULONG_SUPPORTED
	       ? F8ILL_KULONG_MASK | uptr : (kernel_ulong_t) uptr;
}

# define LENGTH_OF(arg) ((unsigned int) sizeof(arg) - 1)

/* Zero-extend a signed integer type to unsigned long long. */
# define zero_extend_signed_to_ull(v) \
	(sizeof(v) == sizeof(char) ? (unsigned long long) (unsigned char) (v) : \
	 sizeof(v) == sizeof(short) ? (unsigned long long) (unsigned short) (v) : \
	 sizeof(v) == sizeof(int) ? (unsigned long long) (unsigned int) (v) : \
	 sizeof(v) == sizeof(long) ? (unsigned long long) (unsigned long) (v) : \
	 (unsigned long long) (v))

/* Sign-extend an unsigned integer type to long long. */
# define sign_extend_unsigned_to_ll(v) \
	(sizeof(v) == sizeof(char) ? (long long) (char) (v) : \
	 sizeof(v) == sizeof(short) ? (long long) (short) (v) : \
	 sizeof(v) == sizeof(int) ? (long long) (int) (v) : \
	 sizeof(v) == sizeof(long) ? (long long) (long) (v) : \
	 (long long) (v))

# define SKIP_MAIN_UNDEFINED(arg) \
	int main(void) { error_msg_and_skip("undefined: %s", arg); }

# ifdef WORDS_BIGENDIAN
#  define BE_LE(be_, le_) be_
#  define LL_PAIR(HI, LO) (HI), (LO)
# else
#  define BE_LE(be_, le_) le_
#  define LL_PAIR(HI, LO) (LO), (HI)
# endif
# define LL_VAL_TO_PAIR(llval) LL_PAIR((long) ((llval) >> 32), (long) (llval))

# define ARG_STR(_arg) (_arg), #_arg
# define ARG_ULL_STR(_arg) _arg##ULL, #_arg

/*
 * Assign an object of type DEST_TYPE at address DEST_ADDR
 * using memcpy to avoid potential unaligned access.
 */
# define SET_STRUCT(DEST_TYPE, DEST_ADDR, ...)						\
	do {										\
		DEST_TYPE dest_type_tmp_var = { __VA_ARGS__ };				\
		memcpy(DEST_ADDR, &dest_type_tmp_var, sizeof(dest_type_tmp_var));	\
	} while (0)

# define NLMSG_ATTR(nlh, hdrlen) ((void *)(nlh) + NLMSG_SPACE(hdrlen))

#endif /* !STRACE_TESTS_H */
