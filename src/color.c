/*
 * Copyright (c) 2026 Jonas Jelten <jj@sft.lol>
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


#ifdef ENABLE_COLORS
bool color_is_enabled = false;
enum color_mode_t color_mode = COLOR_AUTO;
const char *color_seq_table[COLOR_KIND_MAX];

struct color_key {
	const char *name;
	enum color_kind_t kind;
};

static const struct color_key color_keys[] = {
	{ "syscall", COLOR_SYSCALL },
	{ "argname", COLOR_ARGNAME },
	{ "arg", COLOR_ARGVAL },
	{ "argval", COLOR_ARGVAL },
	{ "value", COLOR_ARGVAL },
	{ "constant", COLOR_CONST },
	{ "comment", COLOR_COMMENT },
	{ "punct", COLOR_PUNCT },
	{ "punctuation", COLOR_PUNCT },
	{ "retval", COLOR_RETVAL },
	{ "return", COLOR_RETVAL },
	{ "error", COLOR_ERROR },
};

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

	if (!*s)
		return s;

	char *end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char) *end))
		*end-- = '\0';

	return s;
}

/* Populate default color sequences based on background luminance. */
static void
set_default_colors(bool light_bg)
{
	if (light_bg) {
		color_seq_table[COLOR_SYSCALL] = "\033[33m";
		color_seq_table[COLOR_ARGNAME] = "\033[30m";
		color_seq_table[COLOR_ARGVAL] = "\033[35m";
		color_seq_table[COLOR_CONST] = "\033[1;36m";
		color_seq_table[COLOR_COMMENT] = "\033[36m";
		color_seq_table[COLOR_PUNCT] = "\033[0m";
		color_seq_table[COLOR_RETVAL] = "\033[32m";
		color_seq_table[COLOR_ERROR] = "\033[31m";
	} else {
		color_seq_table[COLOR_SYSCALL] = "\033[93m";
		color_seq_table[COLOR_ARGNAME] = "\033[37m";
		color_seq_table[COLOR_ARGVAL] = "\033[95m";
		color_seq_table[COLOR_CONST] = "\033[1;96m";
		color_seq_table[COLOR_COMMENT] = "\033[96m";
		color_seq_table[COLOR_PUNCT] = "\033[0m";
		color_seq_table[COLOR_RETVAL] = "\033[92m";
		color_seq_table[COLOR_ERROR] = "\033[91m";
	}
	color_seq_table[COLOR_RESET] = "\033[0m";
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
make_sgr_seq(const char *s)
{
	const size_t len = strlen(s);
	char *buf = xmalloc(len + 4);

	buf[0] = '\033';
	buf[1] = '[';
	memcpy(buf + 2, s, len);
	buf[len + 2] = 'm';
	buf[len + 3] = '\0';

	return buf;
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
 * Parse one hex component from "rgb:R/G/B", tracking its max (0xF/0xFF/...) for scaling.
 * The value can have 1 to 4 hex digits, so maxv will be 0xF, 0xFF, 0xFFF, or 0xFFFF accordingly.
 */
static bool
parse_rgb_component(const char **cursor, size_t *len_remaining,
		     unsigned int *value, unsigned int *maxv)
{
	unsigned int val = 0;
	unsigned int max_val = 0;
	int digit_count = 0;

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

	if (!digit_count || !max_val)
		return false;

	*value = val;
	*maxv = max_val;
	return true;
}

/*
 * Parse a terminal background-color OSC11 reply like "\033]11;rgb:R/G/B".
 * each component in R/G/B can be 1-4 hex digits.
 */
static bool
parse_color_is_light(const char *color_seq, size_t len, bool *is_light)
{
	static const char prefix[] = "\033]11;rgb:";
	const size_t prefix_len = sizeof(prefix) - 1;
	const char *cursor = NULL;

	if (!color_seq || len < prefix_len)
		return false;

	// validate expected prefix
	for (size_t i = 0; i + prefix_len <= len; i++) {
		if (!memcmp(color_seq + i, prefix, prefix_len)) {
			cursor = color_seq + i + prefix_len;
			break;
		}
	}

	if (!cursor)
		return false;

	size_t remaining = len - (size_t) (cursor - color_seq);
	// the r/g/b component values
	unsigned int val[3] = { 0, 0, 0 };
	// the max possible r/g/b value due to variable lenghts
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

	// scale down to [0.0, 1.0] range
	const double r = (double) val[0] / (double) max_value[0];
	const double g = (double) val[1] / (double) max_value[1];
	const double b = (double) val[2] / (double) max_value[2];
	// Relative luminance (BT.601) for perceived brightness
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

	// set terminal mode to raw without echo
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

	char buf[128]; // we expect about 30 bytes max
	size_t len = read_osc11_reply(fd, buf, sizeof(buf));

	// restore terminal settings
	(void) tcsetattr(fd, TCSANOW, &oldt);
	close(fd);

	bool is_light = false;
	if (!parse_color_is_light(buf, len, &is_light))
		return false;

	return is_light;
}

/*
 * Parse STRACE_COLORS into color sequences, ignoring invalid parts.
 * expected format: `colorname=sgrseq:...`, where sgrseq is a raw sgr parameter like `1;31`
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
		if (is_sgr_seq(val))
			color_seq_table[kind] = make_sgr_seq(val);
	}

	free(buf);
}

/*
 * Initialize color output based on mode, tty, and environment.
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
		if (getenv("NO_COLOR"))
			return;
		if (output_separately)
			return;
		if (!is_tty)
			return;
	}
	color_is_enabled = true;

	const bool light_bg = is_tty && probe_light_bg();

	set_default_colors(light_bg);
	parse_strace_colors(getenv("STRACE_COLORS"));
}
#endif
