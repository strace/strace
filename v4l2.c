/*
 * Copyright (c) 2014 Philippe De Muyter <phdm@macqel.be>
 * Copyright (c) 2014 William Manley <will@williammanley.net>
 * Copyright (c) 2011 Peter Zotov <whitequark@whitequark.org>
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

#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
/* some historical constants */
#ifndef V4L2_CID_HCENTER
#define V4L2_CID_HCENTER (V4L2_CID_BASE+22)
#endif
#ifndef V4L2_CID_VCENTER
#define V4L2_CID_VCENTER (V4L2_CID_BASE+23)
#endif
#ifndef V4L2_CID_BAND_STOP_FILTER
#define V4L2_CID_BAND_STOP_FILTER (V4L2_CID_BASE+33)
#endif

#include "xlat/v4l2_device_capabilities_flags.h"
#include "xlat/v4l2_buf_types.h"
#include "xlat/v4l2_buf_flags.h"
#include "xlat/v4l2_framesize_types.h"
#include "xlat/v4l2_frameinterval_types.h"
#include "xlat/v4l2_fields.h"
#include "xlat/v4l2_colorspaces.h"
#include "xlat/v4l2_format_description_flags.h"
#include "xlat/v4l2_memories.h"
#include "xlat/v4l2_control_ids.h"
#include "xlat/v4l2_control_types.h"
#include "xlat/v4l2_control_flags.h"
#include "xlat/v4l2_control_classes.h"
#include "xlat/v4l2_streaming_capabilities.h"
#include "xlat/v4l2_capture_modes.h"
#include "xlat/v4l2_input_types.h"

#define FMT_FRACT "%u/%u"
#define ARGS_FRACT(x) ((x).numerator), ((x).denominator)

#define FMT_RECT "{left=%i, top=%i, width=%i, height=%i}"
#define ARGS_RECT(x) (x).left, (x).top, (x).width, (x).height

static void print_pixelformat(uint32_t fourcc)
{
	union {
		uint32_t pixelformat;
		unsigned char cc[sizeof(uint32_t)];
	} u = {
		.pixelformat =
#if WORDS_BIGENDIAN
			htole32(fourcc)
#else
			fourcc
#endif
	};
	unsigned int i;

	tprints("v4l2_fourcc(");
	for (i = 0; i < sizeof(u.cc); ++i) {
		unsigned int c = u.cc[i];

		if (i)
			tprints(", ");
		if (c == ' ' ||
		    (c >= '0' && c <= '9') ||
		    (c >= 'A' && c <= 'Z') ||
		    (c >= 'a' && c <= 'z')) {
			char sym[] = {
				'\'',
				u.cc[i],
				'\''
			};
			tprints(sym);
		} else {
			char hex[] = {
				'\'',
				'\\',
				'x',
				"0123456789abcdef"[c >> 4],
				"0123456789abcdef"[c & 0xf],
				'\'',
				'\0'
			};
			tprints(hex);
		}
	}
	tprints(")");
}

static void print_v4l2_format_fmt(const struct v4l2_format *f)
{
	tprints("fmt.");
	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT: {
		const struct v4l2_pix_format *pix = &f->fmt.pix;

		tprintf("pix={width=%u, height=%u, pixelformat=",
			pix->width, pix->height);
		print_pixelformat(pix->pixelformat);
		tprints(", field=");
		printxval(v4l2_fields, pix->field, "V4L2_FIELD_???");
		tprintf(", bytesperline=%u, sizeimage=%u, colorspace=",
			pix->bytesperline, pix->sizeimage);
		printxval(v4l2_colorspaces, pix->colorspace,
			  "V4L2_COLORSPACE_???");
		tprints("}");
		break;
	}
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: {
		const struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
		unsigned int i, max;

		tprintf("pix_mp={width=%u, height=%u, pixelformat=",
			pix_mp->width, pix_mp->height);
		print_pixelformat(pix_mp->pixelformat);
		tprints(", field=");
		printxval(v4l2_fields, pix_mp->field, "V4L2_FIELD_???");
		tprints(", colorspace=");
		printxval(v4l2_colorspaces, pix_mp->colorspace,
			  "V4L2_COLORSPACE_???");
		tprints("plane_fmt=[");
		max = pix_mp->num_planes;
		if (max > VIDEO_MAX_PLANES)
			max = VIDEO_MAX_PLANES;
		for (i = 0; i < max; i++) {
			if (i > 0)
				tprints(", ");
			tprintf("{sizeimage=%u, bytesperline=%u}",
				pix_mp->plane_fmt[i].sizeimage,
				pix_mp->plane_fmt[i].bytesperline);
		}
		tprintf("], num_planes=%u}", (unsigned) pix_mp->num_planes);
		break;
	}
#endif

	/* TODO: Complete this switch statement */
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
		tprints("win={???}");
		break;

	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		tprints("vbi={???}");
		break;

	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		tprints("sliced={???}");
		break;

	default:
		tprints("???");
		break;
	}
}

