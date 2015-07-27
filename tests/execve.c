#include <unistd.h>

#define FILENAME "execve\nfilename"
static const char * const argv[] =
	{ FILENAME, "first", "second", NULL, NULL, NULL };
static const char * const envp[] =
	{ "foobar=1", "foo\nbar=2", NULL , "", NULL , "", NULL, NULL};

int
main(void)
{
	execve(FILENAME, (char * const *) argv, (char * const *) envp);
	return 0;
}
