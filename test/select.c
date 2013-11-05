/* dave@treblig.org */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buffer[1024*1024*2];

int main()
{
	fd_set rds;
	struct timeval timeout;

	FD_ZERO(&rds);
	FD_SET(2, &rds);
	/* Start with a nice simple select */
	select(3, &rds, &rds, &rds, NULL);

	/* Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO((fd_set*)buffer);
	FD_SET(2,(fd_set*)buffer);
	select(-1, (fd_set *)buffer, NULL, NULL, NULL);

	/* Another variant, with nfds exceeding allowed limit. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;
	select(FD_SETSIZE + 1, (fd_set *)buffer, NULL, NULL, &timeout);

	return 0;
}
