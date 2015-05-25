#include "defs.h"

void die_out_of_memory(void)
{
	static bool recursed = false;

	if (recursed)
		exit(1);
	recursed = 1;

	error_msg_and_die("Out of memory");
}

void *xmalloc(size_t size)
{
	void *p = malloc(size);

	if (!p)
		die_out_of_memory();

	return p;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);

	if (!p)
		die_out_of_memory();

	return p;
}

#define HALF_SIZE_T	(((size_t) 1) << (sizeof(size_t) * 4))

void *xreallocarray(void *ptr, size_t nmemb, size_t size)
{
	size_t bytes = nmemb * size;

	if ((nmemb | size) >= HALF_SIZE_T &&
	    size && bytes / size != nmemb)
		die_out_of_memory();

	void *p = realloc(ptr, bytes);

	if (!p)
		die_out_of_memory();

	return p;
}

char *xstrdup(const char *str)
{
	char *p = strdup(str);

	if (!p)
		die_out_of_memory();

	return p;
}
