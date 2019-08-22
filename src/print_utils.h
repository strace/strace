#ifndef STRACE_PRINT_UTILS_H
# define STRACE_PRINT_UTILS_H

# include <inttypes.h>

/* Hexadecimal output utils */

static const char hex_chars[16] = "0123456789abcdef";

/**
 * Character array representing hexadecimal encoding of a character value.
 *
 * @param b_ Byte to provide representation for.
 */
# define BYTE_HEX_CHARS(b_) \
	hex_chars[((uint8_t) (b_)) >> 4], hex_chars[((uint8_t) (b_)) & 0xf]

/* Character classification utils */

static inline bool
is_print(uint8_t c)
{
	return (c >= ' ') && (c < 0x7f);
}

/* Character printing functions */

static inline char *
sprint_byte_hex(char *buf, uint8_t val)
{
	*buf++ = hex_chars[val >> 4];
	*buf++ = hex_chars[val & 0xf];

	return buf;
}

/** Maximum number of characters emitted by sprint_char */
# define SPRINT_CHAR_BUFSZ 7

enum sprint_char_flag_bits {
	SCF_QUOTES_BIT,
	SCF_NUL_BIT,
	SCF_ESC_WS_BIT,
};

enum sprint_char_flags {
	FLAG(SCF_QUOTES), /**< Whether to emit quotes */
	FLAG(SCF_NUL),    /**< Whether to terminate output with \0 */
	FLAG(SCF_ESC_WS), /**< Whether to print \t\n\v\f\r in symbolic form */
};

/** Emits a character into buf (SPRINT_CHAR_BUFSZ max), returns new position. */
static inline char *
sprint_char(char *buf, const unsigned char c, const enum sprint_char_flags f)
{
	if (f & SCF_QUOTES)
		*buf++ = '\'';

	if (is_print(c)) {
		if (c == '\'' || c == '\\')
			*buf++ = '\\';
		*buf++ = c;
	} else if ((f & SCF_ESC_WS) && (c >= '\t') && (c <= '\r')) {
		static const char ws_chars[] = "tnvfr";

		*buf++ = '\\';
		*buf++ = ws_chars[c - '\t'];
	} else {
		*buf++ = '\\';
		*buf++ = 'x';
		buf = sprint_byte_hex(buf, c);
	}

	if (f & SCF_QUOTES)
		*buf++ = '\'';
	if (f & SCF_NUL)
		*buf++ = '\0';

	return buf;
}

# define print_char(c_, flags_)					\
	do {							\
		char buf[SPRINT_CHAR_BUFSZ];			\
								\
		sprint_char(buf, (c_), (flags_) | SCF_NUL);	\
		tprints(buf);					\
	} while (0)

#endif /* STRACE_PRINT_UTILS_H */
