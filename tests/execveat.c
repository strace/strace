#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <sys/syscall.h>

#ifdef __NR_execveat

#define FILENAME "execveat\nfilename"
static const char * const argv[] =
	{ FILENAME, "first", "second", NULL, NULL, NULL };
static const char * const envp[] =
	{ "foobar=1", "foo\nbar=2", NULL , "", NULL , "", NULL, NULL};

int
main(void)
{
	syscall(__NR_execveat, -100, FILENAME, argv, envp, 0x1100);
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
