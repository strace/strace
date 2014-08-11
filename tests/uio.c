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
	int fd;
	char buf[4];
	struct iovec iov = { buf, sizeof buf };

	assert((fd = open("/dev/zero", O_RDONLY)) >= 0);
	assert(pread(fd, buf, sizeof buf, offset) == 4);
	assert(preadv(fd, &iov, 1, offset) == 4);
	assert(!close(fd));

	assert((fd = open("/dev/null", O_WRONLY)) >= 0);
	assert(pwrite(fd, buf, sizeof buf, offset) == 4);
	assert(pwritev(fd, &iov, 1, offset) == 4);
	assert(!close(fd));

	return 0;
#else
	return 77;
#endif
}
