static inline const char *
nr_prefix(kernel_ulong_t scno)
{
	/*
	 * For now, the set of syscalls that are shuffled is equivalent to the
	 * set of syscalls that have __ARM_NR_ prefix.
	 */
	if (shuffle_scno(scno) != scno)
		return "__ARM_NR_";
	else
		return "__NR_";
}
