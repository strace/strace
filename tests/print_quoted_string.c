#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Based on string_quote() from util.c.
 * Assumes instr is NUL-terminated.
 */

void
print_quoted_string(const char *instr)
{
	print_quoted_memory(instr, strlen(instr));
}

void
print_quoted_memory(const char *instr, const size_t len)
{
	const unsigned char *str = (const unsigned char*) instr;
	size_t i;

	for (i = 0; i < len; ++i) {
		const int c = str[i];
		switch (c) {
			case '\"':
				printf("\\\"");
				break;
			case '\\':
				printf("\\\\");
				break;
			case '\f':
				printf("\\f");
				break;
			case '\n':
				printf("\\n");
				break;
			case '\r':
				printf("\\r");
				break;
			case '\t':
				printf("\\t");
				break;
			case '\v':
				printf("\\v");
				break;
			default:
				if (c >= ' ' && c <= 0x7e)
					putchar(c);
				else {
					putchar('\\');

					char c1 = '0' + (c & 0x7);
					char c2 = '0' + ((c >> 3) & 0x7);
					char c3 = '0' + (c >> 6);

					if (*str >= '0' && *str <= '9') {
						/* Print \octal */
						putchar(c3);
						putchar(c2);
					} else {
						/* Print \[[o]o]o */
						if (c3 != '0')
							putchar(c3);
						if (c3 != '0' || c2 != '0')
							putchar(c2);
					}
					putchar(c1);
				}
				break;
		}
	}

}
