#!/bin/sh
#
# Define syntax testing primitives.
#
# Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2016-2017 The strace developers.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

. "${srcdir=.}/init.sh"

check_exit_status_and_stderr()
{
	$STRACE "$@" 2> "$LOG" &&
		dump_log_and_fail_with \
			"strace $* failed to handle the error properly"
	match_diff "$LOG" "$EXP" ||
		dump_log_and_fail_with \
			"strace $* failed to print expected diagnostics"
}

check_exit_status_and_stderr_using_grep()
{
	$STRACE "$@" 2> "$LOG" &&
		dump_log_and_fail_with \
			"strace $* failed to handle the error properly"
	match_grep "$LOG" "$EXP" ||
		dump_log_and_fail_with \
			"strace $* failed to print expected diagnostics"
}

check_e()
{
	local pattern="$1"; shift
	cat > "$EXP" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr "$@"
}

check_e_using_grep()
{
	local pattern="$1"; shift
	cat > "$EXP" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr_using_grep "$@"
}

check_h()
{
	local pattern="$1"; shift
	cat > "$EXP" << __EOF__
$STRACE_EXE: $pattern
Try '$STRACE_EXE -h' for more information.
__EOF__
	check_exit_status_and_stderr "$@"
}
