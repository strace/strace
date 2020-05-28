/*
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_V4L2_BUFFER_H
# define STRACE_KERNEL_V4L2_BUFFER_H

# include "types/v4l2.h"
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

#endif /* !STRACE_KERNEL_V4L2_BUFFER_H */
