/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_EVDEV_H
# define STRACE_TYPES_EVDEV_H

# ifdef HAVE_LINUX_INPUT_H
#  include <linux/input.h>
# endif

typedef struct {
	int32_t value;
	int32_t minimum;
	int32_t maximum;
	int32_t fuzz;
	int32_t flat;
	int32_t resolution; /**< Added by Linux commit v2.6.31-rc1~100^2~1 */
} struct_input_absinfo;

/** Added by Linux commit v2.6.37-rc1~5^2~3^2~47 */
typedef struct {
	uint8_t  flags;
	uint8_t  len;
	uint16_t index;
	uint32_t keycode;
	uint8_t  scancode[32];
} struct_input_keymap_entry;

/** Added by Linux commit v4.4-rc1~11^2~3^2~2 */
typedef struct {
	uint32_t type;
	uint32_t codes_size;
	uint64_t codes_ptr;
} struct_input_mask;

#endif /* !STRACE_TYPES_EVDEV_H */
