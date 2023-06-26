/*
 * Copyright (c) 2014 Philippe De Muyter <phdm@macqel.be>
 * Copyright (c) 2014 William Manley <will@williammanley.net>
 * Copyright (c) 2011 Peter Zotov <whitequark@whitequark.org>
 * Copyright (c) 2014-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(kernel_v4l2_buffer_t)
#include DEF_MPERS_TYPE(kernel_v4l2_event_t)
#include DEF_MPERS_TYPE(kernel_v4l2_timeval_t)
#include DEF_MPERS_TYPE(struct_v4l2_clip)
#include DEF_MPERS_TYPE(struct_v4l2_create_buffers)
#include DEF_MPERS_TYPE(struct_v4l2_ext_control)
#include DEF_MPERS_TYPE(struct_v4l2_ext_controls)
#include DEF_MPERS_TYPE(struct_v4l2_format)
#include DEF_MPERS_TYPE(struct_v4l2_framebuffer)
#include DEF_MPERS_TYPE(struct_v4l2_input)
#include DEF_MPERS_TYPE(struct_v4l2_standard)
#include DEF_MPERS_TYPE(struct_v4l2_window)

#include "kernel_v4l2_types.h"
typedef struct v4l2_clip struct_v4l2_clip;
typedef struct v4l2_create_buffers struct_v4l2_create_buffers;
typedef struct v4l2_ext_control struct_v4l2_ext_control;
typedef struct v4l2_ext_controls struct_v4l2_ext_controls;
typedef struct v4l2_format struct_v4l2_format;
typedef struct v4l2_framebuffer struct_v4l2_framebuffer;
typedef struct v4l2_input struct_v4l2_input;
typedef struct v4l2_standard struct_v4l2_standard;
typedef struct v4l2_window struct_v4l2_window;

#include MPERS_DEFS

#include "print_utils.h"
#include "xstring.h"

#include "xlat/v4l2_meta_fmts.h"
#include "xlat/v4l2_pix_fmts.h"
#include "xlat/v4l2_sdr_fmts.h"

static void
print_v4l2_rect(const MPERS_PTR_ARG(struct v4l2_rect *) const arg)
{
	const struct v4l2_rect *const p = arg;
	tprint_struct_begin();
	PRINT_FIELD_D(*p, left);
	tprint_struct_next();
	PRINT_FIELD_D(*p, top);
	tprint_struct_next();
	PRINT_FIELD_U(*p, width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, height);
	tprint_struct_end();
}

#define PRINT_FIELD_FRACT(where_, field_)			\
	do {							\
		tprints_field_name(#field_);			\
		tprintf_string("%u/%u",				\
			      (where_).field_.numerator,	\
			      (where_).field_.denominator);	\
	} while (0)

static void
print_pixelformat(uint32_t fourcc, const struct xlat *xlat)
{
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		PRINT_VAL_X(fourcc);
		return;
	}

	unsigned char a[] = {
		(unsigned char) fourcc,
		(unsigned char) (fourcc >> 8),
		(unsigned char) (fourcc >> 16),
		(unsigned char) (fourcc >> 24),
	};

	tprints_arg_begin("v4l2_fourcc");
	/* Generic char array printing routine.  */
	for (unsigned int i = 0; i < ARRAY_SIZE(a); ++i) {
		unsigned char c = a[i];

		if (i)
			tprint_arg_next();

		print_char(c, SCF_QUOTES);
	}
	tprint_arg_end();

	if (xlat) {
		const char *pixfmt_name = xlookup(xlat, fourcc);

		if (pixfmt_name)
			tprints_comment(pixfmt_name);
	}
}

