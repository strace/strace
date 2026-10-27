/*
 * Dynamic xlat table interface.
 *
 * Copyright (c) 2017-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_DYXLAT_H
# define STRACE_DYXLAT_H

# include <stddef.h>
# include <stdint.h>
# include "xlat.h"

struct dyxlat;

struct dyxlat *dyxlat_alloc(size_t nmemb, enum xlat_type type);
void dyxlat_free(struct dyxlat *);
const struct xlat *dyxlat_get(const struct dyxlat *);
void dyxlat_add_pair(struct dyxlat *, uint64_t val, const char *str,
		     size_t len);

#endif /* !STRACE_DYXLAT_H */
