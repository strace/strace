#!/bin/sh -efu
#
# Check that ESRCH from PTRACE_GETSIGINFO does not cause a false group-stop
# and that the subsequent termination events are still handled.
#
# Model the termination race by forcing the siginfo query to fail, rather
# than relying on a tracee to die between wait4 and PTRACE_GETSIGINFO.
# Arguments are passed to the inner strace; the seccomp variant adds
# --seccomp-bpf.
#
# Copyright (c) 2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_prog grep
check_prog sed

fault_helper='../../src/disable_ptrace_getsiginfo'
$fault_helper true || skip_ "$fault_helper does not work"
[ $# -eq 0 ] || . "${srcdir=.}/filter_seccomp.sh"

tracee_pid=
cleanup()
{
	[ -z "$tracee_pid" ] || kill -KILL "$tracee_pid" 2> /dev/null ||:
}
trap cleanup EXIT

query='^ptrace(PTRACE_GETSIGINFO, \([0-9]*\), '
> "$OUT"
> "$LOG"
# The outer strace records ptrace calls in OUT.  The helper installs a
# seccomp filter in the inner strace to make PTRACE_GETSIGINFO return ESRCH.
# The tracee raises SIGTERM (15), causing the inner strace to query siginfo.
# Its signal output goes to LOG, which must contain no false group-stop.
../run_expect_termsig 9 \
	$STRACE -qq -e trace=ptrace -e signal=none -o "$OUT" \
	$fault_helper $STRACE -qq -f -e trace=none -o "$LOG" "$@" \
	../unblock_reset_raise 15 2> "$LOG.err" &
runner_pid=$!

# Wait until the outer trace confirms the failed query.  This synchronizes
# the kill below with the injected error and also gives us the tracee PID.
while :; do
	tracee_pid=$(sed -n "s/$query.* = -1 ESRCH .*/\1/p" "$OUT")
	[ -z "$tracee_pid" ] || break
	kill -0 "$runner_pid" 2> /dev/null || {
		cat "$OUT" "$LOG.err" >&2
		dump_log_and_fail_with 'tracer exited before GETSIGINFO failed'
	}
	$SLEEP_A_BIT
done

# Unlike a real termination race, the forced ESRCH leaves the tracee in its
# SIGTERM delivery stop.  Kill it now to supply real termination events,
# which the inner strace must process after skipping the stale stop.
kill -KILL "$tracee_pid" 2> /dev/null ||:
# Both strace processes propagate the tracee's SIGKILL (9) termination;
# run_expect_termsig turns that expected outcome into a successful exit.
wait "$runner_pid" || {
	cat "$OUT" "$LOG.err" >&2
	dump_log_and_fail_with 'tracer did not terminate with SIGKILL'
}

printf '%s +++ killed by SIGKILL +++\n' "$tracee_pid" > "$EXP"
tracee_pid=
match_diff "$LOG" "$EXP" 'unexpected signal or group-stop output'
match_diff "$LOG.err" /dev/null 'unexpected tracer diagnostics'
