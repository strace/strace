#include "tests.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# include <errno.h>
# include <fcntl.h>
# include <inttypes.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>
# include "print_fields.h"

# ifdef HAVE_DRM_H
#  include <drm.h>
# else
#  include <drm/drm.h>
# endif

# include "xlat.h"
# include "xlat/drm_buf_desc_flags.h"
# include "xlat/drm_capability.h"
# include "xlat/drm_client_capability.h"
# include "xlat/drm_control_func.h"
# include "xlat/drm_crtc_sequence_flags.h"
# include "xlat/drm_ctx_flags.h"
# include "xlat/drm_lock_flags.h"
# include "xlat/drm_map_flags.h"
# include "xlat/drm_map_type.h"
# include "xlat/drm_modeset_cmd.h"
# include "xlat/drm_mode_atomic_flags.h"
# include "xlat/drm_mode_create_lease_flags.h"
# include "xlat/drm_mode_cursor_flags.h"
# include "xlat/drm_mode_encoder_type.h"
# include "xlat/drm_mode_fb_cmd2_flags.h"
# include "xlat/drm_mode_fb_dirty_cmd_flags.h"
# include "xlat/drm_mode_flags.h"
# include "xlat/drm_mode_get_property_flags.h"
# include "xlat/drm_mode_page_flip_flags.h"
# include "xlat/drm_mode_set_plane_flags.h"
# include "xlat/drm_mode_type.h"
# include "xlat/drm_prime_handle_flags.h"
# include "xlat/drm_syncobj_fd_to_handle_flags.h"
# include "xlat/drm_syncobj_flags.h"
# include "xlat/drm_syncobj_handle_to_fd_flags.h"
# include "xlat/drm_syncobj_wait_flags.h"
# include "xlat/drm_vblank_seq_type.h"
# include "xlat/drm_vblank_seq_type_flags.h"

static const uint16_t u16_magic = 0xbeefU;
static const int smagic = 0xdeadbeef;
static const unsigned int umagic = 0xdeadbeefU;
static const long slmagic = (long) 0xdeadbeefbadc0ded;
static const unsigned long ulmagic = (unsigned long) 0xdeadbeefbadc0dedULL;
static const char *errstr;


struct drm_check {
	unsigned long cmd;
	const char *cmd_str;
	void *arg_ptr;
	void (*print_arg)(long rc, void *ptr, void *arg);
};

static void
printaddr64(uint64_t addr)
{
	if (!addr)
		printf("NULL");
	else
		printf("%#" PRIx64, addr);
}

static long
invoke_test_syscall(unsigned long cmd, void *p)
{
	long rc = ioctl(-1, cmd, p);
	errstr = sprintrc(rc);
	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
	return rc;
}

static void
test_drm(struct drm_check *check, void *arg)
{

	long rc = invoke_test_syscall(check->cmd, check->arg_ptr);
	printf("ioctl(-1, %s, ", check->cmd_str);
	if (check->print_arg)
		check->print_arg(rc, check->arg_ptr, arg);
	else
		printf("%p", check->arg_ptr);
	printf(") = %s\n", errstr);
}

static void
print_drm_version(long rc, void *ptr, void *arg)
{
	struct drm_version *ver = ptr;

	if (rc < 0) {
		printf("%p", ver);
		return;
	}
	PRINT_FIELD_U("{", *ver, name_len);
	PRINT_FIELD_U(", ", *ver, date_len);
	PRINT_FIELD_U(", ", *ver, desc_len);
	PRINT_FIELD_D("} => {", *ver, version_major);
	PRINT_FIELD_D(", ", *ver, version_minor);
	PRINT_FIELD_D(", ", *ver, version_patchlevel);
	PRINT_FIELD_U(", ", *ver, name_len);
	printf(", name=\"%s\"", ver->name);
	PRINT_FIELD_U(", ", *ver, date_len);
	printf(", date=\"%s\"", ver->date);
	PRINT_FIELD_U(", ", *ver, desc_len);
	printf(", desc=\"%s\"", ver->desc);
	printf("}");
}

static void
print_drm_get_unique(long rc, void *ptr, void *arg)
{
	struct drm_unique *unique = ptr;

	if (rc < 0) {
		printf("%p", unique);
		return;
	}
	PRINT_FIELD_U("{", *unique, unique_len);
	PRINT_FIELD_U("} => {", *unique, unique_len);
	printf(", unique=\"%s\"", unique->unique);
	printf("}");
}

static void
print_drm_get_magic(long rc, void *ptr, void *arg)
{
	struct drm_auth *auth = ptr;

	if (rc < 0) {
		printf("%p", auth);
		return;
	}
	PRINT_FIELD_U("{", *auth, magic);
	printf("}");
}

static void
print_drm_irq_busid(long rc, void *ptr, void *arg)
{
	struct drm_irq_busid *busid = ptr;

	if (rc < 0) {
		printf("%p", busid);
		return;
	}
	PRINT_FIELD_D("{", *busid, busnum);
	PRINT_FIELD_D(", ", *busid, devnum);
	PRINT_FIELD_D(", ", *busid, funcnum);
	PRINT_FIELD_D(", ", *busid, irq);
	printf("}");
}

static void
print_drm_get_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_X("{", *map, offset);
	PRINT_FIELD_X("} => {", *map, offset);
	PRINT_FIELD_U(", ", *map, size);
	printf(", type=");
	printxval(drm_map_type, map->type, "_DRM_???");
	printf(", flags=");
	printflags(drm_map_flags, map->flags, "_DRM_???");
	printf(", handle=%p", map->handle);
	PRINT_FIELD_D(", ", *map, mtrr);
	printf("}");
}

static void
print_drm_get_client(long rc, void *ptr, void *arg)
{
	struct drm_client *client = ptr;

	if (rc < 0) {
		printf("%p", client);
		return;
	}
	PRINT_FIELD_D("{", *client, idx);
	PRINT_FIELD_D(", ", *client, auth);
	PRINT_FIELD_U(", ", *client, pid);
	PRINT_FIELD_UID(", ", *client, uid);
	PRINT_FIELD_U(", ", *client, magic);
	PRINT_FIELD_U(", ", *client, iocs);
	printf("}");
}

static void
print_drm_set_version(long rc, void *ptr, void *arg)
{
	struct drm_set_version *ver = ptr;

	if (rc < 0) {
		printf("%p", ver);
		return;
	}
	PRINT_FIELD_D("{", *ver, drm_di_major);
	PRINT_FIELD_D(", ", *ver, drm_di_minor);
	PRINT_FIELD_D(", ", *ver, drm_dd_major);
	PRINT_FIELD_D(", ", *ver, drm_dd_minor);
	PRINT_FIELD_D("} => {", *ver, drm_di_major);
	PRINT_FIELD_D(", ", *ver, drm_di_minor);
	PRINT_FIELD_D(", ", *ver, drm_dd_major);
	PRINT_FIELD_D(", ", *ver, drm_dd_minor);
	printf("}");

}

static void
print_drm_modeset_ctl(long rc, void *ptr, void *arg)
{
	struct drm_modeset_ctl *ctl = ptr;

	if (rc < 0) {
		printf("%p", ctl);
		return;
	}
	PRINT_FIELD_U("{", *ctl, crtc);
	printf(", cmd=");
	printxval(drm_modeset_cmd, ctl->cmd, "_DRM_???");
	printf("}");
}

static void
print_drm_gem_close(long rc, void *ptr, void *arg)
{
	struct drm_gem_close *close = ptr;

	if (rc < 0) {
		printf("%p", close);
		return;
	}
	PRINT_FIELD_U("{", *close, handle);
	printf("}");
}

static void
print_drm_gem_flink(long rc, void *ptr, void *arg)
{
	struct drm_gem_flink *flink = ptr;

	if (rc < 0) {
		printf("%p", flink);
		return;
	}
	PRINT_FIELD_U("{", *flink, handle);
	PRINT_FIELD_U(", ", *flink, name);
	printf("}");
}

static void
print_drm_gem_open(long rc, void *ptr, void *arg)
{
	struct drm_gem_open *open = ptr;

	if (rc < 0) {
		printf("%p", open);
		return;
	}
	PRINT_FIELD_U("{", *open, name);
	PRINT_FIELD_U(", ", *open, handle);
	PRINT_FIELD_U(", ", *open, size);
	printf("}");
}

static void
print_drm_get_cap(long rc, void *ptr, void *arg)
{
	struct drm_get_cap *cap = ptr;

	if (rc < 0) {
		printf("%p", cap);
		return;
	}
	printf("{capability=");
	printxval(drm_capability, cap->capability, "DRM_CAP_???");
	PRINT_FIELD_U(", ", *cap, value);
	printf("}");
}

static void
print_drm_set_client_cap(long rc, void *ptr, void *arg)
{
	struct drm_set_client_cap *cap = ptr;

	if (rc < 0) {
		printf("%p", cap);
		return;
	}
	printf("{capability=");
	printxval(drm_client_capability, cap->capability,
		  "DRM_CLIENT_CAP_???");
	PRINT_FIELD_U(", ", *cap, value);
	printf("}");
}

static void
print_drm_auth_magic(long rc, void *ptr, void *arg)
{
	struct drm_auth *auth = ptr;

	if (rc < 0) {
		printf("%p", auth);
		return;
	}
	PRINT_FIELD_U("{", *auth, magic);
	printf("}");
}

