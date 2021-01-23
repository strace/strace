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

# ifndef GPIO_V2_LINES_MAX
#  define GPIO_V2_LINES_MAX 64
# endif

# ifndef GPIO_V2_LINE_NUM_ATTRS_MAX
#  define GPIO_V2_LINE_NUM_ATTRS_MAX 10
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

typedef struct {
	uint64_t bits;
	uint64_t mask;
} struct_gpio_v2_line_values;

typedef struct {
	uint32_t id;
	uint32_t padding;
	union {
		uint32_t debounce_period_us;
		uint64_t flags;
		uint64_t values;
	};
} struct_gpio_v2_line_attribute;

typedef struct {
	struct_gpio_v2_line_attribute attr;
	uint64_t mask;
} struct_gpio_v2_line_config_attribute;

typedef struct {
	uint64_t flags;
	uint32_t num_attrs;
	uint32_t padding[5];
	struct_gpio_v2_line_config_attribute attrs[GPIO_V2_LINE_NUM_ATTRS_MAX];
} struct_gpio_v2_line_config;

typedef struct {
	uint32_t offsets[GPIO_V2_LINES_MAX];
	char consumer[GPIO_MAX_NAME_SIZE];
	struct_gpio_v2_line_config config;
	uint32_t num_lines;
	uint32_t event_buffer_size;
	uint32_t padding[5];
	int32_t fd;
} struct_gpio_v2_line_request;

typedef struct {
	char name[GPIO_MAX_NAME_SIZE];
	char consumer[GPIO_MAX_NAME_SIZE];
	uint32_t offset;
	uint32_t num_attrs;
	uint64_t flags;
	struct_gpio_v2_line_attribute attrs[GPIO_V2_LINE_NUM_ATTRS_MAX];
	uint32_t padding[4];
} struct_gpio_v2_line_info;

#endif /* STRACE_TYPES_GPIO_H */
