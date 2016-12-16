/*
 * Check decoding of keyctl syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#include "tests.h"

#include <asm/unistd.h>

#ifdef __NR_keyctl

# include <linux/types.h>
# include <linux/keyctl.h>

# include <errno.h>
# include <inttypes.h>
# include <stdarg.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/uio.h>

/* This check should be before #include "xlat/keyctl_commands.h" */
# ifndef KEYCTL_DH_COMPUTE
struct keyctl_dh_params {
	int32_t private;
	int32_t prime;
	int32_t base;
};
# endif

# include "xlat.h"
# include "xlat/keyctl_commands.h"

# ifndef KEY_SPEC_REQKEY_AUTH_KEY
#  define KEY_SPEC_REQKEY_AUTH_KEY   -7
# endif

# ifndef KEY_SPEC_REQUESTOR_KEYRING
#  define KEY_SPEC_REQUESTOR_KEYRING -8
# endif

static const size_t limit = 10;

/*
 * Well, this is true for DESCRIBE and GET_SECURITY, and false for READ and
 * DH_COMPUTE and I see no ability to pass this information without
 * significantly breaking interface.
 */
bool nul_terminated_buf = true;
bool buf_in_arg = false;

/*
 * When this is called with positive size, the buffer provided is an "out"
 * argument and rc contains resulting size (globally defined nul_terminated_buf
 * controls whether it is nul-terminated or not). If size is negative,
 * it contains "in" argument.
 */
void
print_quoted_string_limit(const char *str, size_t size, long rc)
{
	size_t print_size = ((rc >= 0) && (size > 0)) ?
		((unsigned long) rc > size ? size :
		(unsigned long) rc) : size;
	size_t limited_size = print_size > limit ? limit : print_size;

	if ((rc == -1) && !buf_in_arg) {
		printf("%p", str);
		return;
	}

	if (!nul_terminated_buf ||
	    (strnlen(str, limited_size) == limited_size)) {
		printf("\"");
		print_quoted_memory(str, limited_size);
		if (print_size > limit)
			printf("\"...");
		else
			printf("\"");
	} else {
		printf("\"");
		print_quoted_string(str);
		printf("\"");
	}
}

static void
print_arg(kernel_ulong_t arg, const char *str, const char *fmt, size_t size,
	long rc)
{
	if (size == (size_t) -1)
		size = 0;

	if (str) {
		printf("%s", str);
	} else {
		if (size == sizeof(uint64_t))
			printf(fmt, (uint64_t)arg);
		else if (size == sizeof(uint32_t))
			printf(fmt, (uint32_t)arg);
		else
			print_quoted_string_limit((void *) (uintptr_t) arg,
						  size, rc);
	}
}

/*
 * Arguments are passed as sz, val, str, fmt. Arguments are read until 4
 * arguments are retrieved or size of 0 is occurred.
 *
 * str == NULL && fmt == NULL && sz not in {4, 8} - print_quoted_string_limit is
 *   used for argument printing. If sz is negative, in argument is assumed, out
 *   otherwise.
 */
void
do_keyctl(kernel_ulong_t cmd, const char *cmd_str, ...)
{
	kernel_ulong_t args[4] = {
		(kernel_ulong_t) 0xdeadfee1badc0de5ULL,
		(kernel_ulong_t) 0xdeadfee2badc0de6ULL,
		(kernel_ulong_t) 0xdeadfee3badc0de7ULL,
		(kernel_ulong_t) 0xdeadfee4badc0de8ULL,
	};
	const char *arg_str[4] = { NULL };
	const char *arg_fmt[4] = { "%llu", "%llu", "%llu", "%llu" };
	size_t arg_sz[4] = {
		sizeof(kernel_ulong_t),
		sizeof(kernel_ulong_t),
		sizeof(kernel_ulong_t),
		sizeof(kernel_ulong_t),
	};
	unsigned i;
	unsigned cnt = 0;

	va_list ap;

	va_start(ap, cmd_str);

	do {
		arg_sz[cnt] = va_arg(ap, size_t);
		if (!arg_sz[cnt])
			break;

		if (arg_sz[cnt] == sizeof(uint64_t))
			args[cnt] = va_arg(ap, uint64_t);
		else if (arg_sz[cnt] == sizeof(uint32_t))
			args[cnt] = va_arg(ap, uint32_t);
		else
			args[cnt] = (uintptr_t) va_arg(ap, void *);

		arg_str[cnt] = va_arg(ap, char *);
		arg_fmt[cnt] = va_arg(ap, char *);
	} while (++cnt < 4);

	long rc = syscall(__NR_keyctl, cmd, args[0], args[1], args[2], args[3]);
	const char *errstr = sprintrc(rc);
	printf("keyctl(%s", cmd_str);
	for (i = 0; i < cnt; i++) {
		printf(", ");
		print_arg(args[i], arg_str[i], arg_fmt[i], arg_sz[i], rc);
	}
	printf(") = %s\n", errstr);
}

