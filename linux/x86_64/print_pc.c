if (x86_io.iov_len == sizeof(i386_regs))
	tprintf(fmt, (unsigned long) i386_regs.eip);
else
	tprintf(fmt, (unsigned long) x86_64_regs.rip);
