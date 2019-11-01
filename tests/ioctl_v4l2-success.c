/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/types.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

static bool
fill_fmt(struct v4l2_format *f)
{
	static struct v4l2_clip *clips;

	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		f->fmt.pix.width        = 0xdeadc0de;
		f->fmt.pix.height       = 0xfeedbeef;
		f->fmt.pix.pixelformat  = 0xb5315258; /* forurcc_be("XR15") */
		f->fmt.pix.field = f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE
			? V4L2_FIELD_ALTERNATE : 0xdec0ded1;
		f->fmt.pix.bytesperline = 0xbadc0ded;
		f->fmt.pix.sizeimage    = 0xface1e55;
		f->fmt.pix.colorspace   = V4L2_COLORSPACE_REC709;
		break;

	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
		f->fmt.win.w.left    = 0xa0a1a2a3;
		f->fmt.win.w.top     = 0xb0b1b2b3;
		f->fmt.win.w.width   = 0xc0c1c2c3;
		f->fmt.win.w.height  = 0xd0d1d2d3;
		f->fmt.win.field     = f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY
			? V4L2_FIELD_ANY : 10;
		f->fmt.win.chromakey = 0xbeefface;

		if (!clips)
			clips = tail_alloc(sizeof(*clips) * 3);
		f->fmt.win.clips = clips;

		f->fmt.win.clips[0].c.left   = 0xa4a5a6a7;
		f->fmt.win.clips[0].c.top    = 0xb4b5b6b7;
		f->fmt.win.clips[0].c.width  = 0xc4c5c6c7;
		f->fmt.win.clips[0].c.height = 0xd4d5d6d7;
		f->fmt.win.clips[0].next     = clips;

		f->fmt.win.clips[1].c.left   = 0xa8a9aaab;
		f->fmt.win.clips[1].c.top    = 0xb8b9babb;
		f->fmt.win.clips[1].c.width  = 0xc8c9cacb;
		f->fmt.win.clips[1].c.height = 0xd8d9dadb;

		f->fmt.win.clips[2].c.left   = 0xacadaeaf;
		f->fmt.win.clips[2].c.top    = 0xbcbdbebf;
		f->fmt.win.clips[2].c.width  = 0xcccdcecf;
		f->fmt.win.clips[2].c.height = 0xdcdddedf;
		f->fmt.win.clips[2].next     = clips + 1;

		f->fmt.win.clipcount = f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY
			? 4 : 0;
		f->fmt.win.bitmap    = f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY
			? NULL : clips;
		break;

	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		f->fmt.vbi.sampling_rate    = 0xdecaffed;
		f->fmt.vbi.offset           = 0xcafefeed;
		f->fmt.vbi.samples_per_line = 0xbeefaced;
		f->fmt.vbi.sample_format    = V4L2_PIX_FMT_RGB555X;

		f->fmt.vbi.start[0] = 0xdec0ded0;
		f->fmt.vbi.start[1] = 0xdec0ded1;
		f->fmt.vbi.count[0] = 0xacceded2;
		f->fmt.vbi.count[1] = 0xacceded3;

		f->fmt.vbi.flags = f->type == V4L2_BUF_TYPE_VBI_CAPTURE
			? 0x3 : 0x1ce50d1c;
		break;

#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		f->fmt.pix_mp.width        = 0xdeaffade;
		f->fmt.pix_mp.height       = 0xfadeb1a5;
		f->fmt.pix_mp.pixelformat  = 0x36314454;
		f->fmt.pix_mp.field        = V4L2_FIELD_NONE;
		f->fmt.pix_mp.colorspace   = 13;

		for (size_t i = 0; i < VIDEO_MAX_PLANES; i++) {
			f->fmt.pix_mp.plane_fmt[i].sizeimage = 0xd0decad0 ^ i;
			if (sizeof(f->fmt.pix_mp.plane_fmt[i].bytesperline) ==
			    sizeof(uint32_t)) {
				f->fmt.pix_mp.plane_fmt[i].bytesperline
					= 0xd0decad1 ^ i;
			} else {
#if WORDS_BIGENDIAN
				f->fmt.pix_mp.plane_fmt[i].bytesperline
					= 0xd0de;
				f->fmt.pix_mp.plane_fmt[i].reserved[0]
					= 0xcad1 ^ i;
#else
				f->fmt.pix_mp.plane_fmt[i].bytesperline
					= 0xcad1 ^ i;
				f->fmt.pix_mp.plane_fmt[i].reserved[0]
					= 0xd0de;
#endif
			}
		}

		f->fmt.pix_mp.num_planes   = f->type ==
			V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ? 0xd5 : 0;
		break;
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		f->fmt.sliced.service_set = 0xfeed;
		for (size_t i = 0; i < 2; i++) {
			for (size_t j = 0; j < 24; j++) {
				f->fmt.sliced.service_lines[i][j] =
					0xdead ^ (i << 8) ^ j;
			}
		}
		f->fmt.sliced.io_size = 0xdefaceed;
		break;
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	case V4L2_BUF_TYPE_SDR_CAPTURE:
# if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	case V4L2_BUF_TYPE_SDR_OUTPUT:
# endif
		f->fmt.sdr.pixelformat = V4L2_SDR_FMT_CU8;
# ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		if (sizeof(f->fmt.sdr.buffersize == sizeof(uint32_t)))
			f->fmt.sdr.buffersize = 0xbadc0ded;
		else
			((uint32_t *) &f->fmt.sdr)[1] = 0xbadc0ded;
# else
		((uint32_t *) &f->fmt.sdr)[1] = 0xbadc0ded;
# endif
		break;
#endif
	default:
		return false;
	}

	return true;
}

