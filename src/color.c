/*
 * Copyright (c) 2026 Jonas Jelten <jj@sft.lol>
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "color.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifdef HAVE_TGETNUM
# include <termcap.h>
#endif

bool color_is_enabled = false;
enum color_mode_t color_mode = COLOR_AUTO;
const char *color_seq_table[COLOR_KIND_MAX];

struct color_key {
	const char *name;
	enum color_kind_t kind;
};

#define DEF_COLOR_KEY(name)	{ #name, COLOR_ ## name }
static const struct color_key color_keys[] = {
	DEF_COLOR_KEY(ARGNAME),
	DEF_COLOR_KEY(ARGVAL),
	DEF_COLOR_KEY(COMMENT),
	DEF_COLOR_KEY(CONST),
	DEF_COLOR_KEY(ERROR),
	DEF_COLOR_KEY(PUNCT),
	DEF_COLOR_KEY(RETVAL),
	DEF_COLOR_KEY(SYSCALL),
};
#undef DEF_COLOR_KEY

/* Map a key name to a color kind, or COLOR_KIND_MAX if unknown. */
static enum color_kind_t
lookup_color_kind(const char *name)
{
	if (!name || !*name)
		return COLOR_KIND_MAX;

	for (size_t i = 0; i < ARRAY_SIZE(color_keys); i++) {
		if (!strcasecmp(name, color_keys[i].name))
			return color_keys[i].kind;
	}

	return COLOR_KIND_MAX;
}

/* Trim leading and trailing whitespace in-place and return the start. */
static char *
trim_spaces(char *s)
{
	if (!s)
		return NULL;

	while (isspace((unsigned char) *s))
		++s;

	char *end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char) *end))
		*end-- = '\0';

	return s;
}

static const char *color_seq_table_default[COLOR_KIND_MAX] = {
	"\033[39m",	/* ARGNAME:  foreground */
	"\033[35m",	/* ARGVAL:   magenta */
	"\033[36m",	/* COMMENT:  cyan */
	"\033[34m",	/* CONST:    blue */
	"\033[31m",	/* ERROR:    red */
	"\033[0m",	/* PUNCT:    reset */
	"\033[32m",	/* RETVAL:   green */
	"\033[33m",	/* SYSCALL:  yellow */
	"\033[0m",	/* RESET */
};
static const char **color_seq_table_def;

/* Validate that the value is a raw SGR parameter list (only digits and ;) */
static bool
is_sgr_seq(const char *s)
{
	bool has_digit = false;

	if (!s || !*s)
		return false;

	for (const unsigned char *p = (const unsigned char *) s; *p; ++p) {
		if (isdigit(*p))
			has_digit = true;
		else if (*p != ';')
			return false;
	}

	return has_digit;
}

/* Build an SGR escape sequence from a SGR parameter list. */
static const char *
make_sgr_seq(const char *str)
{
	return xasprintf("\033[%sm", str);
}


/*
 * Parse STRACE_COLORS into color sequences, ignoring invalid parts.
 * expected format: `colorname=sgrseq:...`,
 * where sgrseq is a raw sgr parameter like `1;31`.
 * so like LS_COLORS.
 */
static void
parse_strace_colors(const char *spec)
{
	if (!spec || !*spec)
		return;

	char *buf = xstrdup(spec);
	char *saveptr = NULL;

	for (char *tok = strtok_r(buf, ":", &saveptr);
	     tok; tok = strtok_r(NULL, ":", &saveptr)) {
		char *eq = strchr(tok, '=');
		if (!eq)
			continue;
		*eq++ = '\0';
		char *key = trim_spaces(tok);
		char *val = trim_spaces(eq);

		enum color_kind_t kind = lookup_color_kind(key);
		if (kind >= COLOR_KIND_MAX)
			continue;
		if (is_sgr_seq(val)) {
			if (color_seq_table[kind] != color_seq_table_def[kind])
				free((char *) color_seq_table[kind]);
			color_seq_table[kind] = make_sgr_seq(val);
		}
	}

	free(buf);
}

/*
 * Return true when colors should be considered unavailable:
 * either NO_COLOR is set or TERM indicates a non-color terminal.
 */
static bool
is_no_color(void)
{
	if (getenv("NO_COLOR"))
		return true;

	const char *term = getenv("TERM");

	if (!term || !*term)
		return true;

#ifdef HAVE_TGETNUM
	return tgetent(NULL, term) <= 0 || tgetnum("Co") < 8;
#else
	return !strcasecmp(term, "dumb") ||
	       !strcasecmp(term, "unknown");
#endif
}

/*
 * Initialize color output based on mode, trace output stream, and environment.
 * Set NO_COLOR to disable in auto mode, or STRACE_COLORS to customize.
 *
 * output_separately says if we store per-pid output in separate files,
 * where by default we won't store colors.  One can still set --color=always
 * to include colors in the files (and view them with e.g. `less -R`).
 */
void
color_init(int out_fd, bool output_separately)
{
	color_is_enabled = false;

	if (color_mode == COLOR_NEVER)
		return;

	const bool is_tty = isatty(out_fd);
	if (color_mode == COLOR_AUTO) {
		if (!is_tty || output_separately || is_no_color())
			return;
	}

	color_seq_table_def = color_seq_table_default;
	memcpy(color_seq_table, color_seq_table_default, sizeof(color_seq_table));
	parse_strace_colors(getenv("STRACE_COLORS"));

	color_is_enabled = true;
}
