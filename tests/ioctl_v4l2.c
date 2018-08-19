/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
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

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

#define cc0(arg) ((unsigned int) (unsigned char) (arg))
#define cc1(arg) ((unsigned int) (unsigned char) ((unsigned int) (arg) >> 8))
#define cc2(arg) ((unsigned int) (unsigned char) ((unsigned int) (arg) >> 16))
#define cc3(arg) ((unsigned int) (unsigned char) ((unsigned int) (arg) >> 24))
#define fourcc(a0, a1, a2, a3) \
	((unsigned int)(a0) | \
	 ((unsigned int)(a1) << 8) | \
	 ((unsigned int)(a2) << 16) | \
	 ((unsigned int)(a3) << 24))

static const unsigned int magic = 0xdeadbeef;
static const unsigned int pf_magic = fourcc('S', '5', '0', '8');
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
static const unsigned int sf_magic = fourcc('R', 'U', '1', '2');
#endif

static void
init_v4l2_format(struct v4l2_format *const f,
		 const unsigned int buf_type)
{
	memset(f, -1, sizeof(*f));
	f->type = buf_type;
	switch (buf_type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		f->fmt.pix.width = 0x657b8160;
		f->fmt.pix.height = 0x951c0047;
		if (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
			f->fmt.pix.pixelformat = magic;
		else
			f->fmt.pix.pixelformat = pf_magic;
		f->fmt.pix.field = V4L2_FIELD_NONE;
		f->fmt.pix.bytesperline = 0xdf20d185;
		f->fmt.pix.sizeimage = 0x0cf7be41;
		f->fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
		break;
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: {
		unsigned int i;

		f->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		f->fmt.pix_mp.width = 0x1f3b774b;
		f->fmt.pix_mp.height = 0xab96a8d6;
		if (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
			f->fmt.pix_mp.pixelformat = magic;
		else
			f->fmt.pix_mp.pixelformat = pf_magic;
		f->fmt.pix_mp.field = V4L2_FIELD_NONE;
		f->fmt.pix_mp.colorspace = V4L2_COLORSPACE_JPEG;
		struct v4l2_plane_pix_format *cur_pix =
		       f->fmt.pix_mp.plane_fmt;
		for (i = 0;
		     i < ARRAY_SIZE(f->fmt.pix_mp.plane_fmt);
		     i++) {
			cur_pix[i].sizeimage = 0x1e3c531c | i;
			cur_pix[i].bytesperline = 0xa983d721 | i;
		}
		break;
	}
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		f->fmt.win.w.left = 0xe8373662;
		f->fmt.win.w.top = 0x0336d283;
		f->fmt.win.w.width = 0x9235fe72;
		f->fmt.win.w.height = 0xbbd886c8;
		f->fmt.win.field = V4L2_FIELD_ANY;
		f->fmt.win.chromakey = 0xdb1f991f;
		f->fmt.win.clipcount = 2;
		f->fmt.win.clips =
			tail_alloc(sizeof(*f->fmt.win.clips) *
			f->fmt.win.clipcount);
		f->fmt.win.clips[0].c.left = 0x3313d36e;
		f->fmt.win.clips[0].c.top = 0xcdffe510;
		f->fmt.win.clips[0].c.width = 0x2064f3a8;
		f->fmt.win.clips[0].c.height = 0xd06d314a;
		f->fmt.win.clips[1].c.left = 0xd8c8a83f;
		f->fmt.win.clips[1].c.top = 0x336e87ba;
		f->fmt.win.clips[1].c.width = 0x9e3a6fb3;
		f->fmt.win.clips[1].c.height = 0x05617b76;

		f->fmt.win.bitmap = (void *) -2UL;
#ifdef HAVE_STRUCT_V4L2_WINDOW_GLOBAL_ALPHA
		f->fmt.win.global_alpha = 0xce;
#endif
		break;
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		f->fmt.vbi.sampling_rate = 0x3d9b5b79;
		f->fmt.vbi.offset = 0x055b3a09;
		f->fmt.vbi.samples_per_line = 0xf176d436;
		if (buf_type == V4L2_BUF_TYPE_VBI_CAPTURE)
			f->fmt.vbi.sample_format = magic;
		else
			f->fmt.vbi.sample_format = pf_magic;
		f->fmt.vbi.start[0] = 0x9858e2eb;
		f->fmt.vbi.start[1] = 0x8a4dc8c1;
		f->fmt.vbi.count[0] = 0x4bcf36a3;
		f->fmt.vbi.count[1] = 0x97dff65f;
		f->fmt.vbi.flags = V4L2_VBI_INTERLACED;
		break;
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: {
		unsigned int i;

		f->fmt.sliced.service_set = V4L2_SLICED_VPS;
		f->fmt.sliced.io_size = 0xd897925a;
		for (i = 0;
		     i < ARRAY_SIZE(f->fmt.sliced.service_lines[0]);
		     i++) {
			f->fmt.sliced.service_lines[0][i] = 0xc38e | i;
			f->fmt.sliced.service_lines[1][i] = 0x3abb | i;
		}
		break;
	}
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		f->fmt.sdr.pixelformat = sf_magic;
# ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		f->fmt.sdr.buffersize = 0x25afabfb;
# endif
		break;
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		f->fmt.sdr.pixelformat = magic;
# ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		f->fmt.sdr.buffersize = 0x25afabfb;
# endif
		break;
#endif
	}
}

static void
dprint_ioctl_v4l2(struct v4l2_format *const f,
		  const char *request, const unsigned int buf_type,
		  const char *buf_type_string)
{
	int saved_errno;

	switch (buf_type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		saved_errno = errno;
		printf("ioctl(-1, %s, {type=%s"
		       ", fmt.pix={width=%u, height=%u, pixelformat=",
		       request,
		       buf_type_string,
		       f->fmt.pix.width, f->fmt.pix.height);

		if (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
			printf("v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')",
			       cc0(magic), cc1(magic), cc2(magic), cc3(magic));
		else
			printf("v4l2_fourcc('%c', '%c', '%c', '%c') "
			       "/* V4L2_PIX_FMT_SPCA508 */",
			       cc0(pf_magic), cc1(pf_magic), cc2(pf_magic),
			       cc3(pf_magic));

		errno = saved_errno;
		printf(", field=V4L2_FIELD_NONE, bytesperline=%u, sizeimage=%u"
		       ", colorspace=V4L2_COLORSPACE_JPEG}}) = -1 EBADF (%m)\n",
		       f->fmt.pix.bytesperline,
		       f->fmt.pix.sizeimage);
		break;
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: {
		unsigned int i;

		saved_errno = errno;
		printf("ioctl(-1, %s"
		       ", {type=%s"
		       ", fmt.pix_mp={width=%u, height=%u, pixelformat=",
		       request,
		       buf_type_string,
		       f->fmt.pix_mp.width, f->fmt.pix_mp.height);

		if (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
			printf("v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')",
			       cc0(magic), cc1(magic), cc2(magic), cc3(magic));
		else
			printf("v4l2_fourcc('%c', '%c', '%c', '%c') "
			       "/* V4L2_PIX_FMT_SPCA508 */",
			       cc0(pf_magic), cc1(pf_magic), cc2(pf_magic),
			       cc3(pf_magic));

		printf(", field=V4L2_FIELD_NONE, colorspace="
		       "V4L2_COLORSPACE_JPEG, plane_fmt=[");
		for (i = 0;
		     i < ARRAY_SIZE(f->fmt.pix_mp.plane_fmt);
		     ++i) {
			if (i)
				printf(", ");
			printf("{sizeimage=%u, bytesperline=%u}",
			f->fmt.pix_mp.plane_fmt[i].sizeimage,
			f->fmt.pix_mp.plane_fmt[i].bytesperline);
		}
		errno = saved_errno;
		printf("], num_planes=%u}}) = -1 EBADF (%m)\n",
		f->fmt.pix_mp.num_planes);
		break;
	}
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		printf("ioctl(-1, %s, {type=%s"
		       ", fmt.win={left=%d, top=%d, width=%u, height=%u"
		       ", field=V4L2_FIELD_ANY, chromakey=%#x, clips="
		       "[{left=%d, top=%d, width=%u, height=%u}, "
		       "{left=%d, top=%d, width=%u, height=%u}]"
		       ", clipcount=%u, bitmap=%p"
#ifdef HAVE_STRUCT_V4L2_WINDOW_GLOBAL_ALPHA
		       ", global_alpha=%#x"
#endif
		       "}}) = -1 EBADF (%m)\n",
		       request,
		       buf_type_string,
		       f->fmt.win.w.left, f->fmt.win.w.top,
		       f->fmt.win.w.width, f->fmt.win.w.height,
		       f->fmt.win.chromakey,
		       f->fmt.win.clips[0].c.left,
		       f->fmt.win.clips[0].c.top,
		       f->fmt.win.clips[0].c.width,
		       f->fmt.win.clips[0].c.height,
		       f->fmt.win.clips[1].c.left,
		       f->fmt.win.clips[1].c.top,
		       f->fmt.win.clips[1].c.width,
		       f->fmt.win.clips[1].c.height,
		       f->fmt.win.clipcount, f->fmt.win.bitmap
#ifdef HAVE_STRUCT_V4L2_WINDOW_GLOBAL_ALPHA
		       , f->fmt.win.global_alpha
#endif
		       );
		break;
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		saved_errno = errno;
		printf("ioctl(-1, %s, {type=%s"
		       ", fmt.vbi={sampling_rate=%u, offset=%u"
		       ", samples_per_line=%u, sample_format=",
		       request,
		       buf_type_string,
		       f->fmt.vbi.sampling_rate, f->fmt.vbi.offset,
		       f->fmt.vbi.samples_per_line);

		if (buf_type == V4L2_BUF_TYPE_VBI_CAPTURE)
			printf("v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')",
			       cc0(magic), cc1(magic), cc2(magic), cc3(magic));
		else
			printf("v4l2_fourcc('%c', '%c', '%c', '%c') "
			       "/* V4L2_PIX_FMT_SPCA508 */",
			       cc0(pf_magic), cc1(pf_magic), cc2(pf_magic),
			       cc3(pf_magic));

		errno = saved_errno;
		printf(", start=[%u, %u], count=[%u, %u]"
		       ", flags=V4L2_VBI_INTERLACED}})"
		       " = -1 EBADF (%m)\n",
		       f->fmt.vbi.start[0], f->fmt.vbi.start[1],
		       f->fmt.vbi.count[0], f->fmt.vbi.count[1]);
		break;
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: {
		unsigned int i, j;

		printf("ioctl(-1, %s, {type=%s"
		       ", fmt.sliced={service_set=V4L2_SLICED_VPS"
		       ", io_size=%u, service_lines=[",
		       request,
		       buf_type_string,
		       f->fmt.sliced.io_size);
		for (i = 0;
		     i < ARRAY_SIZE(f->fmt.sliced.service_lines);
		     i++) {
			if (i > 0)
				printf(", ");
			printf("[");
			for (j = 0;
			     j < ARRAY_SIZE(f->fmt.sliced.service_lines[0]);
			     j++) {
				if (j > 0)
					printf(", ");
				printf("%#x",
				       f->fmt.sliced.service_lines[i][j]);
			}
			printf("]");
		}
		printf("]}}) = -1 EBADF (%m)\n");
		break;
	}
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	case V4L2_BUF_TYPE_SDR_OUTPUT:
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		saved_errno = errno;
		printf("ioctl(-1, %s, {type=%s"
		       ", fmt.sdr={pixelformat=",
		       request,
		       buf_type_string);

		if (buf_type == V4L2_BUF_TYPE_SDR_CAPTURE)
			printf("v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')",
			       cc0(magic), cc1(magic), cc2(magic), cc3(magic));
# if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
		else
			printf("v4l2_fourcc('%c', '%c', '%c', '%c') "
			       "/* V4L2_SDR_FMT_RU12LE */",
			       cc0(sf_magic), cc1(sf_magic), cc2(sf_magic),
			       cc3(sf_magic));
# endif

		errno = saved_errno;
		printf(
#ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		       ", buffersize=%u"
#endif
		       "}}) = -1 EBADF (%m)\n"
#ifdef HAVE_STRUCT_V4L2_SDR_FORMAT_BUFFERSIZE
		       , f->fmt.sdr.buffersize
#endif
		       );
		break;
#endif
	}
}
#define print_ioctl_v4l2(v4l2_format, request, buf_type)	\
	dprint_ioctl_v4l2((v4l2_format), (request), (buf_type), #buf_type)

int
main(void)
{
	const unsigned int size = get_page_size();
	void *const page = tail_alloc(size);
	void *const page_end = page + size;
	fill_memory(page, size);

	unsigned char cc[sizeof(int)] = { 'A', '\'', '\\', '\xfa' };

	/* VIDIOC_QUERYCAP */
	ioctl(-1, VIDIOC_QUERYCAP, 0);
	printf("ioctl(-1, VIDIOC_QUERYCAP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QUERYCAP, page);
	printf("ioctl(-1, VIDIOC_QUERYCAP, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_ENUM_FMT */
	ioctl(-1, VIDIOC_ENUM_FMT, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FMT, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_fmtdesc, p_fmtdesc);
	p_fmtdesc->index = magic;
	p_fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(-1, VIDIOC_ENUM_FMT, p_fmtdesc);
	printf("ioctl(-1, VIDIOC_ENUM_FMT, {index=%u"
	       ", type=V4L2_BUF_TYPE_VIDEO_CAPTURE}) = -1 EBADF (%m)\n",
	       p_fmtdesc->index);

	/* VIDIOC_G_FMT */
	ioctl(-1, VIDIOC_G_FMT, 0);
	printf("ioctl(-1, VIDIOC_G_FMT, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_format, p_format);

	p_format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_CAPTURE}) = -1 EBADF (%m)\n");
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	p_format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE}) ="
	       " -1 EBADF (%m)\n");
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	p_format->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY}) ="
	       " -1 EBADF (%m)\n");
#endif
	p_format->type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_OVERLAY}) ="
	       " -1 EBADF (%m)\n");

	p_format->type = V4L2_BUF_TYPE_VBI_CAPTURE;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VBI_CAPTURE}) = -1 EBADF (%m)\n");
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	p_format->type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_SLICED_VBI_CAPTURE}) = -1 EBADF (%m)\n");
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	p_format->type = V4L2_BUF_TYPE_SDR_CAPTURE;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_SDR_CAPTURE}) = -1 EBADF (%m)\n");
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	p_format->type = V4L2_BUF_TYPE_SDR_OUTPUT;
	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_SDR_OUTPUT}) = -1 EBADF (%m)\n");
