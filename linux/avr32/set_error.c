static int
arch_set_error(struct tcb *tcp)
{
	avr32_regs.r12 = -tcp->u_error;
	return set_regs(tcp->pid);
}
