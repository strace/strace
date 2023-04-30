#! /bin/sh -efu
#
# Common code for strace --trace=all@pers tests.
#
# Copyright (c) 2018-2023 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

[ "$#" -eq 1 ] ||
	fail_ 'No personality designation ("64", "32", "x32") specified'

target_pers="$1"
shift

trace_expr="all@$target_pers"

case "$STRACE_NATIVE_ARCH" in
x86_64)
	native_pers='64'
	supported_pers='64 32 x32'
	;;
x32)
	native_pers='x32'
	supported_pers='x32 32'
	;;
aarch64|powerpc64|s390x|sparc64|tile)
	native_pers='64'
	supported_pers='64 32'
	;;
*)
	native_pers=$((SIZEOF_LONG * 8))
	supported_pers=$native_pers
	;;
esac

pers_found=
for i in $supported_pers; do
	if [ "$target_pers" = "$i" ]; then
		pers_found=1
		break
	fi
done

[ -n "$pers_found" ] ||
	skip_ "Personality '$target_pers' is not supported on architecture" \
	      "'$STRACE_NATIVE_ARCH' (supported personalities: $supported_pers)"

cur_pers=$(print_current_personality_designator)
if [ "$target_pers" = "$cur_pers" ]; then
	skip_ "$trace_expr would match all syscalls on $STRACE_ARCH"
fi

if [ "$target_pers" != "$native_pers" ]; then
	# reset $NAME, so that "$NAME.in" used by test_trace_expr
	# would point to an empty file.
	NAME=qualify_personality_empty

	test_trace_expr '' --trace="$trace_expr"
	exit
fi

if [ "$cur_pers:$target_pers" = 'x32:64' ]; then
	skip_ "x32 executables may invoke $STRACE_NATIVE_ARCH syscalls"
fi

# $trace_expr would match the native execve
cat > "$EXP" <<'__EOF__'
execve(at)?\(.*
__EOF__

check_prog head
check_prog tail

while read -r t; do {
	# skip lines beginning with "#" symbol
	case "$t" in
		''|'#'*) continue ;;
	esac

	try_run_prog "../$t" || continue
	run_strace -qq -esignal=none --trace="$trace_expr" "../$t" > /dev/null

	head -n1 < "$LOG" > "$OUT"
	match_grep "$OUT" "$EXP"

	tail -n +2 < "$LOG" > "$OUT"
	match_diff "$OUT" /dev/null
} < /dev/null; done < "$srcdir/pure_executables.list"