#endif
	/* VIDIOC_S_FMT */
	ioctl(-1, VIDIOC_S_FMT, 0);
	printf("ioctl(-1, VIDIOC_S_FMT, NULL) = -1 EBADF (%m)\n");

	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OUTPUT);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT", V4L2_BUF_TYPE_VIDEO_OUTPUT);
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT",
			 V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT",
			 V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
#endif
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OVERLAY);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT",
			 V4L2_BUF_TYPE_VIDEO_OVERLAY);

	init_v4l2_format(p_format, V4L2_BUF_TYPE_VBI_CAPTURE);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT", V4L2_BUF_TYPE_VBI_CAPTURE);
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT",
			 V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SDR_CAPTURE);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT", V4L2_BUF_TYPE_SDR_CAPTURE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SDR_OUTPUT);
	ioctl(-1, VIDIOC_S_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_S_FMT", V4L2_BUF_TYPE_SDR_OUTPUT);
#endif
	/* VIDIOC_TRY_FMT */
	ioctl(-1, VIDIOC_TRY_FMT, 0);
	printf("ioctl(-1, VIDIOC_TRY_FMT, NULL) = -1 EBADF (%m)\n");

	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OUTPUT);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT",
			 V4L2_BUF_TYPE_VIDEO_OUTPUT);
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT",
			 V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT",
			 V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
