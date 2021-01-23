/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_GPIO_H
# define STRACE_TYPES_GPIO_H

# include <stdint.h>
# include <linux/ioctl.h>
# ifdef HAVE_LINUX_GPIO_H
#  include <linux/gpio.h>
# endif

# ifndef GPIO_MAX_NAME_SIZE
#  define GPIO_MAX_NAME_SIZE 32
# endif

# ifndef GPIOHANDLES_MAX
#  define GPIOHANDLES_MAX 64
# endif

typedef struct {
	char name[GPIO_MAX_NAME_SIZE];
	char label[GPIO_MAX_NAME_SIZE];
	uint32_t lines;
} struct_gpiochip_info;

typedef struct {
	uint32_t line_offset;
	uint32_t flags;
	char name[GPIO_MAX_NAME_SIZE];
	char consumer[GPIO_MAX_NAME_SIZE];
} struct_gpioline_info;

typedef struct {
	uint32_t lineoffsets[GPIOHANDLES_MAX];
	uint32_t flags;
	uint8_t default_values[GPIOHANDLES_MAX];
	char consumer_label[GPIO_MAX_NAME_SIZE];
	uint32_t lines;
	int fd;
} struct_gpiohandle_request;

typedef struct {
	uint32_t lineoffset;
	uint32_t handleflags;
	uint32_t eventflags;
	char consumer_label[GPIO_MAX_NAME_SIZE];
	int fd;
} struct_gpioevent_request;

typedef struct {
	uint8_t values[GPIOHANDLES_MAX];
} struct_gpiohandle_data;

typedef struct {
	uint32_t flags;
	uint8_t default_values[GPIOHANDLES_MAX];
	uint32_t padding[4];
} struct_gpiohandle_config;

#endif /* STRACE_TYPES_GPIO_H */
