#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_sysfs

# include <stdio.h>
# include <unistd.h>

# include <linux/limits.h>

int
main(void)
{
	static const kernel_ulong_t bogus_long =
			(kernel_ulong_t) 0xdeadc0de00000000ULL;

	long rc;
	long fs_types;
	char *buf = tail_alloc(PATH_MAX + 1);

	fs_types = syscall(__NR_sysfs, 3);
	printf("sysfs(3 /* total number of filesystem types */) = %s\n",
	       sprintrc(fs_types));

	if (fs_types < 0)
		error_msg_and_skip("sysfs(3)");

	rc = syscall(__NR_sysfs, 4, 1, 2, 3, 4, 5);
	printf("sysfs(4 /* ??? */) = %s\n", sprintrc(rc));

	rc = syscall(__NR_sysfs, 2, bogus_long | 3141592653U, NULL);
	printf("sysfs(2 /* filesystem type name by index */, 3141592653, NULL)"
	       " = %s\n", sprintrc(rc));

	rc = syscall(__NR_sysfs, 1, NULL);
	printf("sysfs(1 /* filesystem type index by name */, NULL) = %s\n",
	       sprintrc(rc));

	rc = syscall(__NR_sysfs, 1, buf + PATH_MAX + 1);
	printf("sysfs(1 /* filesystem type index by name */, %p) = %s\n",
	       buf + PATH_MAX + 1, sprintrc(rc));

	rc = syscall(__NR_sysfs, 1, "");
	printf("sysfs(1 /* filesystem type index by name */, \"\") = %s\n",
	       sprintrc(rc));

	for (size_t i = 0; i < (unsigned long) fs_types; i++) {
		rc = syscall(__NR_sysfs, 2, bogus_long | i, buf);
		if (rc)
			perror_msg_and_fail("sysfs(2, %zu)", i);
		printf("sysfs(2 /* filesystem type name by index */, %zu, ", i);
		print_quoted_cstring(buf, PATH_MAX + 1);
		printf(") = 0\n");

		rc = syscall(__NR_sysfs, 1, buf);
		if ((size_t) rc != i)
			perror_msg_and_fail("sysfs(1, %s)", buf);
		printf("sysfs(1 /* filesystem type index by name */, ");
		print_quoted_cstring(buf, PATH_MAX + 1);
		printf(") = %zu\n", i);
	}

	rc = syscall(__NR_sysfs, 2, fs_types, buf);
	if (rc != -1)
		error_msg_and_fail("sysfs(2, %ld) != -1", fs_types);
	printf("sysfs(2 /* filesystem type name by index */, %ld, %p) = %s\n",
	       fs_types, buf, sprintrc(rc));

	rc = syscall(__NR_sysfs, 2, fs_types, buf);
	if (rc != -1)
		error_msg_and_fail("sysfs(2, %ld) != -1", fs_types);
	printf("sysfs(2 /* filesystem type name by index */, %ld, %p) = %s\n",
	       fs_types, buf, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sysfs")

#endif
