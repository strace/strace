static int
arch_set_error(struct tcb *tcp)
{
	tile_regs.regs[0] = -tcp->u_error;
	return set_regs(tcp->pid);
}
