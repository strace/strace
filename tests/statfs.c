#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <sys/statfs.h>
#include <assert.h>

int
main(void)
{
	struct statfs stb;
	assert(statfs("/proc/self/status", &stb) == 0);
	return 0;
}
