/*
 * Check decoding of keyctl syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#ifdef __NR_keyctl

# include <linux/types.h>
# include <linux/keyctl.h>

# include <assert.h>
# include <errno.h>
# include <inttypes.h>
# include <stdarg.h>
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

# ifndef HAVE_STRUCT_KEYCTL_KDF_PARAMS
struct keyctl_kdf_params {
	char *hashname;
	char *otherinfo;
	uint32_t otherinfolen;
	uint32_t __spare[8];
};
# endif

# ifndef HAVE_STRUCT_KEYCTL_PKEY_QUERY
struct keyctl_pkey_query {
	uint32_t supported_ops;
	uint32_t key_size;
	uint16_t max_data_size;
	uint16_t max_sig_size;
	uint16_t max_enc_size;
	uint16_t max_dec_size;
	uint32_t __spare[10];
};
# endif

# ifndef HAVE_STRUCT_KEYCTL_PKEY_PARAMS
struct keyctl_pkey_params {
	int32_t  key_id;
	uint32_t in_len;
	union {
		uint32_t out_len;
		uint32_t in2_len;
	};
	uint32_t __spare[7];
};
# endif

# include "xlat.h"
# include "xlat/keyctl_caps0.h"
# include "xlat/keyctl_caps1.h"
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
bool buf_in_arg;

/* From ioctl_dm.c */
# define STR32 "AbCdEfGhIjKlMnOpQrStUvWxYz012345"

# if XLAT_RAW
#  define XARG_STR(v_) (v_), STRINGIFY(v_)
#  define XSTR(v_, s_) STRINGIFY(v_)
# elif XLAT_VERBOSE
#  define XARG_STR(v_) (v_), STRINGIFY(v_) " /* " #v_ " */"
#  define XSTR(v_, s_) STRINGIFY(v_) " /* " s_ " */"
# else
#  define XARG_STR ARG_STR
#  define XSTR(v_, s_) s_
# endif

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
		print_quoted_memory(str, limited_size);
		if (print_size > limit)
			printf("...");
	} else
		print_quoted_string(str);
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
			printf(fmt, (uint64_t) arg);
		else if (size == sizeof(uint32_t))
			printf(fmt, (uint32_t) arg);
		else
			print_quoted_string_limit((void *) (uintptr_t) arg,
						  size, rc);
	}
}

void
print_flags(const struct xlat *xlat, unsigned long long flags,
	    const char *const dflt)
{
# if XLAT_RAW
	printf("%#llx", flags);
# elif XLAT_VERBOSE
	printf("%#llx /* ", flags);
	printflags(xlat, flags, dflt);
	printf(" */");
# else
	printflags(xlat, flags, dflt);
# endif
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
# if XLAT_RAW
	printf("keyctl(%#x", (unsigned) cmd);
# elif XLAT_VERBOSE
	printf("keyctl(%#x /* %s */", (unsigned) cmd, cmd_str);
# else
	printf("keyctl(%s", cmd_str);
# endif
	for (i = 0; i < cnt; i++) {
		printf(", ");
		print_arg(args[i], arg_str[i], arg_fmt[i], arg_sz[i], rc);
	}
	printf(") = %s\n", errstr);
}

int
append_str(char **buf, size_t *left, const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsnprintf(*buf, *left, fmt, ap);
	va_end(ap);

	assert((ret >= 0) && ((unsigned) ret < *left));

	*left -= ret;
	*buf += ret;

	return ret;
}

