/*
 * Copyright (c) 2015 Intel Corporation
 * Copyright (c) 2019 Patrik Jakobsson <pjakobsson@suse.de>
 * Copyright (c) 2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <drm/drm.h>

#include DEF_MPERS_TYPE(struct_drm_version)
#include DEF_MPERS_TYPE(struct_drm_unique)
#include DEF_MPERS_TYPE(struct_drm_map)
#include DEF_MPERS_TYPE(struct_drm_client)
#include DEF_MPERS_TYPE(struct_drm_stats)
#include DEF_MPERS_TYPE(struct_drm_buf_desc)
#include DEF_MPERS_TYPE(struct_drm_buf_info)
#include DEF_MPERS_TYPE(struct_drm_buf_map)
#include DEF_MPERS_TYPE(struct_drm_buf_pub)
#include DEF_MPERS_TYPE(struct_drm_buf_free)
#include DEF_MPERS_TYPE(struct_drm_ctx_priv_map)
#include DEF_MPERS_TYPE(struct_drm_ctx_res)
#include DEF_MPERS_TYPE(struct_drm_agp_mode)
#include DEF_MPERS_TYPE(struct_drm_agp_info)
#include DEF_MPERS_TYPE(struct_drm_agp_buffer)
#include DEF_MPERS_TYPE(struct_drm_agp_binding)
#include DEF_MPERS_TYPE(struct_drm_scatter_gather)
#include DEF_MPERS_TYPE(union_drm_wait_vblank)
#include DEF_MPERS_TYPE(struct_drm_mode_get_connector)
#include DEF_MPERS_TYPE(struct_drm_mode_fb_cmd2)
#include DEF_MPERS_TYPE(struct_drm_mode_get_plane_res)
#include DEF_MPERS_TYPE(struct_drm_mode_obj_get_properties)
#include DEF_MPERS_TYPE(struct_drm_mode_obj_set_property)

typedef struct drm_version struct_drm_version;
typedef struct drm_unique struct_drm_unique;
typedef struct drm_map struct_drm_map;
typedef struct drm_client struct_drm_client;
typedef struct drm_stats struct_drm_stats;
typedef struct drm_buf_desc struct_drm_buf_desc;
typedef struct drm_buf_info struct_drm_buf_info;
typedef struct drm_buf_map struct_drm_buf_map;
typedef struct drm_buf_pub struct_drm_buf_pub;
typedef struct drm_buf_free struct_drm_buf_free;
typedef struct drm_ctx_priv_map struct_drm_ctx_priv_map;
typedef struct drm_ctx_res struct_drm_ctx_res;
typedef struct drm_agp_mode struct_drm_agp_mode;
typedef struct drm_agp_info struct_drm_agp_info;
typedef struct drm_agp_buffer struct_drm_agp_buffer;
typedef struct drm_agp_binding struct_drm_agp_binding;
typedef struct drm_scatter_gather struct_drm_scatter_gather;
typedef union drm_wait_vblank union_drm_wait_vblank;
typedef struct drm_mode_get_connector struct_drm_mode_get_connector;
typedef struct drm_mode_fb_cmd2 struct_drm_mode_fb_cmd2;
typedef struct drm_mode_get_plane_res struct_drm_mode_get_plane_res;
typedef struct drm_mode_obj_get_properties struct_drm_mode_obj_get_properties;
typedef struct drm_mode_obj_set_property struct_drm_mode_obj_set_property;

#include MPERS_DEFS

#include "print_fields.h"

#include "xlat/drm_buf_desc_flags.h"
#include "xlat/drm_map_flags.h"
#include "xlat/drm_map_type.h"
#include "xlat/drm_mode_fb_cmd2_flags.h"
#include "xlat/drm_vblank_seq_type.h"
#include "xlat/drm_vblank_seq_type_flags.h"

static int
drm_version(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_version ver;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &ver))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_U("{", ver, name_len);
		PRINT_FIELD_U(", ", ver, date_len);
		PRINT_FIELD_U(", ", ver, desc_len);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &ver)) {
		PRINT_FIELD_D(" => {", ver, version_major);
		PRINT_FIELD_D(", ", ver, version_minor);
		PRINT_FIELD_D(", ", ver, version_patchlevel);
		PRINT_FIELD_U(", ", ver, name_len);
		tprints(", name=");
		printstrn(tcp, ptr_to_kulong(ver.name), ver.name_len);
		PRINT_FIELD_U(", ", ver, date_len);
		tprints(", date=");
		printstrn(tcp, ptr_to_kulong(ver.date), ver.date_len);
		PRINT_FIELD_U(", ", ver, desc_len);
		tprints(", desc=");
		printstrn(tcp, ptr_to_kulong(ver.desc), ver.desc_len);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_unique(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_unique unique;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &unique))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_U("{", unique, unique_len);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &unique)) {
		PRINT_FIELD_U(" => {", unique, unique_len);
		tprints(", unique=");
		printstrn(tcp, ptr_to_kulong(unique.unique), unique.unique_len);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &map))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_X("{", map, offset);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &map)) {
		PRINT_FIELD_X(" => {", map, offset);
		PRINT_FIELD_U(", ", map, size);
		PRINT_FIELD_XVAL(", ", map, type, drm_map_type, "_DRM_???");
		PRINT_FIELD_FLAGS(", ", map, flags, drm_map_flags, "_DRM_???");
		PRINT_FIELD_PTR(", ", map, handle);
		PRINT_FIELD_D(", ", map, mtrr);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_client(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_client client;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &client))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", client, idx);

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &client)) {
		PRINT_FIELD_D(", ", client, auth);
		PRINT_FIELD_U(", ", client, pid);
		PRINT_FIELD_UID(", ", client, uid);
		PRINT_FIELD_U(", ", client, magic);
		PRINT_FIELD_U(", ", client, iocs);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_add_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	if (umove_or_printaddr(tcp, arg, &map))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_X("{", map, offset);
	PRINT_FIELD_U(", ", map, size);
	PRINT_FIELD_XVAL(", ", map, type, drm_map_type, "_DRM_???");
	PRINT_FIELD_FLAGS(", ", map, flags, drm_map_flags, "_DRM_???");
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_add_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_desc desc;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &desc))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_D("{", desc, count);
		PRINT_FIELD_D(", ", desc, size);
		PRINT_FIELD_FLAGS(", ", desc, flags, drm_buf_desc_flags,
				 "_DRM_???");
		PRINT_FIELD_X(", ", desc, agp_start);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &desc)) {
		PRINT_FIELD_D(" => {", desc, count);
		PRINT_FIELD_D(", ", desc, size);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mark_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_desc desc;

	if (umove_or_printaddr(tcp, arg, &desc))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_D("{", desc, size);
	PRINT_FIELD_D(", ", desc, low_mark);
	PRINT_FIELD_D(", ", desc, high_mark);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_buf_desc(struct tcb *tcp, void *elem_buf,
		   size_t elem_size, void *data)
{
	const struct_drm_buf_desc *p = elem_buf;
	PRINT_FIELD_D("{", *p, count);
	PRINT_FIELD_D(", ", *p, size);
	PRINT_FIELD_D(", ", *p, low_mark);
	PRINT_FIELD_D(", ", *p, high_mark);
	PRINT_FIELD_FLAGS(", ", *p, flags, drm_buf_desc_flags,
			  "_DRM_???");
	tprints("}");
	return true;
}

static int
drm_info_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_info info;
	struct_drm_buf_desc desc;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

		tprints("{list=");
		print_array(tcp, ptr_to_kulong(info.list), info.count,
			    &desc, sizeof(desc),
			    tfetch_mem, print_drm_buf_desc, 0);
		PRINT_FIELD_D(", ", info, count);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &info)) {
		PRINT_FIELD_D(" => {", info, count);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_buf_pub(struct tcb *tcp, void *elem_buf,
		  size_t elem_size, void *data)

{
	const struct_drm_buf_pub *p = elem_buf;
	PRINT_FIELD_D("{", *p, idx);
	PRINT_FIELD_D(", ", *p, total);
	PRINT_FIELD_D(", ", *p, used);
	PRINT_FIELD_PTR(", ", *p, address);
	tprints("}");
	return true;
}

static int
drm_map_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_map map;
	struct_drm_buf_pub pub;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &map))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", map, count);
		PRINT_FIELD_PTR(", ", map, virtual);

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &map)) {
		tprints(", list=");
		print_array(tcp, ptr_to_kulong(map.list), map.count,
			    &pub, sizeof(pub),
			    tfetch_mem, print_drm_buf_pub, 0);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_free_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_free free;
	int list_val;

	if (umove_or_printaddr(tcp, arg, &free))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_D("{", free, count);
	tprints(", list=");
	print_array(tcp, ptr_to_kulong(free.list), free.count,
		    &list_val, sizeof(list_val),
		    tfetch_mem, print_int32_array_member, 0);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_rm_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	if (umove_or_printaddr(tcp, arg, &map))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_PTR("{", map, handle);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_sarea_ctx(struct tcb *const tcp, const kernel_ulong_t arg,
	      const bool is_get)
{
	struct_drm_ctx_priv_map map;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &map))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", map, ctx_id);
		if (!is_get)
			PRINT_FIELD_PTR(", ", map, handle);
		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &map) && is_get) {
		PRINT_FIELD_PTR(", ", map, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_ctx(struct tcb *tcp, void *elem_buf,
	      size_t elem_size, void *data)
{
	const struct drm_ctx *p = elem_buf;
	PRINT_FIELD_U("{", *p, handle);
	PRINT_FIELD_FLAGS(", ", *p, flags, drm_ctx_flags, "_DRM_CONTEXT_???");
	tprints("}");
	return true;
}

static int
drm_res_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_ctx_res ctx_res;
	struct drm_ctx ctx;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &ctx_res))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_D("{", ctx_res, count);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &ctx_res)) {
		PRINT_FIELD_D(" => {", ctx_res, count);
		tprints(", contexts=");
		print_array(tcp, ptr_to_kulong(ctx_res.contexts), ctx_res.count,
			    &ctx, sizeof(ctx),
			    tfetch_mem, print_drm_ctx, 0);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}


static int
drm_agp_enable(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_mode mode;

	if (umove_or_printaddr(tcp, arg, &mode))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", mode, mode);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_info info;

	if (entering(tcp))

	/* exiting-only code */
	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;
	PRINT_FIELD_D("{", info, agp_version_major);
	PRINT_FIELD_D(", ", info, agp_version_minor);
	PRINT_FIELD_U(", ", info, mode);
	PRINT_FIELD_X(", ", info, aperture_base);
	PRINT_FIELD_U(", ", info, aperture_size);
	PRINT_FIELD_U(", ", info, memory_allowed);
	PRINT_FIELD_U(", ", info, memory_used);
	PRINT_FIELD_X(", ", info, id_vendor);
	PRINT_FIELD_X(", ", info, id_device);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_alloc(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_buffer buffer;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &buffer))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", buffer, size);
		PRINT_FIELD_U(", ", buffer, type);

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &buffer)) {
		PRINT_FIELD_U(", ", buffer, handle);
		PRINT_FIELD_U(", ", buffer, physical);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_free(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_buffer buffer;

	if (umove_or_printaddr(tcp, arg, &buffer))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", buffer, handle);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_bind(struct tcb *const tcp, const kernel_ulong_t arg, bool is_bind)
{
	struct_drm_agp_binding binding;

	if (umove_or_printaddr(tcp, arg, &binding))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", binding, handle);
	if (is_bind)
		PRINT_FIELD_X(", ", binding, offset);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_scatter_gather(struct tcb *const tcp, const kernel_ulong_t arg,
		   bool is_alloc)
{
	struct_drm_scatter_gather sg;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &sg))
			return RVAL_IOCTL_DECODED;
		if (is_alloc)
			PRINT_FIELD_U("{", sg, size);
		else
			PRINT_FIELD_U("{", sg, handle);

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &sg)) {
		if (is_alloc)
			PRINT_FIELD_U(", ", sg, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_wait_vblank(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union_drm_wait_vblank vblank;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &vblank))
			return RVAL_IOCTL_DECODED;

		/* TODO: constants with _DRM_VBLANK_HIGH_CRTC_MASK is not added */
		tprints("{request={type=");
		printxval(drm_vblank_seq_type,
			  vblank.request.type & _DRM_VBLANK_TYPES_MASK,
			  "_DRM_VBLANK_???");
		tprints(", type.flags=");
		printflags(drm_vblank_seq_type_flags,
			   vblank.request.type & _DRM_VBLANK_FLAGS_MASK,
			   "_DRM_VBLANK_???");
		PRINT_FIELD_U(", ", vblank.request, sequence);
		PRINT_FIELD_U(", ", vblank.request, signal);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &vblank)) {
		tprints(", {reply={type=");
		printxval(drm_vblank_seq_type,
			  vblank.reply.type & _DRM_VBLANK_TYPES_MASK,
			  "_DRM_VBLANK_???");
		tprints(", type.flags=");
		printflags(drm_vblank_seq_type_flags,
			   vblank.reply.type & _DRM_VBLANK_FLAGS_MASK,
			   "_DRM_VBLANK_???");

		PRINT_FIELD_U(", ", vblank.reply, sequence);
		PRINT_FIELD_D(", ", vblank.reply, tval_sec);
		PRINT_FIELD_D(", ", vblank.reply, tval_usec);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_mode_modeinfo(struct tcb *tcp, void *elem_buf,
			size_t elem_size, void *data)
{
	const struct drm_mode_modeinfo *p = elem_buf;
	PRINT_FIELD_U("{", *p, clock);
	PRINT_FIELD_U(", ", *p, hdisplay);
	PRINT_FIELD_U(", ", *p, hsync_start);
	PRINT_FIELD_U(", ", *p, hsync_end);
	PRINT_FIELD_U(", ", *p, htotal);
	PRINT_FIELD_U(", ", *p, hskew);
	PRINT_FIELD_U(", ", *p, vdisplay);
	PRINT_FIELD_U(", ", *p, vsync_start);
	PRINT_FIELD_U(", ", *p, vsync_end);
	PRINT_FIELD_U(", ", *p, vtotal);
	PRINT_FIELD_U(", ", *p, vscan);
	PRINT_FIELD_U(", ", *p, vrefresh);
	PRINT_FIELD_FLAGS(", ", *p, flags, drm_mode_flags,
			 "DRM_MODE_FLAG_PIC_???");
	PRINT_FIELD_XVAL(", ", *p, type, drm_mode_type,
			 "DRM_MODE_TYPE_???");
	PRINT_FIELD_CSTRING(", ", *p, name);
	tprints("}");
	return true;
}

static void
print_mode_get_connector_head(struct tcb *const tcp,
			      const kernel_ulong_t arg,
			      struct_drm_mode_get_connector *con,
			      const char *suffix)
{
		PRINT_FIELD_U("{", *con, connector_id);
		PRINT_FIELD_U(", ", *con, count_modes);
		PRINT_FIELD_U(", ", *con, count_props);
		PRINT_FIELD_U(", ", *con, count_encoders);
		tprints(suffix);
}

static int
drm_mode_get_connector(struct tcb *const tcp, const kernel_ulong_t arg,
		       uint32_t size)
{
	struct_drm_mode_get_connector con;
	struct drm_mode_modeinfo modeinfo;
	uint32_t u32_val;
	uint64_t u64_val;

	if (entering(tcp)) {
		if (umoven_or_printaddr(tcp, arg, size, &con))
			return RVAL_IOCTL_DECODED;

		print_mode_get_connector_head(tcp, arg, &con, "}");

		return 0;
	}

	if (!syserror(tcp) && !umoven(tcp, arg, size, &con)) {
		tprints(" => ");
		print_mode_get_connector_head(tcp, arg, &con, "");
		tprints(", encoders_ptr=");
		print_array(tcp, con.encoders_ptr, con.count_encoders,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", modes_ptr=");
		print_array(tcp, con.modes_ptr, con.count_modes,
			    &modeinfo, sizeof(modeinfo),
			    tfetch_mem, print_drm_mode_modeinfo, 0);
		tprints(", props_ptr=");
		print_array(tcp, con.props_ptr, con.count_props,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", prop_values_ptr=");
		print_array(tcp, con.prop_values_ptr, con.count_props,
			    &u64_val, sizeof(u64_val),
			    tfetch_mem, print_uint64_array_member, 0);
		PRINT_FIELD_U(", ", con, encoder_id);
		PRINT_FIELD_U(", ", con, connector_type);
		PRINT_FIELD_U(", ", con, connector_type_id);
		PRINT_FIELD_U(", ", con, connection);
		PRINT_FIELD_U(", ", con, mm_width);
		PRINT_FIELD_U(", ", con, mm_height);
		PRINT_FIELD_U(", ", con, subpixel);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_add_fb2(struct tcb *const tcp, const kernel_ulong_t arg,
		 uint32_t size)
{
	struct_drm_mode_fb_cmd2 cmd;

	if (entering(tcp)) {
		if (umoven_or_printaddr(tcp, arg, size, &cmd))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", cmd, width);
		PRINT_FIELD_U(", ", cmd, height);
		PRINT_FIELD_X(", ", cmd, pixel_format);
		PRINT_FIELD_FLAGS(", ", cmd, flags, drm_mode_fb_cmd2_flags,
				  "DRM_MODE_FB_???");
		tprintf(", handles=[%u, %u, %u, %u]",
			cmd.handles[0], cmd.handles[1],
			cmd.handles[2], cmd.handles[3]);
		tprintf(", pitches=[%u, %u, %u, %u]",
			cmd.pitches[0], cmd.pitches[1],
			cmd.pitches[2], cmd.pitches[3]);
		tprintf(", offsets=[%u, %u, %u, %u]",
			cmd.offsets[0], cmd.offsets[1],
			cmd.offsets[2], cmd.offsets[3]);
		/* modifier field was added in Linux commit v4.1-rc1~69^2~35^2~57. */
		if (size > offsetof(struct_drm_mode_fb_cmd2, modifier)) {
			tprintf(", modifiers=[%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "]",
				(uint64_t) cmd.modifier[0], (uint64_t) cmd.modifier[1],
				(uint64_t) cmd.modifier[2], (uint64_t) cmd.modifier[3]);
		}

		return 0;
	}

	if (!syserror(tcp) && !umoven(tcp, arg, size, &cmd))
		PRINT_FIELD_U(", ", cmd, fb_id);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_getplaneresources(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_get_plane_res res;
	uint32_t plane_id_val;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &res))
			return RVAL_IOCTL_DECODED;

		tprints("{plane_id_ptr=");
		print_array(tcp, res.plane_id_ptr, res.count_planes,
			    &plane_id_val, sizeof(plane_id_val),
			    tfetch_mem, print_uint32_array_member, 0);
		PRINT_FIELD_U(", ", res, count_planes);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &res)) {
		PRINT_FIELD_U(" => {", res, count_planes);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_obj_getproperties(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_obj_get_properties prop;
	uint32_t u32_val;
	uint64_t u64_val;

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &prop))
			return RVAL_IOCTL_DECODED;

		tprints("{props_ptr=");
		print_array(tcp, prop.props_ptr, prop.count_props,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", prop_values_ptr=");
		print_array(tcp, prop.prop_values_ptr, prop.count_props,
			    &u64_val, sizeof(u64_val),
			    tfetch_mem, print_uint64_array_member, 0);
		PRINT_FIELD_U(", ", prop, count_props);
		PRINT_FIELD_U(", ", prop, obj_id);
		PRINT_FIELD_U(", ", prop, obj_type);
		tprints("}");

		return 0;
	}

	if (umove_nor_syserror(tcp, arg, &prop)) {
		PRINT_FIELD_U(" => {", prop, count_props);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_obj_setproperty(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_obj_set_property prop;

	if (umove_or_printaddr(tcp, arg, &prop))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", prop, value);
	PRINT_FIELD_U(", ", prop, prop_id);
	PRINT_FIELD_U(", ", prop, obj_id);
	PRINT_FIELD_U(", ", prop, obj_type);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, drm_ioctl_mpers, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case DRM_IOCTL_VERSION: /* RW */
		return drm_version(tcp, arg);
	case DRM_IOCTL_GET_UNIQUE: /* RW */
		return drm_unique(tcp, arg);
	case DRM_IOCTL_GET_MAP: /* RW */
		return drm_get_map(tcp, arg);
	case DRM_IOCTL_GET_CLIENT: /* RW */
		return drm_get_client(tcp, arg);
	case DRM_IOCTL_ADD_MAP: /* RW */
		return drm_add_map(tcp, arg);
	case DRM_IOCTL_ADD_BUFS: /* RW */
		return drm_add_bufs(tcp, arg);
	case DRM_IOCTL_MARK_BUFS: /* W */
		return drm_mark_bufs(tcp, arg);
	case DRM_IOCTL_INFO_BUFS: /* RW */
		return drm_info_bufs(tcp, arg);
	case DRM_IOCTL_MAP_BUFS: /* RW */
		return drm_map_bufs(tcp, arg);
	case DRM_IOCTL_FREE_BUFS: /* W */
		return drm_free_bufs(tcp, arg);
	case DRM_IOCTL_RM_MAP: /* W */
		return drm_rm_map(tcp, arg);
	case DRM_IOCTL_SET_SAREA_CTX:
	case DRM_IOCTL_GET_SAREA_CTX:
		return drm_sarea_ctx(tcp, arg, code == DRM_IOCTL_GET_SAREA_CTX);
	case DRM_IOCTL_RES_CTX: /* RW */
		return drm_res_ctx(tcp, arg);
	case DRM_IOCTL_AGP_ENABLE: /* W */
		return drm_agp_enable(tcp, arg);
	case DRM_IOCTL_AGP_INFO: /* R */
		return drm_agp_info(tcp, arg);
	case DRM_IOCTL_AGP_ALLOC: /* RW */
		return drm_agp_alloc(tcp, arg);
	case DRM_IOCTL_AGP_FREE: /* W */
		return drm_agp_free(tcp, arg);
	case DRM_IOCTL_AGP_BIND: /* W */
	case DRM_IOCTL_AGP_UNBIND: /* W */
		return drm_agp_bind(tcp, arg, code == DRM_IOCTL_AGP_BIND);
	case DRM_IOCTL_SG_ALLOC: /* RW */
	case DRM_IOCTL_SG_FREE: /* W */
		return drm_scatter_gather(tcp, arg, code == DRM_IOCTL_SG_ALLOC);
	case DRM_IOCTL_WAIT_VBLANK: /* RW */
		return drm_wait_vblank(tcp, arg);
	}

	/* The structure sizes of ioctl cmds below are different */
	/* among kernels and/or personalities so we can't just use */
	/* switch(code) to identify them but use _IOC_NR instead. */

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_GETCONNECTOR)) {
		return drm_mode_get_connector(tcp, arg, _IOC_SIZE(code));
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_ADDFB2)) {
		return drm_mode_add_fb2(tcp, arg, _IOC_SIZE(code));
	}

	/* The structure sizes of ioctl cmds DRM_IOCTL_MODE_GETPLANERESOURCES, */
	/* DRM_IOCTL_MODE_OBJ_GETPROPERTIES and DRM_IOCTL_MODE_OBJ_SETPROPERTY */
	/* differs only among personalities, not versions of kernels, this is */
	/* a little bit different from DRM_IOCTL_MODE_GETCONNECTOR which may */
	/* differs among both kernels and personalities */
	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_GETPLANERESOURCES)) {
		return drm_mode_getplaneresources(tcp, arg);
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_OBJ_GETPROPERTIES)) {
		return drm_mode_obj_getproperties(tcp, arg);
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_OBJ_SETPROPERTY)) {
		return drm_mode_obj_setproperty(tcp, arg);
	}

	return RVAL_DECODED;
}
