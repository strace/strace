struct mips_regs {
	uint64_t uregs[38];
};

extern struct mips_regs mips_regs;

#define REG_V0 2
#define REG_A0 4

#define mips_REG_V0 mips_regs.uregs[REG_V0]
#define mips_REG_A0 mips_regs.uregs[REG_A0 + 0]
#define mips_REG_A1 mips_regs.uregs[REG_A0 + 1]
#define mips_REG_A2 mips_regs.uregs[REG_A0 + 2]
#define mips_REG_A3 mips_regs.uregs[REG_A0 + 3]
#define mips_REG_A4 mips_regs.uregs[REG_A0 + 4]
#define mips_REG_A5 mips_regs.uregs[REG_A0 + 5]
#define mips_REG_SP mips_regs.uregs[29]
#define mips_REG_EPC mips_regs.uregs[34]
