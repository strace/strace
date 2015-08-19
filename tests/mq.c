#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_MQUEUE_H

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdlib.h>
#include <unistd.h>

int
main (void)
{
	struct mq_attr attr;
	(void) close(0);
	if (mq_open("/strace-mq.test", O_CREAT, S_IRWXU, 0) ||
		mq_getattr(0, &attr) ||
		mq_setattr(0, &attr, 0) ||
		mq_unlink("/strace-mq.test"))
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
