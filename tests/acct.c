#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_acct

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

# define TMP_FILE "acct_tmpfile"

int
main(void)
{
	assert(syscall(__NR_acct, TMP_FILE) == -1);
	const char *errno_text;
	switch(errno) {
		case ENOSYS:
			errno_text = "ENOSYS";
			break;
		case EPERM:
			errno_text = "EPERM";
			break;
		default:
			errno_text = "ENOENT";
	}
	printf("acct(\"%s\") = -1 %s (%m)\n",
	       TMP_FILE, errno_text);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR__acct")

#endif
