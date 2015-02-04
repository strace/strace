#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#ifdef __NR_getrandom
	char buf[4];

	if (syscall(__NR_getrandom, buf, sizeof(buf) - 1, 0) != sizeof(buf) - 1)
		return 77;
	if (syscall(__NR_getrandom, buf, sizeof(buf), 1) != sizeof(buf))
		return 77;
	if (syscall(__NR_getrandom, buf, sizeof(buf), 0x3003) != -1)
		return 77;

	return 0;
#else
	return 77;
#endif
}