#define PRINT_FIELD_PIXFMT(where_, field_, xlat_)	\
	do {							\
		tprints_field_name(#field_);			\
		print_pixelformat((where_).field_, (xlat_));	\
	} while (0)

#include "xlat/v4l2_device_capabilities_flags.h"

static int
print_v4l2_capability(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_capability caps;

	if (entering(tcp))
		return 0;

	tprint_arg_next();
	if (umove_or_printaddr(tcp, arg, &caps))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_CSTRING(caps, driver);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(caps, card);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(caps, bus_info);
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(caps, version, print_kernel_version);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(caps, capabilities,
			  v4l2_device_capabilities_flags, "V4L2_CAP_???");
	if (caps.device_caps) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(caps, device_caps,
				  v4l2_device_capabilities_flags,
				  "V4L2_CAP_???");
	}
	tprint_struct_end();
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_buf_types.h"
#include "xlat/v4l2_format_description_flags.h"

static int
print_v4l2_fmtdesc(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_fmtdesc f;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(f, index);
		tprint_struct_next();
		PRINT_FIELD_XVAL(f, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(f, flags,
				  v4l2_format_description_flags,
				  "V4L2_FMT_FLAG_???");
		tprint_struct_next();
		PRINT_FIELD_CSTRING(f, description);
		tprint_struct_next();
		PRINT_FIELD_PIXFMT(f, pixelformat, v4l2_pix_fmts);
	}
	tprint_struct_end();
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_fields.h"
#include "xlat/v4l2_colorspaces.h"
#include "xlat/v4l2_vbi_flags.h"
#include "xlat/v4l2_sliced_flags.h"

static bool
print_v4l2_clip(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct_v4l2_clip *p = elem_buf;
	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(*p, c, print_v4l2_rect);
	tprint_struct_end();
	return true;
}

#define DECL_print_v4l2_format_fmt(name_)				\
	print_v4l2_format_fmt_ ## name_(struct tcb *const tcp,		\
		const typeof_field(struct_v4l2_format, fmt.name_) *const p)

static bool
DECL_print_v4l2_format_fmt(pix)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, height);
	tprint_struct_next();
	PRINT_FIELD_PIXFMT(*p, pixelformat, v4l2_pix_fmts);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, field, v4l2_fields, "V4L2_FIELD_???");
	tprint_struct_next();
	PRINT_FIELD_U(*p, bytesperline);
	tprint_struct_next();
	PRINT_FIELD_U(*p, sizeimage);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, colorspace, v4l2_colorspaces,
			 "V4L2_COLORSPACE_???");
	tprint_struct_end();
	return true;
}

static bool
print_v4l2_plane_pix_format_array_member(struct tcb *tcp, void *elem_buf,
					 size_t elem_size, void *data)
{
	struct v4l2_plane_pix_format *p = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*p, sizeimage);
	tprint_struct_next();
	PRINT_FIELD_U(*p, bytesperline);
	tprint_struct_end();

	return true;
}

static bool
DECL_print_v4l2_format_fmt(pix_mp)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, height);
	tprint_struct_next();
	PRINT_FIELD_PIXFMT(*p, pixelformat, v4l2_pix_fmts);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, field, v4l2_fields, "V4L2_FIELD_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, colorspace, v4l2_colorspaces,
			 "V4L2_COLORSPACE_???");
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(*p, plane_fmt, p->num_planes, tcp,
			       print_v4l2_plane_pix_format_array_member);
	tprint_struct_next();
	PRINT_FIELD_U(*p, num_planes);
	tprint_struct_end();
	return true;
}

static bool
DECL_print_v4l2_format_fmt(win)
{
	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(*p, w, print_v4l2_rect);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, field, v4l2_fields, "V4L2_FIELD_???");
	tprint_struct_next();
	PRINT_FIELD_X(*p, chromakey);

	tprint_struct_next();
	tprints_field_name("clips");
	struct_v4l2_clip clip;
	bool rc = print_array(tcp, ptr_to_kulong(p->clips),
			   p->clipcount, &clip, sizeof(clip),
			   tfetch_mem, print_v4l2_clip, 0);

	tprint_struct_next();
	PRINT_FIELD_U(*p, clipcount);
	tprint_struct_next();
	PRINT_FIELD_PTR(*p, bitmap);
	if (p->global_alpha) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, global_alpha);
	}
	tprint_struct_end();
	return rc;
}

static bool
DECL_print_v4l2_format_fmt(vbi)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, sampling_rate);
	tprint_struct_next();
	PRINT_FIELD_U(*p, offset);
	tprint_struct_next();
	PRINT_FIELD_U(*p, samples_per_line);
	tprint_struct_next();
	PRINT_FIELD_PIXFMT(*p, sample_format, v4l2_pix_fmts);
	tprint_struct_next();
	PRINT_FIELD_D_ARRAY(*p, start);
	tprint_struct_next();
	PRINT_FIELD_U_ARRAY(*p, count);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*p, flags, v4l2_vbi_flags, "V4L2_VBI_???");
	tprint_struct_end();
	return true;
}

static bool
DECL_print_v4l2_format_fmt(sliced)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*p, service_set, v4l2_sliced_flags,
			  "V4L2_SLICED_???");
	tprint_struct_next();
	PRINT_FIELD_X_ARRAY2D(*p, service_lines);
	tprint_struct_next();
	PRINT_FIELD_U(*p, io_size);
	tprint_struct_end();
	return true;
}

static bool
DECL_print_v4l2_format_fmt(sdr)
{
	tprint_struct_begin();
	PRINT_FIELD_PIXFMT(*p, pixelformat, v4l2_sdr_fmts);
	if (p->buffersize) {
		tprint_struct_next();
		PRINT_FIELD_U(*p, buffersize);
	}
	tprint_struct_end();
	return true;
}

