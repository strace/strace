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
# define BYTE_HEX_CHARS_PRINTF(b_) \
	'\\', 'x', BYTE_HEX_CHARS(b_)
# define BYTE_HEX_CHARS_PRINTF_QUOTED(b_) \
	'\'', BYTE_HEX_CHARS_PRINTF(b_), '\''

static inline char *
sprint_byte_hex(char *buf, uint8_t val)
{
	*buf++ = hex_chars[val >> 4];
	*buf++ = hex_chars[val & 0xf];

	return buf;
}

/* Character classification utils */

static inline bool
is_print(uint8_t c)
{
	return (c >= ' ') && (c < 0x7f);
}

#endif /* STRACE_PRINT_UTILS_H */