static void
print_fmt(const char *pfx, struct v4l2_format *f)
{
	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		printf("%sfmt.pix={width=3735929054, height=4276993775"
		       ", pixelformat=" RAW("0xb5315258")
		       NRAW("v4l2_fourcc('X', 'R', '1', '\\xb5')"
		            " /* V4L2_PIX_FMT_XRGB555X */")
		       ", field=%s, bytesperline=3134983661"
		       ", sizeimage=4207812181, colorspace="
		       XLAT_KNOWN(0x3, "V4L2_COLORSPACE_REC709") "}",
		       pfx, f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE
			? XLAT_STR(V4L2_FIELD_ALTERNATE)
			: XLAT_UNKNOWN(0xdec0ded1, "V4L2_FIELD_???"));
		break;

	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
		printf("%sfmt.win={left=-1600019805, top=-1330531661"
		       ", width=3233923779, height=3503411923, field=%s"
		       ", chromakey=0xbeefface, clips=[",
		       pfx, f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY
			? XLAT_STR(V4L2_FIELD_ANY)
			: XLAT_UNKNOWN(0xa, "V4L2_FIELD_???"));
		if (f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY) {
			printf("{left=-1532647769, top=-1263159625"
			       ", width=3301295815, height=3570783959}, "
			       "{left=-1465275733, top=-1195787589"
			       ", width=3368667851, height=3638155995}, "
			       "{left=-1397903697, top=-1128415553"
			       ", width=3436039887, height=3705528031}, "
			       "... /* %p */", f->fmt.win.clips + 3);
		}
		printf("], clipcount=%d, bitmap=",
		       f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY ? 4 : 0);

		if (f->type == V4L2_BUF_TYPE_VIDEO_OVERLAY)
			printf("NULL");
		else
			printf("%p", f->fmt.win.bitmap);

#ifdef HAVE_STRUCT_V4L2_WINDOW_GLOBAL_ALPHA
		printf(", global_alpha=%#hhx}", f->fmt.win.global_alpha);
#else
		struct win_ga {
			struct v4l2_rect w;
			uint32_t field;
			uint32_t chromakey;
			struct v4l2_clip *clips;
			uint32_t clipcount;
			void *bitmap;
			uint8_t global_alpha;
		};
		printf(", global_alpha=%#hhx}",
		       ((struct win_ga *) &f->fmt.win)->global_alpha);
#endif
		break;

	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		printf("%sfmt.vbi={sampling_rate=3737845741, offset=3405709037"
		       ", samples_per_line=3203378413, sample_format="
		       RAW("0x51424752") NRAW("v4l2_fourcc('R', 'G', 'B', 'Q')"
		       " /* V4L2_PIX_FMT_RGB555X */")
		       ", start=[-557785392, -557785391]"
		       ", count=[2899238610, 2899238611], flags=%s}",
		       pfx, f->type == V4L2_BUF_TYPE_VBI_CAPTURE
			? XLAT_KNOWN(0x3, "V4L2_VBI_UNSYNC|V4L2_VBI_INTERLACED")
			: XLAT_UNKNOWN(0x1ce50d1c, "V4L2_VBI_???"));
		break;

#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		printf("%sfmt.pix_mp={width=3736074974, height=4208898469"
		       ", pixelformat=" RAW("0x36314454")
		       NRAW("v4l2_fourcc('T', 'D', '1', '6')")
		       ", field=%s, colorspace=0xd"
		       NRAW(" /* V4L2_COLORSPACE_??? */") ", plane_fmt=[",
		       pfx, XLAT_STR(V4L2_FIELD_NONE));
		if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			printf("{sizeimage=3504261840, bytesperline=3504261841}"
			", "
			       "{sizeimage=3504261841, bytesperline=3504261840}"
			       ", "
			       "{sizeimage=3504261842, bytesperline=3504261843}"
			       ", "
			       "{sizeimage=3504261843, bytesperline=3504261842}"
			       ", "
			       "{sizeimage=3504261844, bytesperline=3504261845}"
			       ", "
			       "{sizeimage=3504261845, bytesperline=3504261844}"
			       ", "
			       "{sizeimage=3504261846, bytesperline=3504261847}"
			       ", "
			       "{sizeimage=3504261847, bytesperline=3504261846}"
			       "], num_planes=213}");
		} else {
			printf("], num_planes=0}");
		}
		break;
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		printf("%sfmt.sliced={service_set="
		       XLAT_UNKNOWN(0xfeed, "V4L2_SLICED_???")
		       ", io_size=3740978925, service_lines=[[0xdead, 0xdeac"
		       ", 0xdeaf, 0xdeae, 0xdea9, 0xdea8, 0xdeab, 0xdeaa"
		       ", 0xdea5, 0xdea4, 0xdea7, 0xdea6, 0xdea1, 0xdea0"
		       ", 0xdea3, 0xdea2, 0xdebd, 0xdebc, 0xdebf, 0xdebe"
		       ", 0xdeb9, 0xdeb8, 0xdebb, 0xdeba], [0xdfad, 0xdfac"
		       ", 0xdfaf, 0xdfae, 0xdfa9, 0xdfa8, 0xdfab, 0xdfaa"
		       ", 0xdfa5, 0xdfa4, 0xdfa7, 0xdfa6, 0xdfa1, 0xdfa0"
		       ", 0xdfa3, 0xdfa2, 0xdfbd, 0xdfbc, 0xdfbf, 0xdfbe"
		       ", 0xdfb9, 0xdfb8, 0xdfbb, 0xdfba]]}",
		       pfx);
		break;
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	case V4L2_BUF_TYPE_SDR_CAPTURE:
# if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	case V4L2_BUF_TYPE_SDR_OUTPUT:
# endif
		printf("%sfmt.sdr={pixelformat=" RAW("0x38305543")
		       NRAW("v4l2_fourcc('C', 'U', '0', '8')"
			    " /* V4L2_SDR_FMT_CU8 */")
		       ", buffersize=3134983661}",
		       pfx);
		break;
#endif
	}
}

int
main(int argc, char **argv)
{
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

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
		long rc = ioctl(-1, VIDIOC_QUERYCAP, NULL);
		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(VIDIOC_QUERYCAP), sprintrc(rc),
		       rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", VIDIOC_QUERYCAP, NULL) returning %lu",
				   inject_retval);


	/* VIDIOC_QUERYCAP */
	struct v4l2_capability *caps = tail_alloc(sizeof(*caps));

	fill_memory(caps, sizeof(*caps));
	caps->capabilities = 0xdeadbeef;
#ifdef HAVE_STRUCT_V4L2_CAPABILITY_DEVICE_CAPS
	caps->device_caps = 0xfacefeed;
#else
	caps->reserved[0] = 0xfacefeed;
