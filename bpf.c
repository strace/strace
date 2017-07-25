/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include "xlat/bpf_prog_types.h"
#include "xlat/bpf_map_update_elem_flags.h"
#include "xlat/bpf_attach_type.h"
#include "xlat/bpf_attach_flags.h"

static int
bpf_map_create(struct tcb *const tcp, const kernel_ulong_t addr,
	       unsigned int size)
{
	struct {
		uint32_t map_type, key_size, value_size, max_entries;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return RVAL_DECODED | RVAL_FD;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return RVAL_DECODED | RVAL_FD;

	PRINT_FIELD_XVAL("{", attr, map_type, bpf_map_types, "BPF_MAP_TYPE_???");
	PRINT_FIELD_U(", ", attr, key_size);
	PRINT_FIELD_U(", ", attr, value_size);
	PRINT_FIELD_U(", ", attr, max_entries);
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

static void
bpf_map_update_elem(struct tcb *const tcp, const kernel_ulong_t addr,
		    unsigned int size)
{
	struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key;
		uint64_t ATTRIBUTE_ALIGNED(8) value;
		uint64_t flags;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return;

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	PRINT_FIELD_X(", ", attr, value);
	PRINT_FIELD_XVAL(", ", attr, flags, bpf_map_update_elem_flags,
			 "BPF_???");
	tprints("}");
}

static void
bpf_map_delete_elem(struct tcb *const tcp, const kernel_ulong_t addr,
		    unsigned int size)
{
	struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return;

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);
	tprints("}");
}

static int
bpf_map_io(struct tcb *const tcp, const kernel_ulong_t addr, unsigned int size,
	   const char *const text)
{
	struct bpf_io_elem_struct {
		uint32_t map_fd;
		uint64_t ATTRIBUTE_ALIGNED(8) key;
		uint64_t ATTRIBUTE_ALIGNED(8) value;
	} attr = {};

	if (exiting(tcp)) {
		if (!syserror(tcp) && !umove_or_printaddr(tcp, addr, &attr))
			tprintf(", %s=%#" PRIx64, text, attr.value);
		tprints("}");
		return RVAL_DECODED;
	}

	if (!size) {
		printaddr(addr);
		return RVAL_DECODED;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return RVAL_DECODED;

	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_X(", ", attr, key);

	return 0;
}

static int
bpf_prog_load(struct tcb *const tcp, const kernel_ulong_t addr,
	      unsigned int size)
{
	struct {
		uint32_t prog_type, insn_cnt;
		uint64_t ATTRIBUTE_ALIGNED(8) insns, license;
		uint32_t log_level, log_size;
		uint64_t ATTRIBUTE_ALIGNED(8) log_buf;
		uint32_t kern_version;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return RVAL_DECODED | RVAL_FD;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return RVAL_DECODED | RVAL_FD;

	PRINT_FIELD_XVAL("{", attr, prog_type, bpf_prog_types, "BPF_PROG_TYPE_???");
	PRINT_FIELD_U(", ", attr, insn_cnt);
	PRINT_FIELD_X(", ", attr, insns);
	PRINT_FIELD_STR(", ", attr, license, tcp);
	PRINT_FIELD_U(", ", attr, log_level);
	PRINT_FIELD_U(", ", attr, log_size);
	PRINT_FIELD_X(", ", attr, log_buf);
	PRINT_FIELD_U(", ", attr, kern_version);
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

static int
bpf_obj_manage(struct tcb *const tcp, const kernel_ulong_t addr,
	       unsigned int size)
{
	struct {
		uint64_t ATTRIBUTE_ALIGNED(8) pathname;
		uint32_t bpf_fd;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return RVAL_DECODED | RVAL_FD;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return RVAL_DECODED | RVAL_FD;

	PRINT_FIELD_PATH("{", attr, pathname, tcp);
	PRINT_FIELD_FD(", ", attr, bpf_fd, tcp);
	tprints("}");

	return RVAL_DECODED | RVAL_FD;
}

static int
bpf_prog_attach_detach(struct tcb *const tcp, const kernel_ulong_t addr,
		       unsigned int size, bool print_attach)
{
	struct {
		uint32_t target_fd, attach_bpf_fd, attach_type, attach_flags;
	} attr = {};

	if (!size) {
		printaddr(addr);
		return RVAL_DECODED;
	}
	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return RVAL_DECODED;

	PRINT_FIELD_FD("{", attr, target_fd, tcp);
	if (print_attach)
		PRINT_FIELD_FD(", ", attr, attach_bpf_fd, tcp);
	PRINT_FIELD_XVAL(", ", attr, attach_type, bpf_attach_type, "BPF_???");
	if (print_attach)
		PRINT_FIELD_FLAGS(", ", attr, attach_flags, bpf_attach_flags,
				  "BPF_F_???");
	tprints("}");

	return RVAL_DECODED;
}

static int
bpf_prog_attach(struct tcb *const tcp, const kernel_ulong_t addr,
		unsigned int size)
{
	return bpf_prog_attach_detach(tcp, addr, size, true);
}

static int
bpf_prog_detach(struct tcb *const tcp, const kernel_ulong_t addr,
		unsigned int size)
{
	return bpf_prog_attach_detach(tcp, addr, size, false);
}

SYS_FUNC(bpf)
{
	const unsigned int cmd = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];
	const unsigned int size = tcp->u_arg[2];
	int rc = RVAL_DECODED;

	if (entering(tcp)) {
		printxval(bpf_commands, cmd, "BPF_???");
		tprints(", ");
	}

	switch (cmd) {
	case BPF_MAP_CREATE:
		rc = bpf_map_create(tcp, addr, size);
		break;
	case BPF_MAP_LOOKUP_ELEM:
		rc = bpf_map_io(tcp, addr, size, "value");
		break;
	case BPF_MAP_UPDATE_ELEM:
		bpf_map_update_elem(tcp, addr, size);
		break;
	case BPF_MAP_DELETE_ELEM:
		bpf_map_delete_elem(tcp, addr, size);
		break;
	case BPF_MAP_GET_NEXT_KEY:
		rc = bpf_map_io(tcp, addr, size, "next_key");
		break;
	case BPF_PROG_LOAD:
		rc = bpf_prog_load(tcp, addr, size);
		break;
	case BPF_OBJ_PIN:
		rc = bpf_obj_manage(tcp, addr, size);
		break;
	case BPF_OBJ_GET:
		rc = bpf_obj_manage(tcp, addr, size);
		break;
	case BPF_PROG_ATTACH:
		rc = bpf_prog_attach(tcp, addr, size);
		break;
	case BPF_PROG_DETACH:
		rc = bpf_prog_detach(tcp, addr, size);
		break;
	default:
		printaddr(addr);
		break;
	}

	if (rc & RVAL_DECODED)
		tprintf(", %u", size);

	return rc;
}
