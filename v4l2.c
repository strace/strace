/*
 * Copyright (c) 2014 Philippe De Muyter <phdm@macqel.be>
 * Copyright (c) 2014 William Manley <will@williammanley.net>
 * Copyright (c) 2011 Peter Zotov <whitequark@whitequark.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <stdint.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

#include DEF_MPERS_TYPE(struct_v4l2_buffer)
#include DEF_MPERS_TYPE(struct_v4l2_clip)
#ifdef VIDIOC_CREATE_BUFS
# include DEF_MPERS_TYPE(struct_v4l2_create_buffers)
#endif
#include DEF_MPERS_TYPE(struct_v4l2_ext_control)
#include DEF_MPERS_TYPE(struct_v4l2_ext_controls)
#include DEF_MPERS_TYPE(struct_v4l2_format)
#include DEF_MPERS_TYPE(struct_v4l2_framebuffer)
#include DEF_MPERS_TYPE(struct_v4l2_input)
#include DEF_MPERS_TYPE(struct_v4l2_standard)

typedef struct v4l2_buffer struct_v4l2_buffer;
typedef struct v4l2_clip struct_v4l2_clip;
#ifdef VIDIOC_CREATE_BUFS
typedef struct v4l2_create_buffers struct_v4l2_create_buffers;
#endif
typedef struct v4l2_ext_control struct_v4l2_ext_control;
typedef struct v4l2_ext_controls struct_v4l2_ext_controls;
typedef struct v4l2_format struct_v4l2_format;
typedef struct v4l2_framebuffer struct_v4l2_framebuffer;
typedef struct v4l2_input struct_v4l2_input;
typedef struct v4l2_standard struct_v4l2_standard;

#include MPERS_DEFS

#include "print_fields.h"
#include "print_utils.h"
#include "xstring.h"

/* v4l2_fourcc_be was added by Linux commit v3.18-rc1~101^2^2~127 */
#ifndef v4l2_fourcc_be
# define v4l2_fourcc_be(a, b, c, d) (v4l2_fourcc(a, b, c, d) | (1U << 31))
#endif

#define FMT_FRACT "%u/%u"
#define ARGS_FRACT(x) ((x).numerator), ((x).denominator)

#define FMT_RECT "{left=%d, top=%d, width=%u, height=%u}"
#define ARGS_RECT(x) (x).left, (x).top, (x).width, (x).height

#include "xlat/v4l2_pix_fmts.h"
#include "xlat/v4l2_sdr_fmts.h"

static void
print_pixelformat(uint32_t fourcc, const struct xlat *xlat)
{
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		tprintf("%#x", fourcc);
		return;
	}

	unsigned char a[] = {
		(unsigned char) fourcc,
		(unsigned char) (fourcc >> 8),
		(unsigned char) (fourcc >> 16),
		(unsigned char) (fourcc >> 24),
	};
	unsigned int i;

	tprints("v4l2_fourcc(");
	/* Generic char array printing routine.  */
	for (i = 0; i < ARRAY_SIZE(a); ++i) {
		unsigned char c = a[i];

		if (i)
			tprints(", ");
		if (c == '\'' || c == '\\') {
			char sym[] = {
				'\'',
				'\\',
				c,
				'\'',
				'\0'
			};
			tprints(sym);
		} else if (is_print(c)) {
			char sym[] = {
				'\'',
				c,
				'\'',
				'\0'
			};
			tprints(sym);
		} else {
			char hex[] = {
				BYTE_HEX_CHARS_PRINTF_QUOTED(c),
				'\0'
			};
			tprints(hex);
		}
	}
	tprints(")");

	if (xlat) {
		const char *pixfmt_name = xlookup(xlat, fourcc);

		if (pixfmt_name)
			tprints_comment(pixfmt_name);
	}
}

#include "xlat/v4l2_device_capabilities_flags.h"

