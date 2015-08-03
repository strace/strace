#include <stdio.h>
#include <sys/sysinfo.h>

int
main (void)
{
	struct sysinfo si;
	if (sysinfo(&si) == -1)
		return 77;
	printf("sysinfo({uptime=%llu"
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
		"}) = 0\n",
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
	puts("+++ exited with 0 +++");
	return 0;
}
