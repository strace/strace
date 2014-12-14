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
#if defined(__NR_getuid) \
 && defined(__NR_setuid) \
 && defined(__NR_getresuid) \
 && defined(__NR_setreuid) \
 && defined(__NR_setresuid) \
 && defined(__NR_chown) \
 && defined(__NR_getgroups)
	int r, e, s;
	int size;
	int *list = 0;

	e = syscall(__NR_getuid);
	assert(syscall(__NR_setuid, e) == 0);
	assert(syscall(__NR_getresuid, &r, &e, &s) == 0);
	assert(syscall(__NR_setreuid, -1, -1L) == 0);
	assert(syscall(__NR_setresuid, -1, e, -1L) == 0);
	assert(syscall(__NR_chown, ".", -1, -1L) == 0);
	assert((size = syscall(__NR_getgroups, 0, list)) >= 0);
	assert(list = calloc(size + 1, sizeof(*list)));
	assert(syscall(__NR_getgroups, size, list) == size);
	return 0;
#else
	return 77;
#endif
}
