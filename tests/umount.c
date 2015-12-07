#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>

#ifdef __NR_oldumount
# define TEST_SYSCALL_STR "oldumount"
#else
# if defined __NR_umount && defined __NR_umount2
#  define __NR_oldumount __NR_umount
#  define TEST_SYSCALL_STR "umount"
# endif
#endif

int
main(void)
{
#ifdef __NR_oldumount
	static const char sample[] = "umount.sample";
	if (mkdir(sample, 0700)) {
		perror(sample);
		return 77;
	}
	(void) syscall(__NR_oldumount, sample);
	printf("%s(\"%s\") = -1 ", TEST_SYSCALL_STR, sample);
	switch (errno) {
		case ENOSYS:
			printf("ENOSYS (%m)\n");
			break;
		case EPERM:
			printf("EPERM (%m)\n");
			break;
		default:
			printf("EINVAL (%m)\n");
	}
	(void) rmdir(sample);
	puts("+++ exited with 0 +++");
	return 0;
#else
	return 77;
#endif
}
