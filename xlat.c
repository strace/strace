/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "xstring.h"
#include <stdarg.h>

static inline enum xlat_style
get_xlat_style(enum xlat_style style)
{
	if (xlat_verbose(style) == XLAT_STYLE_DEFAULT)
		return style | xlat_verbosity;

	return style;
}

static inline const char *
sprint_xlat_val(uint64_t val, enum xlat_style style)
{
	static char buf[sizeof(val) * 3];

	switch (xlat_format(style)) {
	case XLAT_STYLE_FMT_D:
		xsprintf(buf, "%" PRId64, val);
		break;

	case XLAT_STYLE_FMT_U:
		xsprintf(buf, "%" PRIu64, val);
		break;

	case XLAT_STYLE_FMT_X:
		xsprintf(buf, "%#" PRIx64, val);
		break;
	}

	return buf;
}

static inline void
print_xlat_val(uint64_t val, enum xlat_style style)
{
	tprints(sprint_xlat_val(val, style));
}

const char *
xlookup(const struct xlat *xlat, const uint64_t val)
{
	static const struct xlat *pos;

	if (xlat)
		pos = xlat;

	for (; pos->str != NULL; pos++)
		if (pos->val == val)
			return pos->str;
	return NULL;
}

static int
xlat_bsearch_compare(const void *a, const void *b)
{
	const uint64_t val1 = *(const uint64_t *) a;
	const uint64_t val2 = ((const struct xlat *) b)->val;
	return (val1 > val2) ? 1 : (val1 < val2) ? -1 : 0;
}

const char *
xlat_search(const struct xlat *xlat, const size_t nmemb, const uint64_t val)
{
	static const struct xlat *pos;
	static size_t memb_left;

	if (xlat) {
		pos = xlat;
		memb_left = nmemb;
	}

	const struct xlat *e =
		bsearch((const void *) &val,
			pos, memb_left, sizeof(*pos), xlat_bsearch_compare);

	if (e) {
		memb_left -= e - pos;
		return e->str;
	} else {
		return NULL;
	}
}

const char *
xlat_search_eq_or_less(const struct xlat *xlat, size_t nmemb, uint64_t *val)
{
	const struct xlat *base = xlat;
	const struct xlat *cur = xlat;

	for (; nmemb > 0; nmemb >>= 1) {
		cur = base + (nmemb >> 1);

		if (*val == cur->val)
			return cur->str;

		if (*val > cur->val) {
			base = cur + 1;
			nmemb--;
		}
	}

	if (*val < cur->val) {
		if (cur > xlat)
			cur--;
		else
			return NULL;
	}

	*val = cur->val;
	return cur->str;
}

/**
 * Print entry in struct xlat table, if there.
 *
 * @param val   Value to search a literal representation for.
 * @param dflt  String (abbreviated in comment syntax) which should be emitted
 *              if no appropriate xlat value has been found.
 * @param style Style in which xlat value should be printed.
 * @param xlat  (And the following arguments) Pointers to arrays of xlat values.
 *              The last argument should be NULL.
 * @return      1 if appropriate xlat value has been found, 0 otherwise.
 */
int
printxvals_ex(const uint64_t val, const char *dflt, enum xlat_style style,
	      const struct xlat *xlat, ...)
{
	static const struct xlat *last;

	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		print_xlat_val(val, style);
		return 0;
	}

	const char *str = NULL;
	va_list args;

	va_start(args, xlat);

	if (!xlat)
		xlat = last;

	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		last = xlat;

		str = xlookup(xlat, val);

		if (str) {
			if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
				print_xlat_val(val, style);
				tprints_comment(str);
			} else {
				tprints(str);
			}

			goto printxvals_ex_end;
		}
	}

	/* No hits -- print raw # instead. */
	print_xlat_val(val, style);
	tprints_comment(dflt);

printxvals_ex_end:
	va_end(args);

	return !!str;
}

int
sprintxval_ex(char *const buf, const size_t size, const struct xlat *const x,
	      const unsigned int val, const char *const dflt,
	      enum xlat_style style)
{
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW)
		return xsnprintf(buf, size, "%s", sprint_xlat_val(val, style));

	const char *const str = xlookup(x, val);

	if (str) {
		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE)
			return xsnprintf(buf, size, "%s /* %s */",
					 sprint_xlat_val(val, style), str);
		else
			return xsnprintf(buf, size, "%s", str);
	}
	if (dflt)
		return xsnprintf(buf, size, "%s /* %s */",
				 sprint_xlat_val(val, style), dflt);

	return xsnprintf(buf, size, "%s", sprint_xlat_val(val, style));
}