#endif
	init_v4l2_format(p_format, V4L2_BUF_TYPE_VIDEO_OVERLAY);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT",
			 V4L2_BUF_TYPE_VIDEO_OVERLAY);

	init_v4l2_format(p_format, V4L2_BUF_TYPE_VBI_CAPTURE);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT", V4L2_BUF_TYPE_VBI_CAPTURE);
#if HAVE_DECL_V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT",
			 V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SDR_CAPTURE);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT", V4L2_BUF_TYPE_SDR_CAPTURE);
#endif
#if HAVE_DECL_V4L2_BUF_TYPE_SDR_OUTPUT
	init_v4l2_format(p_format, V4L2_BUF_TYPE_SDR_OUTPUT);
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	print_ioctl_v4l2(p_format, "VIDIOC_TRY_FMT", V4L2_BUF_TYPE_SDR_OUTPUT);
#endif
	struct v4l2_format *const p_v4l2_format =
		page_end - sizeof(*p_v4l2_format);
	ioctl(-1, VIDIOC_TRY_FMT, p_v4l2_format);
	printf("ioctl(-1, VIDIOC_TRY_FMT, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_format->type);

	/* VIDIOC_REQBUFS */
	ioctl(-1, VIDIOC_REQBUFS, 0);
	printf("ioctl(-1, VIDIOC_REQBUFS, NULL) = -1 EBADF (%m)\n");

	struct v4l2_requestbuffers *const p_v4l2_requestbuffers =
		page_end - sizeof(*p_v4l2_requestbuffers);
	ioctl(-1, VIDIOC_REQBUFS, p_v4l2_requestbuffers);
	printf("ioctl(-1, VIDIOC_REQBUFS, {type=%#x /* V4L2_BUF_TYPE_??? */, "
	       "memory=%#x /* V4L2_MEMORY_??? */, count=%u})"
	       " = -1 EBADF (%m)\n",
	       p_v4l2_requestbuffers->type,
	       p_v4l2_requestbuffers->memory,
	       p_v4l2_requestbuffers->count);

	/* VIDIOC_QUERYBUF */
	ioctl(-1, VIDIOC_QUERYBUF, 0);
	printf("ioctl(-1, VIDIOC_QUERYBUF, NULL) = -1 EBADF (%m)\n");

	struct v4l2_buffer *const p_v4l2_buffer =
		page_end - sizeof(*p_v4l2_buffer);
	ioctl(-1, VIDIOC_QUERYBUF, p_v4l2_buffer);
	printf("ioctl(-1, VIDIOC_QUERYBUF, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", index=%u}) = -1 EBADF (%m)\n",
	       p_v4l2_buffer->type, p_v4l2_buffer->index);

	/* VIDIOC_QBUF */
	ioctl(-1, VIDIOC_QBUF, 0);
	printf("ioctl(-1, VIDIOC_QBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QBUF, p_v4l2_buffer);
	printf("ioctl(-1, VIDIOC_QBUF, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", index=%u}) = -1 EBADF (%m)\n",
	       p_v4l2_buffer->type, p_v4l2_buffer->index);

	/* VIDIOC_DQBUF */
	ioctl(-1, VIDIOC_DQBUF, 0);
	printf("ioctl(-1, VIDIOC_DQBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_DQBUF, p_v4l2_buffer);
	printf("ioctl(-1, VIDIOC_DQBUF, {type=%#x"
	       " /* V4L2_BUF_TYPE_??? */}) = -1 EBADF (%m)\n",
	       p_v4l2_buffer->type);

	/* VIDIOC_G_FBUF */
	ioctl(-1, VIDIOC_G_FBUF, 0);
	printf("ioctl(-1, VIDIOC_G_FBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_FBUF, page);
	printf("ioctl(-1, VIDIOC_G_FBUF, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_FBUF */
	ioctl(-1, VIDIOC_S_FBUF, 0);
	printf("ioctl(-1, VIDIOC_S_FBUF, NULL) = -1 EBADF (%m)\n");

	struct v4l2_framebuffer *const p_v4l2_framebuffer =
		page_end - sizeof(*p_v4l2_framebuffer);
	ioctl(-1, VIDIOC_S_FBUF, p_v4l2_framebuffer);
	printf("ioctl(-1, VIDIOC_S_FBUF, {capability=%#x"
	       ", flags=%#x, base=%p}) = -1 EBADF (%m)\n",
	       p_v4l2_framebuffer->capability,
	       p_v4l2_framebuffer->flags,
	       p_v4l2_framebuffer->base);

	/* VIDIOC_STREAMON */
	ioctl(-1, VIDIOC_STREAMON, 0);
	printf("ioctl(-1, VIDIOC_STREAMON, NULL) = -1 EBADF (%m)\n");

	int *const p_int = page_end - sizeof(int);
	ioctl(-1, VIDIOC_STREAMON, p_int);
	printf("ioctl(-1, VIDIOC_STREAMON, [%#x /* V4L2_BUF_TYPE_??? */])"
	       " = -1 EBADF (%m)\n", *p_int);

	/* VIDIOC_STREAMOFF */
	ioctl(-1, VIDIOC_STREAMOFF, 0);
	printf("ioctl(-1, VIDIOC_STREAMOFF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_STREAMOFF, p_int);
	printf("ioctl(-1, VIDIOC_STREAMOFF, [%#x /* V4L2_BUF_TYPE_??? */])"
	       " = -1 EBADF (%m)\n", *p_int);

	/* VIDIOC_G_PARM */
	ioctl(-1, VIDIOC_G_PARM, 0);
	printf("ioctl(-1, VIDIOC_G_PARM, NULL) = -1 EBADF (%m)\n");

	struct v4l2_streamparm *const p_v4l2_streamparm =
		page_end - sizeof(*p_v4l2_streamparm);
	ioctl(-1, VIDIOC_G_PARM, p_v4l2_streamparm);
	printf("ioctl(-1, VIDIOC_G_PARM, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_streamparm->type);

	/* VIDIOC_S_PARM */
	ioctl(-1, VIDIOC_S_PARM, 0);
	printf("ioctl(-1, VIDIOC_S_PARM, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_PARM, p_v4l2_streamparm);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_streamparm->type);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_streamparm, p_streamparm);
	p_streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	p_streamparm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	p_streamparm->parm.capture.capturemode = V4L2_MODE_HIGHQUALITY;
	p_streamparm->parm.capture.timeperframe.numerator = 0xdeadbeef;
	p_streamparm->parm.capture.timeperframe.denominator = 0xbadc0ded;
	ioctl(-1, VIDIOC_S_PARM, p_streamparm);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE"
	       ", parm.capture={capability=V4L2_CAP_TIMEPERFRAME"
	       ", capturemode=V4L2_MODE_HIGHQUALITY, timeperframe=%u/%u"
	       ", extendedmode=%u, readbuffers=%u}}) = -1 EBADF (%m)\n",
	       p_streamparm->parm.capture.timeperframe.numerator,
	       p_streamparm->parm.capture.timeperframe.denominator, -1U, -1U);

	p_streamparm->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	p_streamparm->parm.output.outputmode = 0;
	ioctl(-1, VIDIOC_S_PARM, p_streamparm);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=V4L2_BUF_TYPE_VIDEO_OUTPUT"
	       ", parm.output={capability=V4L2_CAP_TIMEPERFRAME"
	       ", outputmode=0, timeperframe=%u/%u"
	       ", extendedmode=%u, writebuffers=%u}}) = -1 EBADF (%m)\n",
	       p_streamparm->parm.output.timeperframe.numerator,
	       p_streamparm->parm.output.timeperframe.denominator, -1U, -1U);

	/* VIDIOC_G_STD */
	ioctl(-1, VIDIOC_G_STD, 0);
	printf("ioctl(-1, VIDIOC_G_STD, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_STD, page);
	printf("ioctl(-1, VIDIOC_G_STD, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_STD */
	ioctl(-1, VIDIOC_S_STD, 0);
	printf("ioctl(-1, VIDIOC_S_STD, NULL) = -1 EBADF (%m)\n");

	long long *const p_longlong = page_end - sizeof(*p_longlong);
	ioctl(-1, VIDIOC_S_STD, p_longlong);
	printf("ioctl(-1, VIDIOC_S_STD, [%#llx]) = -1 EBADF (%m)\n",
	       *p_longlong);

	/* VIDIOC_ENUMSTD */
	ioctl(-1, VIDIOC_ENUMSTD, 0);
	printf("ioctl(-1, VIDIOC_ENUMSTD, NULL) = -1 EBADF (%m)\n");

	struct v4l2_standard *const p_v4l2_standard =
		page_end - sizeof(*p_v4l2_standard);
	ioctl(-1, VIDIOC_ENUMSTD, p_v4l2_standard);
	printf("ioctl(-1, VIDIOC_ENUMSTD, {index=%u}) = -1 EBADF (%m)\n",
	       p_v4l2_standard->index);

	/* VIDIOC_ENUMINPUT */
	ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, NULL) = -1 EBADF (%m)\n");

	struct v4l2_input *const p_v4l2_input =
		page_end - sizeof(*p_v4l2_input);
	ioctl(-1, VIDIOC_ENUMINPUT, p_v4l2_input);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, {index=%u}) = -1 EBADF (%m)\n",
	       p_v4l2_input->index);

	/* VIDIOC_G_CTRL */
	ioctl(-1, VIDIOC_G_CTRL, 0);
	printf("ioctl(-1, VIDIOC_G_CTRL, NULL) = -1 EBADF (%m)\n");

	struct v4l2_control *const p_v4l2_control =
		page_end - sizeof(*p_v4l2_control);
	ioctl(-1, VIDIOC_G_CTRL, p_v4l2_control);
	printf("ioctl(-1, VIDIOC_G_CTRL, {id=%#x /* V4L2_CID_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_control->id);

	/* VIDIOC_S_CTRL */
	ioctl(-1, VIDIOC_S_CTRL, 0);
	printf("ioctl(-1, VIDIOC_S_CTRL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_CTRL, p_v4l2_control);
	printf("ioctl(-1, VIDIOC_S_CTRL, {id=%#x /* V4L2_CID_??? */"
	       ", value=%d}) = -1 EBADF (%m)\n",
	       p_v4l2_control->id, p_v4l2_control->value);

	/* VIDIOC_G_TUNER */
	ioctl(-1, VIDIOC_G_TUNER, 0);
	printf("ioctl(-1, VIDIOC_G_TUNER, NULL) = -1 EBADF (%m)\n");

	struct v4l2_tuner *const p_v4l2_tuner =
		page_end - sizeof(*p_v4l2_tuner);
	ioctl(-1, VIDIOC_G_TUNER, p_v4l2_tuner);
	printf("ioctl(-1, VIDIOC_G_TUNER, {index=%u})"
	       " = -1 EBADF (%m)\n", p_v4l2_tuner->index);

	/* VIDIOC_S_TUNER */
	ioctl(-1, VIDIOC_S_TUNER, 0);
	printf("ioctl(-1, VIDIOC_S_TUNER, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_tuner, p_tuner);
	p_tuner->index = 0x4fb6df39;
	strcpy((char *) p_tuner->name, "cum tacent clamant");
	p_tuner->type = V4L2_TUNER_RADIO;
	p_tuner->capability = V4L2_TUNER_CAP_LOW;
	p_tuner->rangelow = 0xa673bc29;
	p_tuner->rangehigh = 0xbaf16d12;
	p_tuner->rxsubchans = V4L2_TUNER_SUB_MONO;
	p_tuner->audmode = V4L2_TUNER_MODE_MONO;
	p_tuner->signal = 0x10bf92c8;
	p_tuner->afc = 0x3bf7e18b;
	ioctl(-1, VIDIOC_S_TUNER, p_tuner);
	printf("ioctl(-1, VIDIOC_S_TUNER, {index=%u"
	       ", name=\"cum tacent clamant\""
	       ", type=V4L2_TUNER_RADIO, capability=V4L2_TUNER_CAP_LOW"
	       ", rangelow=%u, rangehigh=%u"
	       ", rxsubchans=V4L2_TUNER_SUB_MONO"
	       ", audmode=V4L2_TUNER_MODE_MONO, signal=%d, afc=%d"
	       "}) = -1 EBADF (%m)\n",
	       p_tuner->index, p_tuner->rangelow,
	       p_tuner->rangehigh, p_tuner->signal, p_tuner->afc);

	/* VIDIOC_QUERYCTRL */
	ioctl(-1, VIDIOC_QUERYCTRL, 0);
	printf("ioctl(-1, VIDIOC_QUERYCTRL, NULL) = -1 EBADF (%m)\n");

	struct v4l2_queryctrl *const p_v4l2_queryctrl =
		page_end - sizeof(*p_v4l2_queryctrl);
	ioctl(-1, VIDIOC_QUERYCTRL, p_v4l2_queryctrl);
# ifdef V4L2_CTRL_FLAG_NEXT_CTRL
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=V4L2_CTRL_FLAG_NEXT_CTRL"
	       "|%#x /* V4L2_CID_??? */}) = -1 EBADF (%m)\n",
	       p_v4l2_queryctrl->id & ~V4L2_CTRL_FLAG_NEXT_CTRL);
# else
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=%#x /* V4L2_CID_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_queryctrl->id);
# endif

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_queryctrl, p_queryctrl);
	p_queryctrl->id = V4L2_CID_SATURATION;
	ioctl(-1, VIDIOC_QUERYCTRL, p_queryctrl);
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=V4L2_CID_SATURATION})"
	       " = -1 EBADF (%m)\n");

	/* VIDIOC_G_INPUT */
	ioctl(-1, VIDIOC_G_INPUT, 0);
	printf("ioctl(-1, VIDIOC_G_INPUT, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_INPUT, page);
	printf("ioctl(-1, VIDIOC_G_INPUT, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_INPUT */
	ioctl(-1, VIDIOC_S_INPUT, 0);
	printf("ioctl(-1, VIDIOC_S_INPUT, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_INPUT, p_int);
	printf("ioctl(-1, VIDIOC_S_INPUT, [%u]) = -1 EBADF (%m)\n", *p_int);

	/* VIDIOC_CROPCAP */
	ioctl(-1, VIDIOC_CROPCAP, 0);
	printf("ioctl(-1, VIDIOC_CROPCAP, NULL) = -1 EBADF (%m)\n");

	struct v4l2_cropcap *const p_v4l2_cropcap =
		page_end - sizeof(*p_v4l2_cropcap);
	ioctl(-1, VIDIOC_CROPCAP, p_v4l2_cropcap);
	printf("ioctl(-1, VIDIOC_CROPCAP, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_cropcap->type);

	/* VIDIOC_G_CROP */
	ioctl(-1, VIDIOC_G_CROP, 0);
	printf("ioctl(-1, VIDIOC_G_CROP, NULL) = -1 EBADF (%m)\n");

	struct v4l2_crop *const p_v4l2_crop =
		page_end - sizeof(*p_v4l2_crop);
	ioctl(-1, VIDIOC_G_CROP, p_v4l2_crop);
	printf("ioctl(-1, VIDIOC_G_CROP, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", p_v4l2_crop->type);

	/* VIDIOC_S_CROP */
	ioctl(-1, VIDIOC_S_CROP, 0);
	printf("ioctl(-1, VIDIOC_S_CROP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_CROP, p_v4l2_crop);
	printf("ioctl(-1, VIDIOC_S_CROP, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", c={left=%d, top=%d, width=%u, height=%u}}) = -1 EBADF (%m)\n",
	       p_v4l2_crop->type,
	       p_v4l2_crop->c.left,
	       p_v4l2_crop->c.top,
	       p_v4l2_crop->c.width,
	       p_v4l2_crop->c.height);

#ifdef VIDIOC_S_EXT_CTRLS
	/* VIDIOC_S_EXT_CTRLS */
	ioctl(-1, VIDIOC_S_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_ext_controls, p_ext_controls);
	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_USER;
	p_ext_controls->count = 0;
	p_ext_controls->controls = (void *) -2UL;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, {ctrl_class=V4L2_CTRL_CLASS_USER"
	       ", count=%u}) = -1 EBADF (%m)\n", p_ext_controls->count);

	p_ext_controls->ctrl_class = 0x00a30000;
	p_ext_controls->count = magic;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, {ctrl_class=V4L2_CTRL_CLASS_DETECT"
	       ", count=%u, controls=%p}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

	p_ext_controls->ctrl_class = 0x00a40000;
	p_ext_controls->count = magic;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS"
	       ", {ctrl_class=0xa40000 /* V4L2_CTRL_CLASS_??? */"
	       ", count=%u, controls=%p}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_MPEG;
	p_ext_controls->count = magic;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, {ctrl_class=V4L2_CTRL_CLASS_MPEG"
	       ", count=%u, controls=%p}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

# if HAVE_DECL_V4L2_CTRL_TYPE_STRING
	p_ext_controls->count = 2;
	p_ext_controls->controls =
		tail_alloc(sizeof(*p_ext_controls->controls) * p_ext_controls->count);
	p_ext_controls->controls[0].id = V4L2_CID_BRIGHTNESS;
	p_ext_controls->controls[0].size = 0;
	p_ext_controls->controls[0].value64 = 0xfacefeeddeadbeefULL;
	p_ext_controls->controls[1].id = V4L2_CID_CONTRAST;
	p_ext_controls->controls[1].size = 2;
	p_ext_controls->controls[1].string =
		tail_alloc(p_ext_controls->controls[1].size);

	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_MPEG, count=%u, controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       "] => controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       "], error_idx=%u}) = -1 EBADF (%m)\n",
	       p_ext_controls->count,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->error_idx);

	++p_ext_controls->count;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_MPEG, count=%u, controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       ", ... /* %p */]}) = -1 EBADF (%m)\n",
	       p_ext_controls->count,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->controls + 2);
# endif /* HAVE_DECL_V4L2_CTRL_TYPE_STRING */

	/* VIDIOC_TRY_EXT_CTRLS */
	ioctl(-1, VIDIOC_TRY_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_TRY_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_USER;
	p_ext_controls->count = magic;
	p_ext_controls->controls = (void *) -2UL;
	ioctl(-1, VIDIOC_TRY_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_TRY_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_USER, count=%u, controls=%p})"
	       " = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

	/* VIDIOC_G_EXT_CTRLS */
	ioctl(-1, VIDIOC_G_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_G_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_G_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_USER, count=%u, controls=%p"
	       ", error_idx=%u}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls,
	       p_ext_controls->error_idx);
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMESIZES, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct v4l2_frmsizeenum, p_frmsizeenum);
	p_frmsizeenum->index = magic;
	p_frmsizeenum->pixel_format = fourcc(cc[0], cc[1], cc[2], cc[3]);

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, p_frmsizeenum);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMESIZES, {index=%u"
	       ", pixel_format=v4l2_fourcc('%c', '\\%c', '\\%c', '\\x%x')})"
	       " = -1 EBADF (%m)\n", p_frmsizeenum->index,
	       cc[0], cc[1], cc[2], cc[3]);
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, NULL) = -1 EBADF (%m)\n");

	struct v4l2_frmivalenum *const p_v4l2_frmivalenum =
		page_end - sizeof(*p_v4l2_frmivalenum);
	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, p_v4l2_frmivalenum);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, {index=%u"
	       ", pixel_format=v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')"
	       ", width=%u, height=%u}) = -1 EBADF (%m)\n",
	       p_v4l2_frmivalenum->index,
	       cc0(p_v4l2_frmivalenum->pixel_format),
	       cc1(p_v4l2_frmivalenum->pixel_format),
	       cc2(p_v4l2_frmivalenum->pixel_format),
	       cc3(p_v4l2_frmivalenum->pixel_format),
	       p_v4l2_frmivalenum->width,
	       p_v4l2_frmivalenum->height);
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
	ioctl(-1, VIDIOC_CREATE_BUFS, 0);
	printf("ioctl(-1, VIDIOC_CREATE_BUFS, NULL) = -1 EBADF (%m)\n");

	struct v4l2_create_buffers *const p_v4l2_create_buffers =
		page_end - sizeof(*p_v4l2_create_buffers);
	ioctl(-1, VIDIOC_CREATE_BUFS, p_v4l2_create_buffers);
	printf("ioctl(-1, VIDIOC_CREATE_BUFS, {count=%u, memory=%#x"
	       " /* V4L2_MEMORY_??? */, format={type=%#x"
	       " /* V4L2_BUF_TYPE_??? */}}) = -1 EBADF (%m)\n",
	       p_v4l2_create_buffers->count,
	       p_v4l2_create_buffers->memory,
	       p_v4l2_create_buffers->format.type);
#endif /* VIDIOC_CREATE_BUFS */

	puts("+++ exited with 0 +++");
	return 0;
}
