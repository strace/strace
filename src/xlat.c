/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"
#include <stdarg.h>

static enum xlat_style
get_xlat_style(enum xlat_style style)
{
	if (xlat_verbose(style) == XLAT_STYLE_DEFAULT)
		return style | xlat_verbosity;

	return style;
}

static const char *
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

static void
print_xlat_val(uint64_t val, enum xlat_style style)
{
	tprints_string(sprint_xlat_val(val, style));
}

static int
xlat_bsearch_compare(const void *a, const void *b)
{
	const uint64_t val1 = *(const uint64_t *) a;
	const uint64_t val2 = ((const struct xlat_data *) b)->val;
	return (val1 > val2) ? 1 : (val1 < val2) ? -1 : 0;
}

const char *
xlookup(const struct xlat *x, const uint64_t val)
{
	const struct xlat_data *e;

	if (!x || !x->data)
		return NULL;

	switch (x->type) {
	case XT_NORMAL:
		for (size_t idx = 0; idx < x->size; idx++)
			if (x->data[idx].val == val)
				return x->data[idx].str;
		break;

	case XT_SORTED:
		e = bsearch((const void *) &val,
			    x->data, x->size,
			    sizeof(x->data[0]),
			    xlat_bsearch_compare);
		if (e)
			return e->str;
		break;

	case XT_INDEXED:
		if (val < x->size) {
			if (val == x->data[val].val)
				return x->data[val].str;
			if (x->data[val].val == 0)
				break; /* a hole in the index */
			error_func_msg("Unexpected xlat value %" PRIu64
				       " at index %" PRIu64 " (str %s)",
				       x->data[val].val, val,
				       x->data[val].str);
		}
		break;

	default:
		error_func_msg("Invalid xlat type: %#x", x->type);
	}

	return NULL;
}

static const char *
xlat_search_eq_or_less(const struct xlat *xlat, uint64_t *val)
{
	const struct xlat_data *base = xlat->data;
	const struct xlat_data *cur = xlat->data;
	size_t nmemb = xlat->size;

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
		if (cur > xlat->data)
			cur--;
		else
			return NULL;
	}

	*val = cur->val;
	return cur->str;
}

const char *
xlookup_le(const struct xlat *xlat, uint64_t *val)
{
	if (!xlat || !xlat->data)
		return NULL;

	switch (xlat->type) {
	case XT_SORTED:
		return xlat_search_eq_or_less(xlat, val);

#if 0 /* enable when used */
	case XT_NORMAL: {
		uint64_t best_hit = 0;
		const char *str = NULL;

		for (size_t idx = 0; idx < xlat->size; idx++) {
			if (xlat->data[idx].val == *val)
				return xlat->data[idx].str;

			if (xlat->data[idx].val < *val
			    && xlat->data[idx].val > best_hit) {
				best_hit = xlat->data[idx].val;
				str = xlat->data[idx].str;
			}
		}

		*val = best_hit;
		return str;
	}

	case XT_INDEXED: {
		size_t idx = *val;

		if (idx >= xlat->size) {
			if (!xlat->size)
				return NULL;

			idx = xlat->size - 1;
		}

		do {
			if (idx == xlat->data[idx].val && xlat->data[idx].str) {
				*val = idx;
				return xlat->data[idx].str;
			}
			if (xlat->data[idx].val == 0)
				continue; /* a hole in the index */
			error_func_msg("Unexpected xlat value %" PRIu64
				       " at index %zu (str %s)",
				       xlat->data[idx].val, idx,
				       xlat->data[idx].str);
		} while (idx--);
		return NULL;
	}
#endif

	default:
		error_func_msg("Invalid xlat type: %#x", xlat->type);
	}

	return NULL;
}

/**
 * Print an entry in struct xlat table, if it is there.
 *
 * @param val   A value to search a literal representation for.
 * @param dflt  A string (encased in comment syntax) which is to be emitted
 *              if no appropriate xlat value has been found.
 * @param style A style which is to be used for xlat value printing.
 * @param xlat  (and the following arguments) Pointers xlat description
 *              structures.
 *              The last argument should be NULL.
 * @return      1 if an appropriate xlat value has been found, 0 otherwise.
 */