static void
print_drm_control(long rc, void *ptr, void *arg)
{
	struct drm_control *control = ptr;

	if (rc < 0) {
		printf("%p", control);
		return;
	}
	printf("{func=");
	printxval(drm_control_func, control->func, "DRM_???");
	PRINT_FIELD_D(", ", *control, irq);
	printf("}");
}

static void
print_drm_add_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_X("{", *map, offset);
	PRINT_FIELD_U(", ", *map, size);
	printf(", type=");
	printxval(drm_map_type, map->type, "_DRM_???");
	printf(", flags=");
	printflags(drm_map_flags, map->flags, "_DRM_???");
	printf("}");
}

static void
print_drm_add_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_desc *desc = ptr;

	if (rc < 0) {
		printf("%p", desc);
		return;
	}
	PRINT_FIELD_D("{", *desc, count);
	PRINT_FIELD_D(", ", *desc, size);
	printf(", flags=");
	printflags(drm_buf_desc_flags, desc->flags, "_DRM_???");
	PRINT_FIELD_X(", ", *desc, agp_start);
	PRINT_FIELD_D("} => {", *desc, count);
	PRINT_FIELD_D(", ", *desc, size);
	printf("}");
}

static void
print_drm_mark_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_desc *desc = ptr;

	if (rc < 0) {
		printf("%p", desc);
		return;
	}
	PRINT_FIELD_D("{", *desc, size);
	PRINT_FIELD_D(", ", *desc, low_mark);
	PRINT_FIELD_D(", ", *desc, high_mark);
	printf("}");
}

static void
print_drm_info_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_info *info = ptr;
	struct drm_buf_desc *desc = arg;

	if (rc < 0) {
		printf("%p", info);
		return;
	}
	printf("{list=[");
	for (int i = 0; i < info->count; i++) {
		if (i)
			printf(", ");
		PRINT_FIELD_D("{", desc[i], count);
		PRINT_FIELD_D(", ", desc[i], size);
		PRINT_FIELD_D(", ", desc[i], low_mark);
		PRINT_FIELD_D(", ", desc[i], high_mark);
		printf(", flags=");
		printflags(drm_buf_desc_flags, desc[i].flags, "_DRM_???");
		printf("}");
	}
	printf("]");
	PRINT_FIELD_D(", ", *info, count);
	PRINT_FIELD_D("} => {", *info, count);
	printf("}");
}

static void
print_drm_map_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_map *map = ptr;
	struct drm_buf_pub *buf_pub = arg;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_D("{", *map, count);
	printf(", virtual=%p", map->virtual);
	printf(", list=[");
	for (int i = 0; i < map->count; i++) {
		if (i)
			printf(", ");
		PRINT_FIELD_D("{", buf_pub[i], idx);
		PRINT_FIELD_D(", ", buf_pub[i], total);
		PRINT_FIELD_D(", ", buf_pub[i], used);
		printf(", address=%p", buf_pub[i].address);
		printf("}");
	}
	printf("]}");
}

static void
print_drm_free_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_free *free = ptr;

	if (rc < 0) {
		printf("%p", free);
		return;
	}
	PRINT_FIELD_D("{", *free, count);
	printf(", list=[%d, %d]", smagic, smagic);
	printf("}");
}

static void
print_drm_rm_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	printf("{handle=%p}", map->handle);
}

static void
print_drm_sarea_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx_priv_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_U("{", *map, ctx_id);
	printf(", handle=%p}", map->handle);
}


static void
print_drm_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx *ctx = ptr;

	if (rc < 0) {
		printf("%p", ctx);
		return;
	}
	PRINT_FIELD_U("{", *ctx, handle);
	printf("}");
}

static void
print_drm_get_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx *ctx = ptr;

	if (rc < 0) {
		printf("%p", ctx);
		return;
	}
	printf("{flags=");
	printflags(drm_ctx_flags, ctx->flags, "_DRM_CONTEXT_???");
	printf("}");
}

static void
print_drm_res_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx_res *res = ptr;
	struct drm_ctx *ctx = arg;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	PRINT_FIELD_D("{", *res, count);
	PRINT_FIELD_D("} => {", *res, count);
	printf(", contexts=[");
	for (int i = 0; i < res->count; i++) {
		if (i)
			printf(", ");
		PRINT_FIELD_U("{", ctx[i], handle);
		printf(", flags=");
		printflags(drm_ctx_flags, ctx[i].flags, "_DRM_CONTEXT_???");
		printf("}");
	}
	printf("]}");
}

static void
print_drm_lock(long rc, void *ptr, void *arg)
{
	struct drm_lock *lock = ptr;

	if (rc < 0) {
		printf("%p", lock);
		return;
	}
	PRINT_FIELD_D("{", *lock, context);
	printf(", flags=");
	printflags(drm_lock_flags, lock->flags, "_DRM_LOCK_???");
	printf("}");
}

static void
print_drm_prime_handle_to_fd(long rc, void *ptr, void *arg)
{
	struct drm_prime_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_U("{", *handle, handle);
	printf(", flags=");
	printflags(drm_prime_handle_flags, handle->flags, "DRM_???");
	PRINT_FIELD_D(", ", *handle, fd);
	printf("}");
}

static void
print_drm_prime_fd_to_handle(long rc, void *ptr, void *arg)
{
	struct drm_prime_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_D("{", *handle, fd);
	PRINT_FIELD_U(", ", *handle, handle);
	printf("}");
}

static void
print_drm_agp_enable(long rc, void *ptr, void *arg)
{
	struct drm_agp_mode *mode = ptr;

	if (rc < 0) {
		printf("%p", mode);
		return;
	}
	PRINT_FIELD_U("{", *mode, mode);
	printf("}");
}

static void
print_drm_agp_info(long rc, void *ptr, void *arg)
{
	struct drm_agp_info *info = ptr;

	if (rc < 0) {
		printf("%p", info);
		return;
	}
	PRINT_FIELD_D("{", *info, agp_version_major);
	PRINT_FIELD_D(", ", *info, agp_version_minor);
	PRINT_FIELD_U(", ", *info, mode);
	PRINT_FIELD_X(", ", *info, aperture_base);
	PRINT_FIELD_U(", ", *info, aperture_size);
	PRINT_FIELD_U(", ", *info, memory_allowed);
	PRINT_FIELD_U(", ", *info, memory_used);
	PRINT_FIELD_X(", ", *info, id_vendor);
	PRINT_FIELD_X(", ", *info, id_device);
	printf("}");
}

static void
print_drm_agp_alloc(long rc, void *ptr, void *arg)
{
	struct drm_agp_buffer *buffer = ptr;

	if (rc < 0) {
		printf("%p", buffer);
		return;
	}
	PRINT_FIELD_U("{", *buffer, size);
	PRINT_FIELD_U(", ", *buffer, type);
	PRINT_FIELD_U(", ", *buffer, handle);
	PRINT_FIELD_U(", ", *buffer, physical);
	printf("}");
}

static void
print_drm_agp_free(long rc, void *ptr, void *arg)
{
	struct drm_agp_buffer *buffer = ptr;

	if (rc < 0) {
		printf("%p", buffer);
		return;
	}
	PRINT_FIELD_U("{", *buffer, handle);
	printf("}");
}

static void
print_drm_agp_bind(long rc, void *ptr, void *arg)
{
	struct drm_agp_binding *binding = ptr;

	if (rc < 0) {
		printf("%p", binding);
		return;
	}
	PRINT_FIELD_U("{", *binding, handle);
	PRINT_FIELD_X(", ", *binding, offset);
	printf("}");
}

static void
print_drm_agp_unbind(long rc, void *ptr, void *arg)
{
	struct drm_agp_binding *binding = ptr;

	if (rc < 0) {
		printf("%p", binding);
		return;
	}
	PRINT_FIELD_U("{", *binding, handle);
	printf("}");
}

static void
print_drm_sg_alloc(long rc, void *ptr, void *arg)
{
	struct drm_scatter_gather *sg = ptr;

	if (rc < 0) {
		printf("%p", sg);
		return;
	}
	PRINT_FIELD_U("{", *sg, size);
	PRINT_FIELD_U(", ", *sg, handle);
	printf("}");
}

static void
print_drm_sg_free(long rc, void *ptr, void *arg)
{
	struct drm_scatter_gather *sg = ptr;

	if (rc < 0) {
		printf("%p", sg);
		return;
	}
	PRINT_FIELD_U("{", *sg, handle);
	printf("}");
}

static void
print_drm_wait_vblank(long rc, void *ptr, void *arg)
{
	union drm_wait_vblank *vblank = ptr;

	if (rc < 0) {
		printf("%p", vblank);
		return;
	}
	printf("{request={type=");
	printxval(drm_vblank_seq_type,
		  vblank->request.type & _DRM_VBLANK_TYPES_MASK,
		  "_DRM_VBLANK_???");
	printf(", type.flags=");
	printflags(drm_vblank_seq_type_flags,
		   vblank->request.type & _DRM_VBLANK_FLAGS_MASK,
		   "_DRM_VBLANK_???");
	PRINT_FIELD_U(", ", vblank->request, sequence);
	PRINT_FIELD_U(", ", vblank->request, signal);
	printf("}");

	printf(", {reply={type=");
	printxval(drm_vblank_seq_type,
		  vblank->reply.type & _DRM_VBLANK_TYPES_MASK,
		  "_DRM_VBLANK_???");
	printf(", type.flags=");
	printflags(drm_vblank_seq_type_flags,
		   vblank->reply.type & _DRM_VBLANK_FLAGS_MASK,
		   "_DRM_VBLANK_???");
	PRINT_FIELD_U(", ", vblank->reply, sequence);
	PRINT_FIELD_D(", ", vblank->reply, tval_sec);
	PRINT_FIELD_D(", ", vblank->reply, tval_usec);
	printf("}}");
}

# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
static void
print_drm_crtc_get_sequence(long rc, void *ptr, void *arg)
{
	struct drm_crtc_get_sequence *seq = ptr;

	if (rc < 0) {
		printf("%p", seq);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *seq, crtc_id);
	PRINT_FIELD_U(", ", *seq, active);
	PRINT_FIELD_U(", ", *seq, sequence);
	PRINT_FIELD_D(", ", *seq, sequence_ns);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
static void
print_drm_crtc_queue_sequence(long rc, void *ptr, void *arg)
{
	struct drm_crtc_queue_sequence *seq = ptr;

	if (rc < 0) {
		printf("%p", seq);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *seq, crtc_id);
	printf(", flags=");
	printflags(drm_crtc_sequence_flags, seq->flags,
		   "DRM_CRTC_SEQUENCE_???");
	PRINT_FIELD_X(", ", *seq, user_data);
	PRINT_FIELD_U(", ", *seq, sequence);
	PRINT_FIELD_U("} => {", *seq, sequence);
	printf("}");
}
# endif

static void
print_drm_mode_get_resources(long rc, void *ptr, void *arg)
{
	struct drm_mode_card_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	printf("{fb_id_ptr=[%u, %u]", umagic, umagic);
	printf(", crtc_id_ptr=[%u, %u]", umagic, umagic);
	printf(", connector_id_ptr=[%u, %u]", umagic, umagic);
	printf(", encoder_id_ptr=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U(", ", *res, count_fbs);
	PRINT_FIELD_U(", ", *res, count_crtcs);
	PRINT_FIELD_U(", ", *res, count_connectors);
	PRINT_FIELD_U(", ", *res, count_encoders);
	PRINT_FIELD_U("} => {", *res, count_fbs);
	PRINT_FIELD_U(", ", *res, count_crtcs);
	PRINT_FIELD_U(", ", *res, count_connectors);
	PRINT_FIELD_U(", ", *res, count_encoders);
	PRINT_FIELD_U(", ", *res, min_width);
	PRINT_FIELD_U(", ", *res, max_width);
	PRINT_FIELD_U(", ", *res, min_height);
	PRINT_FIELD_U(", ", *res, max_height);
	printf("}");
}

static void
drm_mode_print_modeinfo(struct drm_mode_modeinfo *info)
{
	PRINT_FIELD_U("{", *info, clock);
	PRINT_FIELD_U(", ", *info, hdisplay);
	PRINT_FIELD_U(", ", *info, hsync_start);
	PRINT_FIELD_U(", ", *info, hsync_end);
	PRINT_FIELD_U(", ", *info, htotal);
	PRINT_FIELD_U(", ", *info, hskew);
	PRINT_FIELD_U(", ", *info, vdisplay);
	PRINT_FIELD_U(", ", *info, vsync_start);
	PRINT_FIELD_U(", ", *info, vsync_end);
	PRINT_FIELD_U(", ", *info, vtotal);
	PRINT_FIELD_U(", ", *info, vscan);
	PRINT_FIELD_U(", ", *info, vrefresh);
	printf(", flags=");
	printflags(drm_mode_flags, info->flags, "DRM_MODE_FLAG_PIC_???");
	printf(", type=");
	printxval(drm_mode_type, info->type, "DRM_MODE_TYPE_???");
	printf(", name=\"%s\"}", info->name);
}

static void
print_drm_mode_crtc(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc *crtc = ptr;
	int *is_get = arg;

	if (rc < 0) {
		printf("%p", crtc);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *crtc, crtc_id);
	if (!*is_get) {
		printf(", set_connectors_ptr=[%u, %u]", umagic, umagic);
		PRINT_FIELD_U(", ", *crtc, count_connectors);
	}
	PRINT_FIELD_U(", ", *crtc, fb_id);
	PRINT_FIELD_U(", ", *crtc, x);
	PRINT_FIELD_U(", ", *crtc, y);
	PRINT_FIELD_U(", ", *crtc, gamma_size);
	PRINT_FIELD_U(", ", *crtc, mode_valid);
	printf(", mode=");
	drm_mode_print_modeinfo(&crtc->mode);
	printf("}");
}

static void
print_drm_mode_cursor(long rc, void *ptr, void *arg)
{
	struct drm_mode_cursor *cursor = ptr;

	if (rc < 0) {
		printf("%p", cursor);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_cursor_flags, cursor->flags,
		   "DRM_MODE_CURSOR_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *cursor, crtc_id);
	PRINT_FIELD_D(", ", *cursor, x);
	PRINT_FIELD_D(", ", *cursor, y);
	PRINT_FIELD_U(", ", *cursor, width);
	PRINT_FIELD_U(", ", *cursor, height);
	PRINT_FIELD_U(", ", *cursor, handle);
	printf("}");
}

static void
print_drm_mode_gamma(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_lut *lut = ptr;

	if (rc < 0) {
		printf("%p", lut);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *lut, crtc_id);
	PRINT_FIELD_U(", ", *lut, gamma_size);
	printf(", red=[%u, %u]", u16_magic, u16_magic);
	printf(", green=[%u, %u]", u16_magic, u16_magic);
	printf(", blue=[%u, %u]", u16_magic, u16_magic);
	printf("}");
}

static void
print_drm_mode_get_encoder(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_encoder *enc = ptr;

	if (rc < 0) {
		printf("%p", enc);
		return;
	}
	PRINT_FIELD_U("{", *enc, encoder_id);
	PRINT_FIELD_U("} => {", *enc, encoder_id);
	printf(", encoder_type=");
	printxval(drm_mode_encoder_type, enc->encoder_type,
		  "DRM_MODE_ENCODER_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *enc, crtc_id);
	PRINT_FIELD_X(", ", *enc, possible_crtcs);
	PRINT_FIELD_X(", ", *enc, possible_clones);
	printf("}");
}

static void
print_drm_mode_get_connector(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_connector *con = ptr;
	struct drm_mode_modeinfo *info = arg;

	if (rc < 0) {
		printf("%p", con);
		return;
	}
	PRINT_FIELD_U("{", *con, connector_id);
	PRINT_FIELD_U(", ", *con, count_modes);
	PRINT_FIELD_U(", ", *con, count_props);
	PRINT_FIELD_U(", ", *con, count_encoders);
	PRINT_FIELD_U("} => {", *con, connector_id);
	PRINT_FIELD_U(", ", *con, count_modes);
	PRINT_FIELD_U(", ", *con, count_props);
	PRINT_FIELD_U(", ", *con, count_encoders);
	printf(", encoders_ptr=[%u, %u]", umagic, umagic);
	printf(", modes_ptr=[");
	for (unsigned int i = 0; i < con->count_modes; i++) {
		if (i)
			printf(", ");
		drm_mode_print_modeinfo(&info[i]);
	}
	printf("]");
	printf(", props_ptr=[%u, %u]", umagic, umagic);
	printf(", prop_values_ptr=[%lu, %lu]", ulmagic, ulmagic);
	PRINT_FIELD_U(", ", *con, encoder_id);
	PRINT_FIELD_U(", ", *con, connector_type);
	PRINT_FIELD_U(", ", *con, connector_type_id);
	PRINT_FIELD_U(", ", *con, connection);
	PRINT_FIELD_U(", ", *con, mm_width);
	PRINT_FIELD_U(", ", *con, mm_height);
	PRINT_FIELD_U(", ", *con, subpixel);
	printf("}");
}

static void
print_drm_mode_get_property(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_property *prop = ptr;
	struct drm_mode_property_enum *prop_enum = arg;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, count_values);
	PRINT_FIELD_U(", ", *prop, count_enum_blobs);
	printf("} => {values_ptr=[%lu, %lu]", ulmagic, ulmagic);
	printf(", enum_blob_ptr=[");
	for (unsigned int i = 0; i < prop->count_enum_blobs; i++) {
		if (i)
			printf(", ");
		PRINT_FIELD_U("{", prop_enum[i], value);
		PRINT_FIELD_CSTRING(", ", prop_enum[i], name);
		printf("}");
	}
	printf("]");
	printf(", flags=");
	printflags(drm_mode_get_property_flags, prop->flags,
		   "DRM_MODE_PROP_???");
	PRINT_FIELD_CSTRING(", ", *prop, name);
	PRINT_FIELD_U(", ", *prop, count_values);
	PRINT_FIELD_U(", ", *prop, count_enum_blobs);
	printf("}");
}

static void
print_drm_mode_set_property(long rc, void *ptr, void *arg)
{
	struct drm_mode_connector_set_property *prop = ptr;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, value);
	PRINT_FIELD_U(", ", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, connector_id);
	printf("}");
}

static void
print_drm_mode_get_prop_blob(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_U("{", *blob, blob_id);
	PRINT_FIELD_U(", ", *blob, length);
	PRINT_FIELD_U("} => {", *blob, length);
	PRINT_FIELD_ADDR64(", ", *blob, data);
	printf("}");
}

static void
print_drm_mode_get_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, fb_id);
	PRINT_FIELD_U(", ", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_U(", ", *cmd, pitch);
	PRINT_FIELD_U(", ", *cmd, bpp);
	PRINT_FIELD_U(", ", *cmd, depth);
	PRINT_FIELD_U(", ", *cmd, handle);
	printf("}");

}

static void
print_drm_mode_add_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_U(", ", *cmd, pitch);
	PRINT_FIELD_U(", ", *cmd, bpp);
	PRINT_FIELD_U(", ", *cmd, depth);
	PRINT_FIELD_U(", ", *cmd, handle);
	PRINT_FIELD_U(", ", *cmd, fb_id);
	printf("}");
}

