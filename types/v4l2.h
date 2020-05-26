/*
 * V4L2-related type definitions.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_V4L2_H
# define STRACE_TYPES_V4L2_H

# include <stdint.h>
# include <linux/ioctl.h>
# include <linux/types.h>
# include <linux/videodev2.h>

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
	uint32_t width;
	uint32_t height;
	uint32_t pixelformat;
	uint32_t field;
	uint32_t bytesperline;
	uint32_t sizeimage;
	uint32_t colorspace; /**< enum v4l2_colorspace */
	uint32_t priv;
	/** Format flags (V4L2_PIX_FMT_FLAG_*), added by v3.17-rc1~112^2~326 */
	uint32_t flags;
	union {
		/** enum v4l2_ycbcr_encoding, added by v3.19-rc1~29^2~72 */
		uint32_t ycbcr_enc;
		/** enum v4l2_hsv_encoding, added by v4.10-rc1~71^2^2~352 */
		uint32_t hsv_enc;
	};
	/** enum v4l2_quantization, added by v3.19-rc1~29^2~72 */
	uint32_t quantization;
	/** enum v4l2_xfer_func, added by Linux commit v4.2-rc1~107^2~136 */
	uint32_t xfer_func;
} struct_v4l2_pix_format;

/** Added by Linux commit v2.6.39-rc1~86^2~437 */
typedef struct {
	uint32_t sizeimage;
	uint32_t bytesperline; /**< Type has changed in v4.1-rc1~59^2~1^2~88 */
	uint16_t reserved[6];
} struct_v4l2_plane_pix_format;

/** Added by Linux commit v2.6.39-rc1~86^2~437 */
typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t pixelformat;
	uint32_t field;
	uint32_t colorspace;

	struct_v4l2_plane_pix_format plane_fmt[8 /* VIDEO_MAX_PLANES */];
	uint8_t num_planes;
	/** Format flags (V4L2_PIX_FMT_FLAG_*), added by v3.17-rc1~112^2~326 */
	uint8_t flags;
	union {
		/** enum v4l2_ycbcr_encoding, added by v3.19-rc1~29^2~72 */
		uint8_t ycbcr_enc;
		/** enum v4l2_hsv_encoding, added by v4.10-rc1~71^2^2~352 */
		uint8_t hsv_enc;
	};
	/** enum v4l2_quantization, added by v3.19-rc1~29^2~72 */
	uint8_t quantization;
	/** enum v4l2_xfer_func, added by Linux commit v4.2-rc1~107^2~136 */
	uint8_t xfer_func;
	uint8_t reserved[7];
} ATTRIBUTE_PACKED struct_v4l2_pix_format_mplane;

typedef struct strace_v4l2_clip {
	struct v4l2_rect c;
	struct strace_v4l2_clip * next;
} struct_v4l2_clip;

typedef struct {
	struct v4l2_rect w;
	uint32_t field; /* enum v4l2_field */
	uint32_t chromakey;
	struct_v4l2_clip *clips;
	uint32_t clipcount;
	void *bitmap;
	uint8_t global_alpha; /**< Added by v2.6.22-rc1~1118^2~179 */
} struct_v4l2_window;

typedef struct {
	uint32_t sampling_rate;
	uint32_t offset;
	uint32_t samples_per_line;
	uint32_t sample_format; /* V4L2_PIX_FMT_* */
	int32_t  start[2];
	uint32_t count[2];
	uint32_t flags; /* V4L2_VBI_* */
	uint32_t reserved[2];
} struct_v4l2_vbi_format;

/** Added by Linux commit v2.6.16.28-rc1~3732 */
typedef struct {
	uint16_t service_set;
	uint16_t service_lines[2][24];
	uint32_t io_size;
	uint32_t reserved[2];
} struct_v4l2_sliced_vbi_format;

typedef struct {
	uint16_t service_set;
	uint16_t service_lines[2][24];
	uint32_t type; /**< enum v4l2_buf_type, added by v2.6.19-rc1~643^2~52 */
	uint32_t reserved[3];
} struct_v4l2_sliced_vbi_cap;

