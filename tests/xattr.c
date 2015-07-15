#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>

int
main(void)
{
#define NAME "strace.test"
#define VALUE "foo\0bar"
	if (!removexattr(".", NAME) ||
	    !setxattr(".", NAME, VALUE, sizeof(VALUE), XATTR_CREATE))
		return 77;
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