static void
print_drm_mode_rm_fb(long rc, void *ptr, void *arg)
{
	unsigned int *fb_id = ptr;

	if (rc < 0) {
		printf("%p", fb_id);
		return;
	}
	printf("{fb_id=%u}", *fb_id);
}

static void
print_drm_mode_page_flip(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_page_flip *flip = ptr;

	if (rc < 0) {
		printf("%p", flip);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *flip, crtc_id);
	PRINT_FIELD_U(", ", *flip, fb_id);
	printf(", flags=");
	printflags(drm_mode_page_flip_flags, flip->flags,
		   "DRM_MODE_PAGE_FLIP_???");
	PRINT_FIELD_X(", ", *flip, user_data);
	printf("}");
}

static void
print_drm_mode_dirty_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_dirty_cmd *cmd = ptr;
	struct drm_clip_rect *rect = arg;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, fb_id);
	printf(", flags=");
	printflags(drm_mode_fb_dirty_cmd_flags, cmd->flags,
		   "DRM_MODE_FB_DIRTY_ANNOTATE_???");
	PRINT_FIELD_X(", ", *cmd, color);
	PRINT_FIELD_U(", ", *cmd, num_clips);
	printf(", clips_ptr=[");
	for (unsigned int i = 0; i < cmd->num_clips; i++) {
		if (i)
			printf(", ");
		PRINT_FIELD_U("{", rect[i], x1);
		PRINT_FIELD_U(", ", rect[i], y1);
		PRINT_FIELD_U(", ", rect[i], x2);
		PRINT_FIELD_U(", ", rect[i], y2);
		printf("}");
	}
	printf("]}");
}

static void
print_drm_mode_create_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, width);
	PRINT_FIELD_U(", ", *dumb, height);
	PRINT_FIELD_U(", ", *dumb, bpp);
	PRINT_FIELD_X(", ", *dumb, flags);
	PRINT_FIELD_U(", ", *dumb, handle);
	PRINT_FIELD_U(", ", *dumb, pitch);
	PRINT_FIELD_U(", ", *dumb, size);
	printf("}");
}

static void
print_drm_mode_map_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_map_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, handle);
	PRINT_FIELD_U(", ", *dumb, offset);
	printf("}");
}

static void
print_drm_mode_destroy_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_destroy_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, handle);
	printf("}");
}