static int
print_v4l2_capability(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_capability caps;

	if (entering(tcp))
		return 0;
	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &caps))
		return RVAL_IOCTL_DECODED;
	PRINT_FIELD_CSTRING("{", caps, driver);
	PRINT_FIELD_CSTRING(", ", caps, card);
	PRINT_FIELD_CSTRING(", ", caps, bus_info);
	tprintf(", version=%u.%u.%u, capabilities=",
		(caps.version >> 16) & 0xFF,
		(caps.version >> 8) & 0xFF,
		caps.version & 0xFF);
	printflags(v4l2_device_capabilities_flags, caps.capabilities,
		   "V4L2_CAP_???");
#ifdef HAVE_STRUCT_V4L2_CAPABILITY_DEVICE_CAPS
	tprints(", device_caps=");
	printflags(v4l2_device_capabilities_flags, caps.device_caps,
		   "V4L2_CAP_???");
#endif
	tprints("}");
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_buf_types.h"
#include "xlat/v4l2_format_description_flags.h"

static int
print_v4l2_fmtdesc(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_fmtdesc f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u, type=", f.index);
		printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprints(", flags=");
		printflags(v4l2_format_description_flags, f.flags,
			   "V4L2_FMT_FLAG_???");
		PRINT_FIELD_CSTRING(", ", f, description);
		tprints(", pixelformat=");
		print_pixelformat(f.pixelformat, v4l2_pix_fmts);
	}
	tprints("}");
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
	tprintf(FMT_RECT, ARGS_RECT(p->c));
	return true;
}

