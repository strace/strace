static void
arch_decode_pt_regs(struct tcb *const tcp, const kernel_ulong_t addr)
{
	arch_decode_prstatus_regset(tcp, addr,
#if HAVE_ARCH_PRSTATUS_REGSET
			sizeof(struct_prstatus_regset)
#else
			0
#endif
		);
}
