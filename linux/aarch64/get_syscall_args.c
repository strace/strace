if (tcp->currpers == 1) {
	tcp->u_arg[0] = aarch64_regs.regs[0];
	tcp->u_arg[1] = aarch64_regs.regs[1];
	tcp->u_arg[2] = aarch64_regs.regs[2];
	tcp->u_arg[3] = aarch64_regs.regs[3];
	tcp->u_arg[4] = aarch64_regs.regs[4];
	tcp->u_arg[5] = aarch64_regs.regs[5];
} else {
#include "arm/get_syscall_args.c"
}
