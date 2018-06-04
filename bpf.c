/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017 Quentin Monnet <quentin.monnet@6wind.com>
 * Copyright (c) 2015-2018 The strace developers.
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
#include <linux/filter.h>

#include "bpf_attr.h"

#include "xlat/bpf_commands.h"
#include "xlat/bpf_file_mode_flags.h"
#include "xlat/bpf_map_types.h"
#include "xlat/bpf_map_flags.h"
#include "xlat/bpf_prog_types.h"
#include "xlat/bpf_prog_flags.h"
#include "xlat/bpf_map_update_elem_flags.h"
#include "xlat/bpf_attach_type.h"
#include "xlat/bpf_attach_flags.h"
#include "xlat/bpf_query_flags.h"
#include "xlat/ebpf_regs.h"
#include "xlat/numa_node.h"

#define DECL_BPF_CMD_DECODER(bpf_cmd_decoder)				\
int									\
bpf_cmd_decoder(struct tcb *const tcp,					\
		const kernel_ulong_t addr,				\
		const unsigned int size,				\
		void *const data)					\
/* End of DECL_BPF_CMD_DECODER definition. */

#define BEGIN_BPF_CMD_DECODER(bpf_cmd)					\
	static DECL_BPF_CMD_DECODER(decode_ ## bpf_cmd)			\
	{								\
		struct bpf_cmd ## _struct attr = {};			\
		const size_t attr_size = bpf_cmd ## _struct_size;	\
		const unsigned int len = MIN(size, attr_size);		\
		memcpy(&attr, data, len);				\
		do {							\
/* End of BEGIN_BPF_CMD_DECODER definition. */

#define END_BPF_CMD_DECODER(rval)					\
			decode_attr_extra_data(tcp, data, size, attr_size); \
		} while (0);						\
		tprints("}");						\
		return (rval);						\
	}								\
/* End of END_BPF_CMD_DECODER definition. */

#define BPF_CMD_ENTRY(bpf_cmd)						\
	[bpf_cmd] = decode_ ## bpf_cmd

typedef DECL_BPF_CMD_DECODER((*bpf_cmd_decoder_t));

/*
 * A note about bpf syscall decoder: it doesn't perform any size sanity checks,
 * so even if it leads to partial copying of one of the fields, the command
 * handler will still use the (partially-copied-from-userspace, partially
 * zeroed) field value.  That's why we stop decoding and check for known sizes
 * that correspond to released versions of the structure used by the specific
 * command - it looks like the most sensible way to parse this insanity.
 */

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
			if (abbrev(tcp)) {
				tprints("...");
			} else {
				tprintf("/* bytes %zu..%zu */ ",
					attr_size, attr_size + size - 1);
				print_quoted_string(data, size,
						    QUOTE_FORCE_HEX);
			}
			return RVAL_DECODED;
		}
	}

	return 0;
}

struct ebpf_insn {
	uint8_t code;
	uint8_t dst_reg:4;
	uint8_t src_reg:4;
	int16_t off;
	int32_t imm;
};

struct ebpf_insns_data {
	unsigned int count;
};

static bool
print_ebpf_insn(struct tcb * const tcp, void * const elem_buf,
		const size_t elem_size, void * const data)
{
	struct ebpf_insns_data *eid = data;
	struct ebpf_insn *insn = elem_buf;

	if (eid->count++ >= BPF_MAXINSNS) {
		tprints("...");
		return false;
	}

	tprints("{code=");
	print_bpf_filter_code(insn->code, true);

	/* We can't use PRINT_FIELD_XVAL on bit fields */
	tprints(", dst_reg=");
	printxval_index(ebpf_regs, insn->dst_reg, "BPF_REG_???");
	tprints(", src_reg=");
	printxval_index(ebpf_regs, insn->src_reg, "BPF_REG_???");

	PRINT_FIELD_D(", ", *insn, off);
	PRINT_FIELD_X(", ", *insn, imm);
	tprints("}");

	return true;
}

static void
print_ebpf_prog(struct tcb *const tcp, const uint64_t addr, const uint32_t len)
{
	print_big_u64_addr(addr);
	if (abbrev(tcp)) {
		printaddr(addr);
	} else {
		struct ebpf_insns_data eid = {};
		struct ebpf_insn insn;

		print_array(tcp, addr, len, &insn, sizeof(insn),
			    tfetch_mem, print_ebpf_insn, &eid);
	}
}

