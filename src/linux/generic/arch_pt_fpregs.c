static void
arch_decode_pt_fpregs(struct tcb *const tcp, const kernel_ulong_t addr)
{
	arch_decode_fpregset(tcp, addr,
#if HAVE_ARCH_FPREGSET
			sizeof(struct_fpregset)
#else
			0
#endif
		);
}