static bool
print_v4l2_format_fmt(struct tcb *const tcp, const char *prefix,
		      const struct_v4l2_format *f)
{
	bool ret = true;
	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		tprints(prefix);
		tprintf("fmt.pix={width=%u, height=%u, pixelformat=",
			f->fmt.pix.width, f->fmt.pix.height);
		print_pixelformat(f->fmt.pix.pixelformat, v4l2_pix_fmts);
		tprints(", field=");
		printxval(v4l2_fields, f->fmt.pix.field, "V4L2_FIELD_???");
		tprintf(", bytesperline=%u, sizeimage=%u, colorspace=",
			f->fmt.pix.bytesperline, f->fmt.pix.sizeimage);
		printxval(v4l2_colorspaces, f->fmt.pix.colorspace,
			  "V4L2_COLORSPACE_???");
		tprints("}");
		break;
#if HAVE_STRUCT_V4L2_FORMAT_FMT_PIX_MP
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: {
		unsigned int i, max;

		tprints(prefix);
		tprintf("fmt.pix_mp={width=%u, height=%u, pixelformat=",
			f->fmt.pix_mp.width, f->fmt.pix_mp.height);
		print_pixelformat(f->fmt.pix_mp.pixelformat, v4l2_pix_fmts);
		tprints(", field=");
		printxval(v4l2_fields, f->fmt.pix_mp.field, "V4L2_FIELD_???");
		tprints(", colorspace=");
		printxval(v4l2_colorspaces, f->fmt.pix_mp.colorspace,
			  "V4L2_COLORSPACE_???");
		tprints(", plane_fmt=[");
		max = f->fmt.pix_mp.num_planes;
		if (max > VIDEO_MAX_PLANES)
			max = VIDEO_MAX_PLANES;
		for (i = 0; i < max; i++) {
			if (i > 0)
				tprints(", ");
			tprintf("{sizeimage=%u, bytesperline=%u}",
				f->fmt.pix_mp.plane_fmt[i].sizeimage,
				f->fmt.pix_mp.plane_fmt[i].bytesperline);
		}
		tprintf("], num_planes=%u}",
			(unsigned) f->fmt.pix_mp.num_planes);
		break;
	}
#endif
	/* OUTPUT_OVERLAY since Linux v2.6.22-rc1~1118^2~179 */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY: {
		struct_v4l2_clip clip;
		tprints(prefix);
		tprintf("fmt.win={left=%d, top=%d, width=%u, height=%u, field=",
			ARGS_RECT(f->fmt.win.w));
		printxval(v4l2_fields, f->fmt.win.field, "V4L2_FIELD_???");
		tprintf(", chromakey=%#x, clips=", f->fmt.win.chromakey);
		ret = print_array(tcp, ptr_to_kulong(f->fmt.win.clips),
				  f->fmt.win.clipcount, &clip, sizeof(clip),
				  tfetch_mem, print_v4l2_clip, 0);
		tprintf(", clipcount=%u, bitmap=", f->fmt.win.clipcount);
		printaddr(ptr_to_kulong(f->fmt.win.bitmap));
#ifdef HAVE_STRUCT_V4L2_WINDOW_GLOBAL_ALPHA
		tprintf(", global_alpha=%#x", f->fmt.win.global_alpha);
#endif
		tprints("}");
		break;
	}
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		tprints(prefix);
		tprintf("fmt.vbi={sampling_rate=%u, offset=%u, "
			"samples_per_line=%u, sample_format=",
			f->fmt.vbi.sampling_rate, f->fmt.vbi.offset,
			f->fmt.vbi.samples_per_line);
		print_pixelformat(f->fmt.vbi.sample_format, v4l2_pix_fmts);
		tprintf(", start=[%d, %d], count=[%u, %u], ",
			f->fmt.vbi.start[0], f->fmt.vbi.start[1],
			f->fmt.vbi.count[0], f->fmt.vbi.count[1]);
		tprints("flags=");
		printflags(v4l2_vbi_flags, f->fmt.vbi.flags, "V4L2_VBI_???");
		tprints("}");
		break;
	/* both since Linux v2.6.14-rc2~64 */
#if HAVE_STRUCT_V4L2_FORMAT_FMT_SLICED
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: {
		unsigned int i, j;

		tprints(prefix);
		tprints("fmt.sliced={service_set=");
		printxval(v4l2_sliced_flags, f->fmt.sliced.service_set,
			"V4L2_SLICED_???");
		tprintf(", io_size=%u, service_lines=[",
			f->fmt.sliced.io_size);
		for (i = 0; i < ARRAY_SIZE(f->fmt.sliced.service_lines); i++) {
			if (i > 0)
				tprints(", ");
			tprints("[");
			for (j = 0;
			     j < ARRAY_SIZE(f->fmt.sliced.service_lines[0]);
			     j++) {
				if (j > 0)
					tprints(", ");
				tprintf("%#x",
					f->fmt.sliced.service_lines[i][j]);
			}
			tprints("]");
		}
		tprints("]}");
		break;
	}
#endif
#if HAVE_STRUCT_V4L2_FORMAT_FMT_SDR
	/* since Linux v4.4-rc1~118^2~14 */
	case V4L2_BUF_TYPE_SDR_OUTPUT:
	/* since Linux v3.15-rc1~85^2~213 */
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		tprints(prefix);
		tprints("fmt.sdr={pixelformat=");
		print_pixelformat(f->fmt.sdr.pixelformat, v4l2_sdr_fmts);
# ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		tprintf(", buffersize=%u",
			f->fmt.sdr.buffersize);
# endif
		tprints("}");
		break;
#endif
	default:
		return false;
	}
	return ret;
}

static int
print_v4l2_format(struct tcb *const tcp, const kernel_ulong_t arg,
		  const bool is_get)
{
	struct_v4l2_format f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;
		tprints("{type=");
		printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		if (!print_v4l2_format_fmt(tcp, ", ", &f)) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f))
		print_v4l2_format_fmt(tcp, is_get ? ", " : " => ", &f);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_memories.h"

