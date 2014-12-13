#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
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
 \
 && defined(__NR_getuid32) \
 && defined(__NR_setuid32) \
 && defined(__NR_getresuid32) \
 && defined(__NR_setreuid32) \
 && defined(__NR_setresuid32) \
 && defined(__NR_chown32) \
 \
 && __NR_getuid != __NR_getuid32 \
 && __NR_setuid != __NR_setuid32 \
 && __NR_getresuid != __NR_getresuid32 \
 && __NR_setreuid != __NR_setreuid32 \
 && __NR_setresuid != __NR_setresuid32 \
 && __NR_chown != __NR_chown32 \
 /**/
	uid_t r, e, s;

	e = syscall(__NR_getuid);
	assert(syscall(__NR_setuid, e) == 0);
	assert(syscall(__NR_getresuid, &r, &e, &s) == 0);
	assert(syscall(__NR_setreuid, -1, 0xffff) == 0);
	assert(syscall(__NR_setresuid, -1, e, 0xffff) == 0);
	assert(syscall(__NR_chown, ".", -1, 0xffff) == 0);
	return 0;
#else
	return 77;
#endif
}