BEGIN_BPF_CMD_DECODER(BPF_MAP_CREATE)
{
	PRINT_FIELD_XVAL_INDEX("{", attr, map_type, bpf_map_types,
			       "BPF_MAP_TYPE_???");
	PRINT_FIELD_U(", ", attr, key_size);
	PRINT_FIELD_U(", ", attr, value_size);
	PRINT_FIELD_U(", ", attr, max_entries);

	/* map_flags field was added in Linux commit v4.6-rc1~91^2~108^2~6. */
	if (len <= offsetof(struct BPF_MAP_CREATE_struct, map_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, map_flags, bpf_map_flags, "BPF_F_???");

	/*
	 * inner_map_fd field was added in Linux commit
	 * v4.12-rc1~64^3~373^2~2.
	 */
	if (len <= offsetof(struct BPF_MAP_CREATE_struct, inner_map_fd))
		break;
	PRINT_FIELD_FD(", ", attr, inner_map_fd, tcp);

	/* numa_node field was added in Linux commit v4.14-rc1~130^2~196^2~1. */
	if (len <= offsetof(struct BPF_MAP_CREATE_struct, numa_node))
		break;
	if (attr.map_flags & BPF_F_NUMA_NODE) {
		/*
		 * Kernel uses the value of -1 as a designation for "no NUMA
		 * node specified", and even uses NUMA_NO_NODE constant;
		 * however, the constant definition is not a part of UAPI
		 * headers, thus we can't simply print this named constant
		 * instead of the value. Let's force verbose xlat style instead
		 * in order to provide the information for the user while
		 * not hampering the availability to derive the actual value
		 * without the access to the kernel headers.
		 */
		tprints(", numa_node=");
		printxvals_ex(attr.numa_node, NULL,
			      XLAT_STYLE_FMT_U | XLAT_STYLE_VERBOSE,
			      numa_node, NULL);
	}

	/* map_name field was added in Linux commit v4.15-rc1~84^2~605^2~3. */
	if (len <= offsetof(struct BPF_MAP_CREATE_struct, map_name))
		break;
	PRINT_FIELD_CSTRING_SZ(", ", attr, map_name,
			       MIN(sizeof(attr.map_name),
				   len - offsetof(struct BPF_MAP_CREATE_struct,
						  map_name)));

	/*
	 * map_ifindex field was added in Linux commit
	 * v4.16-rc1~123^2~145^2~5^2~8.
	 */
	if (len <= offsetof(struct BPF_MAP_CREATE_struct, map_ifindex))
		break;
	PRINT_FIELD_IFINDEX(", ", attr, map_ifindex);
}
END_BPF_CMD_DECODER(RVAL_DECODED | RVAL_FD)

BEGIN_BPF_CMD_DECODER(BPF_MAP_LOOKUP_ELEM)
{
	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_ADDR64(", ", attr, key);
	PRINT_FIELD_ADDR64(", ", attr, value);
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_MAP_UPDATE_ELEM)
{
	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_ADDR64(", ", attr, key);
	PRINT_FIELD_ADDR64(", ", attr, value);
	PRINT_FIELD_XVAL_INDEX(", ", attr, flags, bpf_map_update_elem_flags,
			       "BPF_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_MAP_DELETE_ELEM)
{
	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_ADDR64(", ", attr, key);
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_MAP_GET_NEXT_KEY)
{
	PRINT_FIELD_FD("{", attr, map_fd, tcp);
	PRINT_FIELD_ADDR64(", ", attr, key);
	PRINT_FIELD_ADDR64(", ", attr, next_key);
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_PROG_LOAD)
{
	PRINT_FIELD_XVAL_INDEX("{", attr, prog_type, bpf_prog_types,
			       "BPF_PROG_TYPE_???");
	PRINT_FIELD_U(", ", attr, insn_cnt);
	tprints(", insns=");
	print_ebpf_prog(tcp, attr.insns, attr.insn_cnt);

	tprintf(", license=");
	print_big_u64_addr(attr.license);
	printstr(tcp, attr.license);

	/* log_* fields were added in Linux commit v3.18-rc1~52^2~1^2~4.  */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, log_level))
		break;
	PRINT_FIELD_U(", ", attr, log_level);
	PRINT_FIELD_U(", ", attr, log_size);
	tprintf(", log_buf=");
	print_big_u64_addr(attr.log_buf);
	printstr_ex(tcp, attr.log_buf, attr.log_size, QUOTE_0_TERMINATED);

	/* kern_version field was added in Linux commit v4.1-rc1~84^2~50.  */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, kern_version))
		break;
	tprintf(", kern_version=KERNEL_VERSION(%u, %u, %u)",
		attr.kern_version >> 16,
		(attr.kern_version >> 8) & 0xFF,
		attr.kern_version & 0xFF);

	/* prog_flags field was added in Linux commit v4.12-rc2~34^2~29^2~2.  */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, prog_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, prog_flags, bpf_prog_flags, "BPF_F_???");

	/* prog_name field was added in Linux commit v4.15-rc1~84^2~605^2~4. */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, prog_name))
		break;
	PRINT_FIELD_CSTRING_SZ(", ", attr, prog_name,
			       MIN(sizeof(attr.prog_name),
				   len - offsetof(struct BPF_PROG_LOAD_struct,
						   prog_name)));

	/*
	 * prog_ifindex field was added as prog_target_ifindex in Linux commit
	 * v4.15-rc1~84^2~127^2~13 and renamed to its current name in
	 * v4.15-rc1~15^2~5^2~3^2~7.
	 */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, prog_ifindex))
		break;
	PRINT_FIELD_IFINDEX(", ", attr, prog_ifindex);

	/*
	 * expected_attach_type was added in Linux commit
	 * v4.17-rc1~148^2~19^2^2~8.
	 */
	if (len <= offsetof(struct BPF_PROG_LOAD_struct, expected_attach_type))
		break;
	PRINT_FIELD_XVAL(", ", attr, expected_attach_type, bpf_attach_type,
			 "BPF_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED | RVAL_FD)

BEGIN_BPF_CMD_DECODER(BPF_OBJ_PIN)
{
	tprintf("{pathname=");
	print_big_u64_addr(attr.pathname);
	printpath(tcp, attr.pathname);

	PRINT_FIELD_FD(", ", attr, bpf_fd, tcp);

	/* file_flags field was added in Linux v4.15-rc1~84^2~384^2~4 */
	if (len <= offsetof(struct BPF_OBJ_PIN_struct, file_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, file_flags, bpf_file_mode_flags,
			  "BPF_F_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED | RVAL_FD)

#define decode_BPF_OBJ_GET decode_BPF_OBJ_PIN

BEGIN_BPF_CMD_DECODER(BPF_PROG_ATTACH)
{
	PRINT_FIELD_FD("{", attr, target_fd, tcp);
	PRINT_FIELD_FD(", ", attr, attach_bpf_fd, tcp);
	PRINT_FIELD_XVAL_INDEX(", ", attr, attach_type, bpf_attach_type,
			       "BPF_???");
	PRINT_FIELD_FLAGS(", ", attr, attach_flags, bpf_attach_flags,
			  "BPF_F_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_PROG_DETACH)
{
	PRINT_FIELD_FD("{", attr, target_fd, tcp);
	PRINT_FIELD_XVAL_INDEX(", ", attr, attach_type, bpf_attach_type,
			       "BPF_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_PROG_TEST_RUN)
{
	PRINT_FIELD_FD("{test={", attr, prog_fd, tcp);
	PRINT_FIELD_U(", ", attr, retval);
	PRINT_FIELD_U(", ", attr, data_size_in);
	PRINT_FIELD_U(", ", attr, data_size_out);
	PRINT_FIELD_ADDR64(", ", attr, data_in);
	PRINT_FIELD_ADDR64(", ", attr, data_out);
	PRINT_FIELD_U(", ", attr, repeat);
	PRINT_FIELD_U(", ", attr, duration);
	tprints("}");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_PROG_GET_NEXT_ID)
{
	PRINT_FIELD_U("{", attr, start_id);
	PRINT_FIELD_U(", ", attr, next_id);

	/* open_flags field has been added in Linux v4.15-rc1~84^2~384^2~4 */
	if (len <= offsetof(struct BPF_PROG_GET_NEXT_ID_struct, open_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, open_flags, bpf_file_mode_flags,
			  "BPF_F_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

#define decode_BPF_MAP_GET_NEXT_ID decode_BPF_PROG_GET_NEXT_ID

BEGIN_BPF_CMD_DECODER(BPF_PROG_GET_FD_BY_ID)
{
	PRINT_FIELD_U("{", attr, prog_id);
	PRINT_FIELD_U(", ", attr, next_id);

	/* open_flags field has been added in Linux v4.15-rc1~84^2~384^2~4 */
	if (len <= offsetof(struct BPF_PROG_GET_FD_BY_ID_struct, open_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, open_flags, bpf_file_mode_flags,
			  "BPF_F_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_MAP_GET_FD_BY_ID)
{
	PRINT_FIELD_U("{", attr, map_id);
	PRINT_FIELD_U(", ", attr, next_id);

	/* open_flags field has been added in Linux v4.15-rc1~84^2~384^2~4 */
	if (len <= offsetof(struct BPF_MAP_GET_FD_BY_ID_struct, open_flags))
		break;
	PRINT_FIELD_FLAGS(", ", attr, open_flags, bpf_file_mode_flags,
			  "BPF_F_???");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

struct obj_get_info_saved;
typedef void (*print_bpf_obj_info_fn)(struct tcb *,
				      uint32_t bpf_fd,
				      const char *info_buf,
				      uint32_t size,
				      struct obj_get_info_saved *saved);

struct obj_get_info_saved {
	print_bpf_obj_info_fn print_fn;

	uint32_t info_len;

	uint32_t jited_prog_len;
	uint32_t xlated_prog_len;
	uint32_t nr_map_ids;
};

static void
print_bpf_map_info(struct tcb * const tcp, uint32_t bpf_fd,
		   const char *info_buf, uint32_t size,
		   struct obj_get_info_saved *saved)
{
	if (entering(tcp))
		return;

	struct bpf_map_info_struct info = { 0 };
	const unsigned int len = MIN(size, bpf_map_info_struct_size);

	memcpy(&info, info_buf, len);

	PRINT_FIELD_XVAL("{", info, type, bpf_map_types, "BPF_MAP_TYPE_???");
	PRINT_FIELD_U(", ", info, id);
	PRINT_FIELD_U(", ", info, key_size);
	PRINT_FIELD_U(", ", info, value_size);
	PRINT_FIELD_U(", ", info, max_entries);
	PRINT_FIELD_FLAGS(", ", info, map_flags, bpf_map_flags, "BPF_F_???");

	/*
	 * "name" field was introduced by Linux commit v4.15-rc1~84^2~605^2~3.
	 */
	if (len <= offsetof(struct bpf_map_info_struct, name))
		goto print_bpf_map_info_end;
	PRINT_FIELD_CSTRING(", ", info, name);

	/*
	 * ifindex, netns_dev, and netns_ino fields were introduced
	 * by Linux commit v4.16-rc1~123^2~109^2~5^2~4.
	 */
	if (len <= offsetof(struct bpf_map_info_struct, ifindex))
		goto print_bpf_map_info_end;
	PRINT_FIELD_IFINDEX(", ", info, ifindex);
	PRINT_FIELD_DEV(", ", info, netns_dev);
	PRINT_FIELD_U(", ", info, netns_ino);

	decode_attr_extra_data(tcp, info_buf, size, bpf_map_info_struct_size);

print_bpf_map_info_end:
	tprints("}");
}

static void
print_bpf_prog_info(struct tcb * const tcp, uint32_t bpf_fd,
		    const char *info_buf, uint32_t size,
		    struct obj_get_info_saved *saved)
{
	struct bpf_prog_info_struct info = { 0 };
	const unsigned int len = MIN(size, bpf_prog_info_struct_size);
	uint64_t map_id_buf;

	memcpy(&info, info_buf, len);

	if (entering(tcp)) {
		saved->jited_prog_len = info.jited_prog_len;
		saved->xlated_prog_len = info.xlated_prog_len;
		saved->nr_map_ids = info.nr_map_ids;

		return;
	}

	PRINT_FIELD_XVAL("{", info, type, bpf_prog_types, "BPF_PROG_TYPE_???");
	PRINT_FIELD_U(", ", info, id);
	PRINT_FIELD_HEX_ARRAY(", ", info, tag);

	tprints(", jited_prog_len=");
	if (saved->jited_prog_len != info.jited_prog_len)
		tprintf("%" PRIu32 " => ", saved->jited_prog_len);
	tprintf("%" PRIu32, info.jited_prog_len);

	tprints(", jited_prog_insns=");
	print_big_u64_addr(info.jited_prog_insns);
	printstr_ex(tcp, info.jited_prog_insns, info.jited_prog_len,
		    QUOTE_FORCE_HEX);

	tprints(", xlated_prog_len=");
	if (saved->xlated_prog_len != info.xlated_prog_len)
		tprintf("%" PRIu32 " => ", saved->xlated_prog_len);
	tprintf("%" PRIu32, info.xlated_prog_len);

	tprints(", xlated_prog_insns=");
	print_ebpf_prog(tcp, info.xlated_prog_insns,
			MIN(saved->xlated_prog_len, info.xlated_prog_len) / 8);

	/*
	 * load_time, created_by_uid, nr_map_ids, map_ids, and name fields
	 * were introduced by Linux commit v4.15-rc1~84^2~605^2~4.
	 */
	if (len <= offsetof(struct bpf_prog_info_struct, load_time))
		goto print_bpf_prog_info_end;
	PRINT_FIELD_U(", ", info, load_time);
	PRINT_FIELD_UID(", ", info, created_by_uid);

	tprints(", nr_map_ids=");
	if (saved->nr_map_ids != info.nr_map_ids)
		tprintf("%" PRIu32 " => ", saved->nr_map_ids);
	tprintf("%" PRIu32, info.nr_map_ids);

	tprints(", map_ids=");
	print_big_u64_addr(info.map_ids);
	print_array(tcp, info.map_ids, MIN(saved->nr_map_ids, info.nr_map_ids),
		    &map_id_buf, sizeof(map_id_buf),
		    tfetch_mem, print_uint32_array_member, 0);

	PRINT_FIELD_CSTRING(", ", info, name);

	/*
	 * ifindex, netns_dev, and netns_ino fields were introduced
	 * by Linux commit v4.16-rc1~123^2~227^2~5^2~2.
	 */
	if (len <= offsetof(struct bpf_prog_info_struct, ifindex))
		goto print_bpf_prog_info_end;
	PRINT_FIELD_IFINDEX(", ", info, ifindex);
	PRINT_FIELD_DEV(", ", info, netns_dev);
	PRINT_FIELD_U(", ", info, netns_ino);

	decode_attr_extra_data(tcp, info_buf, size, bpf_prog_info_struct_size);

print_bpf_prog_info_end:
	tprints("}");
}

static const char *
fetch_bpf_obj_info(struct tcb * const tcp, uint64_t info, uint32_t size)
{
	static char *info_buf;

	if (!info_buf)
		info_buf = xmalloc(get_pagesize());

	memset(info_buf, 0, get_pagesize());

	if (size > 0 && size <= get_pagesize()
	    && !umoven(tcp, info, size, info_buf))
		return info_buf;

	return NULL;
}

static void
print_bpf_obj_info_addr(struct tcb * const tcp, uint64_t addr)
{
	if (exiting(tcp))
		printaddr64(addr);
}

static void
print_bpf_obj_info(struct tcb * const tcp, uint32_t bpf_fd, uint64_t info,
		   uint32_t size, struct obj_get_info_saved *saved)
{
	if (abbrev(tcp)) {
		print_bpf_obj_info_addr(tcp, info);
		return;
	}

	static struct {
		const char *id;
		print_bpf_obj_info_fn print_fn;
	} obj_printers[] = {
		{ "anon_inode:bpf-map", print_bpf_map_info },
		{ "anon_inode:bpf-prog", print_bpf_prog_info }
	};

	if (entering(tcp)) {
		char path[PATH_MAX + 1];

		if (getfdpath(tcp, bpf_fd, path, sizeof(path)) > 0) {
			for (size_t i = 0; i < ARRAY_SIZE(obj_printers); ++i) {
				if (!strcmp(path, obj_printers[i].id)) {
					saved->print_fn =
						obj_printers[i].print_fn;
					break;
				}
			}
		}
	}

	if (!saved || !saved->print_fn) {
		print_bpf_obj_info_addr(tcp, info);
		return;
	}

	const char *info_buf = fetch_bpf_obj_info(tcp, info, size);

	if (info_buf)
		saved->print_fn(tcp, bpf_fd, info_buf, size, saved);
	else
		print_bpf_obj_info_addr(tcp, info);
}

BEGIN_BPF_CMD_DECODER(BPF_OBJ_GET_INFO_BY_FD)
{
	struct obj_get_info_saved *saved;

	if (entering(tcp)) {
		saved = xcalloc(1, sizeof(*saved));
		saved->info_len = attr.info_len;
		set_tcb_priv_data(tcp, saved, free);

		PRINT_FIELD_FD("{info={", attr, bpf_fd, tcp);
		PRINT_FIELD_U(", ", attr, info_len);
	} else {
		saved = get_tcb_priv_data(tcp);

		if (saved && (saved->info_len != attr.info_len))
			tprintf(" => %u", attr.info_len);

		tprintf(", info=");
	}

	print_bpf_obj_info(tcp, attr.bpf_fd, attr.info, attr.info_len, saved);

	if (entering(tcp))
		return 0;

	tprints("}");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_PROG_QUERY)
{
	uint32_t prog_id_buf;

	if (entering(tcp)) {
		PRINT_FIELD_FD("{query={", attr, target_fd, tcp);
		PRINT_FIELD_XVAL_INDEX(", ", attr, attach_type, bpf_attach_type,
				       "BPF_???");
		PRINT_FIELD_FLAGS(", ", attr, query_flags, bpf_query_flags,
				  "BPF_F_QUERY_???");
		PRINT_FIELD_FLAGS(", ", attr, attach_flags, bpf_attach_flags,
				  "BPF_F_???");

		tprints(", prog_ids=");

		set_tcb_priv_ulong(tcp, attr.prog_cnt);

		return 0;
	}

	print_big_u64_addr(attr.prog_ids);
	print_array(tcp, attr.prog_ids, attr.prog_cnt, &prog_id_buf,
		    sizeof(prog_id_buf), tfetch_mem,
		    print_uint32_array_member, 0);

	tprints(", prog_cnt=");
	const uint32_t prog_cnt_entering = get_tcb_priv_ulong(tcp);
	if (prog_cnt_entering != attr.prog_cnt)
		tprintf("%" PRIu32 " => ", prog_cnt_entering);
	tprintf("%" PRIu32, attr.prog_cnt);
	tprints("}");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

BEGIN_BPF_CMD_DECODER(BPF_RAW_TRACEPOINT_OPEN)
{
	enum { TP_NAME_SIZE = 128 };

	tprintf("{raw_tracepoint={name=");
	print_big_u64_addr(attr.name);
	printstr_ex(tcp, attr.name, TP_NAME_SIZE, QUOTE_0_TERMINATED);

	PRINT_FIELD_FD(", ", attr, prog_fd, tcp);

	tprints("}");
}
END_BPF_CMD_DECODER(RVAL_DECODED)

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
		BPF_CMD_ENTRY(BPF_PROG_TEST_RUN),
		BPF_CMD_ENTRY(BPF_PROG_GET_NEXT_ID),
		BPF_CMD_ENTRY(BPF_MAP_GET_NEXT_ID),
		BPF_CMD_ENTRY(BPF_PROG_GET_FD_BY_ID),
		BPF_CMD_ENTRY(BPF_MAP_GET_FD_BY_ID),
		BPF_CMD_ENTRY(BPF_OBJ_GET_INFO_BY_FD),
		BPF_CMD_ENTRY(BPF_PROG_QUERY),
		BPF_CMD_ENTRY(BPF_RAW_TRACEPOINT_OPEN),
	};

	const unsigned int cmd = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];
	const unsigned int size = tcp->u_arg[2];
	int rc = RVAL_DECODED;

	if (entering(tcp)) {
		printxval_index(bpf_commands, cmd, "BPF_???");
		tprints(", ");
	}

	if (size > 0
	    && size <= get_pagesize()
	    && cmd < ARRAY_SIZE(bpf_cmd_decoders)
	    && bpf_cmd_decoders[cmd]) {
		static char *buf;

		if (!buf)
			buf = xmalloc(get_pagesize());

		if (!umoven_or_printaddr_ignore_syserror(tcp, addr, size, buf))
			rc = bpf_cmd_decoders[cmd](tcp, addr, size, buf);
	} else {
		printaddr(addr);
	}

	if (exiting(tcp) || (rc & RVAL_DECODED))
		tprintf(", %u", size);

	return rc;
}