static int
print_v4l2_requestbuffers(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_requestbuffers reqbufs;

	if (entering(tcp)) {
		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &reqbufs))
			return RVAL_IOCTL_DECODED;

		tprintf("{type=");
		printxval(v4l2_buf_types, reqbufs.type, "V4L2_BUF_TYPE_???");
		tprints(", memory=");
		printxval(v4l2_memories, reqbufs.memory, "V4L2_MEMORY_???");
		tprintf(", count=%u", reqbufs.count);

		return 0;
	}

	if (!syserror(tcp)) {
		tprints(" => ");

		if (!umove(tcp, arg, &reqbufs))
			tprintf("%u", reqbufs.count);
		else
			tprints("???");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_buf_flags.h"

static int
print_v4l2_buffer(struct tcb *const tcp, const unsigned int code,
		  const kernel_ulong_t arg)
{
	struct_v4l2_buffer b;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_IOCTL_DECODED;
		tprints("{type=");
		printxval(v4l2_buf_types, b.type, "V4L2_BUF_TYPE_???");
		if (code != VIDIOC_DQBUF)
			tprintf(", index=%u", b.index);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &b)) {
		if (code == VIDIOC_DQBUF)
			tprintf(", index=%u", b.index);
		tprints(", memory=");
		printxval(v4l2_memories, b.memory, "V4L2_MEMORY_???");

		if (b.memory == V4L2_MEMORY_MMAP) {
			tprintf(", m.offset=%#x", b.m.offset);
		} else if (b.memory == V4L2_MEMORY_USERPTR) {
			tprints(", m.userptr=");
			printaddr(b.m.userptr);
		}

		tprintf(", length=%u, bytesused=%u, flags=",
			b.length, b.bytesused);
		printflags(v4l2_buf_flags, b.flags, "V4L2_BUF_FLAG_???");
		if (code == VIDIOC_DQBUF) {
			tprints(", timestamp = ");
			MPERS_FUNC_NAME(print_struct_timeval)(&b.timestamp);
		}
		tprints(", ...");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_framebuffer(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_framebuffer b;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &b)) {
		tprintf("{capability=%#x, flags=%#x, base=",
			b.capability, b.flags);
		printaddr(ptr_to_kulong(b.base));
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_buf_type(struct tcb *const tcp, const kernel_ulong_t arg)
{
	int type;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &type)) {
		tprints("[");
		printxval(v4l2_buf_types, type, "V4L2_BUF_TYPE_???");
		tprints("]");
	}
	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_streaming_capabilities.h"
#include "xlat/v4l2_capture_modes.h"

static int
print_v4l2_streamparm(struct tcb *const tcp, const kernel_ulong_t arg,
		      const bool is_get)
{
	struct v4l2_streamparm s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;
		tprints("{type=");
		printxval(v4l2_buf_types, s.type, "V4L2_BUF_TYPE_???");
		switch (s.type) {
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				if (is_get)
					return 0;
				tprints(", ");
				break;
			default:
				tprints("}");
				return RVAL_IOCTL_DECODED;
		}
	} else {
		if (syserror(tcp) || umove(tcp, arg, &s) < 0) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}
		tprints(is_get ? ", " : " => ");
	}

	if (s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		tprints("parm.capture={capability=");
		printflags(v4l2_streaming_capabilities,
			   s.parm.capture.capability, "V4L2_CAP_???");

		tprints(", capturemode=");
		printflags(v4l2_capture_modes,
			   s.parm.capture.capturemode, "V4L2_MODE_???");

		tprintf(", timeperframe=" FMT_FRACT,
			ARGS_FRACT(s.parm.capture.timeperframe));

		tprintf(", extendedmode=%u, readbuffers=%u}",
			s.parm.capture.extendedmode,
			s.parm.capture.readbuffers);
	} else {
		tprints("parm.output={capability=");
		printflags(v4l2_streaming_capabilities,
			   s.parm.output.capability, "V4L2_CAP_???");

		tprintf(", outputmode=%u", s.parm.output.outputmode);

		tprintf(", timeperframe=" FMT_FRACT,
			ARGS_FRACT(s.parm.output.timeperframe));

		tprintf(", extendedmode=%u, writebuffers=%u}",
			s.parm.output.extendedmode,
			s.parm.output.writebuffers);
	}
	if (entering(tcp)) {
		return 0;
	} else {
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}
}

