#!/bin/sh
#
# Check that PTRACE_O_EXITKILL is set properly.
#
# Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

max_wait_attempts=100

$STRACE -d -f $kill_on_exit_option -e trace=fchdir / > /dev/null 2> "$LOG" ||:
if grep -x "[^:]*strace: PTRACE_O_EXITKILL does not work" "$LOG" > /dev/null; then
	skip_ 'PTRACE_O_EXITKILL does not work'
fi
grep -x "[^:]*strace: PTRACE_O_EXITKILL works" "$LOG" > /dev/null ||
        fail_ 'PTRACE_O_EXITKILL marker not found'

check_prog sed
run_prog_skip_if_failed \
	kill -0 $$

cat >script <<EOF
#!/bin/sh -efu
trap '' TERM
echo ONE
# Do not wait forever, stop waiting after a few iterations.
attempt=0
while [ "\$attempt" -lt "$max_wait_attempts" ] && [ ! -s 'killed' ]; do
        $SLEEP_A_BIT
        attempt=\$((attempt + 1))
done
[ -s 'killed' ] || {
	echo TIMEOUT
	exit 1
}
echo TWO
EOF
chmod +x script

run_tracer()
{
	local attempt expected_rc tracee_pid tracer_pid rc=0
	expected_rc="$1"; shift

	> "$OUT"
	> killed

	$STRACE -d -D -I2 -f -e/exit "$@" ./script > "$OUT" 2> "$LOG" &
	tracee_pid=$!

	# Do not wait forever, stop waiting after a few iterations.
	attempt=0
	while [ "$attempt" -lt "$max_wait_attempts" ] && [ ! -s "$OUT" ]; do
		$SLEEP_A_BIT
		attempt=$((attempt + 1))
	done
	[ -s "$OUT" ] ||
		fail_ 'timeout waiting for the script output'

	tracer_pid=$(sed -n 's/^[^:]*strace: new tracer pid is \([[:digit:]]\+\)$/\1/p' "$LOG")
	[ -n "$tracer_pid" ] || {
		kill -9 $tracee_pid
		fail_ 'tracer pid not found'
	}
	kill $tracer_pid

	# Do not wait forever, stop waiting after a few iterations.
	attempt=0
	while [ "$attempt" -lt "$max_wait_attempts" ] && kill -0 "$tracer_pid" 2> /dev/null; do
		$SLEEP_A_BIT
		attempt=$((attempt + 1))
	done
	[ "$attempt" -lt "$max_wait_attempts" ] ||
		fail_ 'timeout waiting for the tracer to terminate'

	echo > killed
	wait $tracee_pid ||
		rc=$?
	[ "$rc" = "$expected_rc" ] ||
		fail_ "expected rc $expected_rc, got rc $rc"
	match_diff "$OUT" "$EXP"
}

# first time without $kill_on_exit_option
{
	echo ONE
	echo TWO
} > "$EXP"
run_tracer 0

# second time with $kill_on_exit_option
echo ONE > "$EXP"
run_tracer 137 $kill_on_exit_option