static bool
DECL_print_v4l2_format_fmt(meta)
{
	tprint_struct_begin();
	PRINT_FIELD_PIXFMT(*p, dataformat, v4l2_meta_fmts);
	tprint_struct_next();
	PRINT_FIELD_U(*p, buffersize);
	tprint_struct_end();
	return true;
}

#define PRINT_FIELD_V4L2_FORMAT_FMT(where_, fmt_, field_, tcp_, ret_)	\
	do {								\
		tprints_field_name(#fmt_ "." #field_);			\
		(ret_) = (print_v4l2_format_fmt_ ## field_)		\
				((tcp_), &((where_).fmt_.field_));	\
	} while (0)

static bool
print_v4l2_format_fmt(struct tcb *const tcp, void (*const prefix_fun)(void),
		      const struct_v4l2_format *const f)
{
	bool ret = true;
	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, pix, tcp, ret);
		break;

	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, pix_mp, tcp, ret);
		break;

	/* OUTPUT_OVERLAY since Linux v2.6.22-rc1~1118^2~179 */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, win, tcp, ret);
		break;

	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, vbi, tcp, ret);
		break;

	/* both since Linux v2.6.14-rc2~64 */
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, sliced, tcp, ret);
		break;

	/* since Linux v4.4-rc1~118^2~14 */
	case V4L2_BUF_TYPE_SDR_OUTPUT:
	/* since Linux v3.15-rc1~85^2~213 */
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, sdr, tcp, ret);
		break;
	/* since Linux v5.0-rc1~181^2~21 */
	case V4L2_BUF_TYPE_META_OUTPUT:
	/* since Linux v4.12-rc1~85^2~71 */
	case V4L2_BUF_TYPE_META_CAPTURE:
		prefix_fun();
		PRINT_FIELD_V4L2_FORMAT_FMT(*f, fmt, meta, tcp, ret);
		break;
	default:
		return false;
	}
	return ret;
}

static void
tprint_struct_end_value_changed_struct_begin(void)
{
	tprint_struct_end();
	tprint_value_changed();
	tprint_struct_begin();
}

static int
print_v4l2_format(struct tcb *const tcp, const kernel_ulong_t arg,
		  const bool is_get)
{
	struct_v4l2_format f;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(f, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		if (!print_v4l2_format_fmt(tcp, tprint_struct_next, &f)) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		void (*const prefix_fun)(void) =
			is_get ? tprint_struct_next :
			tprint_struct_end_value_changed_struct_begin;
		print_v4l2_format_fmt(tcp, prefix_fun, &f);
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_memories.h"

static int
print_v4l2_requestbuffers(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_requestbuffers reqbufs;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &reqbufs))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(reqbufs, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		tprint_struct_next();
		PRINT_FIELD_XVAL(reqbufs, memory, v4l2_memories,
				 "V4L2_MEMORY_???");
		tprint_struct_next();
		PRINT_FIELD_U(reqbufs, count);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &reqbufs)) {
		tprint_value_changed();
		PRINT_VAL_U(reqbufs.count);
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_buf_flags.h"
#include "xlat/v4l2_buf_flags_ts_type.h"
#include "xlat/v4l2_buf_flags_ts_src.h"

static void
print_v4l2_buffer_flags(uint32_t val)
{
	const uint32_t ts_type = val & V4L2_BUF_FLAG_TIMESTAMP_MASK;
	const uint32_t ts_src  = val & V4L2_BUF_FLAG_TSTAMP_SRC_MASK;
	const uint32_t flags   = val & ~ts_type & ~ts_src;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		PRINT_VAL_X(val);
		return;
	}

	tprint_flags_begin();
	if (flags) {
		printflags_in(v4l2_buf_flags, flags, "V4L2_BUF_FLAG_???");
		tprint_flags_or();
	}
	printxval(v4l2_buf_flags_ts_type, ts_type,
		  "V4L2_BUF_FLAG_TIMESTAMP_???");
	tprint_flags_or();
	printxval(v4l2_buf_flags_ts_src, ts_src,
		  "V4L2_BUF_FLAG_TSTAMP_SRC_???");
	tprint_flags_end();
}

