/*
 * V4L2-related type definitions.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_V4L2_H
#define STRACE_TYPES_V4L2_H

#include <stdint.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

typedef struct v4l2_buffer struct_v4l2_buffer;
typedef struct v4l2_clip struct_v4l2_clip;
typedef struct v4l2_ext_control struct_v4l2_ext_control;
typedef struct v4l2_ext_controls struct_v4l2_ext_controls;
typedef struct v4l2_format struct_v4l2_format;
typedef struct v4l2_framebuffer struct_v4l2_framebuffer;
typedef struct v4l2_input struct_v4l2_input;
typedef struct v4l2_standard struct_v4l2_standard;


typedef struct {
	uint8_t  driver[16];
	uint8_t  card[32];
	uint8_t  bus_info[32];
	uint32_t version;
	uint32_t capabilities;
	uint32_t device_caps; /**< Added by v3.4-rc1~110^2^2~259 */
	uint32_t reserved[3];
} struct_v4l2_capability;


typedef struct {
	uint32_t index;
	uint32_t count;
	uint32_t memory;
	struct_v4l2_format format;
	/** V4L2_BUF_CAP_*, added by Linux commit v4.20-rc1~51^2~14 */
	uint32_t capabilities;
	uint32_t reserved[7];
} struct_v4l2_create_buffers;

#endif /* STRACE_TYPES_V4L2_H */
