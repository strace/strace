/*
 * Copyright (c) 2017-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KEYCTL_KDF_PARAMS_H
# define STRACE_KEYCTL_KDF_PARAMS_H

# include <stdint.h>
# include "kernel_types.h"
# include <linux/keyctl.h>

/* from include/linux/crypto.h */
# define CRYPTO_MAX_ALG_NAME		128

/* from security/keys/internal.h */
# define KEYCTL_KDF_MAX_OI_LEN		64      /* max length of otherinfo */

struct strace_keyctl_kdf_params {
	kernel_ulong_t hashname;
	kernel_ulong_t otherinfo;
	uint32_t otherinfolen;
	uint32_t __spare[8];
};

#endif /* STRACE_KEYCTL_KDF_PARAMS_H */
