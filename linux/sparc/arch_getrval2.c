long
getrval2(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	return sparc_regs.u_regs[U_REG_O1];
}
