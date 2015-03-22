if (aarch64_io.iov_len == sizeof(arm_regs))
	tprintf(fmt, (unsigned long) arm_regs.ARM_pc);
else
	tprintf(fmt, (unsigned long) aarch64_regs.pc);
