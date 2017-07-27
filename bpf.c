/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017 Quentin Monnet <quentin.monnet@6wind.com>
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
#include "print_fields.h"

#ifdef HAVE_LINUX_BPF_H
# include <linux/bpf.h>
#endif

#include "xlat/bpf_commands.h"
#include "xlat/bpf_map_types.h"
#include "xlat/bpf_map_flags.h"
#include "xlat/bpf_prog_types.h"
#include "xlat/bpf_prog_flags.h"
#include "xlat/bpf_map_update_elem_flags.h"
#include "xlat/bpf_attach_type.h"
#include "xlat/bpf_attach_flags.h"

#define DECL_BPF_CMD_DECODER(bpf_cmd_decoder)				\
int									\
bpf_cmd_decoder(struct tcb *const tcp,					\
		const kernel_ulong_t addr,				\
		const unsigned int size,				\
		void *const data)					\
/* End of DECL_BPF_CMD_DECODER definition. */

#define DEF_BPF_CMD_DECODER(bpf_cmd)					\
	static DECL_BPF_CMD_DECODER(decode_ ## bpf_cmd)

#define BPF_CMD_ENTRY(bpf_cmd)						\
	[bpf_cmd] = decode_ ## bpf_cmd

typedef DECL_BPF_CMD_DECODER((*bpf_cmd_decoder_t));

static int
decode_attr_extra_data(struct tcb *const tcp,
		       const char *data,
		       unsigned int size,
		       const size_t attr_size)
{
	if (size <= attr_size)
		return 0;

	data += attr_size;
	size -= attr_size;

	unsigned int i;
	for (i = 0; i < size; ++i) {
		if (data[i]) {
			tprints(", ");
			if (abbrev(tcp))
				tprints("...");
			else
				print_quoted_string(data, size,
						    QUOTE_FORCE_HEX);
			return RVAL_DECODED;
		}
	}

	return 0;
}

DEF_BPF_CMD_DECODER(BPF_MAP_CREATE)
{
	struct {
		uint32_t map_type, key_size, value_size, max_entries,
			 map_flags, inner_map_fd;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_XVAL("{", attr, map_type, bpf_map_types,
			 "BPF_MAP_TYPE_???");
	PRINT_FIELD_U(", ", attr, key_size);
	PRINT_FIELD_U(", ", attr, value_size);
	PRINT_FIELD_U(", ", attr, max_entries);
	PRINT_FIELD_FLAGS(", ", attr, map_flags, bpf_map_flags, "BPF_F_???");
	PRINT_FIELD_FD(", ", attr, inner_map_fd, tcp);
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

DEF_BPF_CMD_DECODER(BPF_MAP_LOOKUP_ELEM)
{
	struct bpf_io_elem_struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key, value;
	} attr = {};

	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	PRINT_FIELD_X(", ", attr, value);
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

DEF_BPF_CMD_DECODER(BPF_MAP_UPDATE_ELEM)
{
	struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key;
		uint64_t ATTRIBUTE_ALIGNED(8) value;
		uint64_t flags;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	PRINT_FIELD_X(", ", attr, value);
	PRINT_FIELD_XVAL(", ", attr, flags, bpf_map_update_elem_flags,
			 "BPF_???");
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

DEF_BPF_CMD_DECODER(BPF_MAP_DELETE_ELEM)
{
	struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

DEF_BPF_CMD_DECODER(BPF_MAP_GET_NEXT_KEY)
{
	struct bpf_io_elem_struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key, next_key;
	} attr = {};

	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	PRINT_FIELD_X(", ", attr, next_key);
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

DEF_BPF_CMD_DECODER(BPF_PROG_LOAD)
{
	struct bpf_prog_load {
		uint32_t prog_type, insn_cnt;
		uint64_t ATTRIBUTE_ALIGNED(8) insns, license;
		uint32_t log_level, log_size;
		uint64_t ATTRIBUTE_ALIGNED(8) log_buf;
		uint32_t kern_version, prog_flags;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_XVAL("{", attr, prog_type, bpf_prog_types,
			 "BPF_PROG_TYPE_???");
	PRINT_FIELD_U(", ", attr, insn_cnt);
	PRINT_FIELD_X(", ", attr, insns);
	PRINT_FIELD_STR(", ", attr, license, tcp);
	PRINT_FIELD_U(", ", attr, log_level);
	PRINT_FIELD_U(", ", attr, log_size);
	PRINT_FIELD_X(", ", attr, log_buf);
	PRINT_FIELD_U(", ", attr, kern_version);
	PRINT_FIELD_FLAGS(", ", attr, prog_flags, bpf_prog_flags, "BPF_F_???");
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

DEF_BPF_CMD_DECODER(BPF_OBJ_PIN)
{
	struct bpf_obj {
		uint64_t ATTRIBUTE_ALIGNED(8) pathname;
		uint32_t bpf_fd;
	} attr = {};
	const size_t attr_size =
		offsetofend(struct bpf_obj, bpf_fd);
	const unsigned int len = size < attr_size ? size : attr_size;

	memcpy(&attr, data, len);

	PRINT_FIELD_PATH("{", attr, pathname, tcp);
	PRINT_FIELD_FD(", ", attr, bpf_fd, tcp);
	decode_attr_extra_data(tcp, data, size, attr_size);
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

#define decode_BPF_OBJ_GET decode_BPF_OBJ_PIN

DEF_BPF_CMD_DECODER(BPF_PROG_ATTACH)
{
	struct {
		uint32_t target_fd, attach_bpf_fd, attach_type, attach_flags;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, target_fd, tcp);
	PRINT_FIELD_FD(", ", attr, attach_bpf_fd, tcp);
	PRINT_FIELD_XVAL(", ", attr, attach_type, bpf_attach_type, "BPF_???");
	PRINT_FIELD_FLAGS(", ", attr, attach_flags, bpf_attach_flags,
			  "BPF_F_???");
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

DEF_BPF_CMD_DECODER(BPF_PROG_DETACH)
{
	struct {
		uint32_t target_fd, dummy, attach_type;
	} attr = {};
	const unsigned int len = size < sizeof(attr) ? size : sizeof(attr);

	memcpy(&attr, data, len);

	PRINT_FIELD_FD("{", attr, target_fd, tcp);
	PRINT_FIELD_XVAL(", ", attr, attach_type, bpf_attach_type, "BPF_???");
	decode_attr_extra_data(tcp, data, size, sizeof(attr));
	tprints("}");

	return RVAL_DECODED;
}

SYS_FUNC(bpf)
{
	static const bpf_cmd_decoder_t bpf_cmd_decoders[] = {
		BPF_CMD_ENTRY(BPF_MAP_CREATE),
		BPF_CMD_ENTRY(BPF_MAP_LOOKUP_ELEM),
		BPF_CMD_ENTRY(BPF_MAP_UPDATE_ELEM),
		BPF_CMD_ENTRY(BPF_MAP_DELETE_ELEM),
		BPF_CMD_ENTRY(BPF_MAP_GET_NEXT_KEY),
		BPF_CMD_ENTRY(BPF_PROG_LOAD),
		BPF_CMD_ENTRY(BPF_OBJ_PIN),
		BPF_CMD_ENTRY(BPF_OBJ_GET),
		BPF_CMD_ENTRY(BPF_PROG_ATTACH),
		BPF_CMD_ENTRY(BPF_PROG_DETACH),
	};

	const unsigned int cmd = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];
	const unsigned int size = tcp->u_arg[2];
	int rc;

	if (entering(tcp)) {
		static size_t page_size;
		static char *buf;

		if (!buf) {
			page_size = get_pagesize();
			buf = xmalloc(page_size);
		}

		printxval(bpf_commands, cmd, "BPF_???");
		tprints(", ");

		if (size > 0
		    && size <= get_pagesize()
		    && cmd < ARRAY_SIZE(bpf_cmd_decoders)
		    && bpf_cmd_decoders[cmd]) {
			rc = umoven_or_printaddr(tcp, addr, size, buf)
			     ? RVAL_DECODED
			     : bpf_cmd_decoders[cmd](tcp, addr, size, buf);
		} else {
			printaddr(addr);
			rc = RVAL_DECODED;
		}
	} else {
		rc = bpf_cmd_decoders[cmd](tcp, addr, size, NULL) | RVAL_DECODED;
	}

	if (rc & RVAL_DECODED)
		tprintf(", %u", size);

	return rc;
}