static void
print_drm_mode_getplaneresources(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_plane_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	printf("{plane_id_ptr=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U(", ", *res, count_planes);
	PRINT_FIELD_U("} => {", *res, count_planes);
	printf("}");
}

static void
print_drm_mode_getplane(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_plane *plane = ptr;

	if (rc < 0) {
		printf("%p", plane);
		return;
	}
	PRINT_FIELD_U("{", *plane, plane_id);
	PRINT_FIELD_U(", ", *plane, count_format_types);
	printf(", format_type_ptr=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U("} => {", *plane, plane_id);
	PRINT_FIELD_DRM_CRTC_ID(", ", *plane, crtc_id);
	PRINT_FIELD_U(", ", *plane, fb_id);
	PRINT_FIELD_U(", ", *plane, possible_crtcs);
	PRINT_FIELD_U(", ", *plane, gamma_size);
	PRINT_FIELD_U(", ", *plane, count_format_types);
	printf("}");
}

static void
print_drm_mode_setplane(long rc, void *ptr, void *arg)
{
	struct drm_mode_set_plane *plane = ptr;

	if (rc < 0) {
		printf("%p", plane);
		return;
	}
	PRINT_FIELD_U("{", *plane, plane_id);
	PRINT_FIELD_DRM_CRTC_ID(", ", *plane, crtc_id);
	PRINT_FIELD_U(", ", *plane, fb_id);
	printf(", flags=");
	printflags(drm_mode_set_plane_flags, plane->flags,
		   "DRM_MODE_PRESENT_???");
	PRINT_FIELD_D(", ", *plane, crtc_x);
	PRINT_FIELD_D(", ", *plane, crtc_y);
	PRINT_FIELD_U(", ", *plane, crtc_w);
	PRINT_FIELD_U(", ", *plane, crtc_h);
	PRINT_FIELD_U(", ", *plane, src_x);
	printf(" /* %u.%06u */", plane->src_x >> 16,
	       ((plane->src_x & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_y);
	printf(" /* %u.%06u */", plane->src_y >> 16,
	       ((plane->src_y & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_h);
	printf(" /* %u.%06u */", plane->src_h >> 16,
	       ((plane->src_h & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_w);
	printf(" /* %u.%06u */", plane->src_w >> 16,
	       ((plane->src_w & 0xffff) * 15625) >> 10);
	printf("}");
}

static void
print_drm_mode_add_fb2(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd2 *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_X(", ", *cmd, pixel_format);
	printf(", flags=");
	printflags(drm_mode_fb_cmd2_flags, cmd->flags, "DRM_MODE_FB_???");
	printf(", handles=[%u, %u, %u, %u]",
	       cmd->handles[0], cmd->handles[1],
	       cmd->handles[2], cmd->handles[3]);
	printf(", pitches=[%u, %u, %u, %u]",
	       cmd->pitches[0], cmd->pitches[1],
	       cmd->pitches[2], cmd->pitches[3]);
	printf(", offsets=[%u, %u, %u, %u]",
	       cmd->offsets[0], cmd->offsets[1],
	       cmd->offsets[2], cmd->offsets[3]);
# ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
	printf(", modifiers=[%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "]",
	       (uint64_t) cmd->modifier[0], (uint64_t) cmd->modifier[1],
	       (uint64_t) cmd->modifier[2], (uint64_t) cmd->modifier[3]);
# endif
	PRINT_FIELD_U(", ", *cmd, fb_id);
	printf("}");
}

static void
print_drm_mode_obj_getproperties(long rc, void *ptr, void *arg)
{
	struct drm_mode_obj_get_properties *props = ptr;

	if (rc < 0) {
		printf("%p", props);
		return;
	}
	printf("{props_ptr=[%u, %u]", umagic, umagic);
	printf(", prop_values_ptr=[%lu, %lu]", ulmagic, ulmagic);
	PRINT_FIELD_U(", ", *props, count_props);
	PRINT_FIELD_U(", ", *props, obj_id);
	PRINT_FIELD_U(", ", *props, obj_type);
	PRINT_FIELD_U("} => {", *props, count_props);
	printf("}");
}

static void
print_drm_mode_obj_setproperty(long rc, void *ptr, void *arg)
{
	struct drm_mode_obj_set_property *prop = ptr;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, value);
	PRINT_FIELD_U(", ", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, obj_id);
	PRINT_FIELD_U(", ", *prop, obj_type);
	printf("}");
}

static void
print_drm_mode_cursor2(long rc, void *ptr, void *arg)
{
	struct drm_mode_cursor2 *cursor2 = ptr;

	if (rc < 0) {
		printf("%p", cursor2);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_cursor_flags, cursor2->flags,
		   "DRM_MODE_CURSOR_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *cursor2, crtc_id);
	PRINT_FIELD_D(", ", *cursor2, x);
	PRINT_FIELD_D(", ", *cursor2, y);
	PRINT_FIELD_U(", ", *cursor2, width);
	PRINT_FIELD_U(", ", *cursor2, height);
	PRINT_FIELD_U(", ", *cursor2, handle);
	PRINT_FIELD_D(", ", *cursor2, hot_x);
	PRINT_FIELD_D(", ", *cursor2, hot_y);
	printf("}");
}

static void
print_drm_mode_atomic(long rc, void *ptr, void *arg)
{
	struct drm_mode_atomic *atomic = ptr;

	if (rc < 0) {
		printf("%p", atomic);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_atomic_flags, atomic->flags,
		   "DRM_MODE_ATOMIC_???");
	PRINT_FIELD_U(", ", *atomic, count_objs);
	printf(", objs_ptr=[%u, %u]", umagic, umagic);
	printf(", count_props_ptr=[%u, %u]", umagic, umagic);
	printf(", props_ptr=[%u, %u]", umagic, umagic);
	printf(", prop_values_ptr=[%lu, %lu]", ulmagic, ulmagic);
	if (atomic->reserved)
		PRINT_FIELD_U(", ", *atomic, reserved);
	PRINT_FIELD_X(", ", *atomic, user_data);
	printf("}");
}

static void
print_drm_mode_createpropblob(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_ADDR64("{", *blob, data);
	PRINT_FIELD_U(", ", *blob, length);
	PRINT_FIELD_U(", ", *blob, blob_id);
	printf("}");
}

static void
print_drm_destroypropblob(long rc, void *ptr, void *arg)
{
	struct drm_mode_destroy_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_U("{", *blob, blob_id);
	printf("}");
}

# ifdef DRM_IOCTL_SYNCOBJ_CREATE
static void
print_drm_syncobj_create(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_create *create = ptr;

	if (rc < 0) {
		printf("%p", create);
		return;
	}
	printf("{flags=");
	printflags(drm_syncobj_flags, create->flags, "DRM_SYNCOJB_???");
	PRINT_FIELD_U(", ", *create, handle);
	printf("}");
}

# endif

# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
static void
print_drm_syncobj_destroy(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_destroy *destroy = ptr;

	if (rc < 0) {
		printf("%p", destroy);
		return;
	}
	PRINT_FIELD_U("{", *destroy, handle);
	if (destroy->pad)
		PRINT_FIELD_U(", ", *destroy, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
static void
print_drm_syncobj_handle_to_fd(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_U("{", *handle, handle);
	printf(", flags=");
	printflags(drm_syncobj_handle_to_fd_flags, handle->flags, "DRM_SYNCOBJ_???");
	if (handle->pad)
		PRINT_FIELD_U(", ", *handle, pad);
	PRINT_FIELD_D(", ", *handle, fd);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE
static void
print_drm_syncobj_fd_to_handle(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_D("{", *handle, fd);
	printf(", flags=");
	printflags(drm_syncobj_fd_to_handle_flags, handle->flags,
		   "DRM_SYNCOBJ_???");
	if (handle->pad)
		PRINT_FIELD_U(", ", *handle, pad);
	PRINT_FIELD_U(", ", *handle, handle);
	printf("}");

}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_WAIT
static void
print_drm_syncobj_wait(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_wait *wait = ptr;

	if (rc < 0) {
		printf("%p", wait);
		return;
	}
	printf("{handles=[%u, %u]", umagic, umagic);
	PRINT_FIELD_D(", ", *wait, timeout_nsec);
	PRINT_FIELD_U(", ", *wait, count_handles);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, wait->flags,
		   "DRM_SYNCOBJ_WAIT_FLAGS_???");
	PRINT_FIELD_U(", ", *wait, first_signaled);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_RESET
static void
print_drm_syncobj_reset_or_signal(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_array *array = ptr;

	if (rc < 0) {
		printf("%p", array);
		return;
	}
	printf("{handles=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U(", ", *array, count_handles);
	if (array->pad)
		PRINT_FIELD_U(", ", *array, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
static void
print_drm_mode_create_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_lease *lease= ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	printf("{object_ids=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U(", ", *lease, object_count);
	PRINT_FIELD_X(", ", *lease, flags);
	printf(", flags=");
	printflags(drm_mode_create_lease_flags, lease->flags, "O_???");
	PRINT_FIELD_U(", ", *lease, lessee_id);
	PRINT_FIELD_D(", ", *lease, fd);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_LIST_LESSEES
static void
print_drm_mode_list_lessees(long rc, void *ptr, void *arg)
{
	struct drm_mode_list_lessees *lessees = ptr;

	if (rc < 0) {
		printf("%p", lessees);
		return;
	}
	PRINT_FIELD_U("{", *lessees, count_lessees);
	if (lessees->pad)
		PRINT_FIELD_U(", ", *lessees, pad);
	printf(", lessees_ptr=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U("} => {", *lessees, count_lessees);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
static void
print_drm_mode_get_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_lease *lease = ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	PRINT_FIELD_U("{", *lease, count_objects);
	if (lease->pad)
		PRINT_FIELD_U(", ", *lease, pad);
	printf(", objects_ptr=[%u, %u]", umagic, umagic);
	PRINT_FIELD_U("} => {", *lease, count_objects);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
static void
print_drm_mode_revoke_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_revoke_lease *lease = ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	PRINT_FIELD_U("{", *lease, lessee_id);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
static void
print_drm_syncobj_timeline_wait(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_timeline_wait *wait = ptr;

	if (rc < 0) {
		printf("%p", wait);
		return;
	}
	printf("{handles=[%u, %u]", umagic, umagic);
	PRINT_FIELD_D(", ", *wait, timeout_nsec);
	PRINT_FIELD_U(", ", *wait, count_handles);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, wait->flags,
		   "DRM_SYNCOBJ_WAIT_FLAGS_???");
	PRINT_FIELD_U(", ", *wait, first_signaled);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_QUERY
static void
print_drm_syncobj_query_or_timeline_signal(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_timeline_array *array = ptr;

	if (rc < 0) {
		printf("%p", array);
		return;
	}
	printf("{handles=[%u, %u]", umagic, umagic);
	printf(", points=[%lu, %lu]", ulmagic, ulmagic);
	PRINT_FIELD_U(", ", *array, count_handles);
	if (array->pad)
		PRINT_FIELD_U(", ", *array, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
static void
print_drm_syncobj_transfer(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_transfer *transfer = ptr;

	if (rc < 0) {
		printf("%p", transfer);
		return;
	}
	PRINT_FIELD_U("{", *transfer, src_handle);
	PRINT_FIELD_U(", ", *transfer, dst_handle);
	PRINT_FIELD_U(", ", *transfer, src_point);
	PRINT_FIELD_U(", ", *transfer, dst_point);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, transfer->flags,
		   "DRM_SYNCOBJ_WAIT_FLAGS_???");
	if (transfer->pad)
		PRINT_FIELD_U(", ", *transfer, pad);
	printf("}");
}
# endif

int
main(int argc, char **argv)
{
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	/* This arrays are for later use of printing array. */
	/* We only set a two-element array in order to reduce */
	/* complication of printing array for some functions. */
	uint16_t u16_array[2] = { u16_magic, u16_magic };
	uint32_t u32_array[2] = { umagic, umagic };
	uint64_t u64_array[2] = { ulmagic, ulmagic };

	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = ioctl(-1, DRM_IOCTL_VERSION, NULL);
		printf("ioctl(-1, DRM_IOCTL_VERSION, NULL) = %s%s\n",
		       sprintrc(rc),
		       rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", DRM_IOCTL_VERSION, NULL) returning %lu",
				   inject_retval);

	/* drm_version */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_version, ver);
	char bogus_name[4096] = "bogus_name";
	char bogus_date[4096] = "bogus_date";
	char bogus_desc[4096] = "bogus_desc";
	ver->version_major = smagic;
	ver->version_minor = smagic;
	ver->version_patchlevel = smagic;
	ver->name_len = strlen(bogus_name);
	ver->name = bogus_name;
	ver->date_len = strlen(bogus_date);
	ver->date = bogus_date;
	ver->desc_len = strlen(bogus_desc);
	ver->desc = bogus_desc;

	/* drm_unique */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_unique, unique);
	char bogus_unique[4096] = "bogus_unique";
	unique->unique_len = strlen(bogus_unique);
	unique->unique = bogus_unique;

	/* drm_auth */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_auth, auth);
	auth->magic = umagic;

	/* drm_irq_busid */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_irq_busid, busid);
	busid->busnum = smagic;
	busid->devnum = smagic;
	busid->funcnum = smagic;
	busid->irq = smagic;

	/* drm_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_map, map);
	map->offset = ulmagic;
	map->size = ulmagic;
	map->type = _DRM_FRAME_BUFFER;
	map->flags = _DRM_RESTRICTED;
	map->handle = (void *) ulmagic;
	map->mtrr = umagic;

	/* drm_client */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_client, client);
	client->idx = umagic;
	client->auth = umagic;
	client->pid = umagic;
	client->uid = umagic;

	/* drm_stats */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_stats, stats);
	stats->count = umagic;
	for (unsigned int i = 0; i < 15; i++) {
		stats->data[i].value = ulmagic;
		stats->data[i].type = _DRM_STAT_LOCK;
	}

	/* drm_set_version */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_set_version, set_ver);
	set_ver->drm_di_major = smagic;
	set_ver->drm_di_minor = smagic;
	set_ver->drm_dd_major = smagic;
	set_ver->drm_dd_minor = smagic;

	/* drm_modeset_ctl */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_modeset_ctl, ctl);
	ctl->crtc = umagic;
	ctl->cmd = _DRM_PRE_MODESET;

	/* drm_gem_close */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_close, close);
	close->handle = umagic;

	/* drm_gem_flink */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_flink, flink);
	flink->handle = umagic;
	flink->name = umagic;

	/* drm_gen_open */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_open, open);
	open->name = umagic;
	open->handle = umagic;
	open->size = ulmagic;

	/* drm_get_cap */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_get_cap, cap);
	cap->capability = DRM_CAP_DUMB_BUFFER;
	cap->value = ulmagic;

	/* drm_set_client_cap */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_set_client_cap, cli_cap);
	cli_cap->capability = DRM_CLIENT_CAP_STEREO_3D;
	cli_cap->value = ulmagic;

	/* drm_control */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_control, control);
	control->func = DRM_ADD_COMMAND;
	control->irq = smagic;

	/* drm_buf_desc */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_desc, desc);
	desc->count = smagic;
	desc->size = smagic;
	desc->low_mark = smagic;
	desc->high_mark = smagic;
	desc->flags = _DRM_PAGE_ALIGN;
	desc->agp_start = ulmagic;

	/* drm_buf_info */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_info, info);
	struct drm_buf_desc buf_desc[2] = {
		{ smagic, smagic, smagic, smagic, _DRM_PAGE_ALIGN | _DRM_AGP_BUFFER },
		{ smagic, smagic, smagic, smagic, _DRM_SG_BUFFER | _DRM_FB_BUFFER },
	};
	info->count = 2;
	info->list = buf_desc;

	/* drm_buf_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_map, buf_map);
	struct drm_buf_pub buf_pub[2] = {
		{ smagic, smagic, smagic, (void *) ulmagic },
		{ smagic, smagic, smagic, (void *) ulmagic },
	};
	buf_map->count = 2;
	buf_map->virtual = (void *) ulmagic;
	buf_map->list = buf_pub;

	/* drm_buf_free */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_free, buf_free);
	int s32_array[2] = { smagic, smagic };
	buf_free->count = 2;
	buf_free->list = s32_array;

	/* drm_ctx_priv_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx_priv_map, priv_map);
	priv_map->ctx_id = umagic;
	priv_map->handle = (void *) ulmagic;

	/* drm_ctx */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx, ctx);
	ctx->handle = umagic;
	ctx->flags = _DRM_CONTEXT_PRESERVED;

	/* drm_ctx_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx_res, ctx_res);
	struct drm_ctx ctx_var[2] = {
		{ umagic, _DRM_CONTEXT_PRESERVED | _DRM_CONTEXT_2DONLY },
		{ umagic, 0 }
	};
	ctx_res->count = 2;
	ctx_res->contexts = ctx_var;

	/* drm_draw */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_draw, draw);
	draw->handle = umagic;

	/* drm_lock */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_lock, lock);
	lock->context = smagic;
	lock->flags = _DRM_LOCK_READY;

	/* drm_prime_handle */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_prime_handle, prime_handle);
	prime_handle->handle = umagic;
	prime_handle->flags = DRM_CLOEXEC;
	prime_handle->fd = smagic;

	/* drm_agp_mode */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_mode, agp_mode);
	agp_mode->mode = ulmagic;

	/* drm_agp_info */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_info, agp_info);
	agp_info->agp_version_major = smagic;
	agp_info->agp_version_minor = smagic;
	agp_info->mode = ulmagic;
	agp_info->aperture_base = ulmagic;
	agp_info->aperture_size = ulmagic;
	agp_info->memory_allowed = ulmagic;
	agp_info->memory_used = ulmagic;
	agp_info->id_vendor = u16_magic;
	agp_info->id_device = u16_magic;

	/* drm_agp_buffer */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_buffer, agp_buffer);
	agp_buffer->size = ulmagic;
	agp_buffer->handle = ulmagic;
	agp_buffer->type = ulmagic;
	agp_buffer->physical = ulmagic;

	/* drm_agp_binding */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_binding, agp_binding);
	agp_binding->handle = ulmagic;
	agp_binding->offset = ulmagic;

	/* drm_scatter_gather */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_scatter_gather, sg);
	sg->size = ulmagic;
	sg->handle = ulmagic;

	/* drm_wait_vblank */
	TAIL_ALLOC_OBJECT_CONST_PTR(union drm_wait_vblank, vblank);
	vblank->request.type = _DRM_VBLANK_ABSOLUTE | _DRM_VBLANK_SIGNAL;
	vblank->request.sequence = umagic;
	vblank->request.signal = ulmagic;
	vblank->reply.tval_usec = slmagic;

# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
	/* drm_crtc_get_sequence */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_crtc_get_sequence, get_seq);
	get_seq->crtc_id = umagic;
	get_seq->active = umagic;
	get_seq->sequence = ulmagic;
	get_seq->sequence_ns = slmagic;
# endif

# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
	/* drm_crtc_queue_sequence */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_crtc_queue_sequence, queue_seq);
	queue_seq->crtc_id = umagic;
	queue_seq->flags = DRM_CRTC_SEQUENCE_RELATIVE;
	queue_seq->sequence = ulmagic;
	queue_seq->user_data = ulmagic;
# endif

	/* drm_update_draw */

	/* drm_mode_card_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_card_res, card_res);

	card_res->fb_id_ptr = (uintptr_t) u32_array;
	card_res->crtc_id_ptr = (uintptr_t) u32_array;
	card_res->connector_id_ptr = (uintptr_t) u32_array;
	card_res->encoder_id_ptr = (uintptr_t) u32_array;
	card_res->count_fbs = 2;
	card_res->count_crtcs = 2;
	card_res->count_connectors = 2;
	card_res->count_encoders = 2;
	card_res->min_width = umagic;
	card_res->max_width = umagic;
	card_res->min_height = umagic;
	card_res->max_height = umagic;

	/* drm_mode_crtc */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc, crtc);
	char bogus_mode_name[DRM_DISPLAY_MODE_LEN] = "bogus_mode_name";
	crtc->crtc_id = umagic;
	crtc->set_connectors_ptr = (uintptr_t) u32_array;
	crtc->count_connectors = 2;
	crtc->fb_id = umagic;
	crtc->x = umagic;
	crtc->y = umagic;
	crtc->gamma_size = umagic;
	crtc->mode_valid = umagic;
	snprintf(crtc->mode.name, sizeof(bogus_mode_name), "%s", bogus_mode_name);

	/* drm_mode_cursor */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor, cursor);
	cursor->flags = DRM_MODE_CURSOR_BO | DRM_MODE_CURSOR_MOVE;
	cursor->crtc_id = umagic;
	cursor->x = smagic;
	cursor->y = smagic;
	cursor->width = umagic;
	cursor->height = umagic;
	cursor->handle = umagic;

	/* drm_mode_crtc_lut */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_lut, lut);
	lut->crtc_id = umagic;
	lut->gamma_size = 2;
	lut->red = (uintptr_t) u16_array;
	lut->green = (uintptr_t) u16_array;
	lut->blue = (uintptr_t) u16_array;

	/* drm_mode_get_encoder */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_encoder, enc);
	enc->encoder_id = umagic;
	enc->encoder_type = DRM_MODE_ENCODER_NONE;
	enc->crtc_id = umagic;
	enc->possible_crtcs = umagic;
	enc->possible_clones = umagic;

	/* drm_mode_get_connector */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_connector, con);
	struct drm_mode_modeinfo modeinfo[2] = {
		{ umagic, u16_magic, u16_magic, u16_magic,
		  u16_magic, u16_magic, u16_magic, u16_magic,
		  u16_magic, u16_magic, u16_magic, umagic,
		  DRM_MODE_FLAG_PIC_AR_NONE | DRM_MODE_FLAG_PIC_AR_4_3,
		  DRM_MODE_TYPE_BUILTIN | DRM_MODE_TYPE_CLOCK_C,
		  "bogus_modeinfo_name" },
		{ umagic, u16_magic, u16_magic, u16_magic,
		  u16_magic, u16_magic, u16_magic, u16_magic,
		  u16_magic, u16_magic, u16_magic, umagic,
		  DRM_MODE_FLAG_PIC_AR_16_9 | DRM_MODE_FLAG_PIC_AR_64_27,
		  DRM_MODE_TYPE_CRTC_C | DRM_MODE_TYPE_PREFERRED,
		  "bogus_modeinfo_name" },
	};
	char con_str[4096];
	snprintf(con_str, sizeof(con_str),
		 "DRM_IOWR(0xa7, %#lx) /* DRM_IOCTL_MODE_GETCONNECTOR */",
		 (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR));
	con->connector_id = umagic;
	con->encoders_ptr = (uintptr_t) u32_array;
	con->modes_ptr = (uintptr_t) modeinfo;
	con->props_ptr = (uintptr_t) u32_array;
	con->prop_values_ptr = (uintptr_t) u64_array;
	con->count_modes = 2;
	con->count_props = 2;
	con->count_encoders = 2;
	con->encoder_id = umagic;
	con->connector_type = umagic;
	con->connector_type_id = umagic;
	con->connection = umagic;
	con->mm_width = umagic;
	con->mm_height = umagic;
	con->subpixel = umagic;

	/* drm_mode_mode_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_mode_cmd, mode_cmd);
	mode_cmd->connector_id = umagic;
	snprintf(mode_cmd->mode.name, sizeof(bogus_mode_name), "%s", bogus_mode_name);

	/* drm_mode_get_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_property, prop);
	char bogus_property_name[DRM_PROP_NAME_LEN] = "bogus_property_name";
	struct drm_mode_property_enum property_enum_array[2] = {
		{ ulmagic, "bogus_property_enum_name" },
		{ ulmagic, "bogus_property_enum_name" }
	};

	prop->values_ptr = (uintptr_t) u64_array;
	prop->enum_blob_ptr = (uintptr_t) property_enum_array;
	prop->prop_id = umagic;
	prop->flags = DRM_MODE_PROP_PENDING | DRM_MODE_PROP_RANGE;
	snprintf(prop->name, sizeof(bogus_property_name), "%s", bogus_property_name);
	prop->count_values = 2;
	prop->count_enum_blobs = 2;

	/* drm_mode_connector_set_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_connector_set_property, set_prop);
	set_prop->value = ulmagic;
	set_prop->prop_id = umagic;
	set_prop->connector_id = umagic;

	/* drm_mode_get_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_blob, blob);
	blob->blob_id = umagic;
	blob->length = umagic;
	blob->data = ulmagic;

	/* drm_mode_fb_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd, cmd);
	cmd->width = umagic;
	cmd->height = umagic;
	cmd->pitch = umagic;
	cmd->bpp = umagic;
	cmd->depth = umagic;
	cmd->handle = umagic;
	cmd->fb_id = umagic;

	/* drm_mode_crtc_page_flip */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_page_flip, flip);
	flip->crtc_id = umagic;
	flip->fb_id = umagic;
	flip->flags = DRM_MODE_PAGE_FLIP_EVENT;
	flip->user_data = ulmagic;

	/* drm_mode_fb_dirty_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_dirty_cmd, dirty_cmd);
	struct drm_clip_rect clip_rect_array[2] = {
		{ 1, 2, 3, 4 },
		{ 5, 6, 7, 8 }
	};
	dirty_cmd->fb_id = umagic;
	dirty_cmd->flags = DRM_MODE_FB_DIRTY_ANNOTATE_COPY |
			   DRM_MODE_FB_DIRTY_ANNOTATE_FILL;
	dirty_cmd->color = umagic;
	dirty_cmd->num_clips = 2;
	dirty_cmd->clips_ptr = (uintptr_t) clip_rect_array;

	/* struct drm_mode_create_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_dumb, dumb);
	dumb->width = umagic;
	dumb->height = umagic;
	dumb->bpp = umagic;
	dumb->flags = umagic;
	dumb->handle = umagic;
	dumb->pitch = umagic;
	dumb->size = ulmagic;

	/* struct drm_mode_map_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_map_dumb, map_dumb);
	map_dumb->handle = umagic;
	map_dumb->offset = umagic;

	/* struct drm_mode_destroy_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_destroy_dumb, destroy_dumb);
	destroy_dumb->handle = umagic;

	/* drm_mode_get_plane_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_plane_res, plane_res);
	plane_res->plane_id_ptr = (uintptr_t) u32_array;
	plane_res->count_planes = 2;

	/* drm_mode_get_plane */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_plane, get_plane);
	get_plane->plane_id = umagic;
	get_plane->crtc_id = umagic;
	get_plane->fb_id = umagic;
	get_plane->possible_crtcs = umagic;
	get_plane->gamma_size = umagic;
	get_plane->count_format_types = 2;
	get_plane->format_type_ptr = (uintptr_t) u32_array;

	/* drm_mode_set_plane */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_set_plane, set_plane);
	set_plane->plane_id = umagic;
	set_plane->crtc_id = umagic;
	set_plane->fb_id = umagic;
	set_plane->flags = DRM_MODE_PRESENT_TOP_FIELD |
			   DRM_MODE_PRESENT_BOTTOM_FIELD;
	set_plane->crtc_x = umagic;
	set_plane->crtc_y = umagic;
	set_plane->crtc_w = umagic;
	set_plane->crtc_h = umagic;
	set_plane->src_x = umagic;
	set_plane->src_y = umagic;
	set_plane->src_h = umagic;
	set_plane->src_w = umagic;

	/* wrap up DRM_IOCTL_MODE_RMFB */
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, fb_id);
	*fb_id = umagic;

	/* drm_mode_fb_cmd2 */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd2, cmd2);
	char cmd2_str[4096];
	snprintf(cmd2_str, sizeof(cmd2_str),
		 "DRM_IOWR(0xb8, %#lx) /* DRM_IOCTL_MODE_ADDFB2 */",
		 (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_ADDFB2));

	cmd2->width = 1;
	cmd2->height = 1;
	cmd2->pixel_format = 0x1;
	cmd2->flags = DRM_MODE_FB_INTERLACED | DRM_MODE_FB_MODIFIERS;
	for (unsigned int i = 0; i < 4; i++) {
		cmd2->handles[i] = 1;
		cmd2->pitches[i] = 1;
		cmd2->offsets[i] = 1;
# ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
		cmd2->modifier[i] = 1;
# endif
	}
	cmd2->fb_id = 1;

	/* drm_mode_obj_get_properties */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_obj_get_properties, obj_get_prop);
	obj_get_prop->props_ptr = (uintptr_t) u32_array;
	obj_get_prop->prop_values_ptr = (uintptr_t) u64_array;
	obj_get_prop->count_props = 2;
	obj_get_prop->obj_id = umagic;
	obj_get_prop->obj_type = umagic;

	/* drm_mode_obj_set_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_obj_set_property, obj_set_prop);
	obj_set_prop->value = ulmagic;
	obj_set_prop->prop_id = umagic;
	obj_set_prop->obj_id = umagic;
	obj_set_prop->obj_type = umagic;

	/* drm_mode_cursor2 */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor2, cursor2);
	cursor2->flags = DRM_MODE_CURSOR_BO | DRM_MODE_CURSOR_MOVE;
	cursor2->crtc_id = umagic;
	cursor2->x = smagic;
	cursor2->y = smagic;
	cursor2->width = umagic;
	cursor2->height = umagic;
	cursor2->handle = umagic;
	cursor2->hot_x = smagic;
	cursor2->hot_y = smagic;

	/* drm_mode_atomic */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_atomic, atomic);
	atomic->flags = DRM_MODE_ATOMIC_TEST_ONLY | DRM_MODE_ATOMIC_NONBLOCK;
	atomic->count_objs = 2;
	atomic->objs_ptr = (uintptr_t) u32_array;
	atomic->count_props_ptr = (uintptr_t) u32_array;
	atomic->props_ptr = (uintptr_t) u32_array;
	atomic->prop_values_ptr = (uintptr_t) u64_array;
	atomic->reserved = ulmagic;
	atomic->user_data = ulmagic;

	/* drm_mode_create_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_blob, create_blob);
	create_blob->data = ulmagic;
	create_blob->length = umagic;
	create_blob->blob_id = umagic;

	/* drm_mode_destroy_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_destroy_blob, destroy_blob);
	destroy_blob->blob_id = umagic;

# ifdef DRM_IOCTL_SYNCOBJ_CREATE
	/* drm_syncobj_create */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_create, sync_create);
	sync_create->handle = umagic;
	sync_create->flags = DRM_SYNCOBJ_CREATE_SIGNALED;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
	/* drm_syncobj_destroy */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_destroy, sync_destroy);
	sync_destroy->handle = umagic;
	sync_destroy->pad = umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
	/* drm_syncobj_handle */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_handle, sync_handle);
	sync_handle->handle = umagic;
	sync_handle->flags = DRM_SYNCOBJ_FD_TO_HANDLE_FLAGS_IMPORT_SYNC_FILE |
			     DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE;
	sync_handle->fd = 123;
	sync_handle->pad= umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_WAIT
	/* drm_syncobj_wait */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_wait, sync_wait);
	sync_wait->handles = (uintptr_t) u32_array;
	sync_wait->timeout_nsec = slmagic;
	sync_wait->count_handles = 2;
	sync_wait->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL |
			   DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;
	sync_wait->first_signaled = umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_RESET
	/* drm_syncobj_array */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_array, sync_array);
	sync_array->handles = (uintptr_t) u32_array;
	sync_array->count_handles = 2;
	sync_array->pad = umagic;
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
	/* drm_mode_create_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_lease, create_lease);
	create_lease->object_ids = (uintptr_t) u32_array;
	create_lease->object_count = 2;
	create_lease->flags = O_CLOEXEC | O_NONBLOCK;
	create_lease->lessee_id = umagic;
	create_lease->fd = smagic;
# endif

# ifdef DRM_IOCTL_MODE_LIST_LESSEES
	/* drm_mode_list_lessees */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_list_lessees, list_lessees);
	list_lessees->count_lessees = 2;
	list_lessees->pad = umagic;
	list_lessees->lessees_ptr = (uintptr_t) u32_array;
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
	/* drm_mode_get_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_lease, get_lease);
	get_lease->count_objects = 2;
	get_lease->pad = umagic;
	get_lease->objects_ptr = (uintptr_t) u32_array;
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
	/* drm_mode_revoke_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_revoke_lease, revoke_lease);
	revoke_lease->lessee_id = umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
	/* drm_syncobj_timeline_wait */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_timeline_wait, timeline_wait);
	timeline_wait->handles = (uintptr_t) u32_array;
	timeline_wait->points = ulmagic;
	timeline_wait->timeout_nsec = slmagic;
	timeline_wait->count_handles = 2;
	timeline_wait->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
	timeline_wait->first_signaled = umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_QUERY
	/* drm_syncobj_timeline_array */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_timeline_array, timeline_array);
	timeline_array->handles = (uintptr_t) u32_array;
	timeline_array->points = (uintptr_t) u64_array;
	timeline_array->count_handles = 2;
	timeline_array->pad = umagic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
	/* drm_syncobj_transfer */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_transfer, sync_transfer);
	sync_transfer->src_handle = umagic;
	sync_transfer->dst_handle = umagic;
	sync_transfer->src_point = ulmagic;
	sync_transfer->dst_point = ulmagic;
	sync_transfer->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
	sync_transfer->pad = umagic;
