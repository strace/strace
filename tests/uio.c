#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

int
main(void)
{
#if defined(HAVE_PREADV) && defined(HAVE_PWRITEV)
	const off_t offset = 0xdefaceddeadbeefLL;
	char buf[4];
	struct iovec iov = { buf, sizeof buf };

	(void) close(0);
	assert(open("/dev/zero", O_RDONLY) == 0);
	assert(pread(0, buf, sizeof buf, offset) == 4);
	assert(preadv(0, &iov, 1, offset) == 4);
	assert(!close(0));

	assert(open("/dev/null", O_WRONLY) == 0);
	assert(pwrite(0, buf, sizeof buf, offset) == 4);
	assert(pwritev(0, &iov, 1, offset) == 4);
	assert(!close(0));

	return 0;
#else
	return 77;
#endif
}
