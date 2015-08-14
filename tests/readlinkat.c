#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int
main(void)
{
#ifdef __NR_readlinkat
	char buf[31];

	if (syscall(__NR_readlinkat, AT_FDCWD, "readlinkat.link", buf, sizeof(buf)) != 12)
		return 77;

	return 0;
#else
	return 77;
#endif
}
