#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
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
 && defined(__NR_chown32)
	uid_t r, e, s;

	e = syscall(__NR_getuid32);
	assert(syscall(__NR_setuid32, e) == 0);
	assert(syscall(__NR_getresuid32, &r, &e, &s) == 0);
	assert(syscall(__NR_setreuid32, -1, -1L) == 0);
	assert(syscall(__NR_setresuid32, -1, e, -1L) == 0);
	assert(syscall(__NR_chown32, ".", -1, -1L) == 0);
	return 0;
#else
	return 77;
#endif
}
