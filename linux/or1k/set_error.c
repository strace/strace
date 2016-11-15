static int
arch_set_error(struct tcb *tcp)
{
	or1k_regs.gpr[11] = -tcp->u_error;
	return set_regs(tcp->pid);
}
