#!/bin/sh
# Find new constants in updated Linux headers that should be considered
# for addition to xlat files.
#
# Copyright (c) 2025 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

set -eu

error()
{
	printf '%s: error: %s\n' "$0" "$*" >&2
}

fatal()
{
	error "$@"
	exit 1
}

usage()
{
	rc=0
	if [ $# -gt 0 ]; then
		error "$@"
		exec >&2
		rc=1
	fi
	cat <<-EOF
		Usage: $0 -d LINUX_REPO COMMIT_RANGE

		Find new constants in Linux headers between commits that should
		be added to xlat files.  Reads table from stdin in format:
		xlat_file<TAB>line_type<TAB>header_file.  Outputs results to
		stdout.

		Options:
		  -d LINUX_REPO      Path to Linux kernel source directory
					(mandatory)
		Arguments:
		  COMMIT_RANGE       Git commit range (e.g., COMMIT1..COMMIT2)

		Examples:
		  $0 -d /path/to/linux v6.17..v6.18-rc4 < headers_table.txt \\
		      > new_constants.txt
	EOF
	exit $rc
}

LINUX_REPO=
COMMIT_RANGE=

while [ $# -gt 0 ]; do
	case "$1" in
		-d)
			shift
			[ $# -gt 0 ] ||
				usage "-d requires an argument"
			LINUX_REPO="$1"
			;;
		-h|--help)
			usage
			;;
		-*)
			usage "unknown option: $1"
			;;
		*)
			break
			;;
	esac
	shift
done

[ $# -eq 1 ] ||
	usage "exactly one argument is required"

COMMIT_RANGE="$1"

[ -n "$LINUX_REPO" ] ||
	usage "-d LINUX_REPO is mandatory"

# Validate inputs
[ -d "$LINUX_REPO" ] ||
	fatal "Linux repository not found: $LINUX_REPO"

# Validate commit range format
case "$COMMIT_RANGE" in
	*...*)
		fatal "Three-dot syntax (COMMIT1...COMMIT2) is not supported"
		;;
	*..*)
		# Valid: two-dot range syntax
		;;
	*)
		fatal "Single commit is not supported, use COMMIT1..COMMIT2"
		;;
esac

# Validate commit range endpoints
COMMIT1="${COMMIT_RANGE%%..*}"
COMMIT2="${COMMIT_RANGE#*..}"

cd "$LINUX_REPO" || exit 1

GIT_DIR=$(git rev-parse --absolute-git-dir)

git --git-dir="$GIT_DIR" rev-parse "$COMMIT1^{commit}" >/dev/null ||
	fatal "Commit $COMMIT1 not found in $LINUX_REPO"

git --git-dir="$GIT_DIR" rev-parse "$COMMIT2^{commit}" >/dev/null ||
	fatal "Commit $COMMIT2 not found in $LINUX_REPO"

cd - > /dev/null || exit 1

# Global temporary files (created once before main loop)
xlat_constants_file=
v1_file=
v2_file=
new_constants_file=

# Global cleanup function for temporary files
cleanup()
{
	trap - EXIT HUP PIPE INT QUIT TERM
	[ -z "$xlat_constants_file" ] || rm -f -- "$xlat_constants_file"
	[ -z "$v1_file" ] || rm -f -- "$v1_file"
	[ -z "$v2_file" ] || rm -f -- "$v2_file"
	[ -z "$new_constants_file" ] || rm -f -- "$new_constants_file"
}

# Extract constants from xlat file
extract_xlat_constants()
{
	local xlat_file="$1"

	[ -f "$xlat_file" ] || return 1

	# Remove comments, directives, and extract constant names
	sed -n 's/^\(1<<\)\?\([A-Z_][A-Z0-9_]*\).*/\2/p' \
		"$xlat_file" |
		sort -u
}

