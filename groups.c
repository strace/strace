#include "defs.h"

#include <asm/posix_types.h>
#undef GETGROUPS_T
#define GETGROUPS_T __kernel_gid_t
#undef GETGROUPS32_T
#define GETGROUPS32_T __kernel_gid32_t

int
sys_setgroups(struct tcb *tcp)
{
	if (entering(tcp)) {
		unsigned long len, size, start, cur, end, abbrev_end;
		GETGROUPS_T gid;
		int failed = 0;

		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_getgroups(struct tcb *tcp)
{
	unsigned long len;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
	} else {
		unsigned long size, start, cur, end, abbrev_end;
		GETGROUPS_T gid;
		int failed = 0;

		len = tcp->u_rval;
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		if (tcp->u_arg[0] == 0) {
			tprintf("%#lx", start);
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || tcp->u_arg[0] == 0 ||
		    size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setgroups32(struct tcb *tcp)
{
	if (entering(tcp)) {
		unsigned long len, size, start, cur, end, abbrev_end;
		GETGROUPS32_T gid;
		int failed = 0;

		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_getgroups32(struct tcb *tcp)
{
	unsigned long len;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
	} else {
		unsigned long size, start, cur, end, abbrev_end;
		GETGROUPS32_T gid;
		int failed = 0;

		len = tcp->u_rval;
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || tcp->u_arg[0] == 0 ||
		    size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}
