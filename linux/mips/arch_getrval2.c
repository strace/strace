long
getrval2(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	return mips_regs.uregs[3];
}
