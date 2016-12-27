static void
arch_sigreturn(struct tcb *tcp)
{
	/* offsetof(struct sigframe, sc) */
#define OFFSETOF_STRUCT_SIGFRAME_SC	0xA0
	const unsigned long addr = *ia64_frame_ptr + 16 +
				   OFFSETOF_STRUCT_SIGFRAME_SC +
				   offsetof(struct sigcontext, sc_mask);
	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG_BYTES);
	tprints("}");
}
