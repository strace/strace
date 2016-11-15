#ifndef PTRACE_SET_SYSCALL
# define PTRACE_SET_SYSCALL 23
#endif

static int
arch_set_scno(struct tcb *tcp, long scno)
{
	unsigned int n = (uint16_t) scno;
	int rc = ptrace(PTRACE_SET_SYSCALL, tcp->pid, NULL, (unsigned long) n);
	if (rc && errno != ESRCH)
		perror_msg("arch_set_scno: PTRACE_SET_SYSCALL pid:%d scno:%#x",
			   tcp->pid, n);
	return rc;
}
