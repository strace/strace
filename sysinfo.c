#include "defs.h"
#include <sys/sysinfo.h>

int
sys_sysinfo(struct tcb *tcp)
{
	struct sysinfo si;

	if (entering(tcp))
		return 0;

	if (syserror(tcp) || !verbose(tcp) ||
	    umove(tcp, tcp->u_arg[0], &si) < 0) {
		tprintf("%#lx", tcp->u_arg[0]);
	} else {
		tprintf("{uptime=%lu"
			", loads=[%lu, %lu, %lu]"
			", totalram=%lu"
			", freeram=%lu"
			", sharedram=%lu"
			", bufferram=%lu"
			", totalswap=%lu"
			", freeswap=%lu"
			", procs=%u"
#ifdef HAVE_STRUCT_SYSINFO_TOTALHIGH
			", totalhigh=%lu"
#endif
#ifdef HAVE_STRUCT_SYSINFO_FREEHIGH
			", freehigh=%lu"
#endif
#ifdef HAVE_STRUCT_SYSINFO_MEM_UNIT
			", mem_unit=%u"
#endif
			"}",
			si.uptime
			, si.loads[0], si.loads[1], si.loads[2]
			, si.totalram
			, si.freeram
			, si.sharedram
			, si.bufferram
			, si.totalswap
			, si.freeswap
			, (unsigned) si.procs
#ifdef HAVE_STRUCT_SYSINFO_TOTALHIGH
			, si.totalhigh
#endif
#ifdef HAVE_STRUCT_SYSINFO_FREEHIGH
			, si.freehigh
#endif
#ifdef HAVE_STRUCT_SYSINFO_MEM_UNIT
			, si.mem_unit
#endif
			);
	}

	return 0;
}
