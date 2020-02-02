/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_IO_URING_H
# define STRACE_TYPES_IO_URING_H

#ifdef HAVE_LINUX_IO_URING_H
# include <linux/io_uring.h>
#endif

typedef struct {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t flags;
	uint32_t dropped;
	uint32_t array;
	uint32_t resv1;
	uint64_t resv2;
} struct_io_sqring_offsets;

typedef struct {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t overflow;
	uint32_t cqes;
	uint64_t resv[2];
} struct_io_cqring_offsets;

typedef struct {
	uint32_t sq_entries;
	uint32_t cq_entries;
	uint32_t flags;
	uint32_t sq_thread_cpu;
	uint32_t sq_thread_idle;
	uint32_t features;
	uint32_t wq_fd;
	uint32_t resv[3];
	struct_io_sqring_offsets sq_off;
	struct_io_cqring_offsets cq_off;
} struct_io_uring_params;

typedef struct {
	uint32_t offset;
	uint32_t resv;
	uint64_t /* int * */ fds;
} struct_io_uring_files_update;

#endif /* !STRACE_TYPES_IO_URING_H */