static int
print_v4l2_standard(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_standard s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u", s.index);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &s)) {
		PRINT_FIELD_CSTRING(", ", s, name);
		tprintf(", frameperiod=" FMT_FRACT,
			ARGS_FRACT(s.frameperiod));
		tprintf(", framelines=%d", s.framelines);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#include "xlat/v4l2_input_types.h"

static int
print_v4l2_input(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_v4l2_input i;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &i))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u", i.index);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &i)) {
		PRINT_FIELD_CSTRING(", ", i, name);
		tprints(", type=");
		printxval(v4l2_input_types, i.type, "V4L2_INPUT_TYPE_???");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

/*
 * We include it here and not before print_v4l2_ext_controls as we need
 * V4L2_CTRL_CLASS_* definitions for V4L2_CID_*_BASE ones.
 */
#include "xlat/v4l2_control_classes.h"
#include "xlat/v4l2_control_id_bases.h"
#include "xlat/v4l2_control_ids.h"

static void
print_v4l2_cid(const uint32_t cid)
{
	const char *id_name = xlookup(v4l2_control_ids, cid);

	if (id_name) {
		print_xlat_ex(cid, id_name, XLAT_STYLE_DEFAULT);
		return;
	}

	uint64_t class_id = cid;
	const char *class_str = xlookup_le(v4l2_control_classes, &class_id);

	if (!class_str || (cid - class_id) >= 0x10000) {
		print_xlat_ex(cid, "V4L2_CID_???", PXF_DEFAULT_STR);
		return;
	}

	char *tmp_str;

	if (asprintf(&tmp_str, "%s+%#" PRIx64,
		     class_str, cid - class_id) < 0)
		tmp_str = NULL;

	print_xlat_ex(cid, tmp_str, XLAT_STYLE_DEFAULT);
	free(tmp_str);
}

static int
print_v4l2_control(struct tcb *const tcp, const kernel_ulong_t arg,
		   const bool is_get)
{
	struct v4l2_control c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprints("{id=");
		print_v4l2_cid(c.id);
		if (!is_get)
			tprintf(", value=%d", c.value);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		tprints(is_get ? ", " : " => ");
		tprintf("value=%d", c.value);
	}

	tprints("}");

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
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u", c.index);
		if (is_get)
			return 0;
		tprints(", ");
	} else {
		if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}
		tprints(is_get ? ", " : " => ");
	}

	PRINT_FIELD_CSTRING("", c, name);
	tprints(", type=");
	printxval(v4l2_tuner_types, c.type, "V4L2_TUNER_TYPE_???");
	tprints(", capability=");
	printxval(v4l2_tuner_capabilities, c.capability,
		  "V4L2_TUNER_CAP_???");
	tprintf(", rangelow=%u, rangehigh=%u, rxsubchans=",
		c.rangelow, c.rangehigh);
	printxval(v4l2_tuner_rxsubchanses, c.rxsubchans,
		  "V4L2_TUNER_SUB_???");
	tprints(", audmode=");
	printxval(v4l2_tuner_audmodes, c.audmode,
		  "V4L2_TUNER_MODE_???");
	tprintf(", signal=%d, afc=%d", c.signal, c.afc);

	if (entering(tcp)) {
		return 0;
	} else {
		tprints("}");
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
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;
		tprints("{id=");
	} else {
		if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}
		if (get_tcb_priv_ulong(tcp))
			tprints(" => ");
	}

	if (entering(tcp) || get_tcb_priv_ulong(tcp)) {
#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
		const unsigned long next = c.id & V4L2_CTRL_FLAG_NEXT_CTRL;
		set_tcb_priv_ulong(tcp, next);
		if (next) {
			print_xlat(V4L2_CTRL_FLAG_NEXT_CTRL);
			tprints("|");
			c.id &= ~V4L2_CTRL_FLAG_NEXT_CTRL;
		}
#endif
		printxval(v4l2_control_ids, c.id, "V4L2_CID_???");
	}

	if (exiting(tcp)) {
		tprints(", type=");
		printxval(v4l2_control_types, c.type, "V4L2_CTRL_TYPE_???");
		PRINT_FIELD_CSTRING(", ", c, name);
		tprintf(", minimum=%d, maximum=%d, step=%d"
			", default_value=%d, flags=",
			c.minimum, c.maximum, c.step, c.default_value);
		printflags(v4l2_control_flags, c.flags, "V4L2_CTRL_FLAG_???");
		tprints("}");
	}
	return entering(tcp) ? 0 : RVAL_IOCTL_DECODED;
}

