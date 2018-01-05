#ifndef STRACE_XSTRING_H
#define STRACE_XSTRING_H

#include <stdarg.h>
#include <stdio.h>

#include "error_prints.h"
#include "gcc_compat.h"

/**
 * Print to static buffer and die on (really unexpected) errors and overflows.
 * Shouldn't be used directly; please refer to helper macros xsnprintf and
 * xsprint instead.
 *
 * @param str    String buffer to print into.
 * @param size   Size of the string buffer in bytes.
 * @param func   Function name from which this function is called.
 * @param argstr Stringified arguments (including format argument).
 * @param format Format string.
 * @param ...    Format arguments.
 * @return       Number of characters printed, excluding terminating null byte
 *               (the same as s(n)printf).
 */
static inline int ATTRIBUTE_FORMAT((printf, 5, 6))
xsnprintf_(char *str, size_t size, const char *func, const char *argstr,
	   const char *format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	ret = vsnprintf(str, size, format, ap);
	va_end(ap);

	if (ret < 0 || (unsigned int) ret >= size)
		error_msg_and_die("%s: got unexpected return value %d for "
				  "snprintf(buf, %zu, %s)",
				  func, ret, size, argstr);

	return ret;
}

/**
 * snprintf that dies on (really unexpected) errors and overflows.
 *
 * @param str_  String buffer to print into.
 * @param size_ Size of the string buffer in bytes.
 * @param fmt_  Format string.
 * @param ...   Format arguments.
 */
#define xsnprintf(str_, size_, fmt_, ...) \
	xsnprintf_((str_), (size_), __func__, #fmt_ ", " #__VA_ARGS__, \
		   (fmt_), __VA_ARGS__)

/**
 * Print to a character array buffer and die on (really unexpected) errors and
 * overflows.  Buffer size is obtained with sizeof().
 *
 * @param str_  Character array buffer to print into.
 * @param fmt_  Format string.
 * @param ...   Format arguments.
 */
#define xsprintf(str_, fmt_, ...) \
	xsnprintf((str_), sizeof(str_) + MUST_BE_ARRAY(str_), (fmt_), \
		  __VA_ARGS__)

#endif /* !STRACE_XSTRING_H */
