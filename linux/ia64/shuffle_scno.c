static_assert(!(IA64_SCNO_BASE & (IA64_SCNO_BASE - 1)),
	      "IA64_SCNO_BASE is not a power of 2 (or zero)");
static_assert(IA64_SCNO_BASE > nsyscalls0,
	      "syscall table is too big, shuffling will only make everything "
	      "worse");

kernel_ulong_t
shuffle_scno(kernel_ulong_t scno)
{
	return scno ^ IA64_SCNO_BASE;
}