static int
print_v4l2_cropcap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_cropcap c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;
		tprints("{type=");
		printxval(v4l2_buf_types, c.type, "V4L2_BUF_TYPE_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		tprintf(", bounds=" FMT_RECT
			", defrect=" FMT_RECT
			", pixelaspect=" FMT_FRACT,
			ARGS_RECT(c.bounds),
			ARGS_RECT(c.defrect),
			ARGS_FRACT(c.pixelaspect));
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
print_v4l2_crop(struct tcb *const tcp, const kernel_ulong_t arg,
		const bool is_get)
{
	struct v4l2_crop c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;
		tprints("{type=");
		printxval(v4l2_buf_types, c.type, "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		tprintf(", c=" FMT_RECT, ARGS_RECT(c.c));
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &c))
			tprintf(", c=" FMT_RECT, ARGS_RECT(c.c));
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#ifdef VIDIOC_S_EXT_CTRLS
static bool
print_v4l2_ext_control(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct_v4l2_ext_control *p = elem_buf;

	tprints("{id=");
	printxval(v4l2_control_ids, p->id, "V4L2_CID_???");
# if HAVE_STRUCT_V4L2_EXT_CONTROL_STRING
	tprintf(", size=%u", p->size);
	if (p->size > 0) {
		tprints(", string=");
		printstrn(tcp, ptr_to_kulong(p->string), p->size);
	} else
# endif
	tprintf(", value=%d, value64=%" PRId64, p->value, (int64_t) p->value64);
	tprints("}");

	return true;
}

static int
print_v4l2_ext_controls(struct tcb *const tcp, const kernel_ulong_t arg,
			const bool is_get)
{
	struct_v4l2_ext_controls c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;
		tprints("{ctrl_class=");
		printxval(v4l2_control_classes, c.ctrl_class,
			  "V4L2_CTRL_CLASS_???");
		tprintf(", count=%u", c.count);
		if (!c.count) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}
		if (is_get)
			return 0;
		tprints(", ");
	} else {
		if (umove(tcp, arg, &c) < 0) {
			tprints("}");
			return RVAL_IOCTL_DECODED;
		}
		tprints(is_get ? ", " : " => ");
	}

	tprints("controls=");
	struct_v4l2_ext_control ctrl;
	bool fail = !print_array(tcp, ptr_to_kulong(c.controls), c.count,
				 &ctrl, sizeof(ctrl),
				 tfetch_mem_ignore_syserror,
				 print_v4l2_ext_control, 0);

	if (exiting(tcp) && syserror(tcp))
		tprintf(", error_idx=%u", c.error_idx);

	if (exiting(tcp) || fail) {
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	return 0;
}
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
# include "xlat/v4l2_framesize_types.h"

static int
print_v4l2_frmsizeenum(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_frmsizeenum s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u, pixel_format=", s.index);
		print_pixelformat(s.pixel_format, v4l2_pix_fmts);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &s)) {
		tprints(", type=");
		printxval(v4l2_framesize_types, s.type, "V4L2_FRMSIZE_TYPE_???");
		switch (s.type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE:
			tprintf(", discrete={width=%u, height=%u}",
				s.discrete.width, s.discrete.height);
			break;
		case V4L2_FRMSIZE_TYPE_STEPWISE:
			tprintf(", stepwise={min_width=%u, max_width=%u, "
				"step_width=%u, min_height=%u, max_height=%u, "
				"step_height=%u}",
				s.stepwise.min_width, s.stepwise.max_width,
				s.stepwise.step_width, s.stepwise.min_height,
				s.stepwise.max_height, s.stepwise.step_height);
			break;
		}
	}
	tprints("}");
	return RVAL_IOCTL_DECODED;
}
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
# include "xlat/v4l2_frameinterval_types.h"

