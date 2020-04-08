/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "netlink.h"
#include "nlattr.h"
#include "print_fields.h"

#include "types/cryptouser.h"

#include "xlat/crypto_nl_attrs.h"

#define XLAT_MACROS_ONLY
# include "xlat/crypto_msgs.h"
#undef XLAT_MACROS_ONLY


static bool
decode_crypto_report_generic(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
	tprints("{type=");
	printstr_ex(tcp, addr, len, QUOTE_0_TERMINATED);
	tprints("}");

	return true;
}

static bool
decode_crypto_report_hash(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	struct_crypto_report_hash rhash;

	if (len < sizeof(rhash))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &rhash)) {
		PRINT_FIELD_CSTRING("{", rhash, type);
		PRINT_FIELD_U(", ", rhash, blocksize);
		PRINT_FIELD_U(", ", rhash, digestsize);
		tprints("}");
	}

	return true;
}

static bool
decode_crypto_report_blkcipher(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       const void *const opaque_data)
{
	struct_crypto_report_blkcipher rblkcipher;

	if (len < sizeof(rblkcipher))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &rblkcipher)) {
		PRINT_FIELD_CSTRING("{", rblkcipher, type);
		PRINT_FIELD_CSTRING(", ", rblkcipher, geniv);
		PRINT_FIELD_U(", ", rblkcipher, blocksize);
		PRINT_FIELD_U(", ", rblkcipher, min_keysize);
		PRINT_FIELD_U(", ", rblkcipher, max_keysize);
		PRINT_FIELD_U(", ", rblkcipher, ivsize);
		tprints("}");
	}

	return true;
}

static bool
decode_crypto_report_aead(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	struct_crypto_report_aead raead;

	if (len < sizeof(raead))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &raead)) {
		PRINT_FIELD_CSTRING("{", raead, type);
		PRINT_FIELD_CSTRING(", ", raead, geniv);
		PRINT_FIELD_U(", ", raead, blocksize);
		PRINT_FIELD_U(", ", raead, maxauthsize);
		PRINT_FIELD_U(", ", raead, ivsize);
		tprints("}");
	}

	return true;
}

static bool
decode_crypto_report_rng(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct_crypto_report_rng rrng;

	if (len < sizeof(rrng))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &rrng)) {
		PRINT_FIELD_CSTRING("{", rrng, type);
		PRINT_FIELD_U(", ", rrng, seedsize);
		tprints("}");
	}

	return true;
}

static bool
decode_crypto_report_cipher(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int len,
			    const void *const opaque_data)
{
	struct_crypto_report_cipher rcipher;

	if (len < sizeof(rcipher))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &rcipher)) {
		PRINT_FIELD_CSTRING("{", rcipher, type);
		PRINT_FIELD_U(", ", rcipher, blocksize);
		PRINT_FIELD_U(", ", rcipher, min_keysize);
		PRINT_FIELD_U(", ", rcipher, max_keysize);
		tprints("}");
	}

	return true;
}

static const nla_decoder_t crypto_user_alg_nla_decoders[] = {
	[CRYPTOCFGA_PRIORITY_VAL]	= decode_nla_u32,
	[CRYPTOCFGA_REPORT_LARVAL]	= decode_crypto_report_generic,
	[CRYPTOCFGA_REPORT_HASH]	= decode_crypto_report_hash,
	[CRYPTOCFGA_REPORT_BLKCIPHER]	= decode_crypto_report_blkcipher,
	[CRYPTOCFGA_REPORT_AEAD]	= decode_crypto_report_aead,
	[CRYPTOCFGA_REPORT_COMPRESS]	= decode_crypto_report_generic,
	[CRYPTOCFGA_REPORT_RNG]		= decode_crypto_report_rng,
	[CRYPTOCFGA_REPORT_CIPHER]	= decode_crypto_report_cipher,
	[CRYPTOCFGA_REPORT_AKCIPHER]	= decode_crypto_report_generic,
	[CRYPTOCFGA_REPORT_KPP]		= decode_crypto_report_generic,
	[CRYPTOCFGA_REPORT_ACOMP]	= decode_crypto_report_generic
};

static void
decode_crypto_user_alg(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len)
{
	struct_crypto_user_alg alg;

	if (len < sizeof(alg))
		printstrn(tcp, addr, len);
	else if (!umove_or_printaddr(tcp, addr, &alg)) {
		PRINT_FIELD_CSTRING("{", alg, cru_name);
		PRINT_FIELD_CSTRING(", ", alg, cru_driver_name);
		PRINT_FIELD_CSTRING(", ", alg, cru_module_name);
		PRINT_FIELD_X(", ", alg, cru_type);
		PRINT_FIELD_X(", ", alg, cru_mask);
		PRINT_FIELD_U(", ", alg, cru_refcnt);
		PRINT_FIELD_X(", ", alg, cru_flags);
		tprints("}");

		const size_t offset = NLMSG_ALIGN(sizeof(alg));
		if (len > offset) {
			tprints(", ");
			decode_nlattr(tcp, addr + offset, len - offset,
				      crypto_nl_attrs, "CRYPTOCFGA_???",
				      crypto_user_alg_nla_decoders,
				      ARRAY_SIZE(crypto_user_alg_nla_decoders),
				      NULL);
		}
	}
}

bool
decode_netlink_crypto(struct tcb *const tcp,
		      const struct nlmsghdr *const nlmsghdr,
		      const kernel_ulong_t addr,
		      const unsigned int len)
{
	switch (nlmsghdr->nlmsg_type) {
	case CRYPTO_MSG_NEWALG:
	case CRYPTO_MSG_DELALG:
	case CRYPTO_MSG_UPDATEALG:
	case CRYPTO_MSG_GETALG:
		decode_crypto_user_alg(tcp, addr, len);
		break;
	default:
		return false;
	}

	return true;
}