#define PRINT_FIELD_V4L2_BUFFER_FLAGS(where_, field_)		\
	do {							\
		tprints_field_name(#field_);			\
		print_v4l2_buffer_flags((where_).field_);	\
	} while (0)

static void
print_v4l2_timeval(const MPERS_PTR_ARG(kernel_v4l2_timeval_t *) const arg)
{
	const kernel_v4l2_timeval_t *const t = arg;
	kernel_timeval64_t tv;

	if (sizeof(tv.tv_sec) == sizeof(t->tv_sec) &&
	    sizeof(tv.tv_usec) == sizeof(t->tv_usec)) {
		print_timeval64_data_size(t, sizeof(*t));
	} else {
		tv.tv_sec = sign_extend_unsigned_to_ll(t->tv_sec);
		tv.tv_usec = zero_extend_signed_to_ull(t->tv_usec);
		print_timeval64_data_size(&tv, sizeof(tv));
	}
}

#define PRINT_FIELD_V4L2_TIMEVAL(where_, field_)	\
	do {						\
		tprints_field_name(#field_);		\
		print_v4l2_timeval(&((where_).field_));	\
	} while (0)

static int
print_v4l2_buffer(struct tcb *const tcp, const unsigned int code,
		  const kernel_ulong_t arg)
{
	kernel_v4l2_buffer_t b;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(b, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		if (code != VIDIOC_DQBUF) {
			tprint_struct_next();
			PRINT_FIELD_U(b, index);
		}

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &b)) {
		if (code == VIDIOC_DQBUF) {
			tprint_struct_next();
			PRINT_FIELD_U(b, index);
		}
		tprint_struct_next();
		PRINT_FIELD_XVAL(b, memory, v4l2_memories,
				 "V4L2_MEMORY_???");

		if (b.memory == V4L2_MEMORY_MMAP) {
			tprint_struct_next();
			PRINT_FIELD_X(b, m.offset);
		} else if (b.memory == V4L2_MEMORY_USERPTR) {
			tprint_struct_next();
			PRINT_FIELD_PTR(b, m.userptr);
		}

		tprint_struct_next();
		PRINT_FIELD_U(b, length);
		tprint_struct_next();
		PRINT_FIELD_U(b, bytesused);
		tprint_struct_next();
		PRINT_FIELD_V4L2_BUFFER_FLAGS(b, flags);
		if (code == VIDIOC_DQBUF) {
			tprint_struct_next();
			PRINT_FIELD_V4L2_TIMEVAL(b, timestamp);
		}
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_framebuffer(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_framebuffer b;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &b)) {
		tprint_struct_begin();
		PRINT_FIELD_X(b, capability);
		tprint_struct_next();
		PRINT_FIELD_X(b, flags);
		tprint_struct_next();
		PRINT_FIELD_PTR(b, base);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_buf_type(struct tcb *const tcp, const kernel_ulong_t arg)
{
	int type;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &type)) {
		tprint_indirect_begin();
		printxval(v4l2_buf_types, type, "V4L2_BUF_TYPE_???");
		tprint_indirect_end();
	}
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_streaming_capabilities.h"
#include "xlat/v4l2_capture_modes.h"

static void
print_v4l2_streamparm_capture(const struct v4l2_captureparm *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*p, capability, v4l2_streaming_capabilities,
			  "V4L2_CAP_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*p, capturemode, v4l2_capture_modes,
			  "V4L2_MODE_???");
	tprint_struct_next();
	PRINT_FIELD_FRACT(*p, timeperframe);
	tprint_struct_next();
	PRINT_FIELD_X(*p, extendedmode);
	tprint_struct_next();
	PRINT_FIELD_U(*p, readbuffers);
	tprint_struct_end();
}

static void
print_v4l2_streamparm_output(const struct v4l2_outputparm *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*p, capability, v4l2_streaming_capabilities,
			  "V4L2_CAP_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*p, outputmode, v4l2_capture_modes,
			  "V4L2_MODE_???");
	tprint_struct_next();
	PRINT_FIELD_FRACT(*p, timeperframe);
	tprint_struct_next();
	PRINT_FIELD_X(*p, extendedmode);
	tprint_struct_next();
	PRINT_FIELD_U(*p, writebuffers);
	tprint_struct_end();
}