const char *
kckdfp_to_str(struct keyctl_kdf_params *kdf, bool deref_hash, bool deref_oi,
	       bool print_spare, const char *hash_str, const char *oi_str)
{
	static char buf[4096];

	size_t left = sizeof(buf);
	char *pos = buf;

	append_str(&pos, &left, "{hashname=");

	if (deref_hash && hash_str) {
		append_str(&pos, &left, "%s", hash_str);
	} else if (!kdf->hashname) {
		append_str(&pos, &left, "NULL");
	} else if (deref_hash) {
		append_str(&pos, &left, "\"%.*s\"", limit, kdf->hashname);

		if (strnlen(kdf->hashname, limit + 1) > limit)
			append_str(&pos, &left, "...");
	} else {
		append_str(&pos, &left, "%p", kdf->hashname);
	}

	append_str(&pos, &left, ", otherinfo=");

	if (deref_oi && oi_str) {
		append_str(&pos, &left, "%s", oi_str);
	} else if (!kdf->otherinfo) {
		append_str(&pos, &left, "NULL");
	} else if (deref_oi) {
		append_str(&pos, &left, "\"%.*s\"", limit, kdf->otherinfo);

		if (strnlen(kdf->otherinfo, limit + 1) > limit)
			append_str(&pos, &left, "...");
	} else {
		append_str(&pos, &left, "%p", kdf->otherinfo);
	}

	append_str(&pos, &left, ", otherinfolen=%u", kdf->otherinfolen);

	if (print_spare) {
		size_t i;

		append_str(&pos, &left, ", __spare=[");

		for (i = 0; i < ARRAY_SIZE(kdf->__spare); i++) {
			if  (i)
				append_str(&pos, &left, ", ");

			append_str(&pos, &left, "%#x", kdf->__spare[i]);
		}

		append_str(&pos, &left, "]");
	}

	append_str(&pos, &left, "}");

	return buf;
}

const char *
kcpp_to_str(struct keyctl_pkey_params *params, bool out, const char *key_str,
	    bool print_spare)
{
	static char buf[4096];

	size_t left = sizeof(buf);
	char *pos = buf;

	append_str(&pos, &left, "{key_id=");

# if XLAT_RAW
	append_str(&pos, &left, "%d", params->key_id);
# elif XLAT_VERBOSE
	if (key_str)
		append_str(&pos, &left, "%d /* %s */", params->key_id, key_str);
	else
		append_str(&pos, &left, "%d", params->key_id);
# else
	if (key_str)
		append_str(&pos, &left, "%s", key_str);
	else
		append_str(&pos, &left, "%d", params->key_id);
# endif

	append_str(&pos, &left, ", in_len=%u, %s=%u",
		   params->in_len,
		   out ? "out_len" : "in2_len", params->out_len);

	if (print_spare) {
		append_str(&pos, &left, ", __spare=[");

		for (size_t i = 0; i < ARRAY_SIZE(params->__spare); i++) {
			append_str(&pos, &left, "%s%#x",
				   i ? ", " : "", params->__spare[i]);
		}

		append_str(&pos, &left, "]");
	}

	append_str(&pos, &left, "}");

	return buf;
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
		.private = KEY_SPEC_GROUP_KEYRING,
		.prime = 1234567890,
		.base = 3141592653U
	};
	static const char *kcdhp_str = "{private="
# if XLAT_RAW || XLAT_VERBOSE
		"-6"
# endif
# if XLAT_VERBOSE
		" /* "
# endif
# if !XLAT_RAW
		"KEY_SPEC_GROUP_KEYRING"
# endif
# if XLAT_VERBOSE
		" */"
