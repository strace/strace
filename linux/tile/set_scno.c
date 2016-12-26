static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	tile_regs.regs[10] = scno;
	return set_regs(tcp->pid);
}