# endif

	struct {
		struct drm_check check;
		void *ptr;
	} a[] = {
		{ { ARG_STR(DRM_IOCTL_VERSION), ver, print_drm_version }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_UNIQUE), unique, print_drm_get_unique }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_MAGIC), auth, print_drm_get_magic }, NULL },
		{ { ARG_STR(DRM_IOCTL_IRQ_BUSID), busid, print_drm_irq_busid }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_MAP), map, print_drm_get_map }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_CLIENT), client, print_drm_get_client }, NULL },
		{ { ARG_STR(DRM_IOCTL_SET_VERSION), set_ver, print_drm_set_version }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODESET_CTL), ctl, print_drm_modeset_ctl }, NULL },
		{ { ARG_STR(DRM_IOCTL_GEM_CLOSE), close, print_drm_gem_close }, NULL },
		{ { ARG_STR(DRM_IOCTL_GEM_FLINK), flink, print_drm_gem_flink }, NULL },
		{ { ARG_STR(DRM_IOCTL_GEM_OPEN), open, print_drm_gem_open }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_CAP), cap, print_drm_get_cap }, NULL },
		{ { ARG_STR(DRM_IOCTL_SET_CLIENT_CAP), cli_cap, print_drm_set_client_cap }, NULL },
		{ { ARG_STR(DRM_IOCTL_AUTH_MAGIC), auth, print_drm_auth_magic }, NULL },
		{ { ARG_STR(DRM_IOCTL_CONTROL), control, print_drm_control }, NULL },
		{ { ARG_STR(DRM_IOCTL_ADD_MAP), map, print_drm_add_map }, NULL },
		{ { ARG_STR(DRM_IOCTL_ADD_BUFS), desc, print_drm_add_bufs }, NULL },
		{ { ARG_STR(DRM_IOCTL_MARK_BUFS), desc, print_drm_mark_bufs }, NULL },
		{ { ARG_STR(DRM_IOCTL_INFO_BUFS), info, print_drm_info_bufs }, buf_desc },
		{ { ARG_STR(DRM_IOCTL_MAP_BUFS), buf_map, print_drm_map_bufs }, buf_pub },
		{ { ARG_STR(DRM_IOCTL_FREE_BUFS), buf_free, print_drm_free_bufs }, NULL },
		{ { ARG_STR(DRM_IOCTL_RM_MAP), map, print_drm_rm_map }, NULL },
		{ { ARG_STR(DRM_IOCTL_SET_SAREA_CTX), priv_map, print_drm_sarea_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_SAREA_CTX), priv_map, print_drm_sarea_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_ADD_CTX), ctx, print_drm_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_RM_CTX), ctx, print_drm_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_GET_CTX), ctx, print_drm_get_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_SWITCH_CTX), ctx, print_drm_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_NEW_CTX), ctx, print_drm_ctx }, NULL },
		{ { ARG_STR(DRM_IOCTL_RES_CTX), ctx_res, print_drm_res_ctx }, ctx_var },
		{ { ARG_STR(DRM_IOCTL_LOCK), lock, print_drm_lock }, NULL },
		{ { ARG_STR(DRM_IOCTL_UNLOCK), lock, print_drm_lock }, NULL },
		{ { ARG_STR(DRM_IOCTL_PRIME_HANDLE_TO_FD), prime_handle, print_drm_prime_handle_to_fd }, NULL },
		{ { ARG_STR(DRM_IOCTL_PRIME_FD_TO_HANDLE), prime_handle, print_drm_prime_fd_to_handle }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_ENABLE), agp_mode, print_drm_agp_enable }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_INFO), agp_info, print_drm_agp_info }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_ALLOC), agp_buffer, print_drm_agp_alloc }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_FREE), agp_buffer, print_drm_agp_free }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_BIND), agp_binding, print_drm_agp_bind }, NULL },
		{ { ARG_STR(DRM_IOCTL_AGP_UNBIND), agp_binding, print_drm_agp_unbind }, NULL },
		{ { ARG_STR(DRM_IOCTL_SG_ALLOC), sg, print_drm_sg_alloc }, NULL },
		{ { ARG_STR(DRM_IOCTL_SG_FREE), sg, print_drm_sg_free }, NULL },
		{ { ARG_STR(DRM_IOCTL_WAIT_VBLANK), vblank, print_drm_wait_vblank }, NULL },
# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
		{ { ARG_STR(DRM_IOCTL_CRTC_GET_SEQUENCE), get_seq, print_drm_crtc_get_sequence }, NULL },
# endif
# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
		{ { ARG_STR(DRM_IOCTL_CRTC_QUEUE_SEQUENCE), queue_seq, print_drm_crtc_queue_sequence }, NULL },
# endif
		{ { ARG_STR(DRM_IOCTL_MODE_GETRESOURCES), card_res, print_drm_mode_get_resources }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_CURSOR), cursor, print_drm_mode_cursor }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETGAMMA), lut, print_drm_mode_gamma }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_SETGAMMA), lut, print_drm_mode_gamma }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETENCODER), enc, print_drm_mode_get_encoder }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETPROPERTY), prop, print_drm_mode_get_property }, property_enum_array },
		{ { ARG_STR(DRM_IOCTL_MODE_SETPROPERTY), set_prop, print_drm_mode_set_property }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETPROPBLOB), blob, print_drm_mode_get_prop_blob }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETFB), cmd, print_drm_mode_get_fb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_ADDFB), cmd, print_drm_mode_add_fb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_RMFB), fb_id, print_drm_mode_rm_fb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_PAGE_FLIP), flip, print_drm_mode_page_flip }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_DIRTYFB), dirty_cmd, print_drm_mode_dirty_fb }, clip_rect_array },
		{ { ARG_STR(DRM_IOCTL_MODE_CREATE_DUMB), dumb, print_drm_mode_create_dumb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_MAP_DUMB), map_dumb, print_drm_mode_map_dumb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_DESTROY_DUMB), destroy_dumb, print_drm_mode_destroy_dumb }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETPLANERESOURCES), plane_res, print_drm_mode_getplaneresources }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_GETPLANE), get_plane, print_drm_mode_getplane }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_SETPLANE), set_plane, print_drm_mode_setplane }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_OBJ_GETPROPERTIES), obj_get_prop, print_drm_mode_obj_getproperties}, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_OBJ_SETPROPERTY), obj_set_prop, print_drm_mode_obj_setproperty }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_CURSOR2), cursor2, print_drm_mode_cursor2 }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_ATOMIC), atomic, print_drm_mode_atomic }, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_CREATEPROPBLOB), create_blob, print_drm_mode_createpropblob}, NULL },
		{ { ARG_STR(DRM_IOCTL_MODE_DESTROYPROPBLOB), destroy_blob, print_drm_destroypropblob }, NULL },
# ifdef DRM_IOCTL_SYNCOBJ_CREATE
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_CREATE), sync_create, print_drm_syncobj_create }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_DESTROY), sync_destroy, print_drm_syncobj_destroy }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD), sync_handle, print_drm_syncobj_handle_to_fd }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE), sync_handle, print_drm_syncobj_fd_to_handle }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_WAIT
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_WAIT), sync_wait, print_drm_syncobj_wait }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_RESET
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_RESET), sync_array, print_drm_syncobj_reset_or_signal }, NULL },
# endif
# ifdef DRM_IOCTL_MODE_CREATE_LEASE
		{ { ARG_STR(DRM_IOCTL_MODE_CREATE_LEASE), create_lease, print_drm_mode_create_lease }, NULL },
# endif
# ifdef DRM_IOCTL_MODE_LIST_LESSEES
		{ { ARG_STR(DRM_IOCTL_MODE_LIST_LESSEES), list_lessees, print_drm_mode_list_lessees }, NULL },
# endif
# ifdef DRM_IOCTL_MODE_GET_LEASE
		{ { ARG_STR(DRM_IOCTL_MODE_GET_LEASE), get_lease, print_drm_mode_get_lease }, NULL },
# endif
# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
		{ { ARG_STR(DRM_IOCTL_MODE_REVOKE_LEASE), revoke_lease, print_drm_mode_revoke_lease }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT), timeline_wait, print_drm_syncobj_timeline_wait }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_QUERY), timeline_array, print_drm_syncobj_query_or_timeline_signal }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_TRANSFER), sync_transfer, print_drm_syncobj_transfer }, NULL },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
		{ { ARG_STR(DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL), timeline_array, print_drm_syncobj_query_or_timeline_signal }, NULL },
# endif
		{ { DRM_IOCTL_MODE_GETCONNECTOR, con_str, con, print_drm_mode_get_connector }, modeinfo },
		{ { DRM_IOCTL_MODE_ADDFB2, cmd2_str, cmd2, print_drm_mode_add_fb2 }, NULL }
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		test_drm(&a[i].check, a[i].ptr);
	}

	struct drm_check check_crtc[] = {
		{ ARG_STR(DRM_IOCTL_MODE_GETCRTC), crtc, print_drm_mode_crtc },
		{ ARG_STR(DRM_IOCTL_MODE_SETCRTC), crtc, print_drm_mode_crtc }
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(check_crtc); i++) {
		int is_get = check_crtc[i].cmd == DRM_IOCTL_MODE_GETCRTC;
		test_drm(&check_crtc[i], (void *)&is_get);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_DRM_H && HAVE_DRM_DRM_H");

#endif