# endif
		", prime=1234567890, base=-1153374643}";

	/*
	 * It's bigger than current hash name size limit, but since it's
	 * implementation-dependent and totally internal, we do not rely
	 * on it much.
	 */
	static const char long_hash_data[] = STR32 STR32 STR32 STR32 "xxx";
	static const char short_hash_data[] = "hmac(aes)";
	static const char otherinfo1_data[] = "\1\2 OH HAI THAR\255\0\1";
	static const char otherinfo2_data[] = "\1\2\n\255\0\1";
	static const struct keyctl_kdf_params kckdfp_data[] = {
		[0] = { NULL, NULL, 0, { 0 } },
		[1] = { NULL /* Changed to unaccessible address in copy */,
			NULL, 0xbadc0dedU, { [7] = 0xdeadfeedU } },
		[2] = { NULL /* long_hash_data */,
			NULL /* Changed to unaccessible address in copy */,
			0, { 0 } },
		[3] = { NULL /* unterminated1 */,
			NULL /* otherinfo_data */, 0, { 1 } },
		[4] = { NULL /* short_hash_data */,
			NULL /* otherinfo1_data */, sizeof(otherinfo1_data),
			{ 0, 0xfacebeef, 0, 0xba5e1ead } },
		[5] = { NULL /* short_hash_data */,
			NULL /* otherinfo2_data */, sizeof(otherinfo2_data),
			{ 0 } },
	};

	char *bogus_str = tail_memdup(unterminated1, sizeof(unterminated1));
	char *bogus_desc = tail_memdup(unterminated2, sizeof(unterminated2));
	char *short_type = tail_memdup(short_type_str, sizeof(short_type_str));
	char *short_desc = tail_memdup(short_desc_str, sizeof(short_desc_str));
	char *long_type = tail_memdup(long_type_str, sizeof(long_type_str));
	char *long_desc = tail_memdup(long_desc_str, sizeof(long_desc_str));
	char *kcdhp = tail_memdup(&kcdhp_data, sizeof(kcdhp_data));
	char *kckdfp_long_hash = tail_memdup(long_hash_data,
					     sizeof(long_hash_data));
	char *kckdfp_short_hash = tail_memdup(short_hash_data,
					      sizeof(short_hash_data));
	char *kckdfp_otherinfo1 = tail_memdup(otherinfo1_data,
					      sizeof(otherinfo1_data));
	char *kckdfp_otherinfo2 = tail_memdup(otherinfo2_data,
					      sizeof(otherinfo2_data));
	char *kckdfp_char = tail_alloc(sizeof(kckdfp_data[0]));
	struct iovec *key_iov = tail_alloc(sizeof(*key_iov) * IOV_SIZE);
	char *bogus_buf1 = tail_alloc(9);
	char *bogus_buf2 = tail_alloc(256);
	char *key_iov_str1;
	char *key_iov_str2 = tail_alloc(4096);
	const char *errstr;
	ssize_t ret;
	ssize_t kis_size = 0;
	long rc;
	size_t i;

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
# if XLAT_VERBOSE
		  "KEYCTL_???"
# else
		  "0xfacefeed"
#  if !XLAT_RAW
		  " /* KEYCTL_??? */"
#  endif
# endif
		  ,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xdeadfee1badc0de5ULL, NULL,
			kulong_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xdeadfee2badc0de6ULL, NULL,
			kulong_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xdeadfee3badc0de7ULL, NULL,
			kulong_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xdeadfee4badc0de8ULL, NULL,
			kulong_fmt);


	/* GET_KEYRING_ID */
	do_keyctl(ARG_STR(KEYCTL_GET_KEYRING_ID),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xbadc0dedffffffffLLU, "-1", NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_KEYRING_ID),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), "%d",
		  sizeof(int), 3141592653U, NULL, "%d", NULL,
		  0UL);


	/* KEYCTL_JOIN_SESSION_KEYRING */
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), ARG_STR(NULL), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), ARG_STR("bogus name"), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_JOIN_SESSION_KEYRING),
		  sizeof(char *), "very long keyring name", "\"very long \"...",
			NULL,
		  0UL);


	/* KEYCTL_UPDATE */

	buf_in_arg = true;

	do_keyctl(ARG_STR(KEYCTL_UPDATE),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 0, NULL, ksize_fmt,
		  0UL);
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
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_REVOKE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_CHOWN */
	do_keyctl(ARG_STR(KEYCTL_CHOWN),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(uid_t), ARG_STR(-1), NULL,
		  sizeof(gid_t), ARG_STR(-1), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_CHOWN),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uid_t), 2718281828U, NULL, "%u",
		  sizeof(gid_t), 3141592653U, NULL, "%u",
		  0UL);


	/* KEYCTL_SETPERM */
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQKEY_AUTH_KEY), NULL,
		  sizeof(uint32_t), 0xffffffffU,