int
v4l2_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	if (!verbose(tcp))
		return 0;

	switch (code) {
	case VIDIOC_QUERYCAP: /* decode on exit */ {
		struct v4l2_capability caps;

		if (entering(tcp) || syserror(tcp) || umove(tcp, arg, &caps) < 0)
			return 0;
		tprints(", {driver=");
		print_quoted_string((const char *) caps.driver,
				    sizeof(caps.driver), QUOTE_0_TERMINATED);
		tprints(", card=");
		print_quoted_string((const char *) caps.card,
				    sizeof(caps.card), QUOTE_0_TERMINATED);
		tprints(", bus_info=");
		print_quoted_string((const char *) caps.bus_info,
				    sizeof(caps.bus_info), QUOTE_0_TERMINATED);
		tprintf(", version=%u.%u.%u, capabilities=",
			(caps.version >> 16) & 0xFF,
			(caps.version >> 8) & 0xFF,
			caps.version & 0xFF);
		printflags(v4l2_device_capabilities_flags, caps.capabilities,
			   "V4L2_CAP_???");
#ifdef V4L2_CAP_DEVICE_CAPS
		tprints(", device_caps=");
		printflags(v4l2_device_capabilities_flags, caps.device_caps,
			   "V4L2_CAP_???");
#endif
		tprints("}");
		return 1;
	}

#ifdef VIDIOC_ENUM_FRAMESIZES
	case VIDIOC_ENUM_FRAMESIZES: /* decode on exit */ {
		struct v4l2_frmsizeenum s;

		if (entering(tcp) || umove(tcp, arg, &s) < 0)
			return 0;
		tprintf(", {index=%u, pixel_format=", s.index);
		print_pixelformat(s.pixel_format);

		if (!syserror(tcp)) {
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
		return 1;
	}
#endif /* VIDIOC_ENUM_FRAMESIZES */

	case VIDIOC_G_FMT:
	case VIDIOC_S_FMT:
	case VIDIOC_TRY_FMT: {
		struct v4l2_format f;

		if (umove(tcp, arg, &f) < 0)
			return 0;
		if (entering(tcp)) {
			tprints(", {type=");
			printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
		}
		if ((entering(tcp) && code != VIDIOC_G_FMT)
		    || (exiting(tcp) && !syserror(tcp))) {
			tprints(exiting(tcp) && code != VIDIOC_G_FMT ? " => " : ", ");
			print_v4l2_format_fmt(&f);
		}
		if (exiting(tcp))
			tprints("}");
		return 1;
	}

	case VIDIOC_ENUM_FMT: {
		struct v4l2_fmtdesc f;

		if (entering(tcp) || umove(tcp, arg, &f) < 0)
			return 0;

		tprintf(", {index=%u", f.index);
		if (!syserror(tcp)) {
			tprints(", type=");
			printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
			tprints(", flags=");
			printflags(v4l2_format_description_flags, f.flags,
				   "V4L2_FMT_FLAG_???");
			tprints(", description=");
			print_quoted_string((const char *) f.description,
					    sizeof(f.description),
					    QUOTE_0_TERMINATED);
			tprints(", pixelformat=");
			print_pixelformat(f.pixelformat);
		}
		tprints("}");
		return 1;
	}

	case VIDIOC_G_PARM:
	case VIDIOC_S_PARM: {
		struct v4l2_streamparm s;

		if (entering(tcp) && code == VIDIOC_G_PARM)
			return 1;
		if (exiting(tcp) && syserror(tcp))
			return code == VIDIOC_S_PARM;
		if (umove(tcp, arg, &s) < 0)
			return 0;
		if (entering(tcp)) {
			tprints(", {type=");
			printxval(v4l2_buf_types, s.type, "V4L2_BUF_TYPE_???");
		}

		tprints(exiting(tcp) && code == VIDIOC_S_PARM ? " => {" : ", {");
		if (s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
			struct v4l2_captureparm *cap = &s.parm.capture;

			tprints("capability=");
			printflags(v4l2_streaming_capabilities,
				   cap->capability, "V4L2_CAP_???");

			tprints(", capturemode=");
			printflags(v4l2_capture_modes,
				   cap->capturemode, "V4L2_MODE_???");

			tprintf(", timeperframe=" FMT_FRACT,
				ARGS_FRACT(cap->timeperframe));

			tprintf(", extendedmode=%u, readbuffers=%u",
				cap->extendedmode,
				cap->readbuffers);
		} else
			tprints("...");
		tprints("}");
		if (exiting(tcp))
			tprints("}");
		return 1;
	}

	case VIDIOC_QUERYCTRL: {
		struct v4l2_queryctrl c;

		if (umove(tcp, arg, &c) < 0)
			return 0;
		/* 'id' field must be printed :
		* on enter
		* on exit if !syserror(tcp) && V4L2_CTRL_FLAG_NEXT_CTRL was set
		*/
		if (entering(tcp)
		    || (exiting(tcp) && tcp->auxstr && !syserror(tcp))) {
			tprints(exiting(tcp) ? " => " : ", {id=");
#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
			tcp->auxstr = (c.id & V4L2_CTRL_FLAG_NEXT_CTRL) ? "" : NULL;
			if (tcp->auxstr) {
				tprints("V4L2_CTRL_FLAG_NEXT_CTRL|");
				c.id &= ~V4L2_CTRL_FLAG_NEXT_CTRL;
			}
#endif
			printxval(v4l2_control_ids, c.id, "V4L2_CID_???");
		}
		if (exiting(tcp)) {
			if (!syserror(tcp)) {
				tprints(", type=");
				printxval(v4l2_control_types, c.type,
					  "V4L2_CTRL_TYPE_???");
				tprints(", name=");
				print_quoted_string((const char *) c.name,
						    sizeof(c.name),
						    QUOTE_0_TERMINATED);
				tprintf(", minimum=%i, maximum=%i, step=%i, "
					"default_value=%i, flags=",
					c.minimum, c.maximum,
					c.step, c.default_value);
				printflags(v4l2_control_flags, c.flags,
					   "V4L2_CTRL_FLAG_???");
			}
			tprints("}");
		}
		return 1;
	}

	case VIDIOC_G_CTRL:
	case VIDIOC_S_CTRL: {
		struct v4l2_control c;

		if (entering(tcp) || umove(tcp, arg, &c) < 0)
			return 0;
		tprints(", {id=");
		printxval(v4l2_control_ids, c.id, "V4L2_CID_???");
		if (!syserror(tcp) || code != VIDIOC_G_CTRL)
			tprintf(", value=%i", c.value);
		tprints("}");
		return 1;
	}

#ifdef VIDIOC_S_EXT_CTRLS
	case VIDIOC_S_EXT_CTRLS:
	case VIDIOC_TRY_EXT_CTRLS:
	case VIDIOC_G_EXT_CTRLS: {
		struct v4l2_ext_controls c;
		unsigned int n;
		bool must_print_values;

		if (entering(tcp) && code == VIDIOC_G_EXT_CTRLS)
			return 0;
		if (exiting(tcp) && syserror(tcp) && code != VIDIOC_G_EXT_CTRLS)
			return 0;
		must_print_values = ((entering(tcp) && code != VIDIOC_G_EXT_CTRLS)
				     || (exiting(tcp) && !syserror(tcp)));
		if (umove(tcp, arg, &c) < 0)
			return 0;
		tprints(code != VIDIOC_G_EXT_CTRLS && exiting(tcp) ? " => " : ", ");
		tprints("{ctrl_class=");
		printxval(v4l2_control_classes, c.ctrl_class,
			  "V4L2_CTRL_CLASS_???");
		tprintf(", count=%u", c.count);
		if (exiting(tcp) && syserror(tcp))
			tprintf(", error_idx=%u", c.error_idx);
		tprints(", controls=[");
		for (n = 0; n < c.count; ++n) {
			struct v4l2_ext_control ctrl;

			if (n > 0)
				tprints(", ");
			if (umove(tcp, (long) (c.controls + n), &ctrl) < 0)
				break;
			if (abbrev(tcp) && n == 2) {
				tprints("...");
				break;
			}
			tprints("{id=");
			printxval(v4l2_control_ids, ctrl.id, "V4L2_CID_???");
# if HAVE_DECL_V4L2_CTRL_TYPE_STRING
			tprintf(", size=%u", ctrl.size);
			if (ctrl.size > 0) {
				if (must_print_values) {
					tprints(", string=");
					printstr(tcp, (long) ctrl.string, ctrl.size);
				}
			} else
# endif
			{
				if (must_print_values) {
					tprintf(", value=%i, value64=%lld", ctrl.value,
						(long long) ctrl.value64);
				}
			}
			tprints("}");
		}
		tprints("]}");
		return 1;
	}
#endif /* VIDIOC_S_EXT_CTRLS */

	case VIDIOC_ENUMSTD: {
		struct v4l2_standard s;

		if (umove(tcp, arg, &s) < 0)
			return 0;
		if (entering(tcp))
			tprintf(", {index=%i", s.index);
		else {
			if (!syserror(tcp)) {
				tprints(", name=");
				print_quoted_string((const char *) s.name,
						    sizeof(s.name),
						    QUOTE_0_TERMINATED);
				tprintf(", frameperiod=" FMT_FRACT, ARGS_FRACT(s.frameperiod));
				tprintf(", framelines=%i", s.framelines);
			}
			tprints("}");
		}
		return 1;
	}

	case VIDIOC_G_STD:
	case VIDIOC_S_STD: {
		v4l2_std_id s;

		if (code == VIDIOC_G_STD && exiting(tcp) && syserror(tcp))
			return 0;
		if (umove(tcp, arg, &s) < 0)
			return 0;
		if ((code == VIDIOC_S_STD) == entering(tcp))
			tprintf(", std=%#llx", (unsigned long long) s);
		return 1;
	}

	case VIDIOC_ENUMINPUT: {
		struct v4l2_input i;

		if (entering(tcp) || umove(tcp, arg, &i) < 0)
			return 0;
		tprintf(", {index=%i", i.index);
		if (!syserror(tcp)) {
			tprints(", name=");
			print_quoted_string((const char *) i.name,
					    sizeof(i.name), QUOTE_0_TERMINATED);
			tprints(", type=");
			printxval(v4l2_input_types, i.type,
				  "V4L2_INPUT_TYPE_???");
		}
		tprints("}");
		return 1;
	}

	case VIDIOC_G_INPUT:
	case VIDIOC_S_INPUT: {
		int index;

		if (entering(tcp) || syserror(tcp) || umove(tcp, arg, &index) < 0)
			return 0;

		tprintf(", index=%i", index);
		return 1;
	}

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	case VIDIOC_ENUM_FRAMEINTERVALS: {
		struct v4l2_frmivalenum f;

		if (entering(tcp) || umove(tcp, arg, &f) < 0)
			return 0;
		tprintf(", {index=%i, pixel_format=", f.index);
		print_pixelformat(f.pixel_format);
		tprintf(", width=%u, height=%u", f.width, f.height);
		if (!syserror(tcp)) {
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
		return 1;
	}
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

	case VIDIOC_CROPCAP: {
		struct v4l2_cropcap c;

		if (entering(tcp) || umove(tcp, arg, &c) < 0)
			return 0;
		tprints(", type=");
		printxval(v4l2_buf_types, c.type, "V4L2_BUF_TYPE_???");
		if (syserror(tcp))
			return 1;
		tprintf(", bounds=" FMT_RECT ", defrect=" FMT_RECT ", "
			"pixelaspect=" FMT_FRACT, ARGS_RECT(c.bounds),
			ARGS_RECT(c.defrect), ARGS_FRACT(c.pixelaspect));
		return 1;
	}

	case VIDIOC_G_FBUF:
	case VIDIOC_S_FBUF: {
		struct v4l2_framebuffer b;

		if (syserror(tcp) && code == VIDIOC_G_FBUF)
			return 0;
		if (entering(tcp) || umove(tcp, arg, &b) < 0)
			return 0;
		tprintf(", {capability=%x, flags=%x, base=%p}",
			b.capability, b.flags, b.base);
		return 1;
	}

	case VIDIOC_REQBUFS: {
		struct v4l2_requestbuffers reqbufs;

		if (umove(tcp, arg, &reqbufs) < 0)
			return 0;
		if (entering(tcp)) {
			tprintf(", {count=%u, type=", reqbufs.count);
			printxval(v4l2_buf_types, reqbufs.type, "V4L2_BUF_TYPE_???");
			tprints(", memory=");
			printxval(v4l2_memories, reqbufs.memory, "V4L2_MEMORY_???");
			tprints("}");
			return 1;
		} else if (syserror(tcp))
			return 1;
		else {
			static char outstr[sizeof("{count=}") + sizeof(int) * 3];

			sprintf(outstr, "{count=%u}", reqbufs.count);
			tcp->auxstr = outstr;
			return 1 + RVAL_STR;
		}
	}

	case VIDIOC_QUERYBUF:
	case VIDIOC_QBUF:
	case VIDIOC_DQBUF: {
		struct v4l2_buffer b;

		if (umove(tcp, arg, &b) < 0)
			return 0;
		if (entering(tcp)) {
			tprints(", {type=");
			printxval(v4l2_buf_types, b.type, "V4L2_BUF_TYPE_???");
			if (code != VIDIOC_DQBUF)
				tprintf(", index=%u", b.index);
		} else {
			if (!syserror(tcp)) {
				if (code == VIDIOC_DQBUF)
					tprintf(", index=%u", b.index);
				tprints(", memory=");
				printxval(v4l2_memories, b.memory, "V4L2_MEMORY_???");

				if (b.memory == V4L2_MEMORY_MMAP) {
					tprintf(", m.offset=%#x", b.m.offset);
				} else if (b.memory == V4L2_MEMORY_USERPTR) {
					tprintf(", m.userptr=%#lx", b.m.userptr);
				}

				tprintf(", length=%u, bytesused=%u, flags=",
					b.length, b.bytesused);
				printflags(v4l2_buf_flags, b.flags, "V4L2_BUF_FLAG_???");
				if (code == VIDIOC_DQBUF)
					tprintf(", timestamp = {%ju.%06ju}",
						(uintmax_t)b.timestamp.tv_sec,
						(uintmax_t)b.timestamp.tv_usec);
				tprints(", ...");
			}
			tprints("}");
		}
		return 1;
	}

	case VIDIOC_STREAMON:
	case VIDIOC_STREAMOFF: {
		int type;

		if (umove(tcp, arg, &type) < 0)
			return 0;
		if (entering(tcp)) {
			tprints(", ");
			printxval(v4l2_buf_types, type, "V4L2_BUF_TYPE_???");
		}
		return 1;
	}

	default: /* decode on exit */
		return 0;
	}
}