/**
 * Print entry in sorted struct xlat table, if it is there.
 *
 * @param xlat      Pointer to an array of xlat values (not terminated with
 *                  XLAT_END).
 * @param xlat_size Number of xlat elements present in array (usually ARRAY_SIZE
 *                  if array is declared in the unit's scope and not
 *                  terminated with XLAT_END).
 * @param val       Value to search literal representation for.
 * @param dflt      String (abbreviated in comment syntax) which should be
 *                  emitted if no appropriate xlat value has been found.
 * @param style     Style in which xlat value should be printed.
 * @param fn        Search function.
 * @return          1 if appropriate xlat value has been found, 0
 *                  otherwise.
 */
static int
printxval_sized(const struct xlat *xlat, size_t xlat_size, uint64_t val,
		const char *dflt, enum xlat_style style,
		const char *(* fn)(const struct xlat *, size_t, uint64_t))
{
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		print_xlat_val(val, style);
		return 0;
	}

	const char *s = fn(xlat, xlat_size, val);

	if (s) {
		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
			print_xlat_val(val, style);
			tprints_comment(s);
		} else {
			tprints(s);
		}
		return 1;
	}

	print_xlat_val(val, style);
	tprints_comment(dflt);

	return 0;
}

int
printxval_searchn_ex(const struct xlat *xlat, size_t xlat_size, uint64_t val,
		     const char *dflt, enum xlat_style style)
{
	return printxval_sized(xlat, xlat_size, val, dflt, style,
				  xlat_search);
}

const char *
xlat_idx(const struct xlat *xlat, size_t nmemb, uint64_t val)
{
	static const struct xlat *pos;
	static size_t memb_left;

	if (xlat) {
		pos = xlat;
		memb_left = nmemb;
	}

	if (val >= memb_left)
		return NULL;

	if (val != pos[val].val) {
		error_func_msg("Unexpected xlat value %" PRIu64
			       " at index %" PRIu64,
			       pos[val].val, val);
		return NULL;
	}

	return pos[val].str;
}

int
printxval_indexn_ex(const struct xlat *xlat, size_t xlat_size, uint64_t val,
		    const char *dflt, enum xlat_style style)
{
	return printxval_sized(xlat, xlat_size, val, dflt, style, xlat_idx);
}

/*
 * Interpret `xlat' as an array of flags.
 * Print to static string the entries whose bits are on in `flags'
 * Return static string.  If 0 is provided as flags, and there is no flag that
 * has the value of 0 (it should be the first in xlat table), return NULL.
 *
 * Expected output:
 * +------------+------------+---------+------------+
 * | flags != 0 | xlat found | style   | output     |
 * +------------+------------+---------+------------+
 * | false      | (any)      | raw     | <none>     |
 * | true       | (any)      | raw     | VAL        |
 * +------------+------------+---------+------------+
 * | false      | false      | abbrev  | <none>     |
 * | true       | false      | abbrev  | VAL        |
 * | (any)      | true       | abbrev  | XLAT       |
 * +------------+------------+---------+------------+
 * | false      | false      | verbose | <none>     |
 * | true       | false      | verbose | VAL        |
 * | (any)      | true       | verbose | VAL (XLAT) |
 * +------------+------------+---------+------------+
 */
const char *
sprintflags_ex(const char *prefix, const struct xlat *xlat, uint64_t flags,
	       enum xlat_style style)
{
	static char outstr[1024];
	char *outptr;
	int found = 0;

	outptr = stpcpy(outstr, prefix);
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		if (!flags)
			return NULL;

		outptr = xappendstr(outstr, outptr, "%s",
				    sprint_xlat_val(flags, style));

		return outstr;
	}

	if (flags == 0 && xlat->val == 0 && xlat->str) {
		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
			outptr = xappendstr(outstr, outptr, "0 /* %s */",
					    xlat->str);
		} else {
			strcpy(outptr, xlat->str);
		}

		return outstr;
	}

	if (xlat_verbose(style) == XLAT_STYLE_VERBOSE && flags)
		outptr = xappendstr(outstr, outptr, "%s",
				    sprint_xlat_val(flags, style));

	for (; flags && xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			if (found)
				*outptr++ = '|';
			else if (xlat_verbose(style) == XLAT_STYLE_VERBOSE)
				outptr = stpcpy(outptr, " /* ");

			outptr = stpcpy(outptr, xlat->str);
			found = 1;
			flags &= ~xlat->val;
		}
	}

	if (flags) {
		if (found)
			*outptr++ = '|';
		if (found || xlat_verbose(style) != XLAT_STYLE_VERBOSE)
			outptr = xappendstr(outstr, outptr, "%s",
					    sprint_xlat_val(flags, style));
	} else {
		if (!found)
			return NULL;
	}

	if (found && xlat_verbose(style) == XLAT_STYLE_VERBOSE)
		outptr = stpcpy(outptr, " */");

	return outstr;
}