#endif

	ioctl(-1, VIDIOC_QUERYCAP, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCAP), inject_retval);

	ioctl(-1, VIDIOC_QUERYCAP, (char *) caps + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCAP), (char *) caps + 1, inject_retval);

	ioctl(-1, VIDIOC_QUERYCAP, caps);
	printf("ioctl(-1, %s, {driver=", XLAT_STR(VIDIOC_QUERYCAP));
	print_quoted_cstring((char *) caps->driver, sizeof(caps->driver));
	printf(", card=");
	print_quoted_cstring((char *) caps->card, sizeof(caps->card));
	printf(", bus_info=");
	print_quoted_cstring((char *) caps->bus_info, sizeof(caps->bus_info));
	printf(", version="
#ifdef WORDS_BIGENDIAN
	       XLAT_KNOWN(0xd0d1d2d3, "KERNEL_VERSION(53457, 210, 211)")
#else
	       XLAT_KNOWN(0xd3d2d1d0, "KERNEL_VERSION(54226, 209, 208)")
#endif
	       ", capabilities=" XLAT_KNOWN(0xdeadbeef,
	       "V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_VIDEO_OUTPUT"
	       "|V4L2_CAP_VIDEO_OVERLAY|V4L2_CAP_VBI_OUTPUT"
	       "|V4L2_CAP_SLICED_VBI_CAPTURE|V4L2_CAP_SLICED_VBI_OUTPUT"
	       "|V4L2_CAP_VIDEO_OUTPUT_OVERLAY|V4L2_CAP_HW_FREQ_SEEK"
	       "|V4L2_CAP_RDS_OUTPUT|V4L2_CAP_VIDEO_CAPTURE_MPLANE"
	       "|V4L2_CAP_VIDEO_OUTPUT_MPLANE|V4L2_CAP_VIDEO_M2M"
	       "|V4L2_CAP_TUNER|V4L2_CAP_RADIO|V4L2_CAP_MODULATOR"
	       "|V4L2_CAP_EXT_PIX_FORMAT|V4L2_CAP_META_CAPTURE|V4L2_CAP_ASYNCIO"
	       "|V4L2_CAP_STREAMING|V4L2_CAP_META_OUTPUT|V4L2_CAP_TOUCH"
	       "|V4L2_CAP_DEVICE_CAPS|0x40000008"));
	printf(", device_caps=" XLAT_KNOWN(0xfacefeed,
	       "V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_VIDEO_OVERLAY"
	       "|V4L2_CAP_VBI_OUTPUT|V4L2_CAP_SLICED_VBI_CAPTURE"
	       "|V4L2_CAP_SLICED_VBI_OUTPUT|V4L2_CAP_VIDEO_OUTPUT_OVERLAY"
	       "|V4L2_CAP_HW_FREQ_SEEK|V4L2_CAP_RDS_OUTPUT"
	       "|V4L2_CAP_VIDEO_CAPTURE_MPLANE|V4L2_CAP_VIDEO_OUTPUT_MPLANE"
	       "|V4L2_CAP_VIDEO_M2M_MPLANE|V4L2_CAP_VIDEO_M2M|V4L2_CAP_AUDIO"
	       "|V4L2_CAP_RADIO|V4L2_CAP_MODULATOR|V4L2_CAP_SDR_OUTPUT"
	       "|V4L2_CAP_META_CAPTURE|V4L2_CAP_ASYNCIO|V4L2_CAP_META_OUTPUT"
	       "|V4L2_CAP_TOUCH|V4L2_CAP_DEVICE_CAPS|0x60000008"));
	printf("}) = %ld (INJECTED)\n", inject_retval);


	/* VIDIOC_ENUM_FMT */
	static const struct strval32 buf_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_BUF_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_BUF_TYPE_VIDEO_CAPTURE") },
		{ ARG_XLAT_KNOWN(0x2, "V4L2_BUF_TYPE_VIDEO_OUTPUT") },
		{ ARG_XLAT_KNOWN(0x3, "V4L2_BUF_TYPE_VIDEO_OVERLAY") },
		{ ARG_XLAT_KNOWN(0x4, "V4L2_BUF_TYPE_VBI_CAPTURE") },
		{ ARG_XLAT_KNOWN(0x5, "V4L2_BUF_TYPE_VBI_OUTPUT") },
		{ ARG_XLAT_KNOWN(0x6, "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE") },
		{ ARG_XLAT_KNOWN(0x7, "V4L2_BUF_TYPE_SLICED_VBI_OUTPUT") },
		{ ARG_XLAT_KNOWN(0x8, "V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY") },
		{ ARG_XLAT_KNOWN(0x9, "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE") },
		{ ARG_XLAT_KNOWN(0xa, "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE") },
		{ ARG_XLAT_KNOWN(0xb, "V4L2_BUF_TYPE_SDR_CAPTURE") },
		{ ARG_XLAT_KNOWN(0xc, "V4L2_BUF_TYPE_SDR_OUTPUT") },
		{ ARG_XLAT_KNOWN(0xd, "V4L2_BUF_TYPE_META_CAPTURE") },
		{ ARG_XLAT_KNOWN(0xe, "V4L2_BUF_TYPE_META_OUTPUT") },
		{ ARG_XLAT_UNKNOWN(0xf, "V4L2_BUF_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0x80, "V4L2_BUF_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xbadc0ded, "V4L2_BUF_TYPE_???") },
	};
	static const struct strval32 fmtdesc_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_FMT_FLAG_COMPRESSED") },
		{ ARG_XLAT_KNOWN(0x1e, "V4L2_FMT_FLAG_EMULATED"
				       "|V4L2_FMT_FLAG_CONTINUOUS_BYTESTREAM"
				       "|V4L2_FMT_FLAG_DYN_RESOLUTION|0x10") },
		{ ARG_XLAT_UNKNOWN(0xdead0000, "V4L2_FMT_FLAG_???") },
	};
	static const struct strval32 fmtdesc_fmts[] = {
		{ 0x4c47504a, RAW("0x4c47504a")
			      NRAW("v4l2_fourcc('J', 'P', 'G', 'L')"
			           " /* V4L2_PIX_FMT_JPGL */") },
		{ 0xbadc0ded, RAW("0xbadc0ded")
			      NRAW("v4l2_fourcc('\\xed', '\\x0d', '\\xdc',"
			           " '\\xba')") },
	};
	struct v4l2_fmtdesc *fmtdesc = tail_alloc(sizeof(*fmtdesc));

	fill_memory(fmtdesc, sizeof(*fmtdesc));
	fmtdesc->index = 0xdeac0de;

	ioctl(-1, VIDIOC_ENUM_FMT, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FMT), inject_retval);

	ioctl(-1, VIDIOC_ENUM_FMT, (char *) fmtdesc + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FMT), (char *) fmtdesc + 1, inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(buf_types); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(fmtdesc_flags); j++) {
			for (size_t k = 0; k < ARRAY_SIZE(fmtdesc_fmts); k++) {
				fmtdesc->type = buf_types[i].val;
				fmtdesc->flags = fmtdesc_flags[j].val;
				fmtdesc->pixelformat = fmtdesc_fmts[k].val;

				ioctl(-1, VIDIOC_ENUM_FMT, fmtdesc);
				printf("ioctl(-1, %s, {index=233488606, type=%s"
				       ", flags=%s, description=",
				       XLAT_STR(VIDIOC_ENUM_FMT),
				       buf_types[i].str,
				       fmtdesc_flags[j].str);
				print_quoted_cstring((char *) fmtdesc->description,
					sizeof(fmtdesc->description));
				printf(", pixelformat=%s}) = %ld (INJECTED)\n",
				       fmtdesc_fmts[k].str, inject_retval);

				fill_memory_ex(fmtdesc->description,
					       sizeof(fmtdesc->description),
					       (i * 9 + j) * 7 + k,
					       (k * 3 + j) * 11 + i + 5);
			}
		}
	}


	/* VIDIOC_REQBUFS */
	static const struct strval32 reqb_mems[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_MEMORY_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_MEMORY_MMAP") },
		{ ARG_XLAT_KNOWN(0x4, "V4L2_MEMORY_DMABUF") },
		{ ARG_XLAT_UNKNOWN(0x5, "V4L2_MEMORY_???") },
		{ ARG_XLAT_UNKNOWN(0xbadc0ded, "V4L2_MEMORY_???") },
	};
	struct v4l2_requestbuffers *reqb = tail_alloc(sizeof(*reqb));

	fill_memory(reqb, sizeof(*reqb));
	reqb->count = 0xfeedface;

	ioctl(-1, VIDIOC_REQBUFS, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_REQBUFS), inject_retval);

	ioctl(-1, VIDIOC_REQBUFS, (char *) reqb + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_REQBUFS), (char *) reqb + 1, inject_retval);

	for (size_t i = 0; i < MAX(ARRAY_SIZE(buf_types),
				   ARRAY_SIZE(reqb_mems)); i++) {
		reqb->type = buf_types[i % ARRAY_SIZE(buf_types)].val;
		reqb->memory = reqb_mems[i % ARRAY_SIZE(reqb_mems)].val;

		ioctl(-1, VIDIOC_REQBUFS, reqb);
		printf("ioctl(-1, %s, {type=%s, memory=%s"
		       ", count=4277009102 => 4277009102}) = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_REQBUFS),
		       buf_types[i % ARRAY_SIZE(buf_types)].str,
		       reqb_mems[i % ARRAY_SIZE(reqb_mems)].str,
		       inject_retval);
	}


	/* VIDIOC_G_FMT, VIDIOC_S_FMT, VIDIOC_TRY_FMT */
	static const struct strval32 fmt_cmds[] = {
		{ ARG_STR(VIDIOC_G_FMT) },
		{ ARG_STR(VIDIOC_S_FMT) },
		{ ARG_STR(VIDIOC_TRY_FMT) },
	};

	struct v4l2_format *fmt = tail_alloc(sizeof(*fmt));

	for (size_t i = 0; i < ARRAY_SIZE(fmt_cmds); i++) {
		ioctl(-1, fmt_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(fmt_cmds[i].str, fmt_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, fmt_cmds[i].val, (char *) fmt + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(fmt_cmds[i].str, fmt_cmds[i].val, NULL),
		       (char *) fmt + 1, inject_retval);

		for (size_t j = 0; j < ARRAY_SIZE(buf_types); j++) {
			fill_memory(fmt, sizeof(*fmt));

			fmt->type = buf_types[j].val;
			if (!fill_fmt(fmt))
				continue;

			ioctl(-1, fmt_cmds[i].val, fmt);
			printf("ioctl(-1, %s, {type=%s",
			       sprintxlat(fmt_cmds[i].str, fmt_cmds[i].val,
					  NULL),
			       buf_types[j].str);
			print_fmt(", ", fmt);
			if (fmt_cmds[i].val != VIDIOC_G_FMT &&
			    buf_types[j].val != V4L2_BUF_TYPE_VIDEO_OVERLAY &&
			    buf_types[j].val != 8)
				print_fmt("} => {", fmt);
			printf("}) = %ld (INJECTED)\n", inject_retval);
		}
	}


	/* VIDIOC_QUERYBUF, VIDIOC_QBUF, VIDIOC_DQBUF */
	static const struct strval32 buf_cmds[] = {
		{ ARG_STR(VIDIOC_QUERYBUF) },
		{ ARG_STR(VIDIOC_QBUF) },
		{ ARG_STR(VIDIOC_DQBUF) },
	};

	struct v4l2_buffer *buf = tail_alloc(sizeof(*buf));

	for (size_t i = 0; i < ARRAY_SIZE(buf_cmds); i++) {
		ioctl(-1, buf_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, buf_cmds[i].val, (char *) buf + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       (char *) buf + 1, inject_retval);

		fill_memory(buf, sizeof(*buf));
		buf->index     = 0xdeadc0de;
		buf->type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->bytesused = 0xdecaffee;
		buf->flags     = 0x1ff; /* TODO: update */
		buf->field     = V4L2_FIELD_TOP;

		buf->timestamp.tv_sec  = 0x1e55c0de;
		buf->timestamp.tv_usec = 999999;

		buf->timecode.type = V4L2_TC_TYPE_24FPS;
		buf->timecode.flags = 0xbeefdeaf;

		buf->memory    = V4L2_MEMORY_MMAP;
		buf->m.offset  = 0xfacefeed;
		buf->length    = 0xcafebed5;
		buf->reserved  = 0xdeefaced;

		ioctl(-1, buf_cmds[i].val, buf);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x1, "V4L2_BUF_TYPE_VIDEO_CAPTURE")
		       ", index=3735929054, memory="
		       XLAT_KNOWN(0x1, "V4L2_MEMORY_MMAP")
		       ", m.offset=0xfacefeed, length=3405692629"
		       ", bytesused=3737845742, flags=" RAW("0x1ff")
#if !XLAT_RAW
		       XLAT_KNOWN(0x1ff, "V4L2_BUF_FLAG_MAPPED"
		       "|V4L2_BUF_FLAG_QUEUED|V4L2_BUF_FLAG_DONE"
		       "|V4L2_BUF_FLAG_KEYFRAME|V4L2_BUF_FLAG_PFRAME"
		       "|V4L2_BUF_FLAG_BFRAME|V4L2_BUF_FLAG_ERROR"
		       "|V4L2_BUF_FLAG_IN_REQUEST|V4L2_BUF_FLAG_TIMECODE") "|"
		       XLAT_KNOWN(0, "V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN") "|"
		       XLAT_KNOWN(0, "V4L2_BUF_FLAG_TSTAMP_SRC_EOF")
#endif
		       "%s, ...}) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       buf_cmds[i].val == VIDIOC_DQBUF
			? ", timestamp={tv_sec=508936414, tv_usec=999999}" : "",
		       inject_retval);

		buf->type      = V4L2_BUF_TYPE_VBI_CAPTURE;
		buf->flags     = 0x268040;
		buf->field     = 0xb;
		buf->memory    = V4L2_MEMORY_USERPTR;
		buf->m.userptr = (long) 0xdefaced0dec0ded1LL;

		ioctl(-1, buf_cmds[i].val, buf);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x4, "V4L2_BUF_TYPE_VBI_CAPTURE")
		       ", index=3735929054, memory="
		       XLAT_KNOWN(0x2, "V4L2_MEMORY_USERPTR")
		       ", m.userptr=%p, length=3405692629"
		       ", bytesused=3737845742, flags=" RAW("0x268040")
