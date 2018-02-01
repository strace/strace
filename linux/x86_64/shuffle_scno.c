kernel_ulong_t
shuffle_scno(kernel_ulong_t scno)
{
	if (current_personality == 2)
		scno ^= __X32_SYSCALL_BIT;

	return scno;
}
