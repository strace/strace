#ifndef STRACE_KEYCTL_KDF_PARAMS_H
#define STRACE_KEYCTL_KDF_PARAMS_H

#include <stdint.h>
#include "kernel_types.h"

/* from include/linux/crypto.h */
#define CRYPTO_MAX_ALG_NAME		128

/* from security/keys/internal.h */
#define KEYCTL_KDF_MAX_OI_LEN		64      /* max length of otherinfo */

struct keyctl_kdf_params {
	char *hashname;
	char *otherinfo;
	uint32_t otherinfolen;
	uint32_t __spare[8];
};

struct strace_keyctl_kdf_params {
	kernel_ulong_t hashname;
	kernel_ulong_t otherinfo;
	uint32_t otherinfolen;
	uint32_t __spare[8];
};

#endif /* STRACE_KEYCTL_KDF_PARAMS_H */
