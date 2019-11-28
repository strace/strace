/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_CRYPTOUSER_H

# include <stdio.h>
# include <stdint.h>
# include "test_nlattr.h"
# include <linux/cryptouser.h>

# define CRYPTOCFGA_REPORT_LARVAL 2

static void
init_crypto_user_alg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = CRYPTO_MSG_GETALG,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct crypto_user_alg *const alg = NLMSG_DATA(nlh);
	SET_STRUCT(struct crypto_user_alg, alg,
		.cru_name = "abcd",
		.cru_driver_name = "efgh",
		.cru_module_name = "ijkl",
	);
}

static void
print_crypto_user_alg(const unsigned int msg_len)
{
	printf("{len=%u, type=CRYPTO_MSG_GETALG"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {cru_name=\"abcd\", cru_driver_name=\"efgh\""
	       ", cru_module_name=\"ijkl\", cru_type=0"
	       ", cru_mask=0, cru_refcnt=0, cru_flags=0}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_CRYPTO);
	const unsigned int hdrlen = sizeof(struct crypto_user_alg);
	/*
	 * There are also other structures, but they are not bigger than
	 * DEFAULT_STRLEN so far.
	 */
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN + DEFAULT_STRLEN);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	char *const str = tail_alloc(DEFAULT_STRLEN);
	fill_memory_ex(str, DEFAULT_STRLEN, '0', 10);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_crypto_user_alg, print_crypto_user_alg,
		    CRYPTOCFGA_REPORT_LARVAL,
		    DEFAULT_STRLEN, str, DEFAULT_STRLEN,
		    printf("{type=\"%.*s\"...}", DEFAULT_STRLEN, str));
	str[DEFAULT_STRLEN - 1] = '\0';
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_crypto_user_alg, print_crypto_user_alg,
		    CRYPTOCFGA_REPORT_LARVAL,
		    DEFAULT_STRLEN, str, DEFAULT_STRLEN,
		    printf("{type=\"%s\"}", str));

# ifdef HAVE_STRUCT_CRYPTO_REPORT_HASH
	static const struct crypto_report_hash rhash = {
		.type = "efgh",
		.blocksize = 0xabcdefdc,
		.digestsize = 0xfebcdacd
	};
	TEST_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
			      init_crypto_user_alg, print_crypto_user_alg,
			      CRYPTOCFGA_REPORT_HASH,
			      pattern, rhash, sizeof(rhash),
			      print_quoted_memory,
			      printf("{type=\"efgh\"");
			      PRINT_FIELD_U(", ", rhash, blocksize);
			      PRINT_FIELD_U(", ", rhash, digestsize);
			      printf("}"));
# endif

# ifdef HAVE_STRUCT_CRYPTO_REPORT_BLKCIPHER
	static const struct crypto_report_blkcipher rblkcipher = {
		.type = "abcd",
		.geniv = "efgh",
		.blocksize = 0xabcdefac,
		.min_keysize = 0xfeadbcda,
		.max_keysize = 0xbdacdeac,
		.ivsize = 0xefacbdac
	};
	TEST_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
			      init_crypto_user_alg, print_crypto_user_alg,
			      CRYPTOCFGA_REPORT_BLKCIPHER,
			      pattern, rblkcipher, sizeof(rblkcipher),
			      print_quoted_memory,
			      printf("{type=\"abcd\", geniv=\"efgh\"");
			      PRINT_FIELD_U(", ", rblkcipher, blocksize);
			      PRINT_FIELD_U(", ", rblkcipher, min_keysize);
			      PRINT_FIELD_U(", ", rblkcipher, max_keysize);
			      PRINT_FIELD_U(", ", rblkcipher, ivsize);
			      printf("}"));
# endif

# ifdef HAVE_STRUCT_CRYPTO_REPORT_AEAD
	static const struct crypto_report_aead raead = {
		.type = "abcd",
		.geniv = "efgh",
		.blocksize = 0xbaefdbac,
		.maxauthsize = 0xfdbdbcda,
		.ivsize = 0xacbefdac
	};
	TEST_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
			      init_crypto_user_alg, print_crypto_user_alg,
			      CRYPTOCFGA_REPORT_AEAD,
			      pattern, raead, sizeof(raead),
			      print_quoted_memory,
			      printf("{type=\"abcd\", geniv=\"efgh\"");
			      PRINT_FIELD_U(", ", raead, blocksize);
			      PRINT_FIELD_U(", ", raead, maxauthsize);
			      PRINT_FIELD_U(", ", raead, ivsize);
			      printf("}"));
# endif

# ifdef HAVE_STRUCT_CRYPTO_REPORT_RNG
	static const struct crypto_report_rng rrng = {
		.type = "abcd",
		.seedsize = 0xabcdefac
	};
	TEST_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
			      init_crypto_user_alg, print_crypto_user_alg,
			      CRYPTOCFGA_REPORT_RNG,
			      pattern, rrng, sizeof(rrng), print_quoted_memory,
			      printf("{type=\"abcd\"");
			      PRINT_FIELD_U(", ", rrng, seedsize);
			      printf("}"));
# endif

# ifdef HAVE_STRUCT_CRYPTO_REPORT_CIPHER
	static const struct crypto_report_cipher rcipher = {
		.type = "abcd",
		.blocksize = 0xabcdefac,
		.min_keysize = 0xfeadbcda,
		.max_keysize = 0xbdacdeac,
	};
	TEST_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
			      init_crypto_user_alg, print_crypto_user_alg,
			      CRYPTOCFGA_REPORT_CIPHER,
			      pattern, rcipher, sizeof(rcipher),
			      print_quoted_memory,
			      printf("{type=\"abcd\"");
			      PRINT_FIELD_U(", ", rcipher, blocksize);
			      PRINT_FIELD_U(", ", rcipher, min_keysize);
			      PRINT_FIELD_U(", ", rcipher, max_keysize);
			      printf("}"));
# endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_CRYPTOUSER_H");

#endif