# if XLAT_RAW || XLAT_VERBOSE
			"0xffffffff"
# endif
# if XLAT_VERBOSE
			" /* "
# endif
# if !XLAT_RAW
			"KEY_POS_VIEW|KEY_POS_READ|KEY_POS_WRITE|"
			"KEY_POS_SEARCH|KEY_POS_LINK|KEY_POS_SETATTR|"
			"KEY_USR_VIEW|KEY_USR_READ|KEY_USR_WRITE|"
			"KEY_USR_SEARCH|KEY_USR_LINK|KEY_USR_SETATTR|"
			"KEY_GRP_VIEW|KEY_GRP_READ|KEY_GRP_WRITE|"
			"KEY_GRP_SEARCH|KEY_GRP_LINK|KEY_GRP_SETATTR|"
			"KEY_OTH_VIEW|KEY_OTH_READ|KEY_OTH_WRITE|"
			"KEY_OTH_SEARCH|KEY_OTH_LINK|KEY_OTH_SETATTR|"
			"0xc0c0c0c0"
# endif
# if XLAT_VERBOSE
			" */"
# endif
			, NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%#x",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_SETPERM),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(uint32_t), 0xc0c0c0c0,
			  "0xc0c0c0c0"
# if !XLAT_RAW
			  " /* KEY_??? */"
