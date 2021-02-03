/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_XLAT_H
# define STRACE_XLAT_H

# include <stdint.h>

enum xlat_type {
	XT_NORMAL,
	XT_SORTED,
	XT_INDEXED,
};

enum xlat_style {
	/**
	 * Special value that is used for passing to *print{xval,flags}*_ex
	 * routines that indicates that no overriding of user-supplied xlat
	 * verbosity/formatting configuration is intended.
	 */
	XLAT_STYLE_DEFAULT = 0,

	/** Print xlat value as is without xlat processing */
	XLAT_STYLE_RAW     = 1 << 0,
	/**
	 * Historic strace style, process xlat and print the result (xlat
	 * constant name/combination of flags), raw number only if nothing is
	 * found.
	 */
	XLAT_STYLE_ABBREV  = 1 << 1,
	/** Always print both raw number and xlat processing result. */
	XLAT_STYLE_VERBOSE = XLAT_STYLE_RAW | XLAT_STYLE_ABBREV,

# define XLAT_STYLE_FORMAT_SHIFT   2
# define XLAT_STYLE_VERBOSITY_MASK ((1 << XLAT_STYLE_FORMAT_SHIFT) - 1)

	XLAT_STYLE_FMT_X   = 0 << XLAT_STYLE_FORMAT_SHIFT,
	XLAT_STYLE_FMT_U   = 1 << XLAT_STYLE_FORMAT_SHIFT,
	XLAT_STYLE_FMT_D   = 2 << XLAT_STYLE_FORMAT_SHIFT,

# define XLAT_STYLE_FORMAT_MASK    (3 << XLAT_STYLE_FORMAT_SHIFT)

# define XLAT_STYLE_SPEC_BITS (XLAT_STYLE_FORMAT_SHIFT + 2)
# define XLAT_STYLE_MASK ((1 << XLAT_STYLE_SPEC_BITS) - 1)
};

struct xlat_data {
	uint64_t val;
	const char *str;
};

struct xlat {
	const struct xlat_data *data;
	size_t flags_strsz;
	uint32_t size;
	enum xlat_type type;
	uint64_t flags_mask;
};

# define XLAT(val)			{ (unsigned)(val), #val }
# define XLAT_PAIR(val, str)		{ (unsigned)(val), str  }
# define XLAT_TYPE(type, val)		{     (type)(val), #val }
# define XLAT_TYPE_PAIR(type, val, str)	{     (type)(val), str  }

#endif /* !STRACE_XLAT_H */