int
main(void)
{
	enum { PR_LIMIT = 10, IOV_SIZE = 11, IOV_STR_SIZE = 4096 };

	static const char *kulong_fmt =
		sizeof(kernel_ulong_t) == sizeof(uint64_t) ? "%#llx" : "%#x";
	static const char *ksize_fmt =
		sizeof(kernel_ulong_t) == sizeof(uint64_t) ? "%llu" : "%u";
	static const char *ptr_fmt =
		sizeof(void *) == sizeof(uint64_t) ? "%#llx" : "%#x";
	static const char unterminated1[] = { '\1', '\2', '\3', '\4', '\5' };
	static const char unterminated2[] = { '\6', '\7', '\10', '\11', '\12' };
	static const char short_type_str[] = "shrt type";
	static const char short_desc_str[] = "shrt desc";
	static const char long_type_str[] = "overly long key type";
	static const char long_desc_str[] = "overly long key description";
	static const int32_t bogus_key1 = 0xdeadf00d;
	static const int32_t bogus_key2 = 0x1eefdead;
	static const kernel_ulong_t bogus_key3 =
		(kernel_ulong_t) 0xdec0ded1dec0ded2ULL;
	static const char *bogus_key3_str = "-557785390";

	static const struct keyctl_dh_params kcdhp_data = {
		KEY_SPEC_GROUP_KEYRING, 1234567890, 3141592653U };
	static const char *kcdhp_str = "{private=KEY_SPEC_GROUP_KEYRING, "
		"prime=1234567890, base=-1153374643}";

	char *bogus_str = tail_memdup(unterminated1, sizeof(unterminated1));
	char *bogus_desc = tail_memdup(unterminated2, sizeof(unterminated2));
	char *short_type = tail_memdup(short_type_str, sizeof(short_type_str));
	char *short_desc = tail_memdup(short_desc_str, sizeof(short_desc_str));
	char *long_type = tail_memdup(long_type_str, sizeof(long_type_str));
	char *long_desc = tail_memdup(long_desc_str, sizeof(long_desc_str));
	char *kcdhp = tail_memdup(&kcdhp_data, sizeof(kcdhp_data));
	struct iovec *key_iov = tail_alloc(sizeof(*key_iov) * IOV_SIZE);
	char *bogus_buf1 = tail_alloc(9);
	char *bogus_buf2 = tail_alloc(256);
	char *key_iov_str1;
	char *key_iov_str2 = tail_alloc(4096);
	ssize_t ret;
	ssize_t kis_size = 0;
	int i;

	key_iov[0].iov_base = short_type;
	key_iov[0].iov_len = sizeof(short_type_str);
	key_iov[1].iov_base = long_type;
	key_iov[1].iov_len = sizeof(long_type_str);
	key_iov[2].iov_base = short_desc;
	key_iov[2].iov_len = sizeof(short_desc_str);
	key_iov[3].iov_base = long_desc;
	key_iov[3].iov_len = sizeof(long_desc_str);
	key_iov[4].iov_base = bogus_str;
	key_iov[4].iov_len = 32;

	for (i = 5; i < IOV_SIZE; i++) {
		key_iov[i].iov_base =
			(void *) (uintptr_t) (0xfffffacefffff00dULL +
			0x100000001ULL * i);
		key_iov[i].iov_len = (size_t) (0xcaffeeeddefaced7ULL +
			0x100000001ULL * i);
	}

	ret = asprintf(&key_iov_str1, "[{iov_base=%p, iov_len=%zu}, "
		       "{iov_base=%p, iov_len=%zu}, "
		       "{iov_base=%p, iov_len=%zu}, "
		       "{iov_base=%p, iov_len=%zu}]",
		       key_iov[IOV_SIZE - 4].iov_base,
		       key_iov[IOV_SIZE - 4].iov_len,
		       key_iov[IOV_SIZE - 3].iov_base,
		       key_iov[IOV_SIZE - 3].iov_len,
		       key_iov[IOV_SIZE - 2].iov_base,
		       key_iov[IOV_SIZE - 2].iov_len,
		       key_iov[IOV_SIZE - 1].iov_base,
		       key_iov[IOV_SIZE - 1].iov_len);

	if (ret < 0)
		error_msg_and_fail("asprintf");

	ret = snprintf(key_iov_str2, IOV_STR_SIZE,
		       "[{iov_base=\"%s\\0\", iov_len=%zu}, "
		       "{iov_base=\"%.10s\"..., iov_len=%zu}, "
		       "{iov_base=\"%s\\0\", iov_len=%zu}, "
		       "{iov_base=\"%.10s\"..., iov_len=%zu}, ",
		       (char *) key_iov[0].iov_base, key_iov[0].iov_len,
		       (char *) key_iov[1].iov_base, key_iov[1].iov_len,
		       (char *) key_iov[2].iov_base, key_iov[2].iov_len,
		       (char *) key_iov[3].iov_base, key_iov[3].iov_len);

	if ((ret < 0) || (ret >= IOV_STR_SIZE))
		error_msg_and_fail("snprintf");

	for (i = 4; i < PR_LIMIT; i++) {
		kis_size += ret;

		ret = snprintf(key_iov_str2 + kis_size, IOV_STR_SIZE - kis_size,
			       "{iov_base=%p, iov_len=%zu}, ",
			       key_iov[i].iov_base, key_iov[i].iov_len);

		if ((ret < 0) || (ret >= (IOV_STR_SIZE - kis_size)))
			error_msg_and_fail("snprintf");
	}

	kis_size += ret;
	snprintf(key_iov_str2 + kis_size, IOV_STR_SIZE - kis_size, "...]");


	/* Invalid command */
	do_keyctl((kernel_ulong_t) 0xbadc0dedfacefeedULL,
		  "0xfacefeed /* KEYCTL_??? */",
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee1badc0de5ULL, NULL, kulong_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee2badc0de6ULL, NULL, kulong_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee3badc0de7ULL, NULL, kulong_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee4badc0de8ULL, NULL, kulong_fmt);


	/* GET_KEYRING_ID */
	do_keyctl(ARG_STR(KEYCTL_GET_KEYRING_ID),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xbadc0dedffffffffLLU, "-1",
		  NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_KEYRING_ID),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), "%d",
		  sizeof(int), 3141592653U, NULL, "%d",
		  NULL, 0UL);


	/* KEYCTL_JOIN_SESSION_KEYRING */
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), ARG_STR(NULL), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), bogus_str, NULL, ptr_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), ARG_STR("bogus name"), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), "very long keyring name", "\"very long \"...",
		  NULL, 0UL);


	/* KEYCTL_UPDATE */

	buf_in_arg = true;

	do_keyctl(ARG_STR(KEYCTL_UPDATE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_UPDATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee4badc0de8ULL, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_UPDATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfee4badc0de8ULL, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_UPDATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(short_desc_str), short_desc, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) sizeof(short_desc_str) - 1, NULL,
			  ksize_fmt,
		  0UL);

	buf_in_arg = false;


	/* KEYCTL_REVOKE */
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_CHOWN */
	do_keyctl(ARG_STR(KEYCTL_CHOWN),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(uid_t), ARG_STR(-1), NULL,
		  sizeof(gid_t), ARG_STR(-1), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_CHOWN),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uid_t), 2718281828U, NULL, "%u",
		  sizeof(gid_t), 3141592653U, NULL, "%u", 0UL);


	/* KEYCTL_SETPERM */
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQKEY_AUTH_KEY), NULL,
		  sizeof(uint32_t), 0xffffffffU,
		  "KEY_POS_VIEW|KEY_POS_READ|KEY_POS_WRITE|"
		  "KEY_POS_SEARCH|KEY_POS_LINK|KEY_POS_SETATTR|"
		  "KEY_USR_VIEW|KEY_USR_READ|KEY_USR_WRITE|"
		  "KEY_USR_SEARCH|KEY_USR_LINK|KEY_USR_SETATTR|"
		  "KEY_GRP_VIEW|KEY_GRP_READ|KEY_GRP_WRITE|"
		  "KEY_GRP_SEARCH|KEY_GRP_LINK|KEY_GRP_SETATTR|"
		  "KEY_OTH_VIEW|KEY_OTH_READ|KEY_OTH_WRITE|"
		  "KEY_OTH_SEARCH|KEY_OTH_LINK|KEY_OTH_SETATTR|"
		  "0xc0c0c0c0", NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%#x", 0UL);
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(uint32_t), 0xc0c0c0c0, "0xc0c0c0c0 /* KEY_??? */",
			  NULL,
		  0UL);


	/* KEYCTL_DESCRIBE */
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 9, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 256, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) -4, NULL, ksize_fmt, 0UL);


	/* KEYCTL_CLEAR */
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_LINK */
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_UNLINK */
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_SEARCH */
	buf_in_arg = true;

	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(int32_t), 0, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(char *), (char *) 0xfffff00dfffff157ULL, NULL, ptr_fmt,
		  sizeof(int32_t), ARG_STR(KEY_SPEC_USER_SESSION_KEYRING),
			  NULL);
	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  sizeof(char *), bogus_desc, NULL, ptr_fmt,
		  sizeof(int32_t), bogus_key1, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(short_type_str), short_type, NULL, NULL,
		  sizeof(short_desc_str), short_desc, NULL, NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(long_type_str), long_type, NULL, NULL,
		  sizeof(long_type_str), long_desc, NULL, NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL);

	buf_in_arg = false;


	/* KEYCTL_READ */
	nul_terminated_buf = false;

	/* Empty result is expected for these */
	bogus_buf1[0] = '\377';
	bogus_buf2[0] = '\377';

	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 9, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 256, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) -4, NULL, ksize_fmt, 0UL);

	nul_terminated_buf = true;

	/* KEYCTL_INSTANTIATE */
	buf_in_arg = true;

	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  sizeof(int32_t), 0, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfeedLLU, NULL, ksize_fmt,
		  sizeof(int32_t), bogus_key1, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 32LLU, NULL, ksize_fmt,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(short_type_str), short_desc, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) sizeof(short_type_str) - 1, NULL,
			  ksize_fmt,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL);
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  sizeof(long_type_str), long_desc, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) sizeof(long_type_str), NULL, ksize_fmt,
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL);

	buf_in_arg = false;


	/* KEYCTL_NEGATE */
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%u",
		  sizeof(int32_t), 0, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 3141592653U, NULL, "%u",
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_SET_REQKEY_KEYRING */
	do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(int32_t), ARG_STR(KEY_REQKEY_DEFL_NO_CHANGE), NULL,
		  0UL);
	/*
	 * Keep it commented out until proper way of faking syscalls is not
	 * implemented.
	 */
	/* do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(int32_t),
		  ARG_STR(KEY_REQKEY_DEFL_REQUESTOR_KEYRING), NULL, 0UL); */
	do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU,
		  "0xbadc0ded /* KEY_REQKEY_DEFL_??? */", NULL, 0UL);


	/* KEYCTL_SET_TIMEOUT */
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%u", 0UL);
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 3141592653U, NULL, "%u", 0UL);
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  0UL);


	/* KEYCTL_ASSUME_AUTHORITY */
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_GET_SECURITY */
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(uint32_t), 0xbadc0dedU, NULL, "%u", 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 9, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 256, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) -4, NULL, ksize_fmt, 0UL);


	/* KEYCTL_SESSION_TO_PARENT */
	do_keyctl(ARG_STR(KEYCTL_SESSION_TO_PARENT), 0UL);


	/* KEYCTL_REJECT */
	do_keyctl(ARG_STR(KEYCTL_REJECT),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%u",
		  sizeof(uint32_t), 0, NULL, "%u",
		  sizeof(int32_t), 0, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_REJECT),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 3141592653U, NULL, "%u",
		  sizeof(uint32_t), 2718281828U, NULL, "%u",
		  sizeof(int32_t), bogus_key1, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_REJECT),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadca75facef157LLU, "4207866199", NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_REJECT),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  sizeof(uint32_t), ARG_STR(ENODEV), NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL);


	/* KEYCTL_INSTANTIATE_IOV */
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  sizeof(int32_t), 0, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xdeadfeedLLU, NULL, ksize_fmt,
		  sizeof(int32_t), bogus_key1, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), key_iov + IOV_SIZE, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 32LLU, NULL, ksize_fmt,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(key_iov), key_iov + IOV_SIZE - 4, key_iov_str1, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 4, NULL,
			  ksize_fmt,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL);
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  sizeof(key_iov), key_iov, key_iov_str2, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) IOV_SIZE, NULL, ksize_fmt,
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL);


	/* KEYCTL_INVALIDATE */
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_GET_PERSISTENT */
	do_keyctl(ARG_STR(KEYCTL_GET_PERSISTENT),
		  sizeof(uid_t), ARG_STR(-1), NULL,
		  sizeof(int32_t), ARG_STR(KEY_SPEC_GROUP_KEYRING), NULL, 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_PERSISTENT),
		  sizeof(uid_t), 2718281828U, NULL, "%u",
		  sizeof(int32_t), bogus_key1, NULL, "%d", 0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_PERSISTENT),
		  sizeof(uid_t), 2718281828U, NULL, "%u",
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_DH_COMPUTE */
	nul_terminated_buf = false;

	/* Empty result is expected for these */
	bogus_buf1[0] = '\377';
	bogus_buf2[0] = '\377';

	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(char *), kcdhp + 1, NULL, ptr_fmt,
		  sizeof(char *), (char *) 0xfffff157ffffdeadULL, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 9, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 256, NULL, ksize_fmt, 0UL);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) -1, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) -1, NULL, ksize_fmt, 0UL);

	nul_terminated_buf = true;

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_keyctl");

#endif