#if !XLAT_RAW
		       XLAT_KNOWN(0x200040, "V4L2_BUF_FLAG_ERROR|0x200000") "|"
		       XLAT_UNKNOWN(0x8000, "V4L2_BUF_FLAG_TIMESTAMP_???") "|"
		       XLAT_UNKNOWN(0x60000, "V4L2_BUF_FLAG_TSTAMP_SRC_???")
#endif
		       "%s, ...}) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       (void *) (intptr_t) 0xdefaced0dec0ded1LL,
		       buf_cmds[i].val == VIDIOC_DQBUF
			? ", timestamp={tv_sec=508936414, tv_usec=999999}" : "",
		       inject_retval);

		buf->type      = 0x9;
		buf->flags     = 0;

		ioctl(-1, buf_cmds[i].val, buf);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x9, "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE")
		       ", index=3735929054, memory="
		       XLAT_KNOWN(0x2, "V4L2_MEMORY_USERPTR")
		       ", m.userptr=%p, length=3405692629"
		       ", bytesused=3737845742, flags=" RAW("0")
#if !XLAT_RAW
		       XLAT_KNOWN(0, "V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN") "|"
		       XLAT_KNOWN(0, "V4L2_BUF_FLAG_TSTAMP_SRC_EOF")
#endif
		       "%s, ...}) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       (void *) (intptr_t) 0xdefaced0dec0ded1LL,
		       buf_cmds[i].val == VIDIOC_DQBUF
			? ", timestamp={tv_sec=508936414, tv_usec=999999}" : "",
		       inject_retval);

		buf->type      = 0xa;
		buf->memory    = V4L2_MEMORY_OVERLAY;
		buf->flags     = 0x2000;

		ioctl(-1, buf_cmds[i].val, buf);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0xa, "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE")
		       ", index=3735929054, memory="
		       XLAT_KNOWN(0x3, "V4L2_MEMORY_OVERLAY")
		       ", length=3405692629, bytesused=3737845742"
		       ", flags=" RAW("0x2000")
#if !XLAT_RAW
		       XLAT_KNOWN(0x2000, "V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC")
		       "|" XLAT_KNOWN(0, "V4L2_BUF_FLAG_TSTAMP_SRC_EOF")