/**
 * Print flags from multiple xlat tables.
 *
 * Expected output:
 * +------------+--------------+------------+---------+------------+
 * | flags != 0 | dflt != NULL | xlat found | style   | output     |
 * +------------+--------------+------------+---------+------------+
 * | false      | false        | (any)      | raw     | <none>     |
 * | false      | true         | (any)      | raw     | VAL        |
 * | true       | (any)        | (any)      | raw     | VAL        |
 * +------------+--------------+------------+---------+------------+
 * | false      | false        | false      | abbrev  | <none>     |
 * | false      | true         | false      | abbrev  | VAL        |
 * | true       | false        | false      | abbrev  | VAL        |
 * | true       | true         | false      | abbrev  | VAL (DFLT) |
 * | (any)      | (any)        | true       | abbrev  | XLAT       |
 * +------------+--------------+------------+---------+------------+
 * | false      | false        | false      | verbose | <none>     |
 * | false      | true         | false      | verbose | VAL        |
 * | true       | false        | false      | verbose | VAL        |
 * | true       | true         | false      | verbose | VAL (DFLT) |
 * | (any)      | (any)        | true       | verbose | VAL (XLAT) |
 * +------------+--------------+------------+---------+------------+
 */
int
printflags_ex(uint64_t flags, const char *dflt, enum xlat_style style,
	      const struct xlat *xlat, ...)
{
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		if (flags || dflt) {
			print_xlat_val(flags, style);
			return 1;
		}

		return 0;
	}

	const char *init_sep = "";
	unsigned int n = 0;
	va_list args;

	if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
		init_sep = " /* ";
		if (flags)
			print_xlat_val(flags, style);
	}

	va_start(args, xlat);
	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		for (; (flags || !n) && xlat->str; ++xlat) {
			if ((flags == xlat->val) ||
			    (xlat->val && (flags & xlat->val) == xlat->val)) {
				if (xlat_verbose(style) == XLAT_STYLE_VERBOSE
				    && !flags)
					tprints("0");
				tprintf("%s%s",
					(n++ ? "|" : init_sep), xlat->str);
				flags &= ~xlat->val;
			}
			if (!flags)
				break;
		}
	}
	va_end(args);

	if (n) {
		if (flags) {
			tprints("|");
			print_xlat_val(flags, style);
			n++;
		}

		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE)
			tprints(" */");
	} else {
		if (flags) {
			if (xlat_verbose(style) != XLAT_STYLE_VERBOSE)
				print_xlat_val(flags, style);
			tprints_comment(dflt);
		} else {
			if (dflt)
				tprints("0");
		}
	}

	return n;
}

void
print_xlat_ex(const uint64_t val, const char *str, enum xlat_style style)
{
	bool default_str = style & PXF_DEFAULT_STR;
	style = get_xlat_style(style);

	switch (xlat_verbose(style)) {
	case XLAT_STYLE_ABBREV:
		if (str) {
			if (default_str) {
				print_xlat_val(val, style);
				tprints_comment(str);
			} else {
				tprints(str);
			}
			break;
		}
		ATTRIBUTE_FALLTHROUGH;

	case XLAT_STYLE_RAW:
		print_xlat_val(val, style);
		break;

	default:
		error_func_msg("Unexpected style value of %#x", style);
		ATTRIBUTE_FALLTHROUGH;

	case XLAT_STYLE_VERBOSE:
		print_xlat_val(val, style);
		tprints_comment(str);
	}
}

void
printxval_dispatch_ex(const struct xlat *xlat, size_t xlat_size, uint64_t val,
		      const char *dflt, enum xlat_type xt,
		      enum xlat_style style)
{
	switch (xt) {
	case XT_NORMAL:
		printxvals_ex(val, dflt, style, xlat, NULL);
		break;

	case XT_SORTED:
		printxval_searchn_ex(xlat, xlat_size, val, dflt, style);
		break;

	case XT_INDEXED:
		printxval_indexn_ex(xlat, xlat_size, val, dflt, style);
		break;
	}
}