#define PRINT_FIELD_V4L2_STREAMPARM_PARM(where_, parm_, field_)			\
	do {									\
		tprints_field_name(#parm_ "." #field_);				\
		print_v4l2_streamparm_ ## field_(&((where_).parm_.field_));	\
	} while (0)

static int
print_v4l2_streamparm(struct tcb *const tcp, const kernel_ulong_t arg,
		      const bool is_get)
{
	struct v4l2_streamparm s;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(s, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		switch (s.type) {
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				if (is_get)
					return 0;
				tprint_struct_next();
				break;
			default:
				tprint_struct_end();
				return RVAL_IOCTL_DECODED;
		}
	} else {
		if (syserror(tcp) || umove(tcp, arg, &s) < 0) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
		if (is_get) {
			tprint_struct_next();
		} else {
			tprint_struct_end();
			tprint_value_changed();
			tprint_struct_begin();
		}
	}

	if (s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		PRINT_FIELD_V4L2_STREAMPARM_PARM(s, parm, capture);
	} else {
		PRINT_FIELD_V4L2_STREAMPARM_PARM(s, parm, output);
	}

	if (entering(tcp)) {
		return 0;
	} else {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}
}

static int
print_v4l2_standard(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_standard s;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(s, index);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &s)) {
		tprint_struct_next();
		PRINT_FIELD_CSTRING(s, name);
		tprint_struct_next();
		PRINT_FIELD_FRACT(s, frameperiod);
		tprint_struct_next();
		PRINT_FIELD_U(s, framelines);
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_input_capabilities_flags.h"
#include "xlat/v4l2_input_status_flags.h"
#include "xlat/v4l2_input_types.h"
#include "xlat/v4l2_std_ids.h"

static int
print_v4l2_input(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_input i;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &i))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(i, index);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &i)) {
		tprint_struct_next();
		PRINT_FIELD_CSTRING(i, name);
		tprint_struct_next();
		PRINT_FIELD_XVAL(i, type, v4l2_input_types,
				 "V4L2_INPUT_TYPE_???");
		tprint_struct_next();
		PRINT_FIELD_X(i, audioset);
		tprint_struct_next();
		PRINT_FIELD_U(i, tuner);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(i, std, v4l2_std_ids, "V4L2_STD_???");
		tprint_struct_next();
		PRINT_FIELD_FLAGS(i, status, v4l2_input_status_flags,
				  "V4L2_IN_ST_???");
		tprint_struct_next();
		PRINT_FIELD_FLAGS(i, capabilities,
				  v4l2_input_capabilities_flags,
				  "V4L2_IN_CAP_???");
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

/*
 * We include it here and not before print_v4l2_ext_controls as we need
 * V4L2_CTRL_CLASS_* definitions for V4L2_CID_*_BASE ones.
 */
#include "xlat/v4l2_control_classes.h"
#include "xlat/v4l2_control_id_bases.h"
#include "xlat/v4l2_control_ids.h"
#include "xlat/v4l2_control_query_flags.h"

static void
print_v4l2_cid(uint32_t cid, bool next_flags)
{
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		PRINT_VAL_X(cid);
		return;
	}

	tprint_flags_begin();
	if (next_flags) {
		uint32_t flags = cid & v4l2_control_query_flags->flags_mask;

		if (flags) {
			printflags_in(v4l2_control_query_flags, flags,
				      "V4L2_CTRL_FLAG_NEXT_???");
			tprint_flags_or();
			cid &= ~flags;
		}
	}

	const char *id_name = xlookup(v4l2_control_ids, cid);

	if (id_name) {
		print_xlat_ex(cid, id_name, XLAT_STYLE_DEFAULT);
		tprint_flags_end();
		return;
	}

	uint64_t class_id = cid;
	const char *class_str = xlookup_le(v4l2_control_classes, &class_id);

	if (!class_str || (cid - class_id) >= 0x10000) {
		print_xlat_ex(cid, "V4L2_CID_???", PXF_DEFAULT_STR);
		tprint_flags_end();
		return;
	}

	/*
	 * As of now, the longest control class name is V4L2_CTRL_CLASS_IMAGE_SOURCE,
	 * of 28 characters long.
	 */
	char tmp_str[64 + sizeof("+%#") + sizeof(class_id) * 2];

	xsprintf(tmp_str, "%s+%#" PRIx64, class_str, cid - class_id);
	print_xlat_ex(cid, tmp_str, XLAT_STYLE_DEFAULT);
	tprint_flags_end();
}

