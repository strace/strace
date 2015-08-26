#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_SYS_SIGNALFD_H
# include <sys/signalfd.h>
#endif

int
main(void)
{
#if defined HAVE_SYS_SIGNALFD_H && defined HAVE_SIGNALFD && defined O_CLOEXEC
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);
	(void) close(0);
	return signalfd(-1, &mask, O_CLOEXEC | O_NONBLOCK) == 0 ?
		0 : 77;
#else
        return 77;
#endif
}