#endif
		       "%s, ...}) = %ld (INJECTED)\n",
		       sprintxlat(buf_cmds[i].str, buf_cmds[i].val, NULL),
		       buf_cmds[i].val == VIDIOC_DQBUF
			? ", timestamp={tv_sec=508936414, tv_usec=999999}" : "",
		       inject_retval);
	}


	/* VIDIOC_G_FBUF, VIDIOC_S_FBUF */
	static const struct strval32 fbuf_cmds[] = {
		{ ARG_STR(VIDIOC_G_FBUF) },
		{ ARG_STR(VIDIOC_S_FBUF) },
	};

	struct v4l2_framebuffer *fbuf = tail_alloc(sizeof(*fbuf));

	for (size_t i = 0; i < ARRAY_SIZE(fbuf_cmds); i++) {
		ioctl(-1, fbuf_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(fbuf_cmds[i].str, fbuf_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, fbuf_cmds[i].val, (char *) fbuf + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(fbuf_cmds[i].str, fbuf_cmds[i].val, NULL),
		       (char *) fbuf + 1, inject_retval);

		fill_memory32(fbuf, sizeof(*fbuf));
		fbuf->base = NULL;

		ioctl(-1, fbuf_cmds[i].val, fbuf);
		printf("ioctl(-1, %s, {capability=0x80a0c0e0, flags=0x80a0c0e1"
		       ", base=NULL}) = %ld (INJECTED)\n",
		       sprintxlat(fbuf_cmds[i].str, fbuf_cmds[i].val, NULL),
		       inject_retval);
	}


	/* VIDIOC_G_PARM, VIDIOC_S_PARM */
	static const struct strval32 sparm_cmds[] = {
		{ ARG_STR(VIDIOC_G_PARM) },
		{ ARG_STR(VIDIOC_S_PARM) },
	};

	struct v4l2_streamparm *sparm = tail_alloc(sizeof(*sparm));

	for (size_t i = 0; i < ARRAY_SIZE(sparm_cmds); i++) {
		ioctl(-1, sparm_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, sparm_cmds[i].val, (char *) sparm + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       (char *) sparm + 1, inject_retval);

		fill_memory32(sparm, sizeof(*sparm));

		ioctl(-1, sparm_cmds[i].val, sparm);
		printf("ioctl(-1, %s, {type="
		       XLAT_UNKNOWN(0x80a0c0e0, "V4L2_BUF_TYPE_???")
		       "}) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       inject_retval);

		sparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		static const char *parm_str1 = "parm.capture={capability="
			XLAT_UNKNOWN(0x80a0c0e1, "V4L2_CAP_???")
			", capturemode="
			XLAT_UNKNOWN(0x80a0c0e2, "V4L2_MODE_???")
			", timeperframe=2158018787/2158018788"
			", extendedmode=0x80a0c0e5, readbuffers=2158018790}";

		ioctl(-1, sparm_cmds[i].val, sparm);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x1, "V4L2_BUF_TYPE_VIDEO_CAPTURE")
		       ", %s}%s%s%s) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       parm_str1,
		       sparm_cmds[i].val == VIDIOC_S_PARM ? " => {" : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? parm_str1 : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? "}" : "",
		       inject_retval);

		sparm->parm.capture.capability = 0x1000;
		sparm->parm.capture.capturemode = 0x1;

		static const char *parm_str2 = "parm.capture={capability="
			XLAT_KNOWN(0x1000, "V4L2_CAP_TIMEPERFRAME")
			", capturemode="
			XLAT_KNOWN(0x1, "V4L2_MODE_HIGHQUALITY")
			", timeperframe=2158018787/2158018788"
			", extendedmode=0x80a0c0e5, readbuffers=2158018790}";

		ioctl(-1, sparm_cmds[i].val, sparm);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x1, "V4L2_BUF_TYPE_VIDEO_CAPTURE")
		       ", %s}%s%s%s) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       parm_str2,
		       sparm_cmds[i].val == VIDIOC_S_PARM ? " => {" : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? parm_str2 : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? "}" : "",
		       inject_retval);

		sparm->type = 0x9;
		sparm->parm.capture.capability = 0xdeadbeef;
		sparm->parm.capture.capturemode = 0xadec0ded;

		ioctl(-1, sparm_cmds[i].val, sparm);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x9, "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE")
		       "}) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       inject_retval);

		fill_memory32(sparm, sizeof(*sparm));
		sparm->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

		static const char *parm_str3 = "parm.output={capability="
			XLAT_UNKNOWN(0x80a0c0e1, "V4L2_CAP_???")
			", outputmode="
			XLAT_UNKNOWN(0x80a0c0e2, "V4L2_MODE_???")
			", timeperframe=2158018787/2158018788"
			", extendedmode=0x80a0c0e5, writebuffers=2158018790}";

		ioctl(-1, sparm_cmds[i].val, sparm);
		printf("ioctl(-1, %s, {type="
		       XLAT_KNOWN(0x2, "V4L2_BUF_TYPE_VIDEO_OUTPUT")
		       ", %s}%s%s%s) = %ld (INJECTED)\n",
		       sprintxlat(sparm_cmds[i].str, sparm_cmds[i].val, NULL),
		       parm_str3,
		       sparm_cmds[i].val == VIDIOC_S_PARM ? " => {" : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? parm_str3 : "",
		       sparm_cmds[i].val == VIDIOC_S_PARM ? "}" : "",
		       inject_retval);

	}


	/* VIDIOC_G_STD, VIDIOC_S_STD */
	static const struct strval32 stdid_cmds[] = {
		{ ARG_STR(VIDIOC_G_STD) },
		{ ARG_STR(VIDIOC_S_STD) },
	};
	static const struct strval64 stdids[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x1) },
		{ ARG_STR(0xb000) },
		{ ARG_STR(0x3ffffff) },
		{ ARG_STR(0xffffffff) },
		{ ARG_ULL_STR(0xbadc0deddeadface) },
	};

	v4l2_std_id *stdid = tail_alloc(sizeof(*stdid));

	for (size_t i = 0; i < ARRAY_SIZE(stdid_cmds); i++) {
		ioctl(-1, stdid_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(stdid_cmds[i].str, stdid_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, stdid_cmds[i].val, (char *) stdid + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(stdid_cmds[i].str, stdid_cmds[i].val, NULL),
		       (char *) stdid + 1, inject_retval);

		for (size_t j = 0; j < ARRAY_SIZE(stdids); j++) {
			*stdid = stdids[j].val;

			ioctl(-1, stdid_cmds[i].val, stdid);
			printf("ioctl(-1, %s, [%s]) = %ld (INJECTED)\n",
			       sprintxlat(stdid_cmds[i].str, stdid_cmds[i].val, NULL),
			       stdids[j].str, inject_retval);

		}
	}


	/* VIDIOC_ENUMSTD */
	struct v4l2_standard *std = tail_alloc(sizeof(*std));

	ioctl(-1, VIDIOC_ENUMSTD, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUMSTD), inject_retval);

	ioctl(-1, VIDIOC_ENUMSTD, (char *) std + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUMSTD), (char *) std + 1, inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(stdids); i++) {
		fill_memory32(&std->frameperiod, sizeof(*std) -
			      offsetof(struct v4l2_standard, frameperiod));
		fill_memory_ex(std->name, sizeof(std->name), i * 47 + 1, 255);
		std->index = 0xdeadface;
		std->id = stdids[i].val;

		ioctl(-1, VIDIOC_ENUMSTD, std);
		printf("ioctl(-1, %s, {index=3735943886, name=",
		       XLAT_STR(VIDIOC_ENUMSTD));
		print_quoted_cstring((char *) std->name, sizeof(std->name));
		printf(", frameperiod=2158018784/2158018785"
		       ", framelines=2158018786}) = %ld (INJECTED)\n",
		       inject_retval);
	}


	/* VIDIOC_ENUMINPUT */
	static const struct strval32 input_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_INPUT_TYPE_???") },
		{ V4L2_INPUT_TYPE_TUNER,
		  XLAT_KNOWN(0x1, "V4L2_INPUT_TYPE_TUNER") },
		{ V4L2_INPUT_TYPE_CAMERA,
		  XLAT_KNOWN(0x2, "V4L2_INPUT_TYPE_CAMERA") },
		{ ARG_XLAT_UNKNOWN(0x4, "V4L2_INPUT_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadc0de, "V4L2_INPUT_TYPE_???") },
	};

	struct v4l2_input *input = tail_alloc(sizeof(*input));

	ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUMINPUT), inject_retval);

	ioctl(-1, VIDIOC_ENUMINPUT, (char *) input + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUMINPUT), (char *) input + 1, inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(stdids); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(input_types); j++) {
			fill_memory32(input, sizeof(*input));
			fill_memory_ex(input->name, sizeof(input->name),
				       i * 47 + 13, 255);
			input->type = input_types[j].val;
			input->std = stdids[i].val;

			ioctl(-1, VIDIOC_ENUMINPUT, input);
			printf("ioctl(-1, %s, {index=2158018784, name=",
			       XLAT_STR(VIDIOC_ENUMINPUT));
			print_quoted_cstring((char *) input->name,
					     sizeof(input->name));
			printf(", type=%s}) = %ld (INJECTED)\n",
			       input_types[j].str, inject_retval);
		}
	}


	/* VIDIOC_G_CTRL, VIDIOC_S_CTRL */
	static const struct strval32 ctrl_cmds[] = {
		{ ARG_STR(VIDIOC_G_CTRL) },
		{ ARG_STR(VIDIOC_S_CTRL) },
	};

	struct v4l2_control *ctrl = tail_alloc(sizeof(*ctrl));

	for (size_t i = 0; i < ARRAY_SIZE(ctrl_cmds); i++) {
		ioctl(-1, ctrl_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(ctrl_cmds[i].str, ctrl_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, ctrl_cmds[i].val, (char *) ctrl + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(ctrl_cmds[i].str, ctrl_cmds[i].val, NULL),
		       (char *) ctrl + 1, inject_retval);

		/* NB: cid printing is mostly tested in ioctl_v4l2.c */
		fill_memory32(ctrl, sizeof(*ctrl));
		ioctl(-1, ctrl_cmds[i].val, ctrl);
		printf("ioctl(-1, %s, {id=0x80a0c0e0"
		       NRAW(" /* V4L2_CID_??? */")
		       ", value=-2136948511%s}) = %ld (INJECTED)\n",
		       sprintxlat(ctrl_cmds[i].str, ctrl_cmds[i].val, NULL),
		       ctrl_cmds[i].val == VIDIOC_S_CTRL
			? " => -2136948511" : "",
		       inject_retval);
	}


	/* VIDIOC_G_TUNER, VIDIOC_S_TUNER */
	static const struct strval32 tuner_cmds[] = {
		{ ARG_STR(VIDIOC_G_TUNER) },
		{ ARG_STR(VIDIOC_S_TUNER) },
	};
	static const struct strval32 tuner_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_TUNER_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_TUNER_RADIO") },
		{ ARG_XLAT_KNOWN(0x5, "V4L2_TUNER_RF") },
		{ ARG_XLAT_UNKNOWN(0x6, "V4L2_TUNER_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadc0de, "V4L2_TUNER_???") },
	};
	static const struct strval32 tuner_caps[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1fff, "V4L2_TUNER_CAP_LOW"
				 "|V4L2_TUNER_CAP_NORM"
				 "|V4L2_TUNER_CAP_HWSEEK_BOUNDED"
				 "|V4L2_TUNER_CAP_HWSEEK_WRAP"
				 "|V4L2_TUNER_CAP_STEREO|V4L2_TUNER_CAP_LANG2"
				 "|V4L2_TUNER_CAP_LANG1|V4L2_TUNER_CAP_RDS"
				 "|V4L2_TUNER_CAP_RDS_BLOCK_IO"
				 "|V4L2_TUNER_CAP_RDS_CONTROLS"
				 "|V4L2_TUNER_CAP_FREQ_BANDS"
				 "|V4L2_TUNER_CAP_HWSEEK_PROG_LIM"
				 "|V4L2_TUNER_CAP_1HZ") },
		{ ARG_XLAT_KNOWN(0xdeadc0de, "V4L2_TUNER_CAP_NORM"
				 "|V4L2_TUNER_CAP_HWSEEK_BOUNDED"
				 "|V4L2_TUNER_CAP_HWSEEK_WRAP"
				 "|V4L2_TUNER_CAP_STEREO|V4L2_TUNER_CAP_LANG1"
				 "|V4L2_TUNER_CAP_RDS|0xdeadc000") },
		{ ARG_XLAT_UNKNOWN(0xffffe000, "V4L2_TUNER_CAP_???") },
	};
	static const struct strval32 tuner_rxsc[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1f, "V4L2_TUNER_SUB_MONO"
				 "|V4L2_TUNER_SUB_STEREO|V4L2_TUNER_SUB_LANG2"
				 "|V4L2_TUNER_SUB_LANG1|V4L2_TUNER_SUB_RDS") },
		{ ARG_XLAT_KNOWN(0xbeefface, "V4L2_TUNER_SUB_STEREO"
				 "|V4L2_TUNER_SUB_LANG2|V4L2_TUNER_SUB_LANG1"
				 "|0xbeeffac0") },
		{ ARG_XLAT_UNKNOWN(0xffffffe0, "V4L2_TUNER_SUB_???") },

	};
	static const struct strval32 tuner_amodes[] = {
		{ ARG_XLAT_KNOWN(0, "V4L2_TUNER_MODE_MONO") },
		{ ARG_XLAT_KNOWN(0x2, "V4L2_TUNER_MODE_LANG2") },
		{ ARG_XLAT_KNOWN(0x4, "V4L2_TUNER_MODE_LANG1_LANG2") },
		{ ARG_XLAT_UNKNOWN(0x5, "V4L2_TUNER_MODE_???") },
		{ ARG_XLAT_UNKNOWN(0xcaffeeed, "V4L2_TUNER_MODE_???") },
	};
	static const size_t tuner_iters = MAX(MAX(MAX(ARRAY_SIZE(tuner_types),
						      ARRAY_SIZE(tuner_caps)),
						  ARRAY_SIZE(tuner_rxsc)),
					      ARRAY_SIZE(tuner_amodes));

	struct v4l2_tuner *tuner = tail_alloc(sizeof(*tuner));

	for (size_t i = 0; i < ARRAY_SIZE(tuner_cmds); i++) {
		ioctl(-1, tuner_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(tuner_cmds[i].str, tuner_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, tuner_cmds[i].val, (char *) tuner + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(tuner_cmds[i].str, tuner_cmds[i].val, NULL),
		       (char *) tuner + 1, inject_retval);

		for (size_t j = 0; j < tuner_iters; j++) {
			fill_memory32(tuner, sizeof(*tuner));
			fill_memory_ex(tuner->name, sizeof(tuner->name),
				       j * 47 + 7, 255);
			tuner->type =
				tuner_types[j % ARRAY_SIZE(tuner_types)].val;
			tuner->capability =
				tuner_caps[j % ARRAY_SIZE(tuner_caps)].val;
			tuner->rxsubchans =
				tuner_rxsc[j % ARRAY_SIZE(tuner_rxsc)].val;
			tuner->audmode =
				tuner_amodes[j % ARRAY_SIZE(tuner_amodes)].val;

			ioctl(-1, tuner_cmds[i].val, tuner);
			printf("ioctl(-1, %s, {index=2158018784",
			       sprintxlat(tuner_cmds[i].str,
					  tuner_cmds[i].val, NULL));
			for (size_t k = 0;
			     k < (tuner_cmds[i].val == VIDIOC_S_TUNER ? 2 : 1);
			     k++) {
				printf("%sname=", k ? "} => {" : ", ");
				print_quoted_cstring((char *) tuner->name,
						     sizeof(tuner->name));
				printf(", type=%s, capability=%s"
				       ", rangelow=2158018795"
				       ", rangehigh=2158018796, rxsubchans=%s"
				       ", audmode=%s, signal=-2136948497"
				       ", afc=-2136948496",
				       tuner_types[j %
						   ARRAY_SIZE(tuner_types)].str,
				       tuner_caps[j %
						  ARRAY_SIZE(tuner_caps)].str,
				       tuner_rxsc[j %
						  ARRAY_SIZE(tuner_rxsc)].str,
				       tuner_amodes[j %
						 ARRAY_SIZE(tuner_amodes)].str);
			}
			printf("}) = %ld (INJECTED)\n", inject_retval);
		}
	}


	/* VIDIOC_QUERYCTRL */
	static const struct strval32 cids[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_CID_???") },
		{ ARG_XLAT_UNKNOWN(0x97abcd, "V4L2_CID_???") },
		{ ARG_XLAT_UNKNOWN(0x980000, "V4L2_CID_???") },
		{ ARG_XLAT_KNOWN(0x990a64,
				 "V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE") },
		{ ARG_XLAT_UNKNOWN(0xa31234, "V4L2_CID_???") },
		{ ARG_XLAT_UNKNOWN(0xa40000, "V4L2_CID_???") },
		{ 0xdeadc0de, XLAT_KNOWN(0x80000000, "V4L2_CTRL_FLAG_NEXT_CTRL")
			      "|" XLAT_UNKNOWN(0x5eadc0de, "V4L2_CID_???") },
	};
	static const struct strval32 ctrl_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_CTRL_TYPE_INTEGER") },
		{ ARG_XLAT_KNOWN(0x2, "V4L2_CTRL_TYPE_BOOLEAN") },
		{ ARG_XLAT_KNOWN(0x3, "V4L2_CTRL_TYPE_MENU") },
		{ ARG_XLAT_KNOWN(0x4, "V4L2_CTRL_TYPE_BUTTON") },
		{ ARG_XLAT_KNOWN(0x5, "V4L2_CTRL_TYPE_INTEGER64") },
		{ ARG_XLAT_KNOWN(0x6, "V4L2_CTRL_TYPE_CTRL_CLASS") },
		{ ARG_XLAT_KNOWN(0x7, "V4L2_CTRL_TYPE_STRING") },
		{ ARG_XLAT_KNOWN(0x8, "V4L2_CTRL_TYPE_BITMASK") },
		{ ARG_XLAT_KNOWN(0x9, "V4L2_CTRL_TYPE_INTEGER_MENU") },
		{ ARG_XLAT_UNKNOWN(0xa, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xff, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x100, "V4L2_CTRL_TYPE_U8") },
		{ ARG_XLAT_KNOWN(0x101, "V4L2_CTRL_TYPE_U16") },
		{ ARG_XLAT_KNOWN(0x102, "V4L2_CTRL_TYPE_U32") },
		{ ARG_XLAT_UNKNOWN(0x103, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0x104, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0x105, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x106, "V4L2_CTRL_TYPE_AREA") },
		{ ARG_XLAT_UNKNOWN(0x107, "V4L2_CTRL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadc0de, "V4L2_CTRL_TYPE_???") },
	};
	static const struct strval32 ctrl_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x7ff, "V4L2_CTRL_FLAG_DISABLED"
					"|V4L2_CTRL_FLAG_GRABBED"
					"|V4L2_CTRL_FLAG_READ_ONLY"
					"|V4L2_CTRL_FLAG_UPDATE"
					"|V4L2_CTRL_FLAG_INACTIVE"
					"|V4L2_CTRL_FLAG_SLIDER"
					"|V4L2_CTRL_FLAG_WRITE_ONLY"
					"|V4L2_CTRL_FLAG_VOLATILE"
					"|V4L2_CTRL_FLAG_HAS_PAYLOAD"
					"|V4L2_CTRL_FLAG_EXECUTE_ON_WRITE"
					"|V4L2_CTRL_FLAG_MODIFY_LAYOUT") },
		{ ARG_XLAT_KNOWN(0xbeefface, "V4L2_CTRL_FLAG_GRABBED"
					     "|V4L2_CTRL_FLAG_READ_ONLY"
					     "|V4L2_CTRL_FLAG_UPDATE"
					     "|V4L2_CTRL_FLAG_WRITE_ONLY"
					     "|V4L2_CTRL_FLAG_VOLATILE"
					     "|V4L2_CTRL_FLAG_EXECUTE_ON_WRITE"
					     "|0xbeeff800") },
		{ ARG_XLAT_UNKNOWN(0xfffff800, "V4L2_CTRL_FLAG_???") },
	};
	static const size_t qctrl_iters = MAX(MAX(ARRAY_SIZE(cids),
						  ARRAY_SIZE(ctrl_types)),
					       ARRAY_SIZE(ctrl_flags));

	struct v4l2_queryctrl *qctrl = tail_alloc(sizeof(*qctrl));

	ioctl(-1, VIDIOC_QUERYCTRL, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCTRL), inject_retval);

	ioctl(-1, VIDIOC_QUERYCTRL, (char *) qctrl + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCTRL), (char *) qctrl + 1, inject_retval);

	for (size_t i = 0; i < qctrl_iters; i++) {
		fill_memory32(qctrl, sizeof(*qctrl));
		fill_memory_ex(qctrl->name, sizeof(qctrl->name),
			       i * 47 + 5, 255);
		qctrl->id    = cids[i % ARRAY_SIZE(cids)].val;
		qctrl->type  = ctrl_types[i % ARRAY_SIZE(ctrl_types)].val;
		qctrl->flags = ctrl_flags[i % ARRAY_SIZE(ctrl_flags)].val;

		ioctl(-1, VIDIOC_QUERYCTRL, qctrl);
		printf("ioctl(-1, %s, {id=%s%s%s, type=%s, name=",
		       XLAT_STR(VIDIOC_QUERYCTRL),
		       cids[i % ARRAY_SIZE(cids)].str,
		       cids[i % ARRAY_SIZE(cids)].val & 0x80000000
			? " => " : "",
		       cids[i % ARRAY_SIZE(cids)].val & 0x80000000
			? cids[i % ARRAY_SIZE(cids)].str : "",
		       ctrl_types[i % ARRAY_SIZE(ctrl_types)].str);
		print_quoted_cstring((char *) qctrl->name, sizeof(qctrl->name));
		printf(", minimum=-2136948502, maximum=-2136948501"
		       ", step=-2136948500, default_value=-2136948499, flags=%s"
		       "}) = %ld (INJECTED)\n",
		       ctrl_flags[i % ARRAY_SIZE(ctrl_flags)].str,
		       inject_retval);
	}


	/* VIDIOC_G_INPUT, VIDIOC_S_INPUT */
	static const struct strval32 input_cmds[] = {
		{ ARG_STR(VIDIOC_G_INPUT) },
		{ ARG_STR(VIDIOC_S_INPUT) },
	};
	static const struct strval32 inputids[] = {
		{ ARG_STR(0) },
		{ ARG_STR(1) },
		{ ARG_STR(1578221295) },
		{ ARG_STR(31415926) },
		{ ARG_STR(4294967295) },
	};

	int *inputid = tail_alloc(sizeof(*inputid));

	for (size_t i = 0; i < ARRAY_SIZE(input_cmds); i++) {
		ioctl(-1, input_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(input_cmds[i].str, input_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, input_cmds[i].val, (char *) inputid + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(input_cmds[i].str, input_cmds[i].val, NULL),
		       (char *) inputid + 1, inject_retval);

		for (size_t j = 0; j < ARRAY_SIZE(inputids); j++) {
			*inputid = inputids[j].val;

			ioctl(-1, input_cmds[i].val, inputid);
			printf("ioctl(-1, %s, [%s]) = %ld (INJECTED)\n",
			       sprintxlat(input_cmds[i].str, input_cmds[i].val,
					  NULL),
			       inputids[j].str, inject_retval);

		}
	}


	/* VIDIOC_CROPCAP */
	struct v4l2_cropcap *ccap = tail_alloc(sizeof(*ccap));

	fill_memory32(ccap, sizeof(*ccap));

	ioctl(-1, VIDIOC_CROPCAP, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_CROPCAP), inject_retval);

	ioctl(-1, VIDIOC_CROPCAP, (char *) ccap + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_CROPCAP), (char *) ccap + 1, inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(buf_types); i++) {
		ccap->type = buf_types[i].val;

		ioctl(-1, VIDIOC_CROPCAP, ccap);
		printf("ioctl(-1, %s, {type=%s"
		       ", bounds={left=-2136948511, top=-2136948510"
		       ", width=2158018787, height=2158018788}"
		       ", defrect={left=-2136948507, top=-2136948506"
		       ", width=2158018791, height=2158018792}"
		       ", pixelaspect=2158018793/2158018794})"
		       " = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_CROPCAP),
		       buf_types[i].str, inject_retval);
	}


	/* VIDIOC_G_CROP, VIDIOC_S_CROP */
	static const struct strval32 crop_cmds[] = {
		{ ARG_STR(VIDIOC_G_CROP) },
		{ ARG_STR(VIDIOC_S_CROP) },
	};
	struct v4l2_crop *crop = tail_alloc(sizeof(*crop));

	for (size_t i = 0; i < ARRAY_SIZE(crop_cmds); i++) {
		fill_memory32(crop, sizeof(*crop));

		ioctl(-1, crop_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(crop_cmds[i].str, crop_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, crop_cmds[i].val, (char *) crop + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(crop_cmds[i].str, crop_cmds[i].val, NULL),
		       (char *) crop + 1, inject_retval);

		for (size_t j = 0; j < ARRAY_SIZE(buf_types); j++) {
			crop->type = buf_types[j].val;

			ioctl(-1, crop_cmds[i].val, crop);
			printf("ioctl(-1, %s, {type=%s, c={left=-2136948511"
			       ", top=-2136948510, width=2158018787"
			       ", height=2158018788}}) = %ld (INJECTED)\n",
			       sprintxlat(crop_cmds[i].str, crop_cmds[i].val,
					  NULL),
			       buf_types[j].str, inject_retval);
		}
	}


#ifdef VIDIOC_S_EXT_CTRLS
	/* VIDIOC_S_EXT_CTRLS, VIDIOC_TRY_EXT_CTRLS, VIDIOC_G_EXT_CTRLS */
	static const struct strval32 ectrl_cmds[] = {
		{ ARG_STR(VIDIOC_S_EXT_CTRLS) },
		{ ARG_STR(VIDIOC_TRY_EXT_CTRLS) },
		{ ARG_STR(VIDIOC_G_EXT_CTRLS) },
	};
	/* static const struct strval32 ectrl_which = {
	}; */

	struct v4l2_ext_controls *ectrls = tail_alloc(sizeof(*ectrls));
	/* struct v4l2_ext_control *ectrl = tail_alloc(sizeof(*ectrl) * 2); */

	for (size_t i = 0; i < ARRAY_SIZE(ectrl_cmds); i++) {
		fill_memory32(ectrls, sizeof(*ectrls));

		ioctl(-1, ectrl_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
		       sprintxlat(ectrl_cmds[i].str, ectrl_cmds[i].val, NULL),
		       inject_retval);

		ioctl(-1, ectrl_cmds[i].val, (char *) ectrls + 1);
		printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
		       sprintxlat(ectrl_cmds[i].str, ectrl_cmds[i].val, NULL),
		       (char *) ectrls + 1, inject_retval);
	}

