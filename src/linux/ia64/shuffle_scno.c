static_assert(!(SYSCALLENT_BASE_NR & (SYSCALLENT_BASE_NR - 1)),
	      "SYSCALLENT_BASE_NR is not a power of 2 (or zero)");
static_assert(nsyscalls0 < SYSCALLENT_BASE_NR,
	      "syscall table is too big, "
	      "shuffling will only make everything worse");

kernel_ulong_t
shuffle_scno(kernel_ulong_t scno)
{
	return scno ^ SYSCALLENT_BASE_NR;
}
