#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

#include "ref_asinfo_output.h"

static inline void
print_cannot_detect(char *arch_name)
{
	printf("../../asinfo: ABI mode cannot be automatically detected for "
	       "non-target architecture \'%s\'\n", arch_name);
}

int
main(int argc, char *argv[])
{
	struct utsname buf;
	uname(&buf);
#if defined(bfin)
	puts("1" BFIN_32bit_STR);
	return 0;
#endif
#if defined(IA64)
	puts("1" IA64_64bit_STR);
	return 0;
#endif
#if defined(M68K)
	puts("1" M68K_32bit_STR);
#endif
#if defined(SPARC64)
	print_cannot_detect(buf.machine);
	return 0;
#endif
#if defined(SPARC)
	puts("1" SPARC_32bit_STR);
	return 0;
#endif
#if defined(METAG)
	puts("1" METAG_32bit_STR);
	return 0;
#endif
#if defined(MIPS)
	if (strstr(buf.machine, "mips64")) {
		puts(
#if defined(LINUX_MIPSO32)
		     "1" MIPS64_O32_STR
#elif defined(LINUX_MIPSN32)
		     "1" MIPS64_N32_STR
#elif defined(LINUX_MIPSN64)
		     "1" MIPS64_N64_STR
#endif
		);
		return 0;
	}
	if (strstr(buf.machine, "mips")) {
		puts("1" MIPS_O32_STR);
		return 0;
	}
#endif
#if defined(ALPHA)
	puts("1" ALPHA_64bit_STR);
	return 0;
#endif
#if defined(POWERPC64LE)
	puts("1" PPC64LE_64bit_STR);
	return 0;
#elif defined(POWERPC64)
	print_cannot_detect(buf.machine);
	return 0;
#elif defined(POWERPC)
	puts("1" PPC_32bit_STR);
	return 0;
#endif
#if defined(ARM)
	if (strstr(buf.machine, "arm")) {
		puts(
#if defined(__ARM_EABI__) || !defined(ENABLE_ARM_OABI)
		     "1" ARM_eabi_STR
#else
		     "1" ARM_oabi_STR
#endif
		);
		return 0;
	}
#endif
#if defined(AARCH64)
	puts(
#if defined(__ARM_EABI__)
	     "1" AARCH64_eabi_STR
#else
	     "1" AARCH64_64bit_STR
#endif
	);
	return 0;
#endif
#if defined(AVR32)
	puts("1" AVR32_32bit_STR);
	return 0;
#endif
#if defined(ARC)
	puts("1" ARC_32bit_STR);
	return 0;
#endif
#if defined(S390)
	puts("1" S390_32bit_STR);
	return 0;
#endif
#if defined(S390X)
	puts("1" S390X_64bit_STR);
	return 0;
#endif
#if defined(HPPA)
	puts("1" PARISC_32bit_STR);
	return 0;
#endif
#if defined(SH64)
	puts("1" SH64_64bit_STR);
	return 0;
#endif
#if defined(SH)
	puts("1" SH_32bit_STR);
	return 0;
#endif
#if defined(X86_64) || defined(X32)
	puts(
#if defined(X86_64)
	     "1" X86_64_64bit_STR
#elif defined(X32)
	     "1" X86_64_X32_STR
#endif
	);
	return 0;
#endif
#if defined(I386)
	if (strstr(buf.machine, "64"))
		puts("1" X86_64_32bit_STR);
	else
		puts("1" X86_32bit_STR);
	return 0;
#endif
#if defined(TILE)
	puts(
#if defined(__tilepro__)
	     "1" TILE_64bit_STR
#else
	     "1" TILE_32bit_STR
#endif
	);
#endif
#if defined(MICROBLAZE)
	puts("1" MICROBLAZE_32bit_STR);
	return 0;
#endif
#if defined(NIOS2)
	puts("1" NIOS2_32bit_STR);
	return 0;
#endif
#if defined(OR1K)
	puts("1" OR1K_32bit_STR);
	return 0;
#endif
#if defined(XTENSA)
	puts("1" XTENSA_32bit_STR);
	return 0;
#endif
#if defined(RISCV64)
	print_cannot_detect(buf.machine);
	return 0;
#endif
	printf("../../asinfo: architecture \'%s\' is unsupported\n",
	       buf.machine);
	return 0;
}
