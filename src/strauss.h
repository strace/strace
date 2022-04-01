/*
 * Strauss awareness interface declarations.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STRAUSS_H
#define STRACE_STRAUSS_H

enum { STRAUSS_START_VERBOSITY = 5 };

extern const size_t strauss_lines;

extern void print_strauss(size_t verbosity);

#endif /* STRACE_STRAUSS_H */
