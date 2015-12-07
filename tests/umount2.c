#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>

#ifdef __NR_umount2
# define TEST_SYSCALL_STR "umount2"
#else
# define __NR_umount2 __NR_umount
# define TEST_SYSCALL_STR "umount"
#endif

int
main(void)
{
	static const char sample[] = "umount2.sample";
	if (mkdir(sample, 0700)) {
		perror(sample);
		return 77;
	}
	(void) syscall(__NR_umount2, sample, 31);
	printf("%s(\"%s\", MNT_FORCE|MNT_DETACH|MNT_EXPIRE|UMOUNT_NOFOLLOW|0x10)"
	       " = -1 EINVAL (%m)\n", TEST_SYSCALL_STR, sample);
	(void) rmdir(sample);
	puts("+++ exited with 0 +++");
	return 0;
}
