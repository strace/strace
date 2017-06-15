/* [],[aarch64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(aarch64,	64bit,		PASS({ARCH_arm_eabi}),				PASS({"aarch64", "arm64"})			),
/* [],[alpha/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(alpha,		64bit,		PASS({}),					PASS({"alpha"})					),
/* [],[arc/syscallent.h,linux/32/syscallent-time32.h],[0,0] */
ARCH_DESC_DEFINE(arc,		32bit,		PASS({}),					PASS({"arc"})					),
/* [!__ARM_EABI__],[arm/syscallent.h],[ARM_FIRST_SHUFFLED_SYSCALL,ARM_LAST_SPECIAL_SYSCALL] */
ARCH_DESC_DEFINE(arm,		oabi,		PASS({ARCH_arm_eabi}),				PASS({"arm"})					),
/* [__ARM_EABI__],[arm/syscallent.h],[ARM_FIRST_SHUFFLED_SYSCALL,ARM_LAST_SPECIAL_SYSCALL] */
ARCH_DESC_DEFINE(arm,		eabi,		PASS({}),					PASS({})					),
/* [],[avr32/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(avr32,		32bit,		PASS({}),					PASS({"avr32"})					),
/* [],[bfin/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(blackfin,	32bit,		PASS({}),					PASS({"blackfin", "bfin"})			),
/* [],[ia64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(ia64,		64bit,		PASS({}),					PASS({"ia64"})					),
/* [],[m68k/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(m68k,		32bit,		PASS({}),					PASS({"m68k"})					),
/* [],[metag/syscallent.h,linux/32/syscallent-time32.h],[0,0] */
ARCH_DESC_DEFINE(metag,		32bit,		PASS({}),					PASS({"metag"})					),
/* [],[microblaze/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(microblaze,	32bit,		PASS({}),					PASS({"microblaze"})				),
/* [LINUX_MIPSN64],[dummy.h,mips/syscallent-compat.h,mips/syscallent-n64.h],[0,0] */
ARCH_DESC_DEFINE(mips64,	n64,		PASS({ARCH_mips64_n32, ARCH_mips_o32}),		PASS({"mips64", "mips64le"})			),
/* [LINUX_MIPSN32],[dummy.h,mips/syscallent-compat.h,mips/syscallent-n32.h],[0,0] */
ARCH_DESC_DEFINE(mips64,	n32,		PASS({ARCH_mips_o32}),				PASS({})					),
/* [LINUX_MIPSO32],[dummy.h,mips/syscallent-compat.h,mips/syscallent-o32.h],[0,0] */
ARCH_DESC_DEFINE(mips,		o32,		PASS({}),					PASS({"mips", "mipsle"})			),
/* [],[nios2/syscallent.h,linux/32/syscallent-time32.h],[0,0] */
ARCH_DESC_DEFINE(nios2,		32bit,		PASS({}),					PASS({"nios2"})					),
/* [],[or1k/syscallent.h,linux/32/syscallent-time32.h],[0,0] */
ARCH_DESC_DEFINE(openrisc,	32bit,		PASS({}),					PASS({"openrisc", "or1k"})			),
/* [],[hppa/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(hppa,		32bit,		PASS({}),					PASS({"parisc", "hppa"})			),
/* [],[powerpc/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(ppc,		32bit,		PASS({}),					PASS({"ppc", "ppcle", "powerpc"})		),
/* [],[powerpc64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(ppc64,		64bit,		PASS({ARCH_ppc_32bit}),				PASS({"ppc64", "powerpc64"})		),
/* [],[powerpc64le/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(ppc64le,	64bit,		PASS({}),					PASS({"ppc64le", "powerpc64le"})		),
/* [],[riscv64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(riscv64,	64bit,		PASS({}),					PASS({"riscv64"})					),
/* [],[s390/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(s390,		32bit,		PASS({}),					PASS({"s390"})					),
/* [],[s390x/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(s390x,		64bit,		PASS({}),					PASS({"s390x"})					),
/* [],[sh/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(sh,		32bit,		PASS({}),					PASS({"sh"})					),
/* [],[sh64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(sh64,		64bit,		PASS({}),					PASS({"sh64"})					),
/* [],[sparc/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(sparc,		32bit,		PASS({}),					PASS({"sparc"})					),
/* [],[sparc64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(sparc64,	64bit,		PASS({ARCH_sparc_32bit}),			PASS({"sparc64"})				),
/* [],[tile/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(tile,		64bit,		PASS({ARCH_tile_32bit}),			PASS({"tile", "tilegx"})			),
/* [],[tile/syscallent1.h,linux/32/syscallent-time32.h],[0,0] */
ARCH_DESC_DEFINE(tile,		32bit,		PASS({}),					PASS({"tilepro"})				),
/* [],[x86_64/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(x86_64,	64bit,		PASS({ARCH_x86_64_x32, ARCH_x86_32bit}),	PASS({"x86_64", "amd64", "EM64T"})		),
/* [],[x86_64/syscallent2.h],[0,0] */
ARCH_DESC_DEFINE(x86_64,	x32,		PASS({ARCH_x86_32bit}),				PASS({})					),
/* [],[i386/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(x86,		32bit,		PASS({}),					PASS({"x86", "i386", "i486", "i586", "i686"})	),
/* [],[xtensa/syscallent.h],[0,0] */
ARCH_DESC_DEFINE(xtensa,	32bit,		PASS({}),					PASS({"xtensa"})				)
