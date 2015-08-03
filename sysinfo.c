#include "defs.h"
#include DEF_MPERS_TYPE(sysinfo_t)
#include <sys/sysinfo.h>
typedef struct sysinfo sysinfo_t;
#include MPERS_DEFS

SYS_FUNC(sysinfo)
{
	sysinfo_t si;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &si)) {
		tprintf("{uptime=%llu"
			", loads=[%llu, %llu, %llu]"
			", totalram=%llu"
			", freeram=%llu"
			", sharedram=%llu"
			", bufferram=%llu"
			", totalswap=%llu"
			", freeswap=%llu"
			", procs=%u"
			", totalhigh=%llu"
			", freehigh=%llu"
			", mem_unit=%u"
			"}",
			(unsigned long long) si.uptime
			, (unsigned long long) si.loads[0]
			, (unsigned long long) si.loads[1]
			, (unsigned long long) si.loads[2]
			, (unsigned long long) si.totalram
			, (unsigned long long) si.freeram
			, (unsigned long long) si.sharedram
			, (unsigned long long) si.bufferram
			, (unsigned long long) si.totalswap
			, (unsigned long long) si.freeswap
			, (unsigned) si.procs
			, (unsigned long long) si.totalhigh
			, (unsigned long long) si.freehigh
			, si.mem_unit
			);
	}

	return 0;
}