#endif /* VIDIOC_S_EXT_CTRLS */


#ifdef VIDIOC_ENUM_FRAMESIZES
	/* VIDIOC_ENUM_FRAMESIZES */
	static const struct strval32 frmsz_simple_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_FRMSIZE_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x2, "V4L2_FRMSIZE_TYPE_CONTINUOUS") },
		{ ARG_XLAT_UNKNOWN(0x4, "V4L2_FRMSIZE_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadf157, "V4L2_FRMSIZE_TYPE_???") },
	};

	struct v4l2_frmsizeenum *fse = tail_alloc(sizeof(*fse));

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMESIZES), inject_retval);

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, (char *) fse + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMESIZES), (char *) fse + 1,
	       inject_retval);

	fill_memory32(fse, sizeof(*fse));
	fse->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, fse);
	printf("ioctl(-1, %s, {index=2158018784, pixel_format="
	       RAW("0x80a0c0e1")
	       NRAW("v4l2_fourcc('\\xe1', '\\xc0', '\\xa0', '\\x80')")
	       ", type=" XLAT_KNOWN(0x1, "V4L2_FRMSIZE_TYPE_DISCRETE")
	       ", discrete={width=2158018787, height=2158018788}"
	       "}) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMESIZES), inject_retval);

	fse->pixel_format = 0x5c22270d;
	fse->type = V4L2_FRMSIZE_TYPE_STEPWISE;

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, fse);
	printf("ioctl(-1, %s, {index=2158018784, pixel_format="
	       RAW("0x5c22270d")
	       NRAW("v4l2_fourcc('\\x0d', '\\\'', '\"', '\\\\')")
	       ", type=" XLAT_KNOWN(0x3, "V4L2_FRMSIZE_TYPE_STEPWISE")
	       ", stepwise={min_width=2158018787, max_width=2158018788"
	       ", step_width=2158018789, min_height=2158018790"
	       ", max_height=2158018791, step_height=2158018792}"
	       "}) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMESIZES), inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(frmsz_simple_types); i++) {
		fill_memory32(fse, sizeof(*fse));
		fse->type = frmsz_simple_types[i].val;

		ioctl(-1, VIDIOC_ENUM_FRAMESIZES, fse);
		printf("ioctl(-1, %s, {index=2158018784, pixel_format="
		       RAW("0x80a0c0e1")
		       NRAW("v4l2_fourcc('\\xe1', '\\xc0', '\\xa0', '\\x80')")
		       ", type=%s}) = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_ENUM_FRAMESIZES),
		       frmsz_simple_types[i].str, inject_retval);

	}
