/*
 * Interface to the auxiliary-clock helpers used by the time decoder.
 *
 * Only built-in C types are used here, so this header needs no includes.
 * Keeping both libc headers and <linux/time.h> out lets the time decoder
 * use libc time definitions without mixing them with Linux UAPI ones.
 *
 * Copyright (c) 2026 Thomas Weißschuh <thomas.weissschuh@linutronix.de>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

int
is_auxiliary_clock(int clockid);

unsigned int
auxiliary_clock_num(int clockid);
