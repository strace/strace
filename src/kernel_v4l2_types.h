/*
 * Copyright (c) 2020-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_V4L2_BUFFER_H
# define STRACE_KERNEL_V4L2_BUFFER_H

# include <linux/videodev2.h>
# include "kernel_timeval.h"
# include "kernel_timespec.h"

# if defined __sparc__ && defined __arch64__
typedef struct {
	long long tv_sec;
	int tv_usec;
	int pad;
} kernel_v4l2_timeval_t;
# else
typedef kernel_timeval64_t kernel_v4l2_timeval_t;
# endif

typedef struct {
	uint32_t			index;
	uint32_t			type;
	uint32_t			bytesused;
	uint32_t			flags;
	uint32_t			field;
	kernel_v4l2_timeval_t		timestamp;
	struct v4l2_timecode		timecode;
	uint32_t			sequence;
	uint32_t			memory;
	union {
		uint32_t		offset;
		unsigned long		userptr;
		struct v4l2_plane	*planes;
		int32_t			fd;
	} m;
	uint32_t			length;
	uint32_t			reserved2;
	union {
		int32_t			request_fd;
		uint32_t		reserved;
	};
} kernel_v4l2_buffer_t;

typedef struct {
	uint32_t				type;
	union {
		uint64_t			data[8];
	} u;
	uint32_t				pending;
	uint32_t				sequence;
	kernel_timespec64_t			timestamp;
	uint32_t				id;
	uint32_t				reserved[8];
} kernel_v4l2_event_t;

/* Removed by Linux kernel commit v3.6-rc1~28^2~240.  */
# define V4L2_BUF_FLAG_INPUT	0x0200

/* Removed by Linux kernel commit v3.9-rc1~93^2~237.  */
# define V4L2_CID_HCENTER	(V4L2_CID_BASE+22)
# define V4L2_CID_VCENTER	(V4L2_CID_BASE+23)

/* Removed by Linux kernel commit v4.20-rc7~16^2~2.  */
# define V4L2_CID_MPEG_VIDEO_MPEG2_SLICE_PARAMS         (V4L2_CID_CODEC_BASE + 250)
# define V4L2_CID_MPEG_VIDEO_MPEG2_QUANTIZATION         (V4L2_CID_CODEC_BASE + 251)

/*
 * Constants based on struct v4l2_buffer are unreliable
 * as the latter uses struct timeval.
 */
# undef VIDIOC_QUERYBUF
# define VIDIOC_QUERYBUF	_IOWR('V',   9, kernel_v4l2_buffer_t)

# undef VIDIOC_QBUF
# define VIDIOC_QBUF		_IOWR('V',  15, kernel_v4l2_buffer_t)

# undef VIDIOC_DQBUF
# define VIDIOC_DQBUF		_IOWR('V',  17, kernel_v4l2_buffer_t)

# undef VIDIOC_PREPARE_BUF
# define VIDIOC_PREPARE_BUF	_IOWR('V',  93, kernel_v4l2_buffer_t)

/*
 * Constants based on struct v4l2_event are unreliable
 * as the latter uses struct timespec.
 */
# undef VIDIOC_DQEVENT
# define VIDIOC_DQEVENT		_IOR ('V',  89, kernel_v4l2_event_t)

#endif /* !STRACE_KERNEL_V4L2_BUFFER_H */
