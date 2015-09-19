#include "defs.h"

#include DEF_MPERS_TYPE(seccomp_fprog_t)

#include "seccomp_fprog.h"
typedef struct seccomp_fprog seccomp_fprog_t;

#include MPERS_DEFS

MPERS_PRINTER_DECL(bool, fetch_seccomp_fprog)(struct tcb *tcp, const long addr, void *p)
{
	struct seccomp_fprog *pfp = p;
	seccomp_fprog_t mfp;

	if (sizeof(*pfp) == sizeof(mfp))
		return !umove_or_printaddr(tcp, addr, pfp);

	if (umove_or_printaddr(tcp, addr, &mfp))
		return false;

	pfp->len = mfp.len;
	pfp->filter = mfp.filter;
	return true;
}
