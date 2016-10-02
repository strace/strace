#include "tests.h"

void
fill_memory_ex(char *ptr, size_t size, unsigned char start,
	       unsigned char period)
{
	size_t i;

	for (i = 0; i < size; i++) {
		ptr[i] = start + i % period;
	}
}

void
fill_memory(char *ptr, size_t size)
{
	fill_memory_ex(ptr, size, 0x80, 0x80);
}