# endif
			  ,
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
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 9, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 256, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_DESCRIBE),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) -4, NULL, ksize_fmt,
		  0UL);


	/* KEYCTL_CLEAR */
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_CLEAR),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_LINK */
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_LINK),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_UNLINK */
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_UNLINK),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_SEARCH */
	buf_in_arg = true;

	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(int32_t), 0, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_SEARCH),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(char *), (char *) 0xfffff00dfffff157ULL, NULL, ptr_fmt,
		  sizeof(int32_t), XARG_STR(KEY_SPEC_USER_SESSION_KEYRING),
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

	/* KEYCTL_RESTRICT_KEYRING */

	do_keyctl(ARG_STR(KEYCTL_RESTRICT_KEYRING),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_REQUESTOR_KEYRING), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
		  sizeof(char *), ARG_STR(NULL), NULL,
			  NULL);
	do_keyctl(ARG_STR(KEYCTL_RESTRICT_KEYRING),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), (char *) 0xfffffacefffffeedULL, NULL, ptr_fmt,
		  sizeof(char *), (char *) 0xfffff00dfffff157ULL, NULL, ptr_fmt,
			  NULL);
	do_keyctl(ARG_STR(KEYCTL_RESTRICT_KEYRING),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  sizeof(char *), bogus_desc, NULL, ptr_fmt,
			  NULL);
	do_keyctl(ARG_STR(KEYCTL_RESTRICT_KEYRING),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(short_type_str), short_type, NULL, NULL,
		  sizeof(short_desc_str), short_desc, NULL, NULL,
			  NULL);
	do_keyctl(ARG_STR(KEYCTL_RESTRICT_KEYRING),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(long_type_str), long_type, NULL, NULL,
		  sizeof(long_type_str), long_desc, NULL, NULL,
			  NULL);

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
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 9, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 256, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_READ),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) -4, NULL, ksize_fmt,
		  0UL);

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
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  sizeof(long_type_str), long_desc, NULL, NULL,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) sizeof(long_type_str), NULL, ksize_fmt,
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL);

	buf_in_arg = false;


	/* KEYCTL_NEGATE */
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%u",
		  sizeof(int32_t), 0, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 3141592653U, NULL, "%u",
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661",
			NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_NEGATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661", NULL,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_SET_REQKEY_KEYRING */
	do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(int32_t), XARG_STR(KEY_REQKEY_DEFL_NO_CHANGE), NULL,
		  0UL);
	/*
	 * Keep it commented out until proper way of faking syscalls is not
	 * implemented.
	 */
	/* do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(int32_t),
		  XARG_STR(KEY_REQKEY_DEFL_REQUESTOR_KEYRING), NULL, 0UL); */
	do_keyctl(ARG_STR(KEYCTL_SET_REQKEY_KEYRING),
		  sizeof(kernel_ulong_t),
		  (kernel_ulong_t) 0xfeedf157badc0dedLLU,
		  "-1159983635"
# if !XLAT_RAW
		  " /* KEY_REQKEY_DEFL_??? */"
# endif
		  , NULL, 0UL);


	/* KEYCTL_SET_TIMEOUT */
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(int32_t), 0, NULL, "%d",
		  sizeof(uint32_t), 0, NULL, "%u",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(uint32_t), 3141592653U, NULL, "%u",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_SET_TIMEOUT),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661",
			NULL,
		  0UL);


	/* KEYCTL_ASSUME_AUTHORITY */
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_ASSUME_AUTHORITY),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_GET_SECURITY */
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(uint32_t), 0xbadc0dedU, NULL, "%u",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 9, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 256, NULL, ksize_fmt,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_SECURITY),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), NULL,
		  (size_t) -4, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) -4, NULL, ksize_fmt,
		  0UL);


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
			(kernel_ulong_t) 0xdeadca75facef157LLU, "4207866199",
			NULL,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661",
			NULL,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_REJECT),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, "3134983661",
			NULL,
		  sizeof(uint32_t), XARG_STR(ENODEV), NULL,
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
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 0xdeadfeedLLU, NULL,
			ksize_fmt,
		  sizeof(int32_t), bogus_key1, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(char *), key_iov + IOV_SIZE, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 32LLU, NULL,
			ksize_fmt,
		  sizeof(int32_t), bogus_key2, NULL, "%d");
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  sizeof(key_iov), key_iov + IOV_SIZE - 4, key_iov_str1, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 4, NULL,
			  ksize_fmt,
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL);
	do_keyctl(ARG_STR(KEYCTL_INSTANTIATE_IOV),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  sizeof(key_iov), key_iov, key_iov_str2, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) IOV_SIZE, NULL,
			ksize_fmt,
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL);


	/* KEYCTL_INVALIDATE */
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_INVALIDATE),
		  sizeof(kernel_ulong_t), bogus_key3, bogus_key3_str, NULL,
		  0UL);


	/* KEYCTL_GET_PERSISTENT */
	do_keyctl(ARG_STR(KEYCTL_GET_PERSISTENT),
		  sizeof(uid_t), ARG_STR(-1), NULL,
		  sizeof(int32_t), XARG_STR(KEY_SPEC_GROUP_KEYRING), NULL,
		  0UL);
	do_keyctl(ARG_STR(KEYCTL_GET_PERSISTENT),
		  sizeof(uid_t), 2718281828U, NULL, "%u",
		  sizeof(int32_t), bogus_key1, NULL, "%d",
		  0UL);
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
		  sizeof(char *), ARG_STR(NULL), ptr_fmt);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(char *), kcdhp + 1, NULL, ptr_fmt,
		  sizeof(char *), (char *) 0xfffff157ffffdeadULL, NULL, ptr_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL, ksize_fmt,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) 9, (uintptr_t) bogus_buf1, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 9, NULL, ksize_fmt,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) 256, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 256, NULL, ksize_fmt,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt);
	do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
		  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
		  (size_t) -1, (uintptr_t) bogus_buf2, NULL, NULL,
		  sizeof(kernel_ulong_t), (kernel_ulong_t) -1, NULL, ksize_fmt,
		  sizeof(char *), kckdfp_char + 1, NULL, ptr_fmt);

	/* KEYCTL_DH_COMPUTE + KDF */

	for (i = 0; i < ARRAY_SIZE(kckdfp_data); i++) {
		struct keyctl_kdf_params *kckdfp =
			(struct keyctl_kdf_params *) kckdfp_char;
		bool deref_hash = true;
		bool deref_opts = true;
		bool print_spare = false;
		const char *hash_str = NULL;
		const char *oi_str = NULL;

		memcpy(kckdfp, kckdfp_data + i, sizeof(kckdfp_data[i]));

		switch (i) {
		case 1:
			deref_hash = false;
			print_spare = true;
			kckdfp->hashname =
				kckdfp_short_hash + sizeof(short_hash_data);
			break;
		case 2:
			deref_opts = false;
			kckdfp->hashname = kckdfp_long_hash;
			kckdfp->otherinfo =
				kckdfp_otherinfo1 + sizeof(otherinfo1_data);
			break;
		case 3:
			deref_opts = false;
			deref_hash = false;
			print_spare = true;
			kckdfp->hashname = bogus_str;
			kckdfp->otherinfo = kckdfp_otherinfo1;
			break;
		case 4:
			oi_str = "\"\\1\\2 OH HAI \"...";
			print_spare = true;
			kckdfp->hashname = kckdfp_short_hash;
			kckdfp->otherinfo = kckdfp_otherinfo1;
			break;
		case 5:
			oi_str = "\"\\1\\2\\n\\255\\0\\1\\0\"";
			kckdfp->hashname = kckdfp_short_hash;
			kckdfp->otherinfo = kckdfp_otherinfo2;
			break;
		}

		do_keyctl(ARG_STR(KEYCTL_DH_COMPUTE),
			  sizeof(kcdhp), kcdhp, kcdhp_str, NULL,
			  (size_t) -1, (uintptr_t) bogus_buf2, NULL, NULL,
			  sizeof(kernel_ulong_t), (kernel_ulong_t) -1, NULL,
				ksize_fmt,
			  sizeof(kckdfp), kckdfp_char,
				kckdfp_to_str(kckdfp, deref_hash, deref_opts,
					      print_spare, hash_str, oi_str),
				NULL);
	}

	nul_terminated_buf = true;

	/* KEYCTL_PKEY_QUERY */
	do_keyctl(ARG_STR(KEYCTL_PKEY_QUERY),
		  sizeof(int32_t), bogus_key2, NULL, "%d",
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL,
			kulong_fmt,
		  sizeof(char *), bogus_str, NULL, ptr_fmt,
		  sizeof(char *), ARG_STR(NULL), ptr_fmt);

	struct keyctl_pkey_query query;
	do_keyctl(ARG_STR(KEYCTL_PKEY_QUERY),
		  sizeof(int32_t), XARG_STR(KEY_SPEC_THREAD_KEYRING), "%d",
		  sizeof(kernel_ulong_t), (kernel_ulong_t) 0, NULL, kulong_fmt,
		  sizeof(char *), "x\377\0\1", "\"x\\377\"", NULL,
		  sizeof(&query), &query, NULL, ptr_fmt);

	/*
	 * KEYCTL_PKEY_ENCRYPT, KEYCTL_PKEY_DECRYPT, KEYCTL_PKEY_SIGN,
	 * KEYCTL_PKEY_VERIFY
	 */
	static const struct {
		int op;
		const char *str;
		bool out;
	} pkey_ops[] = {
		{ ARG_STR(KEYCTL_PKEY_ENCRYPT),	true },
		{ ARG_STR(KEYCTL_PKEY_DECRYPT),	true },
		{ ARG_STR(KEYCTL_PKEY_SIGN),	true },
		{ ARG_STR(KEYCTL_PKEY_VERIFY),	false },
	};
	static const char pkey_str1[] = STR32 "xxx";
	static const char pkey_str2[] = "\1\2HAI\255\0\1";
	static struct {
		struct keyctl_pkey_params params;
		const char * key_str;
		bool print_spare;
		const char *str1;
		const char *str2;
	} pkey_vecs[] = {
		{ { KEY_SPEC_PROCESS_KEYRING, 0, { .out_len = 0 } },
		  "KEY_SPEC_PROCESS_KEYRING", false, "\"\"", "\"\"" },
		{ { 0, 0, { .out_len = 0 }, .__spare = { 1 } },
		  NULL, true, "\"\"", "\"\"" },
		{ { 0xdeadc0deU, 10, { .out_len = 10 },
		    .__spare = { 0, 0xfacefeed } },
		  NULL, true, "\"AbCdEfGhIj\"", NULL },
		{ { 0xdeadc0deU, sizeof(pkey_str1),
		    { .out_len = sizeof(pkey_str2) - 1 },
		    .__spare = { [6] = 0xdec0ded } },
		  NULL, true,
		  "\"AbCdEfGhIj\"...", "\"\\1\\2HAI\\255\\0\\1\"" },
	};

	char *pkey1 = tail_memdup(pkey_str1, sizeof(pkey_str1) - 1);
	char *pkey2 = tail_memdup(pkey_str2, sizeof(pkey_str2) - 1);
	struct keyctl_pkey_params *pkey_params =
		tail_alloc(sizeof(*pkey_params));

	for (i = 0; i < ARRAY_SIZE(pkey_ops); i++) {
		do_keyctl(pkey_ops[i].op, pkey_ops[i].str,
			  sizeof(char *), ARG_STR(NULL), ptr_fmt,
			  sizeof(char *), ARG_STR(NULL), ptr_fmt,
			  sizeof(char *), ARG_STR(NULL), ptr_fmt,
			  sizeof(char *), ARG_STR(NULL), ptr_fmt);

		do_keyctl(pkey_ops[i].op, pkey_ops[i].str,
			  sizeof(char *), (uint32_t *) pkey_params + 1, NULL,
				ptr_fmt,
			  sizeof(char *), "x\377\0\1", "\"x\\377\"", ptr_fmt,
			  sizeof(char *), pkey1, NULL, ptr_fmt,
			  sizeof(char *), pkey2, NULL, ptr_fmt);

		for (size_t j = 0; j < ARRAY_SIZE(pkey_vecs); j++) {
			memcpy(pkey_params, &pkey_vecs[j].params,
			       sizeof(*pkey_params));
			do_keyctl(pkey_ops[i].op, pkey_ops[i].str,
				  sizeof(char *), pkey_params,
					kcpp_to_str(pkey_params,
						    pkey_ops[i].out,
						    pkey_vecs[j].key_str,
						    pkey_vecs[j].print_spare),
					ptr_fmt,
				  sizeof(char *), "", "\"\"", ptr_fmt,
				  sizeof(char *), pkey1, pkey_vecs[j].str1,
					ptr_fmt,
				  sizeof(char *), pkey2,
					pkey_ops[i].out ? NULL
							: pkey_vecs[j].str2,
					ptr_fmt);
		}
	}

	/* KEYCTL_MOVE */
	static const struct {
		kernel_ulong_t key;
		const char *str;
	} move_keys[] = {
		  { 0xbadc0ded, "-1159983635" },
		  { XARG_STR(KEY_SPEC_THREAD_KEYRING) },
	};
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} move_flags[] = {
		{ (kernel_ulong_t) 0xbadc0ded00000000ULL, "0" },
		{ 1, XSTR(0x1, "KEYCTL_MOVE_EXCL") },
		{ (kernel_ulong_t) 0xbadc0ded00000001ULL,
		  XSTR(0x1, "KEYCTL_MOVE_EXCL") },
		{ (kernel_ulong_t) 0xfffffffffffffffeULL,
# if !XLAT_RAW
		  "0xfffffffe /* KEYCTL_MOVE_??? */"
# else
		  "0xfffffffe"
# endif
		 },
		{ (kernel_ulong_t) 0xffffffffffffffffULL,
		  XSTR(0xffffffff, "KEYCTL_MOVE_EXCL|0xfffffffe") },
	};

	for (i = 0; i < ARRAY_SIZE(move_keys) * ARRAY_SIZE(move_flags); i++) {
		do_keyctl(ARG_STR(KEYCTL_MOVE),
			  sizeof(kernel_ulong_t),
				move_keys[i % ARRAY_SIZE(move_keys)].key,
				move_keys[i % ARRAY_SIZE(move_keys)].str,
				kulong_fmt,
			  sizeof(kernel_ulong_t),
				move_keys[(i + 1) % ARRAY_SIZE(move_keys)].key,
				move_keys[(i + 1) % ARRAY_SIZE(move_keys)].str,
				kulong_fmt,
			  sizeof(kernel_ulong_t),
				move_keys[(i + 2) % ARRAY_SIZE(move_keys)].key,
				move_keys[(i + 2) % ARRAY_SIZE(move_keys)].str,
				kulong_fmt,
			  sizeof(kernel_ulong_t),
				move_flags[i % ARRAY_SIZE(move_flags)].val,
				move_flags[i % ARRAY_SIZE(move_flags)].str,
				kulong_fmt);
	}

	/* KEYCTL_CAPABILITIES */
	unsigned char *caps1 = tail_alloc(1);
	unsigned char *caps2 = tail_alloc(2);
	unsigned char *caps4 = tail_alloc(4);

	do_keyctl(ARG_STR(KEYCTL_CAPABILITIES),
		  sizeof(unsigned char *), ARG_STR(NULL), ptr_fmt,
		  sizeof(kernel_ulong_t),
			(kernel_ulong_t) 0xfeedf157badc0dedLLU, NULL,
			ksize_fmt,
		  0);

	const kernel_ulong_t bad_len = (kernel_ulong_t) 0xbadc0ded00000001LLU;
	rc = syscall(__NR_keyctl, KEYCTL_CAPABILITIES, caps1, bad_len);
	errstr = sprintrc(rc);
	printf("keyctl(" XSTR(0x1f, "KEYCTL_CAPABILITIES") ", ");
	if (rc >= 0) {
		printf("[");
		if (rc >= 1)
			print_flags(keyctl_caps0, caps1[0], "KEYCTL_CAPS0_???");
		printf("]");
	} else {
		printf("%p", caps1);
	}
	printf(", %llu) = %s\n", (unsigned long long) bad_len, errstr);

	rc = syscall(__NR_keyctl, KEYCTL_CAPABILITIES, caps1, 2);
	errstr = sprintrc(rc);
	printf("keyctl(" XSTR(0x1f, "KEYCTL_CAPABILITIES") ", ");
	if (rc >= 0) {
		printf("[");
		if (rc == 1)
			print_flags(keyctl_caps0, caps1[0], "KEYCTL_CAPS0_???");
		printf("]");
	} else {
		printf("%p", caps1);
	}
	printf(", 2) = %s\n", errstr);

	rc = syscall(__NR_keyctl, KEYCTL_CAPABILITIES, caps2, 2);
	errstr = sprintrc(rc);
	printf("keyctl(" XSTR(0x1f, "KEYCTL_CAPABILITIES") ", ");
	if (rc >= 0) {
		printf("[");
		if (rc >= 1)
			print_flags(keyctl_caps0, caps2[0], "KEYCTL_CAPS0_???");
		if (rc >= 2) {
			printf(", ");
			print_flags(keyctl_caps1, caps2[1], "KEYCTL_CAPS1_???");
		}
		printf("]");
	} else {
		printf("%p", caps2);
	}
	printf(", 2) = %s\n", errstr);

	rc = syscall(__NR_keyctl, KEYCTL_CAPABILITIES, caps4, 4);
	errstr = sprintrc(rc);
	printf("keyctl(" XSTR(0x1f, "KEYCTL_CAPABILITIES") ", ");
	if (rc >= 0) {
		printf("[");
		if (rc >= 1)
			print_flags(keyctl_caps0, caps4[0], "KEYCTL_CAPS0_???");
		if (rc >= 2) {
			printf(", ");
			print_flags(keyctl_caps1, caps4[1], "KEYCTL_CAPS1_???");
		}
		if (rc >= 3)
			printf(", %hhx", caps4[2]);
		if (rc >= 4)
			printf(", %hhx", caps4[3]);
		printf("]");
	} else {
		printf("%p", caps4);
	}
	printf(", 4) = %s\n", errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_keyctl");

#endif
