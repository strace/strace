#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif

int main(int argc, char **argv)
{
	if (argc < 2)
		return 99;
#if defined HAVE_PRCTL && defined PR_SET_PTRACER && defined PR_SET_PTRACER_ANY
	/* Turn off restrictions on tracing if applicable.  If the options
	 * aren't available on this system, that's OK too.  */
	(void) prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
#endif
	if (write(1, "\n", 1) != 1) {
		perror("write");
		return 99;
	}
	(void) execvp(argv[1], argv + 1);
	perror(argv[1]);
	return 99;
}