#define PRINT_FIELD_V4L2_CID(where_, field_, next_)		\
	do {							\
		tprints_field_name(#field_);			\
		print_v4l2_cid((where_).field_, (next_));	\
	} while (0)

static int
print_v4l2_control(struct tcb *const tcp, const kernel_ulong_t arg,
		   const bool is_get)
{
	struct v4l2_control c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_V4L2_CID(c, id, false);
		if (!is_get) {
			tprint_struct_next();
			PRINT_FIELD_D(c, value);
		}
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		if (is_get) {
			tprint_struct_next();
			PRINT_FIELD_D(c, value);
		} else {
			tprint_value_changed();
			PRINT_VAL_D(c.value);
		}
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_tuner_types.h"
#include "xlat/v4l2_tuner_capabilities.h"
#include "xlat/v4l2_tuner_rxsubchanses.h"
#include "xlat/v4l2_tuner_audmodes.h"

static int
print_v4l2_tuner(struct tcb *const tcp, const kernel_ulong_t arg,
		 const bool is_get)
{
	struct v4l2_tuner c;
	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(c, index);
		if (is_get)
			return 0;
		tprint_struct_next();
	} else {
		if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
		if (is_get) {
			tprint_struct_next();
		} else {
			tprint_struct_end();
			tprint_value_changed();
			tprint_struct_begin();
		}
	}

	PRINT_FIELD_CSTRING(c, name);
	tprint_struct_next();
	PRINT_FIELD_XVAL(c, type, v4l2_tuner_types, "V4L2_TUNER_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(c, capability, v4l2_tuner_capabilities,
			  "V4L2_TUNER_CAP_???");
	tprint_struct_next();
	PRINT_FIELD_U(c, rangelow);
	tprint_struct_next();
	PRINT_FIELD_U(c, rangehigh);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(c, rxsubchans, v4l2_tuner_rxsubchanses,
			  "V4L2_TUNER_SUB_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(c, audmode, v4l2_tuner_audmodes,
			 "V4L2_TUNER_MODE_???");
	tprint_struct_next();
	PRINT_FIELD_D(c, signal);
	tprint_struct_next();
	PRINT_FIELD_D(c, afc);

	if (entering(tcp)) {
		return 0;
	} else {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}
}

#include "xlat/v4l2_control_types.h"
#include "xlat/v4l2_control_flags.h"

static int
print_v4l2_queryctrl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_queryctrl c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		set_tcb_priv_ulong(tcp, c.id);
		tprint_struct_begin();
		PRINT_FIELD_V4L2_CID(c, id, true);

		return 0;
	}

	/* exiting */
	if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	unsigned long entry_id = get_tcb_priv_ulong(tcp);

	if (c.id != entry_id) {
		tprint_value_changed();
		print_v4l2_cid(c.id, false);
	}

	tprint_struct_next();
	PRINT_FIELD_XVAL(c, type, v4l2_control_types,
			 "V4L2_CTRL_TYPE_???");
	tprint_struct_next();
	PRINT_FIELD_CSTRING(c, name);
	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_D(c, minimum);
		tprint_struct_next();
		PRINT_FIELD_D(c, maximum);
		tprint_struct_next();
		PRINT_FIELD_D(c, step);
		tprint_struct_next();
		PRINT_FIELD_D(c, default_value);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(c, flags, v4l2_control_flags,
				  "V4L2_CTRL_FLAG_???");
		if (!IS_ARRAY_ZERO(c.reserved)) {
			tprint_struct_next();
			PRINT_FIELD_ARRAY(c, reserved, tcp,
					  print_xint_array_member);
		}
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_query_ext_ctrl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_query_ext_ctrl c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		set_tcb_priv_ulong(tcp, c.id);
		tprint_struct_begin();
		PRINT_FIELD_V4L2_CID(c, id, true);

		return 0;
	}

	/* exiting */
	if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	unsigned long entry_id = get_tcb_priv_ulong(tcp);

	if (c.id != entry_id) {
		tprint_value_changed();
		print_v4l2_cid(c.id, false);
	}

	tprint_struct_next();
	PRINT_FIELD_XVAL(c, type, v4l2_control_types,
			 "V4L2_CTRL_TYPE_???");
	tprint_struct_next();
	PRINT_FIELD_CSTRING(c, name);
	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_D(c, minimum);
		tprint_struct_next();
		PRINT_FIELD_D(c, maximum);
		tprint_struct_next();
		PRINT_FIELD_U(c, step);
		tprint_struct_next();
		PRINT_FIELD_D(c, default_value);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(c, flags, v4l2_control_flags,
				  "V4L2_CTRL_FLAG_???");
		tprint_struct_next();
		PRINT_FIELD_U(c, elem_size);
		tprint_struct_next();
		PRINT_FIELD_U(c, elems);
		tprint_struct_next();
		PRINT_FIELD_U(c, nr_of_dims);
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(c, dims, c.nr_of_dims, tcp,
				       print_uint_array_member);
		if (!IS_ARRAY_ZERO(c.reserved)) {
			tprint_struct_next();
			PRINT_FIELD_ARRAY(c, reserved, tcp,
					  print_xint_array_member);
		}
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_cropcap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_cropcap c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(c, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(c, bounds, print_v4l2_rect);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(c, defrect, print_v4l2_rect);
		tprint_struct_next();
		PRINT_FIELD_FRACT(c, pixelaspect);
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_crop(struct tcb *const tcp, const kernel_ulong_t arg,
		const bool is_get)
{
	struct v4l2_crop c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(c, type, v4l2_buf_types,
				 "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(c, c, print_v4l2_rect);
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &c)) {
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(c, c, print_v4l2_rect);
		}
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static bool
print_v4l2_ext_control(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct_v4l2_ext_control *p = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*p, id, v4l2_control_ids, "V4L2_CID_???");
	tprint_struct_next();
	PRINT_FIELD_U(*p, size);
	if (p->size > 0) {
		tprint_struct_next();
		tprints_field_name("string");
		printstrn(tcp, ptr_to_kulong(p->string), p->size);
	} else {
		tprint_struct_next();
		PRINT_FIELD_D(*p, value);
		tprint_struct_next();
		PRINT_FIELD_D(*p, value64);
	}
	tprint_struct_end();

	return true;
}

