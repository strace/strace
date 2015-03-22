scno = ppc_regs.gpr[0];
unsigned int currpers;

/*
 * Check for 64/32 bit mode.
 * Embedded implementations covered by Book E extension of PPC use
 * bit 0 (CM) of 32-bit Machine state register (MSR).
 * Other implementations use bit 0 (SF) of 64-bit MSR.
 */
currpers = (ppc_regs.msr & 0x8000000080000000) ? 0 : 1;
update_personality(tcp, currpers);
