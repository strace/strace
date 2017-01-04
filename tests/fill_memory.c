#include "tests.h"

void
fill_memory_ex(void *ptr, size_t size, unsigned char start,
	       unsigned char period)
{
	unsigned char *p = ptr;
	size_t i;

	for (i = 0; i < size; i++) {
		p[i] = start + i % period;
	}
}

void
fill_memory(void *ptr, size_t size)
{
	fill_memory_ex(ptr, size, 0x80, 0x80);
}
