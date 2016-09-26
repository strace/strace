/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_TESTS_H
#define STRACE_TESTS_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include <sys/types.h>
# include "gcc_compat.h"

/* Tests of "strace -v" are expected to define VERBOSE to 1. */
#ifndef VERBOSE
# define VERBOSE 0
#endif

/* Cached sysconf(_SC_PAGESIZE). */
size_t get_page_size(void);

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

/*
 * Allocate memory that ends on the page boundary.
 * Pages allocated by this call are preceeded by an unmapped page
 * and followed also by an unmapped page.
 */
void *tail_alloc(const size_t)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1));
/* Allocate memory using tail_alloc, then memcpy. */
void *tail_memdup(const void *, const size_t)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((2));

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

/* Print string in a quoted form. */
void print_quoted_string(const char *);

/* Print memory in a quoted form. */
void print_quoted_memory(const char *, size_t);

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
int printxval(const struct xlat *, const unsigned long long, const char *);

/* Invoke a socket syscall, either directly or via __NR_socketcall. */
int socketcall(const int nr, const int call,
	       long a1, long a2, long a3, long a4, long a5);

/* Wrappers for recvmmsg and sendmmsg syscalls. */
struct mmsghdr;
struct timespec;
int recv_mmsg(int, struct mmsghdr *, unsigned int, unsigned int, struct timespec *);
int send_mmsg(int, struct mmsghdr *, unsigned int, unsigned int);

# define ARRAY_SIZE(arg) ((unsigned int) (sizeof(arg) / sizeof((arg)[0])))
# define LENGTH_OF(arg) ((unsigned int) sizeof(arg) - 1)

/* Zero-extend a signed integer type to unsigned long long. */
#define zero_extend_signed_to_ull(v) \
	(sizeof(v) == sizeof(char) ? (unsigned long long) (unsigned char) (v) : \
	 sizeof(v) == sizeof(short) ? (unsigned long long) (unsigned short) (v) : \
	 sizeof(v) == sizeof(int) ? (unsigned long long) (unsigned int) (v) : \
	 sizeof(v) == sizeof(long) ? (unsigned long long) (unsigned long) (v) : \
	 (unsigned long long) (v))

/* Sign-extend an unsigned integer type to long long. */
#define sign_extend_unsigned_to_ll(v) \
	(sizeof(v) == sizeof(char) ? (long long) (char) (v) : \
	 sizeof(v) == sizeof(short) ? (long long) (short) (v) : \
	 sizeof(v) == sizeof(int) ? (long long) (int) (v) : \
	 sizeof(v) == sizeof(long) ? (long long) (long) (v) : \
	 (long long) (v))

# define SKIP_MAIN_UNDEFINED(arg) \
	int main(void) { error_msg_and_skip("undefined: %s", arg); }

/*
 * The kernel used to define 64-bit types on 64-bit systems on a per-arch
 * basis.  Some architectures would use unsigned long and others would use
 * unsigned long long.  These types were exported as part of the
 * kernel-userspace ABI and now must be maintained forever.  This matches
 * what the kernel exports for each architecture so we don't need to cast
 * every printing of __u64 or __s64 to stdint types.
 */
# if SIZEOF_LONG == 4
#  define PRI__64 "ll"
# elif defined ALPHA || defined IA64 || defined MIPS || defined POWERPC
#  define PRI__64 "l"
# else
#  define PRI__64 "ll"
# endif

# define PRI__d64 PRI__64"d"
# define PRI__u64 PRI__64"u"
# define PRI__x64 PRI__64"x"

#endif /* !STRACE_TESTS_H */
