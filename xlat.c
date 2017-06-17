/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2017 The strace developers.
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
#include <stdarg.h>

const char *
xlookup(const struct xlat *xlat, const uint64_t val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
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
	const struct xlat *e =
		bsearch((const void *) &val,
			xlat, nmemb, sizeof(*xlat), xlat_bsearch_compare);

	return e ? e->str : NULL;
}

/**
 * Print entry in struct xlat table, if there.
 *
 * @param val  Value to search a literal representation for.
 * @param dflt String (abbreviated in comment syntax) which should be emitted
 *             if no appropriate xlat value has been found.
 * @param xlat (And the following arguments) Pointers to arrays of xlat values.
 *             The last argument should be NULL.
 * @return     1 if appropriate xlat value has been found, 0 otherwise.
 */
int
printxvals(const uint64_t val, const char *dflt, const struct xlat *xlat, ...)
{
	va_list args;

	va_start(args, xlat);
	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		const char *str = xlookup(xlat, val);

		if (str) {
			tprints(str);
			va_end(args);
			return 1;
		}
	}
	/* No hits -- print raw # instead. */
	tprintf("%#" PRIx64, val);
	tprints_comment(dflt);

	va_end(args);

	return 0;
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
 * @return          1 if appropriate xlat value has been found, 0
 *                  otherwise.
 */
int
printxval_searchn(const struct xlat *xlat, size_t xlat_size, uint64_t val,
	const char *dflt)
{
	const char *s = xlat_search(xlat, xlat_size, val);

	if (s) {
		tprints(s);
		return 1;
	}

	tprintf("%#" PRIx64, val);
	tprints_comment(dflt);

	return 0;
}

/*
 * Interpret `xlat' as an array of flags
 * print the entries whose bits are on in `flags'
 */
void
addflags(const struct xlat *xlat, uint64_t flags)
{
	for (; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			tprintf("|%s", xlat->str);
			flags &= ~xlat->val;
		}
	}
	if (flags) {
		tprintf("|%#" PRIx64, flags);
	}
}

/*
 * Interpret `xlat' as an array of flags.
 * Print to static string the entries whose bits are on in `flags'
 * Return static string.
 */
const char *
sprintflags(const char *prefix, const struct xlat *xlat, uint64_t flags)
{
	static char outstr[1024];
	char *outptr;
	int found = 0;

	outptr = stpcpy(outstr, prefix);

	if (flags == 0 && xlat->val == 0 && xlat->str) {
		strcpy(outptr, xlat->str);
		return outstr;
	}

	for (; xlat->str; xlat++) {
		if (xlat->val && (flags & xlat->val) == xlat->val) {
			if (found)
				*outptr++ = '|';
			outptr = stpcpy(outptr, xlat->str);
			found = 1;
			flags &= ~xlat->val;
			if (!flags)
				break;
		}
	}
	if (flags) {
		if (found)
			*outptr++ = '|';
		outptr += sprintf(outptr, "%#" PRIx64, flags);
	}

	return outstr;
}

int
printflags_ex(uint64_t flags, const char *dflt, const struct xlat *xlat, ...)
{
	unsigned int n = 0;
	va_list args;

	va_start(args, xlat);
	for (; xlat; xlat = va_arg(args, const struct xlat *)) {
		for (; (flags || !n) && xlat->str; ++xlat) {
			if ((flags == xlat->val) ||
			    (xlat->val && (flags & xlat->val) == xlat->val)) {
				tprintf("%s%s", (n++ ? "|" : ""), xlat->str);
				flags &= ~xlat->val;
			}
			if (!flags)
				break;
		}
	}
	va_end(args);

	if (n) {
		if (flags) {
			tprintf("|%#" PRIx64, flags);
			n++;
		}
	} else {
		if (flags) {
			tprintf("%#" PRIx64, flags);
			tprints_comment(dflt);
		} else {
			if (dflt)
				tprints("0");
		}
	}

	return n;
}
