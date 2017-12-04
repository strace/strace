#ifdef HAVE_STRUCT_KVM_REGS
static void
arch_print_kvm_regs(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const struct kvm_regs *const regs)
{
	printaddr(addr);
}
#endif	/* HAVE_STRUCT_KVM_REGS */

#ifdef HAVE_STRUCT_KVM_SREGS
static void
arch_print_kvm_sregs(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const struct kvm_sregs *const sregs)
{
	printaddr(addr);
}
#endif	/* HAVE_STRUCT_KVM_SREGS */
