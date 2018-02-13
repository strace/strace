/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
#include "netlink.h"
#include "netlink_sock_diag.h"
#include "nlattr.h"
#include "print_fields.h"

#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

#include "xlat/unix_diag_attrs.h"
#include "xlat/unix_diag_show.h"

DECL_NETLINK_DIAG_DECODER(decode_unix_diag_req)
{
	struct unix_diag_req req = { .sdiag_family = family };
	const size_t offset = sizeof(req.sdiag_family);

	PRINT_FIELD_XVAL("{", req, sdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (char *) &req + offset)) {
			PRINT_FIELD_U("", req, sdiag_protocol);
			PRINT_FIELD_FLAGS(", ", req, udiag_states,
					  tcp_state_flags, "1<<TCP_???");
			PRINT_FIELD_U(", ", req, udiag_ino);
			PRINT_FIELD_FLAGS(", ", req, udiag_show,
					  unix_diag_show, "UDIAG_SHOW_???");
			PRINT_FIELD_COOKIE(", ", req, udiag_cookie);
		}
	} else
		tprints("...");
	tprints("}");
}

static bool
decode_unix_diag_vfs(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct unix_diag_vfs uv;

	if (len < sizeof(uv))
		return false;
	if (umove_or_printaddr(tcp, addr, &uv))
		return true;

	PRINT_FIELD_DEV("{", uv, udiag_vfs_dev);
	PRINT_FIELD_U(", ", uv, udiag_vfs_ino);
	tprints("}");

	return true;
}

static bool
print_inode(struct tcb *const tcp,
	    void *const elem_buf,
	    const size_t elem_size,
	    void *const opaque_data)
{
	tprintf("%" PRIu32, *(uint32_t *) elem_buf);

	return true;
}

static bool
decode_unix_diag_inode(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	uint32_t inode;
	const size_t nmemb = len / sizeof(inode);

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &inode, sizeof(inode),
		    umoven_or_printaddr, print_inode, 0);

	return true;
}

static bool
decode_unix_diag_rqlen(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct unix_diag_rqlen rql;

	if (len < sizeof(rql))
		return false;
	if (umove_or_printaddr(tcp, addr, &rql))
		return true;

	PRINT_FIELD_U("{", rql, udiag_rqueue);
	PRINT_FIELD_U(", ", rql, udiag_wqueue);
	tprints("}");

	return true;
}

static const nla_decoder_t unix_diag_msg_nla_decoders[] = {
	[UNIX_DIAG_NAME]	= decode_nla_str,
	[UNIX_DIAG_VFS]		= decode_unix_diag_vfs,
	[UNIX_DIAG_PEER]	= decode_nla_u32,
	[UNIX_DIAG_ICONS]	= decode_unix_diag_inode,
	[UNIX_DIAG_RQLEN]	= decode_unix_diag_rqlen,
	[UNIX_DIAG_MEMINFO]	= decode_nla_meminfo,
	[UNIX_DIAG_SHUTDOWN]	= decode_nla_u8
};

DECL_NETLINK_DIAG_DECODER(decode_unix_diag_msg)
{
	struct unix_diag_msg msg = { .udiag_family = family };
	size_t offset = sizeof(msg.udiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, udiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (char *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, udiag_type,
					 socktypes, "SOCK_???");
			PRINT_FIELD_XVAL(", ", msg, udiag_state,
					 tcp_states, "TCP_???");
			PRINT_FIELD_U(", ", msg, udiag_ino);
			PRINT_FIELD_COOKIE(", ", msg, udiag_cookie);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      unix_diag_attrs, "UNIX_DIAG_???",
			      unix_diag_msg_nla_decoders,
			      ARRAY_SIZE(unix_diag_msg_nla_decoders), NULL);
	}
}
