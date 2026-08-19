/*
 * Hyperlink support.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "hyperlink.h"

#include <string.h>
#include <strings.h>

enum hyperlink_mode_t hyperlink_mode = HYPERLINK_NEVER;

void
hyperlink_init(int out_fd, bool output_separately)
{
	if (hyperlink_mode != HYPERLINK_AUTO)
		return;

	/* Resolve auto once, so printing only checks ALWAYS or NEVER. */
	hyperlink_mode = HYPERLINK_NEVER;
	if (output_separately || !isatty(out_fd))
		return;

	const char *term = getenv("TERM");
	if (term && *term && strcasecmp(term, "dumb") &&
	    strcasecmp(term, "unknown"))
		hyperlink_mode = HYPERLINK_ALWAYS;
}

/*
 * Build a manual page URL on man7.org for a given system call name.
 * Returns dynamically allocated string, or NULL if not applicable.
 */
char *
make_hyperlink_url_for_syscall(const char *name)
{
	if (!name || *name == '?' || !strcmp(name, "system call") ||
	    !strncmp(name, "syscall_0x", sizeof("syscall_0x") - 1))
		return NULL;

	/*
	 * Strip architecture/ABI prefixes (such as "n32:", "n64:", "o32:"
	 * on MIPS) when constructing the manual page URL.
	 */
	const char *colon = strchr(name, ':');
	const char *base_name = colon ? colon + 1 : name;

	/*
	 * Strip variant suffixes (such as "#64" on x32) when constructing
	 * the manual page URL.
	 */
	const char *hash = strchr(base_name, '#');
	size_t len = hash ? (size_t) (hash - base_name) : strlen(base_name);

	return xasprintf("https://man7.org/linux/man-pages/man2/%.*s.2.html",
			 (int) len, base_name);
}

/* OSC 8 ; params ; URI ST text OSC 8 ;; ST */
static const char osc8_prefix[] = "\033]8;;";
static const char osc8_st[] = "\033\\";

/* Do not count escape sequences in current_tcp->curcol. */
void
tprint_hyperlink_begin(const char *url)
{
	if (url) {
		tprints_string_uncol(osc8_prefix);
		tprints_string_uncol(url);
		tprints_string_uncol(osc8_st);
	}
}

void
tprint_hyperlink_end(void)
{
	tprints_string_uncol(osc8_prefix);
	tprints_string_uncol(osc8_st);
}

void
fprint_hyperlink_begin(FILE *outf, const char *url)
{
	fputs(osc8_prefix, outf);
	fputs(url, outf);
	fputs(osc8_st, outf);
}

void
fprint_hyperlink_end(FILE *outf)
{
	fputs(osc8_prefix, outf);
	fputs(osc8_st, outf);
}