static int
print_v4l2_ext_controls(struct tcb *const tcp, const kernel_ulong_t arg,
			const bool is_get)
{
	struct_v4l2_ext_controls c;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(c, ctrl_class, v4l2_control_classes,
				 "V4L2_CTRL_CLASS_???");
		tprint_struct_next();
		PRINT_FIELD_U(c, count);
		if (!c.count) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
		if (is_get)
			return 0;
		tprint_struct_next();
	} else {
		if (umove(tcp, arg, &c) < 0) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
		if (is_get) {
			tprint_struct_next();
		} else {
			tprint_struct_end();
			tprint_value_changed();
			tprint_struct_begin();
		}
	}

	tprints_field_name("controls");
	struct_v4l2_ext_control ctrl;
	bool fail = !print_array(tcp, ptr_to_kulong(c.controls), c.count,
				 &ctrl, sizeof(ctrl),
				 tfetch_mem_ignore_syserror,
				 print_v4l2_ext_control, 0);

	if (exiting(tcp) && syserror(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_U(c, error_idx);
	}

	if (exiting(tcp) || fail) {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	return 0;
}

#include "xlat/v4l2_framesize_types.h"

static void
print_v4l2_frmsize_discrete(const struct v4l2_frmsize_discrete *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, height);
	tprint_struct_end();
}

static void
print_v4l2_frmsize_stepwise(const struct v4l2_frmsize_stepwise *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, min_width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, max_width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, step_width);
	tprint_struct_next();
	PRINT_FIELD_U(*p, min_height);
	tprint_struct_next();
	PRINT_FIELD_U(*p, max_height);
	tprint_struct_next();
	PRINT_FIELD_U(*p, step_height);
	tprint_struct_end();
}

#define PRINT_FIELD_V4L2_FRMSIZE_TYPE(where_, field_)			\
	do {								\
		tprints_field_name(#field_);				\
		print_v4l2_frmsize_ ## field_(&((where_).field_));	\
	} while (0)

static int
print_v4l2_frmsizeenum(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_frmsizeenum s;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(s, index);
		tprint_struct_next();
		PRINT_FIELD_PIXFMT(s, pixel_format, v4l2_pix_fmts);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &s)) {
		tprint_struct_next();
		PRINT_FIELD_XVAL(s, type, v4l2_framesize_types,
				 "V4L2_FRMSIZE_TYPE_???");
		switch (s.type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE:
			tprint_struct_next();
			PRINT_FIELD_V4L2_FRMSIZE_TYPE(s, discrete);
			break;
		case V4L2_FRMSIZE_TYPE_STEPWISE:
			tprint_struct_next();
			PRINT_FIELD_V4L2_FRMSIZE_TYPE(s, stepwise);
			break;
		}
	}
	tprint_struct_end();
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_frameinterval_types.h"

static void
print_v4l2_frmival_stepwise(const struct v4l2_frmival_stepwise *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_FRACT(*p, min);
	tprint_struct_next();
	PRINT_FIELD_FRACT(*p, max);
	tprint_struct_next();
	PRINT_FIELD_FRACT(*p, step);
	tprint_struct_end();
}

#define PRINT_FIELD_V4L2_FRMIVAL_STEPWISE(where_, field_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_v4l2_frmival_stepwise(&((where_).field_));	\
	} while (0)

static int
print_v4l2_frmivalenum(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_frmivalenum f;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(f, index);
		tprint_struct_next();
		PRINT_FIELD_PIXFMT(f, pixel_format, v4l2_pix_fmts);
		tprint_struct_next();
		PRINT_FIELD_U(f, width);
		tprint_struct_next();
		PRINT_FIELD_U(f, height);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprint_struct_next();
		PRINT_FIELD_XVAL(f, type, v4l2_frameinterval_types,
				 "V4L2_FRMIVAL_TYPE_???");
		switch (f.type) {
		case V4L2_FRMIVAL_TYPE_DISCRETE:
			tprint_struct_next();
			PRINT_FIELD_FRACT(f, discrete);
			break;
		case V4L2_FRMIVAL_TYPE_STEPWISE:
		case V4L2_FRMSIZE_TYPE_CONTINUOUS:
			tprint_struct_next();
			PRINT_FIELD_V4L2_FRMIVAL_STEPWISE(f, stepwise);
			break;
		}
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static void
print_v4l2_create_buffers_format(const typeof_field(struct_v4l2_create_buffers, format) *const p,
				 struct tcb *const tcp)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*p, type, v4l2_buf_types,
			 "V4L2_BUF_TYPE_???");
	print_v4l2_format_fmt(tcp, tprint_struct_next,
			      (const struct_v4l2_format *) p);
	tprint_struct_end();
}

