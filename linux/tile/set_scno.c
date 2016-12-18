static int
arch_set_scno(struct tcb *tcp, kernel_scno_t scno)
{
	tile_regs.regs[10] = scno;
	return set_regs(tcp->pid);
}
