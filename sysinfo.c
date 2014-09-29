#include "defs.h"
#include <sys/sysinfo.h>

int
sys_sysinfo(struct tcb *tcp)
{
	struct sysinfo si;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &si) < 0)
			tprints("{...}");
		else {
			tprintf("{uptime=%lu, loads=[%lu, %lu, %lu] ",
				(long) si.uptime, (long) si.loads[0],
				(long) si.loads[1], (long) si.loads[2]);
			tprintf("totalram=%lu, freeram=%lu, ",
				(long) si.totalram, (long) si.freeram);
			tprintf("sharedram=%lu, bufferram=%lu} ",
				(long) si.sharedram, (long) si.bufferram);
			tprintf("totalswap=%lu, freeswap=%lu, procs=%u}",
				(long) si.totalswap, (long) si.freeswap,
				(unsigned)si.procs);
		}
	}
	return 0;
}
