#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int
main(void)
{
#if (defined __NR_getuid || defined __NR_getxuid) \
 && defined(__NR_setuid) \
 && defined(__NR_getresuid) \
 && defined(__NR_setreuid) \
 && defined(__NR_setresuid) \
 && defined(__NR_fchown) \
 && defined(__NR_getgroups)
	int uid;
	int size;
	int *list = 0;

#ifndef __NR_getuid
# define __NR_getuid __NR_getxuid
#endif
	uid = syscall(__NR_getuid);

	(void) close(0);
	if (open("/proc/sys/kernel/overflowuid", O_RDONLY) == 0) {
		/* we trust the kernel */
		char buf[sizeof(int)*3];
		int n = read(0, buf, sizeof(buf) - 1);
		if (n) {
			buf[n] = '\0';
			n = atoi(buf);
			if (uid == n)
				return 77;
		}
		(void) close(0);
	}

	assert(syscall(__NR_setuid, uid) == 0);
	{
		/*
		 * uids returned by getresuid should be ignored
		 * to avoid 16bit vs 32bit issues.
		 */
		int r, e, s;
		assert(syscall(__NR_getresuid, &r, &e, &s) == 0);
	}
	assert(syscall(__NR_setreuid, -1, -1L) == 0);
	assert(syscall(__NR_setresuid, uid, -1, -1L) == 0);
	assert(syscall(__NR_fchown, 1, -1, -1L) == 0);
	assert((size = syscall(__NR_getgroups, 0, list)) >= 0);
	assert(list = calloc(size + 1, sizeof(*list)));
	assert(syscall(__NR_getgroups, size, list) == size);
	return 0;
#else
	return 77;
#endif
}
