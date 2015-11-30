/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = avr32_regs.r12;
	tcp->u_arg[1] = avr32_regs.r11;
	tcp->u_arg[2] = avr32_regs.r10;
	tcp->u_arg[3] = avr32_regs.r9;
	tcp->u_arg[4] = avr32_regs.r5;
	tcp->u_arg[5] = avr32_regs.r3;
	return 1;
}
