#ifndef SIZEOF_STRUCT_SPARC_STACKF
# define SIZEOF_STRUCT_SPARC_STACKF	sizeof(struct sparc_stackf)
#endif
#ifndef SIZEOF_STRUCT_PT_REGS
# define SIZEOF_STRUCT_PT_REGS		sizeof(struct pt_regs)
#endif
#ifndef PERSONALITY_WORDSIZE
# define PERSONALITY_WORDSIZE		PERSONALITY0_WORDSIZE
#endif

static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr = sparc_regs.u_regs[U_REG_FP] +
		SIZEOF_STRUCT_SPARC_STACKF + SIZEOF_STRUCT_PT_REGS;
	struct {
		unsigned int mask;
		char fpu_save[PERSONALITY_WORDSIZE];
		char insns[PERSONALITY_WORDSIZE * 2] ATTRIBUTE_ALIGNED(8);
		unsigned int extramask[NSIG_BYTES / sizeof(int) - 1];
	} frame;

	if (umove(tcp, addr, &frame) < 0) {
		tprintf("{mask=%#lx}", addr);
	} else {
		unsigned int mask[NSIG_BYTES / sizeof(int)];

		mask[0] = frame.mask;
		memcpy(mask + 1, frame.extramask, sizeof(frame.extramask));
		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}

#undef PERSONALITY_WORDSIZE
#undef SIZEOF_STRUCT_PT_REGS
#undef SIZEOF_STRUCT_SPARC_STACKF
