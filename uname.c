#include "defs.h"

#include <sys/utsname.h>

SYS_FUNC(uname)
{
	struct utsname uname;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &uname)) {
#define PRINT_UTS_MEMBER(prefix, member) \
		tprints(prefix #member "="); \
		print_quoted_string(uname.member, sizeof(uname.member), \
				    QUOTE_0_TERMINATED)

		PRINT_UTS_MEMBER("{", sysname);
		PRINT_UTS_MEMBER(", ", nodename);
		if (abbrev(tcp)) {
			tprints(", ...}");
			return 0;
		}
		PRINT_UTS_MEMBER(", ", release);
		PRINT_UTS_MEMBER(", ", version);
		PRINT_UTS_MEMBER(", ", machine);
#ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
		PRINT_UTS_MEMBER(", ", domainname);
#endif
		tprints("}");
	}

	return 0;
}
