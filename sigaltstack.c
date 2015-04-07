#include "defs.h"
#include "xlat/sigaltstack_flags.h"

static void
print_stack_t(struct tcb *tcp, unsigned long addr)
{
	stack_t ss;
	int r;

	if (!addr) {
		tprints("NULL");
		return;
	}

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize != sizeof(ss.ss_sp) && current_wordsize == 4) {
		struct {
			uint32_t ss_sp;
			int32_t ss_flags;
			uint32_t ss_size;
		} ss32;
		r = umove(tcp, addr, &ss32);
		if (r >= 0) {
			memset(&ss, 0, sizeof(ss));
			ss.ss_sp = (void*)(unsigned long) ss32.ss_sp;
			ss.ss_flags = ss32.ss_flags;
			ss.ss_size = (unsigned long) ss32.ss_size;
		}
	} else
#endif
	{
		r = umove(tcp, addr, &ss);
	}
	if (r < 0) {
		tprintf("%#lx", addr);
	} else {
		tprintf("{ss_sp=%#lx, ss_flags=", (unsigned long) ss.ss_sp);
		printflags(sigaltstack_flags, ss.ss_flags, "SS_???");
		tprintf(", ss_size=%lu}", (unsigned long) ss.ss_size);
	}
}

SYS_FUNC(sigaltstack)
{
	if (entering(tcp)) {
		print_stack_t(tcp, tcp->u_arg[0]);
	} else {
		tprints(", ");
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}
