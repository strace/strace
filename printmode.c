#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "xlat/modetypes.h"

const char *
sprintmode(int mode)
{
	static char buf[sizeof("S_IFSOCK|S_ISUID|S_ISGID|S_ISVTX|%o")
			+ sizeof(int)*3
			+ /*paranoia:*/ 8];
	const char *s;

	if ((mode & S_IFMT) == 0)
		s = "";
	else if ((s = xlookup(modetypes, mode & S_IFMT)) == NULL) {
		sprintf(buf, "%#o", mode);
		return buf;
	}
	s = buf + sprintf(buf, "%s%s%s%s", s,
		(mode & S_ISUID) ? "|S_ISUID" : "",
		(mode & S_ISGID) ? "|S_ISGID" : "",
		(mode & S_ISVTX) ? "|S_ISVTX" : "");
	mode &= ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX);
	if (mode)
		sprintf((char*)s, "|%#o", mode);
	s = (*buf == '|') ? buf + 1 : buf;
	return *s ? s : "0";
}