static int
print_v4l2_frmivalenum(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct v4l2_frmivalenum f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_IOCTL_DECODED;
		tprintf("{index=%u, pixel_format=", f.index);
		print_pixelformat(f.pixel_format, v4l2_pix_fmts);
		tprintf(", width=%u, height=%u", f.width, f.height);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprints(", type=");
		printxval(v4l2_frameinterval_types, f.type,
			  "V4L2_FRMIVAL_TYPE_???");
		switch (f.type) {
		case V4L2_FRMIVAL_TYPE_DISCRETE:
			tprintf(", discrete=" FMT_FRACT,
				ARGS_FRACT(f.discrete));
			break;
		case V4L2_FRMIVAL_TYPE_STEPWISE:
		case V4L2_FRMSIZE_TYPE_CONTINUOUS:
			tprintf(", stepwise={min=" FMT_FRACT ", max="
				FMT_FRACT ", step=" FMT_FRACT "}",
				ARGS_FRACT(f.stepwise.min),
				ARGS_FRACT(f.stepwise.max),
				ARGS_FRACT(f.stepwise.step));
			break;
		}
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
static int
print_v4l2_create_buffers(struct tcb *const tcp, const kernel_ulong_t arg)
{
	static const char fmt[] = "{index=%u, count=%u}";
	static char outstr[sizeof(fmt) + sizeof(int) * 6];

	struct_v4l2_create_buffers b;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_IOCTL_DECODED;
		tprintf("{count=%u, memory=", b.count);
		printxval(v4l2_memories, b.memory, "V4L2_MEMORY_???");
		tprints(", format={type=");
		printxval(v4l2_buf_types, b.format.type,
			  "V4L2_BUF_TYPE_???");
		print_v4l2_format_fmt(tcp, ", ",
				      (struct_v4l2_format *) &b.format);
		tprints("}}");
		return 0;
	}

	if (syserror(tcp) || umove(tcp, arg, &b))
		return RVAL_IOCTL_DECODED;

	xsprintf(outstr, fmt, b.index, b.count);
	tcp->auxstr = outstr;

	return RVAL_IOCTL_DECODED | RVAL_STR;
}
#endif /* VIDIOC_CREATE_BUFS */

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
		tprints(", ");
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

	case VIDIOC_G_INPUT: /* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case VIDIOC_S_INPUT: /* RW */
		tprints(", ");
		printnum_int(tcp, arg, "%u");
		break;

	case VIDIOC_CROPCAP: /* RW */
		return print_v4l2_cropcap(tcp, arg);

	case VIDIOC_G_CROP: /* RW */
	case VIDIOC_S_CROP: /* W */
		return print_v4l2_crop(tcp, arg, code == VIDIOC_G_CROP);

#ifdef VIDIOC_S_EXT_CTRLS
	case VIDIOC_S_EXT_CTRLS: /* RW */
	case VIDIOC_TRY_EXT_CTRLS: /* RW */
	case VIDIOC_G_EXT_CTRLS: /* RW */
		return print_v4l2_ext_controls(tcp, arg,
					       code == VIDIOC_G_EXT_CTRLS);
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
	case VIDIOC_ENUM_FRAMESIZES: /* RW */
		return print_v4l2_frmsizeenum(tcp, arg);
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	case VIDIOC_ENUM_FRAMEINTERVALS: /* RW */
		return print_v4l2_frmivalenum(tcp, arg);
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
	case VIDIOC_CREATE_BUFS: /* RW */
		return print_v4l2_create_buffers(tcp, arg);
#endif /* VIDIOC_CREATE_BUFS */

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
