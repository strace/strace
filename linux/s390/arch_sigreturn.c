#ifndef S390_FRAME_PTR
# define S390_FRAME_PTR s390_frame_ptr
#endif
#ifndef SIGNAL_FRAMESIZE
# define SIGNAL_FRAMESIZE __SIGNAL_FRAMESIZE
#endif
#ifndef PTR_TYPE
# define PTR_TYPE unsigned long
#endif

static void
arch_sigreturn(struct tcb *tcp)
{
	PTR_TYPE mask[NSIG_BYTES / sizeof(PTR_TYPE)];
	const PTR_TYPE addr = *S390_FRAME_PTR + SIGNAL_FRAMESIZE;

	if (umove(tcp, addr, &mask) < 0) {
		tprintf("{mask=%#llx}", zero_extend_signed_to_ull(addr));
	} else {
		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