int
printxvals_ex(const uint64_t val, const char *dflt, enum xlat_style style,
	      const struct xlat *xlat, ...)
{
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		print_xlat_val(val, style);
		return 0;
	}

	const char *str = NULL;
	va_list args;

	va_start(args, xlat);

	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		str = xlookup(xlat, val);

		if (str) {
			if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
				print_xlat_val(val, style);
				tprints_comment(str);
			} else {
				tprints_string(str);
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
	       char sep, enum xlat_style style)
{
	static char outstr[1024];
	char *outptr;
	int found = 0;

	outptr = stpcpy(outstr, prefix);
	style = get_xlat_style(style);

	if (xlat_verbose(style) == XLAT_STYLE_RAW) {
		if (!flags || ((style & SPFF_AUXSTR_MODE) && !sep))
			return NULL;

		if (sep)
			*outptr++ = sep;
		outptr = xappendstr(outstr, outptr, "%s",
				    sprint_xlat_val(flags, style));

		return outstr;
	}

	if (flags == 0 && xlat->data->val == 0 && xlat->data->str) {
		if (sep)
			*outptr++ = sep;
		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE &&
		    !(style & SPFF_AUXSTR_MODE)) {
			outptr = xappendstr(outstr, outptr, "0 /* %s */",
					    xlat->data->str);
		} else {
			strcpy(outptr, xlat->data->str);
		}

		return outstr;
	}

	if (xlat_verbose(style) == XLAT_STYLE_VERBOSE && flags &&
	    !(style & SPFF_AUXSTR_MODE)) {
		if (sep) {
			*outptr++ = sep;
			sep = '\0';
		}
		outptr = xappendstr(outstr, outptr, "%s",
				    sprint_xlat_val(flags, style));
	}

	for (size_t idx = 0; flags && idx < xlat->size; idx++) {
		if (xlat->data[idx].val && xlat->data[idx].str
		    && (flags & xlat->data[idx].val) == xlat->data[idx].val) {
			if (sep) {
				*outptr++ = sep;
			} else if (xlat_verbose(style) == XLAT_STYLE_VERBOSE &&
				   !(style & SPFF_AUXSTR_MODE)) {
				outptr = stpcpy(outptr, " /* ");
			}

			outptr = stpcpy(outptr, xlat->data[idx].str);
			found = 1;
			sep = '|';
			flags &= ~xlat->data[idx].val;
		}
	}

	if (flags) {
		if (sep)
			*outptr++ = sep;
		if (found || (xlat_verbose(style) != XLAT_STYLE_VERBOSE &&
			      (!(style & SPFF_AUXSTR_MODE) || sep)))
			outptr = xappendstr(outstr, outptr, "%s",
					    sprint_xlat_val(flags, style));
	} else {
		if (!found)
			return NULL;
	}

	if (found && xlat_verbose(style) == XLAT_STYLE_VERBOSE &&
	    !(style & SPFF_AUXSTR_MODE))
		outptr = stpcpy(outptr, " */");

	return outptr != outstr ? outstr : NULL;
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

	bool need_comment = false;
	unsigned int n = 0;
	va_list args;

	if (xlat_verbose(style) == XLAT_STYLE_VERBOSE) {
		need_comment = true;
		if (flags)
			print_xlat_val(flags, style);
	}

	va_start(args, xlat);
	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		for (size_t idx = 0; (flags || !n) && idx < xlat->size; ++idx) {
			uint64_t v = xlat->data[idx].val;
			if (xlat->data[idx].str
			    && ((flags == v) || (v && (flags & v) == v))) {
				if (xlat_verbose(style) == XLAT_STYLE_VERBOSE
				    && !flags)
					PRINT_VAL_U(0);
				if (n++)
					tprint_flags_or();
				else if (need_comment)
					tprint_comment_begin();
				tprints_string(xlat->data[idx].str);
				flags &= ~v;
			}
			if (!flags)
				break;
		}
	}
	va_end(args);

	if (n) {
		if (flags) {
			tprint_flags_or();
			print_xlat_val(flags, style);
			n++;
		}

		if (xlat_verbose(style) == XLAT_STYLE_VERBOSE)
			tprint_comment_end();
	} else {
		if (flags) {
			if (xlat_verbose(style) != XLAT_STYLE_VERBOSE)
				print_xlat_val(flags, style);
			tprints_comment(dflt);
		} else {
			if (dflt)
				PRINT_VAL_U(0);
		}
	}

	return n;
}

void
print_xlat_ex(const uint64_t val, const char *str, uint32_t style)
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
				tprints_string(str);
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
