kernel_ulong_t
shuffle_scno(kernel_ulong_t scno)
{
	if (current_personality == 0 && scno != (kernel_ulong_t) -1)
		scno ^= __X32_SYSCALL_BIT;

	return scno;
}