# Calculate longest common prefix from constants in a file
calculate_prefix()
{
	local constants_file="$1"
	local xlat_file="$2"
	local prefix basename_file

	# Determine longest common prefix from xlat constants using awk
	prefix=$(awk '
		BEGIN { prefix = "" }
		{ sub(/[^_]*$/, "", $0) }
		NR == 1 { prefix = $0; next }
		{
			line_len = length($0)
			prefix_len = length(prefix)
			min_len = line_len < prefix_len ? line_len : prefix_len
			for (i = 1; i <= min_len; i++) {
				if (substr(prefix, i, 1) != substr($0, i, 1)) {
					prefix = substr(prefix, 1, i - 1)
					break
				}
			}
			if (i > min_len && line_len < prefix_len)
				prefix = substr(prefix, 1, line_len)
		}
		END { print prefix }
	' < "$constants_file")

	# If prefix is empty, infer from filename
	[ -n "$prefix" ] || {
		# Extract prefix from filename
		# (e.g., kvm_cpuid_flags.in -> KVM_CPUID_FLAG)
		basename_file=$(basename "$xlat_file" .in)
		prefix=$(echo "$basename_file" |
			tr '[:lower:]' '[:upper:]' |
			sed 's/_FLAGS$/_FLAG/; s/_CMDS$/_CMD/')
	}

	echo "$prefix"
}

# Extract constants from Linux header matching a prefix pattern
extract_header_constants()
{
	local commit="$1"
	local header_file="$2"
	local prefix="$3"
	local define_p enum_v enum_c enum_cc enum_ccl enum_l
	local full_p sed_def sed_enum

	# Match both #define and enum constants
	define_p="^[[:space:]]*#[[:space:]]*define[[:space:]]\+${prefix}[A-Z0-9_]*\b"
	enum_v="^[[:space:]]*${prefix}[A-Z0-9_]*[[:space:]]*="
	enum_c="^[[:space:]]*${prefix}[A-Z0-9_]*[[:space:]]*,"
	enum_cc="^[[:space:]]*${prefix}[A-Z0-9_]*[[:space:]]*/\*.*\*/[[:space:]]*,"
	enum_ccl="^[[:space:]]*${prefix}[A-Z0-9_]*[[:space:]]*/\*.*\*/[[:space:]]*$"
	enum_l="^[[:space:]]*${prefix}[A-Z0-9_]*[[:space:]]*$"
	full_p="${define_p}\|${enum_v}\|${enum_c}\|${enum_cc}\|${enum_ccl}\|${enum_l}"
	sed_def="s/^[[:space:]]*#[[:space:]]*define[[:space:]]*\([A-Z0-9_]*\).*/\1/"
	sed_enum="s/^[[:space:]]*\([A-Z0-9_]*\).*/\1/"

	git --git-dir="$GIT_DIR" show "${commit}:${header_file}" 2>/dev/null |
		grep "$full_p" |
		sed -e "$sed_def" -e "$sed_enum" |
		sort -u
}

# Process a single line from the table
process_line()
{
	local xlat_file="$1"
	local line_type="$2"
	local header_file="$3"
	local prefix basename_file

	# Skip empty lines
	[ -n "$xlat_file" ] || return 0

	# Check if files exist
	[ -f "$xlat_file" ] || return 0

	git --git-dir="$GIT_DIR" cat-file -e "${COMMIT1}:${header_file}" >/dev/null 2>&1 ||
		return 0

	git --git-dir="$GIT_DIR" cat-file -e "${COMMIT2}:${header_file}" >/dev/null 2>&1 ||
		return 0

	# Extract constants from xlat file to temporary file
	extract_xlat_constants "$xlat_file" > "$xlat_constants_file" ||
		return 1

	# Skip if no constants found in xlat file
	[ -s "$xlat_constants_file" ] ||
		return 0

	# Calculate prefix from xlat constants
	prefix=$(calculate_prefix "$xlat_constants_file" "$xlat_file")

	# Extract constants from COMMIT1 and COMMIT2 headers to temporary files
	extract_header_constants "$COMMIT1" "$header_file" "$prefix" > "$v1_file"
	extract_header_constants "$COMMIT2" "$header_file" "$prefix" > "$v2_file"

	# Find new constants (in v2 but not in v1 and not in xlat)
	comm -23 "$v2_file" "$v1_file" |
		comm -23 - "$xlat_constants_file" > "$new_constants_file"

	[ -s "$new_constants_file" ] ||
		return 0

	# Format: xlat_file TAB header_file TAB list of constants
	constant_list=$(tr '\n' ' ' < "$new_constants_file" | sed 's/[[:space:]]*$//')
	printf "%s\t%s\t%s\n" "$xlat_file" "$header_file" "$constant_list"
}

# Process each line from stdin table
trap 'cleanup; exit $?' EXIT
trap 'cleanup; exit 1' HUP PIPE INT QUIT TERM

# Create temporary files once before entering main loop
xlat_constants_file=$(mktemp)
v1_file=$(mktemp)
v2_file=$(mktemp)
new_constants_file=$(mktemp)

while IFS=$'\t' read -r xlat_file line_type header_file; do
	process_line "$xlat_file" "$line_type" "$header_file"
done