#endif /* VIDIOC_ENUM_FRAMESIZES */


#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	/* VIDIOC_ENUM_FRAMEINTERVALS */
	static const struct strval32 frmival_simple_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_FRMIVAL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0x4, "V4L2_FRMIVAL_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadf157, "V4L2_FRMIVAL_TYPE_???") },
	};
	static const struct strval32 frmival_step_types[] = {
		{ ARG_XLAT_KNOWN(0x2, "V4L2_FRMIVAL_TYPE_CONTINUOUS") },
		{ ARG_XLAT_KNOWN(0x3, "V4L2_FRMIVAL_TYPE_STEPWISE") },
	};

	struct v4l2_frmivalenum *fie = tail_alloc(sizeof(*fie));

	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMEINTERVALS), inject_retval);

	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, (char *) fie + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMEINTERVALS), (char *) fie + 1,
	       inject_retval);

	fill_memory32(fie, sizeof(*fie));
	fie->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, fie);
	printf("ioctl(-1, %s, {index=2158018784, pixel_format="
	       RAW("0x80a0c0e1")
	       NRAW("v4l2_fourcc('\\xe1', '\\xc0', '\\xa0', '\\x80')")
	       ", width=2158018786, height=2158018787"
	       ", type=" XLAT_KNOWN(0x1, "V4L2_FRMIVAL_TYPE_DISCRETE")
	       ", discrete=2158018789/2158018790}) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FRAMEINTERVALS), inject_retval);

	fie->pixel_format = 0x5c22270d;

	for (size_t i = 0; i < ARRAY_SIZE(frmival_step_types); i++) {
		fie->type = frmival_step_types[i].val;

		ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, fie);
		printf("ioctl(-1, %s, {index=2158018784, pixel_format="
		       RAW("0x5c22270d")
		       NRAW("v4l2_fourcc('\\x0d', '\\\'', '\"', '\\\\')")
		       ", width=2158018786, height=2158018787, type=%s"
		       ", stepwise={min=2158018789/2158018790"
		       ", max=2158018791/2158018792"
		       ", step=2158018793/2158018794}}) = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_ENUM_FRAMEINTERVALS),
		       frmival_step_types[i].str, inject_retval);
	}

	for (size_t i = 0; i < ARRAY_SIZE(frmival_simple_types); i++) {
		fill_memory32(fie, sizeof(*fie));
		fie->type = frmival_simple_types[i].val;

		ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, fie);
		printf("ioctl(-1, %s, {index=2158018784, pixel_format="
		       RAW("0x80a0c0e1")
		       NRAW("v4l2_fourcc('\\xe1', '\\xc0', '\\xa0', '\\x80')")
	               ", width=2158018786, height=2158018787, type=%s})"
		       " = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_ENUM_FRAMEINTERVALS),
		       frmival_simple_types[i].str, inject_retval);

	}
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */


