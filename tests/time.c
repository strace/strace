#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#ifdef __NR_time

int
main(void)
{
	const size_t page_len = sysconf(_SC_PAGESIZE);

	void *p = mmap(NULL, page_len * 2, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED || mprotect(p + page_len, page_len, PROT_NONE))
		return 77;

	time_t *p_t = p + page_len - sizeof(time_t);
	time_t t = syscall(__NR_time, p_t);

	if ((time_t) -1 == t || t != *p_t)
		return 77;

	printf("time([%jd]) = %jd\n", (intmax_t) t, (intmax_t) t);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
