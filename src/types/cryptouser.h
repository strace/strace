/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_CRYPTOUSER_H
# define STRACE_TYPES_CRYPTOUSER_H

# ifdef HAVE_LINUX_CRYPTOUSER_H
#  include <linux/cryptouser.h>
# endif

# ifndef CRYPTO_MAX_NAME
#  define CRYPTO_MAX_NAME 64
# endif

typedef struct {
	char cru_name[CRYPTO_MAX_NAME];
	char cru_driver_name[CRYPTO_MAX_NAME];
	char cru_module_name[CRYPTO_MAX_NAME];
	uint32_t cru_type;
	uint32_t cru_mask;
	int32_t cru_refcnt;
	uint32_t cru_flags;
} struct_crypto_user_alg;

typedef struct {
	char type[CRYPTO_MAX_NAME];
	uint32_t blocksize;
	uint32_t digestsize;
} struct_crypto_report_hash;

typedef struct {
	char type[CRYPTO_MAX_NAME];
	uint32_t blocksize;
	uint32_t min_keysize;
	uint32_t max_keysize;
} struct_crypto_report_cipher;

typedef struct {
	char type[CRYPTO_MAX_NAME];
	char geniv[CRYPTO_MAX_NAME];
	uint32_t blocksize;
	uint32_t min_keysize;
	uint32_t max_keysize;
	uint32_t ivsize;
} struct_crypto_report_blkcipher;

typedef struct {
	char type[CRYPTO_MAX_NAME];
	char geniv[CRYPTO_MAX_NAME];
	uint32_t blocksize;
	uint32_t maxauthsize;
	uint32_t ivsize;
} struct_crypto_report_aead;

typedef struct {
	char type[CRYPTO_MAX_NAME];
	uint32_t seedsize;
} struct_crypto_report_rng;

#endif /* !STRACE_TYPES_CRYPTOUSER_H */