#ifdef VIDIOC_CREATE_BUFS
	/* VIDIOC_CREATE_BUFS */
	struct v4l2_create_buffers *cbuf = tail_alloc(sizeof(*cbuf));

	fill_memory32(cbuf, sizeof(*cbuf));

	ioctl(-1, VIDIOC_CREATE_BUFS, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_CREATE_BUFS), inject_retval);

	ioctl(-1, VIDIOC_CREATE_BUFS, (char *) cbuf + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_CREATE_BUFS), (char *) cbuf + 1, inject_retval);

	for (size_t i = 0; i < MAX(ARRAY_SIZE(buf_types),
				   ARRAY_SIZE(reqb_mems)); i++) {
		cbuf->memory = reqb_mems[i % ARRAY_SIZE(reqb_mems)].val;
		cbuf->format.type = buf_types[i % ARRAY_SIZE(buf_types)].val;
		if (!fill_fmt(&cbuf->format))
			continue;

		ioctl(-1, VIDIOC_CREATE_BUFS, cbuf);
		printf("ioctl(-1, %s, {count=2158018785, memory=%s"
		       ", format={type=%s",
		       XLAT_STR(VIDIOC_CREATE_BUFS),
		       reqb_mems[i % ARRAY_SIZE(reqb_mems)].str,
		       buf_types[i % ARRAY_SIZE(buf_types)].str);
		print_fmt(", ", &cbuf->format);
		printf("}}) = %ld ({index=2158018784, count=2158018785})"
		       " (INJECTED)\n", inject_retval);
	}
#endif /* VIDIOC_CREATE_BUFS */

	puts("+++ exited with 0 +++");

	return 0;
}