static int
print_v4l2_create_buffers(struct tcb *const tcp, const kernel_ulong_t arg)
{
	static const char fmt[] = "{index=%u, count=%u}";
	static char outstr[sizeof(fmt) + sizeof(int) * 6];

	struct_v4l2_create_buffers b;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_U(b, count);
		tprint_struct_next();
		PRINT_FIELD_XVAL(b, memory, v4l2_memories,
				 "V4L2_MEMORY_???");
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(b, format,
				    print_v4l2_create_buffers_format, tcp);
		tprint_struct_end();
		return 0;
	}

	if (syserror(tcp) || umove(tcp, arg, &b))
		return RVAL_IOCTL_DECODED;

	xsprintf(outstr, fmt, b.index, b.count);
	tcp->auxstr = outstr;

	return RVAL_IOCTL_DECODED | RVAL_STR;
}

MPERS_PRINTER_DECL(int, v4l2_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case VIDIOC_QUERYCAP: /* R */
		return print_v4l2_capability(tcp, arg);

	case VIDIOC_ENUM_FMT: /* RW */
		return print_v4l2_fmtdesc(tcp, arg);

	case VIDIOC_G_FMT: /* RW */
	case VIDIOC_S_FMT: /* RW */
	case VIDIOC_TRY_FMT: /* RW */
		return print_v4l2_format(tcp, arg, code == VIDIOC_G_FMT);

	case VIDIOC_REQBUFS: /* RW */
		return print_v4l2_requestbuffers(tcp, arg);

	case VIDIOC_QUERYBUF: /* RW */
	case VIDIOC_QBUF: /* RW */
	case VIDIOC_DQBUF: /* RW */
		return print_v4l2_buffer(tcp, code, arg);

	case VIDIOC_G_FBUF: /* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case VIDIOC_S_FBUF: /* W */
		return print_v4l2_framebuffer(tcp, arg);

	case VIDIOC_STREAMON: /* W */
	case VIDIOC_STREAMOFF: /* W */
		return print_v4l2_buf_type(tcp, arg);

	case VIDIOC_G_PARM: /* RW */
	case VIDIOC_S_PARM: /* RW */
		return print_v4l2_streamparm(tcp, arg, code == VIDIOC_G_PARM);

	case VIDIOC_G_STD: /* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case VIDIOC_S_STD: /* W */
		tprint_arg_next();
		printnum_int64(tcp, arg, "%#" PRIx64);
		break;

	case VIDIOC_ENUMSTD: /* RW */
		return print_v4l2_standard(tcp, arg);

	case VIDIOC_ENUMINPUT: /* RW */
		return print_v4l2_input(tcp, arg);

	case VIDIOC_G_CTRL: /* RW */
	case VIDIOC_S_CTRL: /* RW */
		return print_v4l2_control(tcp, arg, code == VIDIOC_G_CTRL);

	case VIDIOC_G_TUNER: /* RW */
	case VIDIOC_S_TUNER: /* RW */
		return print_v4l2_tuner(tcp, arg, code == VIDIOC_G_TUNER);

	case VIDIOC_QUERYCTRL: /* RW */
		return print_v4l2_queryctrl(tcp, arg);

	case VIDIOC_QUERY_EXT_CTRL: /* RW */
		return print_v4l2_query_ext_ctrl(tcp, arg);

	case VIDIOC_G_INPUT: /* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case VIDIOC_S_INPUT: /* RW */
		tprint_arg_next();
		printnum_int(tcp, arg, "%u");
		break;

	case VIDIOC_CROPCAP: /* RW */
		return print_v4l2_cropcap(tcp, arg);

	case VIDIOC_G_CROP: /* RW */
	case VIDIOC_S_CROP: /* W */
		return print_v4l2_crop(tcp, arg, code == VIDIOC_G_CROP);

	case VIDIOC_S_EXT_CTRLS: /* RW */
	case VIDIOC_TRY_EXT_CTRLS: /* RW */
	case VIDIOC_G_EXT_CTRLS: /* RW */
		return print_v4l2_ext_controls(tcp, arg,
					       code == VIDIOC_G_EXT_CTRLS);

	case VIDIOC_ENUM_FRAMESIZES: /* RW */
		return print_v4l2_frmsizeenum(tcp, arg);

	case VIDIOC_ENUM_FRAMEINTERVALS: /* RW */
		return print_v4l2_frmivalenum(tcp, arg);

	case VIDIOC_CREATE_BUFS: /* RW */
		return print_v4l2_create_buffers(tcp, arg);

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
