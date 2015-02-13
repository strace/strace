/* SH64 Linux - this code assumes the following kernel API for system calls:
        PC           Offset 0
        System Call  Offset 16 (actually, (syscall no.) | (0x1n << 16),
                     where n = no. of parameters.
        Other regs   Offset 24+

        On entry:    R2-7 = parameters 1-6 (as many as necessary)
        On return:   R9   = result.
*/

/* Offset for peeks of registers */
#define REG_OFFSET         (24)
#define REG_GENERAL(x)     (8*(x)+REG_OFFSET)
#define REG_PC             (0*8)
#define REG_SYSCALL        (2*8)
