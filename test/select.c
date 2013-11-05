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
	FD_ZERO(&rds);
	FD_SET(2, &rds);
	/* Start with a nice simple select */
	select(3, &rds, &rds, &rds, NULL);
	/* Now the crash case that trinity found, -ve nfds
	 * but with a pointer to a large chunk of valid memory
	 */
	select(-1, (fd_set *)buffer, NULL, NULL, NULL);
	return 0;
}
