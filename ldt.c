#include "defs.h"

#if defined I386 || defined X86_64 || defined X32

# include <asm/ldt.h>

void
print_user_desc(struct tcb *tcp, long addr)
{
	struct user_desc desc;

	if (umove(tcp, addr, &desc) < 0) {
		tprintf("%lx", addr);
		return;
	}

	if (!verbose(tcp)) {
		tprintf("{entry_number:%d, ...}", desc.entry_number);
		return;
	}

	tprintf("{entry_number:%d, "
		"base_addr:%#08x, "
		"limit:%d, "
		"seg_32bit:%d, "
		"contents:%d, "
		"read_exec_only:%d, "
		"limit_in_pages:%d, "
		"seg_not_present:%d, "
		"useable:%d}",
		desc.entry_number,
		desc.base_addr,
		desc.limit,
		desc.seg_32bit,
		desc.contents,
		desc.read_exec_only,
		desc.limit_in_pages,
		desc.seg_not_present,
		desc.useable);
}

int
sys_modify_ldt(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		if (tcp->u_arg[1] == 0
		    || tcp->u_arg[2] != sizeof(struct user_desc)) {
			tprintf("%lx", tcp->u_arg[1]);
		} else {
			print_user_desc(tcp, tcp->u_arg[1]);
		}
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_set_thread_area(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_user_desc(tcp, tcp->u_arg[0]);
	} else {
		struct user_desc desc;

		if (syserror(tcp) || umove(tcp, tcp->u_arg[0], &desc) < 0) {
			/* returned entry_number is not available */
		} else {
			static char outstr[32];

			sprintf(outstr, "entry_number:%d", desc.entry_number);
			tcp->auxstr = outstr;
			return RVAL_STR;
		}
	}
	return 0;
}

int
sys_get_thread_area(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%lx", tcp->u_arg[0]);
		else
			print_user_desc(tcp, tcp->u_arg[0]);
	}
	return 0;
}

#endif /* I386 || X86_64 || X32 */

#if defined(M68K) || defined(MIPS)
int
sys_set_thread_area(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%#lx", tcp->u_arg[0]);
	return 0;

}
#endif

#if defined(M68K)
int
sys_get_thread_area(struct tcb *tcp)
{
	return RVAL_HEX;
}
#endif
