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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

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

static const char *color_seq_table_light[COLOR_KIND_MAX] = {
	"\033[30m",
	"\033[35m",
	"\033[36m",
	"\033[1;36m",
	"\033[31m",
	"\033[0m",
	"\033[32m",
	"\033[33m",
	"\033[0m",
};
static const char *color_seq_table_dark[COLOR_KIND_MAX] = {
	"\033[37m",
	"\033[95m",
	"\033[96m",
	"\033[1;96m",
	"\033[91m",
	"\033[0m",
	"\033[92m",
	"\033[93m",
	"\033[0m",
};
static const char **color_seq_table_def;

/* Populate default color sequences based on background luminance. */
static void
set_default_colors(bool light_bg)
{
	color_seq_table_def =
		light_bg ? color_seq_table_light : color_seq_table_dark;
	memcpy(color_seq_table, color_seq_table_def, sizeof(color_seq_table));
}

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

/* Convert a hex digit to its integer value, or -1 on error. */
static int
hex2int(const char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/*
 * Parse one hex component from "rgb:R/G/B", tracking its max (0xF/0xFF/...)
 * for scaling.  The value must have 1 to 4 hex digits, longer components are
 * rejected.  maxv will be 0xF, 0xFF, 0xFFF, or 0xFFFF, accordingly.
 */
static bool
parse_rgb_component(const char **cursor, size_t *len_remaining,
		    unsigned int *value, unsigned int *maxv)
{
	unsigned int val = 0;
	unsigned int max_val = 0;
	unsigned int digit_count = 0;

	while (*len_remaining > 0) {
		int hex_digit = hex2int(**cursor);
		if (hex_digit < 0)
			break;
		val = val * 16 + (unsigned) hex_digit;
		max_val = max_val * 16 + 0xF;
		++digit_count;
		++*cursor;
		--*len_remaining;
	}

	if (digit_count == 0 || digit_count > 4)
		return false;

	*value = val;
	*maxv = max_val;
	return true;
}

/*
 * Parse a terminal background-color OSC11 reply like "\033]11;rgb:R/G/B".
 * Each component in R/G/B can be 1-4 hex digits.
 */
static bool
parse_color_is_light(const char *color_seq, size_t len, bool *is_light)
{
	static const char prefix[] = "\033]11;rgb:";
	const size_t prefix_len = sizeof(prefix) - 1;
	const char *cursor = NULL;

	if (!color_seq || len < prefix_len)
		return false;

	/* Validate expected prefix.  */
	for (size_t i = 0; i + prefix_len <= len; i++) {
		if (!memcmp(color_seq + i, prefix, prefix_len)) {
			cursor = color_seq + i + prefix_len;
			break;
		}
	}

	if (!cursor)
		return false;

	size_t remaining = len - (size_t) (cursor - color_seq);
	/* The r/g/b component values.  */
	unsigned int val[3] = { 0, 0, 0 };
	/* The max possible r/g/b value due to variable lenghts.  */
	unsigned int max_value[3] = { 0, 0, 0 };
	for (int comp = 0; comp < 3; comp++) {
		if (!parse_rgb_component(&cursor, &remaining,
		                         &val[comp], &max_value[comp]))
			return false;
		if (comp < 2) {
			if (!remaining || *cursor != '/')
				return false;
			++cursor;
			--remaining;
		}
	}

	/* Scale down to [0.0, 1.0] range.  */
	const double r = (double) val[0] / (double) max_value[0];
	const double g = (double) val[1] / (double) max_value[1];
	const double b = (double) val[2] / (double) max_value[2];
	/* Relative luminance (BT.601) for perceived brightness.  */
	const double lum = 0.299 * r + 0.587 * g + 0.114 * b;

	*is_light = lum >= 0.5;
	return true;
}

/* Read terminal background-color reply until BEL or ESC \\. */
static size_t
read_osc11_reply(int fd, char *buf, size_t size)
{
	size_t len = 0;

	while (len + 1 < size) {
		char c;
		ssize_t r = read(fd, &c, 1);
		if (r <= 0)
			break;
		buf[len++] = c;
		if (c == '\a')
			break;
		if (c == '\033') {
			char next;
			r = read(fd, &next, 1);
			if (r <= 0)
				break;
			buf[len++] = next;
			if (next == '\\')
				break;
		}
	}
	buf[len] = '\0';
	return len;
}

/* Probe the current terminal for its background luminance. */
static bool
probe_light_bg(void)
{
	int fd = open("/dev/tty", O_RDWR | O_CLOEXEC);
	if (fd < 0)
		return false;

	struct termios oldt;
	if (tcgetattr(fd, &oldt) < 0) {
		close(fd);
		return false;
	}

	/* Set terminal mode to raw without echo.  */
	struct termios raw = oldt;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSANOW, &raw) < 0) {
		close(fd);
		return false;
	}

	static const char query[] = "\033]11;?\007";
	if (write(fd, query, sizeof(query) - 1) < 0) {
		(void) tcsetattr(fd, TCSANOW, &oldt);
		close(fd);
		return false;
	}

	char buf[128]; /* we expect about 30 bytes max */
	size_t len = read_osc11_reply(fd, buf, sizeof(buf));

	/* Restore terminal settings.  */
	(void) tcsetattr(fd, TCSANOW, &oldt);
	close(fd);

	bool is_light = false;
	if (!parse_color_is_light(buf, len, &is_light))
		return false;

	return is_light;
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
 * Initialize color output based on mode, trace output stream, and environment.
 * set NO_COLOR to disable, or STRACE_COLORS to customize.
 *
 * output_separately says if we store per-pid output in separate files,
 * where by default we won't store colors. you can still set --color=always
 * to include colors in the files (and view them with e.g. `less -R`).
 */
void
color_init(FILE *outf, bool output_separately)
{
	color_is_enabled = false;
	if (color_mode == COLOR_NEVER)
		return;

	const bool is_tty = outf && isatty(fileno(outf));
	if (color_mode == COLOR_AUTO) {
		if (output_separately)
			return;
		if (!is_tty)
			return;
		if (getenv("NO_COLOR"))
			return;
		char *term = getenv("TERM");
		if (!term || !*term ||
		    !strcasecmp(term, "dumb") ||
		    !strcasecmp(term, "unknown"))
			return;
	}

	set_default_colors(is_tty && probe_light_bg());
	parse_strace_colors(getenv("STRACE_COLORS"));
	color_is_enabled = true;
}
