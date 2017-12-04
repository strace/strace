#ifdef HAVE_STRUCT_KVM_REGS
static void
arch_print_kvm_regs(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const struct kvm_regs *const regs)
{
	PRINT_FIELD_X("{", *regs, rax);
	if (abbrev(tcp))
		tprints(", ...");
	else {
		PRINT_FIELD_X(", ",  *regs, rbx);
		PRINT_FIELD_X(", ",  *regs, rcx);
		PRINT_FIELD_X(", ",  *regs, rdx);
		PRINT_FIELD_X(", ",  *regs, rsi);
		PRINT_FIELD_X(", ",  *regs, rdi);
	}
	PRINT_FIELD_X(", ",  *regs, rsp);
	PRINT_FIELD_X(", ",  *regs, rbp);
	if (abbrev(tcp))
		tprints(", ...");
	else {
		PRINT_FIELD_X(", ",  *regs, r8);
		PRINT_FIELD_X(", ",  *regs, r9);
		PRINT_FIELD_X(", ",  *regs, r10);
		PRINT_FIELD_X(", ",  *regs, r11);
		PRINT_FIELD_X(", ",  *regs, r12);
		PRINT_FIELD_X(", ",  *regs, r13);
		PRINT_FIELD_X(", ",  *regs, r14);
		PRINT_FIELD_X(", ",  *regs, r15);
	}
	PRINT_FIELD_X(", ",  *regs, rip);

	/* TODO: we can decode this more */
	PRINT_FIELD_X(", ",  *regs, rflags);

	tprints("}");
}
#endif	/* HAVE_STRUCT_KVM_REGS */
