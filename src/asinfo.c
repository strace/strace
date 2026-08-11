/*
 * asinfo - utility to map syscall names <-> numbers and list syscalls.
 * Standalone: does not link to libstrace, uses a generated name table.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asinfo_syscalls.h"

static void
print_usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [--arch=AUDIT_ARCH_*] --to-names NR...\n"
		"       %s [--arch=AUDIT_ARCH_*] --to-numbers NAME...\n"
		"       %s [--arch=AUDIT_ARCH_*] --list\n"
		"\n"
		"Options:\n"
		"  --arch=VAL    Set target arch (AUDIT_ARCH_* name or hex value)\n"
		"  --to-names    Convert syscall numbers to names\n"
		"  --to-numbers  Convert syscall names to numbers\n"
		"  --list        List all syscalls for the arch (raw_nr name)\n"
		"  -h, --help    Show this help\n",
		prog, prog, prog);
}

static int
parse_uint(const char *s, unsigned int *out)
{
	if (!s || !*s)
		return -1;
	/* Accept 0xNNN and decimal. */
	char *end = NULL;
	unsigned long v = strtoul(s, &end, 0);
	if (end && *end == '\0') {
		*out = (unsigned int) v;
		return 0;
	}
	return -1;
}

int
main(int argc, char *argv[])
{
	const char *prog = argv[0];
	int mode_to_names = 0;
	int mode_to_numbers = 0;
	int mode_list = 0;
	int arch_specified = 0;
	int args_start = argc; /* index of first positional arg */

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (!strcmp(arg, "--to-names")) {
			mode_to_names = 1;
			continue;
		}
		if (!strcmp(arg, "--to-numbers")) {
			mode_to_numbers = 1;
			continue;
		}
		if (!strcmp(arg, "--list")) {
			mode_list = 1;
			continue;
		}
		if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			print_usage(prog);
			return 0;
		}
		if (!strncmp(arg, "--arch=", 7)) { arch_specified = 1; continue; }
		args_start = i;
		break;
	}

	if (mode_to_names + mode_to_numbers + mode_list != 1) {
		print_usage(prog);
		return 1;
	}

	if (arch_specified) {
		fprintf(stderr, "%s: --arch is not supported in this build\n", prog);
		return 1;
	}
	if (mode_list) {
		for (unsigned int i = 0; i < asinfo_syscall_count; ++i) {
			const char *name = asinfo_syscall_names[i];
			if (name && *name)
				printf("%u %s\n", i, name);
		}
		return 0;
	}

	if (mode_to_names) {
		if (args_start >= argc) {
			print_usage(prog);
			return 1;
		}
		for (int i = args_start; i < argc; ++i) {
			unsigned int nr;
			if (parse_uint(argv[i], &nr) != 0) {
				fprintf(stderr, "%s: invalid number '%s'\n", prog, argv[i]);
				return 2;
			}
			const char *name = (nr < asinfo_syscall_count) ? asinfo_syscall_names[nr] : NULL;
			if (!name) {
				fprintf(stderr, "%s: unknown syscall nr %u\n",
				        prog, nr);
				return 3;
			}
			puts(name);
		}
		return 0;
	}

	/* mode_to_numbers */
	if (args_start >= argc) {
		print_usage(prog);
		return 1;
	}
	for (int i = args_start; i < argc; ++i) {
		const char *name = argv[i];
		unsigned int found = asinfo_syscall_count;
		for (unsigned int j = 0; j < asinfo_syscall_count; ++j) {
			if (asinfo_syscall_names[j] && strcmp(name, asinfo_syscall_names[j]) == 0) {
				found = j;
				break;
			}
		}
		if (found == asinfo_syscall_count) {
			fprintf(stderr, "%s: unknown syscall name '%s'\n", prog, name);
			return 4;
		}
		printf("%u\n", found);
	}
	return 0;
}


