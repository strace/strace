/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
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

#include "defs.h"

#ifdef HAVE_LINUX_CRYPTOUSER_H

# include "netlink.h"
# include "nlattr.h"
# include "print_fields.h"

# include <linux/cryptouser.h>

static void
decode_crypto_user_alg(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len)
{
	struct crypto_user_alg alg;

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

#endif /* HAVE_LINUX_CRYPTOUSER_H */
