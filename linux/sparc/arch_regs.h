extern struct pt_regs sparc_regs;
/* Indexes into the pt_regs.u_reg[] array -- UREG_XX from kernel are all off
 * by 1 and use Ix instead of Ox.  These work for both 32 and 64 bit Linux. */
#define U_REG_G1 0
#define U_REG_O0 7
#define U_REG_O1 8
#define U_REG_FP 13
