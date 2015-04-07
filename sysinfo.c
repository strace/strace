#include "defs.h"
#include <sys/sysinfo.h>

SYS_FUNC(sysinfo)
{
	struct sysinfo si;

	if (entering(tcp))
		return 0;

	if (syserror(tcp) || !verbose(tcp) ||
	    umove(tcp, tcp->u_arg[0], &si) < 0) {
		tprintf("%#lx", tcp->u_arg[0]);
	} else {
		tprintf("{uptime=%llu"
			", loads=[%llu, %llu, %llu]"
			", totalram=%llu"
			", freeram=%llu"
			", sharedram=%llu"
			", bufferram=%llu"
			", totalswap=%llu"
			", freeswap=%llu"
			", procs=%u"
#ifdef HAVE_STRUCT_SYSINFO_TOTALHIGH
			", totalhigh=%llu"
#endif
#ifdef HAVE_STRUCT_SYSINFO_FREEHIGH
			", freehigh=%llu"
#endif
#ifdef HAVE_STRUCT_SYSINFO_MEM_UNIT
			", mem_unit=%u"
#endif
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
#ifdef HAVE_STRUCT_SYSINFO_TOTALHIGH
			, (unsigned long long) si.totalhigh
#endif
#ifdef HAVE_STRUCT_SYSINFO_FREEHIGH
			, (unsigned long long) si.freehigh
#endif
#ifdef HAVE_STRUCT_SYSINFO_MEM_UNIT
			, si.mem_unit
#endif
			);
	}

	return 0;
}
