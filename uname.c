#include "defs.h"

#include <sys/utsname.h>

int
sys_uname(struct tcb *tcp)
{
	struct utsname uname;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &uname) < 0)
			tprints("{...}");
		else if (!abbrev(tcp)) {
			tprintf("{sysname=\"%s\", nodename=\"%s\", ",
				uname.sysname, uname.nodename);
			tprintf("release=\"%s\", version=\"%s\", ",
				uname.release, uname.version);
			tprintf("machine=\"%s\"", uname.machine);
#ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
			tprintf(", domainname=\"%s\"", uname.domainname);
#endif
			tprints("}");
		}
		else
			tprintf("{sys=\"%s\", node=\"%s\", ...}",
				uname.sysname, uname.nodename);
	}
	return 0;
}
