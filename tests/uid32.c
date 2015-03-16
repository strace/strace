#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#if defined(__NR_getuid32) \
 && defined(__NR_setuid32) \
 && defined(__NR_getresuid32) \
 && defined(__NR_setreuid32) \
 && defined(__NR_setresuid32) \
 && defined(__NR_fchown32) \
 && defined(__NR_getgroups32)
	int r, e, s;
	int size;
	int *list = 0;

	r = syscall(__NR_getuid32);
	assert(syscall(__NR_setuid32, r) == 0);
	assert(syscall(__NR_getresuid32, &r, &e, &s) == 0);
	assert(syscall(__NR_setreuid32, -1, -1L) == 0);
	assert(syscall(__NR_setresuid32, r, -1, -1L) == 0);
	assert(syscall(__NR_fchown32, 1, -1, -1L) == 0);
	assert((size = syscall(__NR_getgroups32, 0, list)) >= 0);
	assert(list = calloc(size + 1, sizeof(*list)));
	assert(syscall(__NR_getgroups32, size, list) == size);
	return 0;
#else
	return 77;
#endif
}