/** Added by Linux commits v3.15-rc1~85^2~213, v3.15-rc1~85^2~41 */
typedef struct {
	uint32_t pixelformat;
	uint32_t buffersize; /**< Added by Linux commit v3.17-rc1~112^2~230 */
	uint8_t  reserved[24];
} ATTRIBUTE_PACKED struct_v4l2_sdr_format;

/** Added by Linux commit v4.12-rc1~85^2~71 */
typedef struct {
	uint32_t dataformat;
	uint32_t buffersize;
} ATTRIBUTE_PACKED struct_v4l2_meta_format;

typedef struct {
	uint32_t type;
	union {
		struct_v4l2_pix_format		pix;
		/** Added by Linux commit v2.6.39-rc1~86^2~437 */
		struct_v4l2_pix_format_mplane	pix_mp;
		struct_v4l2_window		win;
		struct_v4l2_vbi_format		vbi;
		/** Added by Linux commit v2.6.16.28-rc1~3732 */
		struct_v4l2_sliced_vbi_format	sliced;
		/** Added by v3.15-rc1~85^2~213, v3.15-rc1~85^2~41 */
		struct_v4l2_sdr_format		sdr;
		/** Added by Linux commit v4.12-rc1~85^2~71 */
		struct_v4l2_meta_format		meta;
		uint8_t raw_data[200];
	} fmt;
} struct_v4l2_format;


/** Added by Linux v5.5-rc1~143^2^2~225 */
typedef struct {
	__u32   width;
	__u32   height;
} struct_v4l2_area;

/** Added by Linux commit v2.6.18-rc1~862^2~18 */
typedef struct {
        uint32_t id;
        uint32_t size; /* Added by v2.6.32-rc1~679^2~72 */
        uint32_t reserved2[1];
        union {
                int32_t value;
                int64_t value64;
                char * string;             /**< Added by v2.6.32-rc1~679^2~72 */
                uint8_t * p_u8;            /**< Added by v3.17-rc1~112^2~343 */
                uint16_t * p_u16;          /**< Added by v3.17-rc1~112^2~343 */
                uint32_t * p_u32;          /**< Added by v3.17-rc1~112^2~112 */
                struct_v4l2_area * p_area; /**< Added by v5.5-rc1~143^2^2~51 */
                void * ptr;                /**< Added by v3.17-rc1~112^2~363 */
        };
} ATTRIBUTE_PACKED struct_v4l2_ext_control;

/** Added by Linux commit v2.6.18-rc1~862^2~18 */
typedef struct {
        union {
		uint32_t ctrl_class;
		uint32_t which;
	};
        uint32_t count;
	uint32_t error_idx;
	int32_t  request_fd; /**< Added by Linux commit v4.20-rc1~51^2~44 */
	uint32_t reserved[1];
	struct_v4l2_ext_control * controls;
} struct_v4l2_ext_controls;


typedef struct {
	uint32_t width;
	uint32_t height;
} struct_v4l2_frmsize_discrete;

typedef struct {
	uint32_t min_width;
	uint32_t max_width;
	uint32_t step_width;
	uint32_t min_height;
	uint32_t max_height;
	uint32_t step_height;
} struct_v4l2_frmsize_stepwise;

/** Added by Linux commit v2.6.19-rc1~183 */
typedef struct {
	uint32_t index;
	uint32_t pixel_format;
	uint32_t type; /**< enum v4l2_frmsizetypes */
	union {
		struct_v4l2_frmsize_discrete discrete;
		struct_v4l2_frmsize_stepwise stepwise;
	};
	uint32_t reserved[2];
} struct_v4l2_frmsizeenum;

typedef struct {
	struct v4l2_fract min;
	struct v4l2_fract max;
	struct v4l2_fract step;
} struct_v4l2_frmival_stepwise;

typedef struct {
	uint32_t index;
	uint32_t pixel_format;
	uint32_t width;
	uint32_t height;
	uint32_t type; /**< enum v4l2_frmivaltypes */
	union {
		struct v4l2_fract		discrete;
		struct_v4l2_frmival_stepwise	stepwise;
	};
	uint32_t reserved[2];
} struct_v4l2_frmivalenum;


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
