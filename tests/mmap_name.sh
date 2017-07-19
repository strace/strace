#!/bin/sh
#
# Define mmap testing helper function.
#
# Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
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

get_mmap_name()
{
	check_prog grep
	run_prog > /dev/null

	local filter="$1"; shift
	local syscall=
	for n in mmap mmap2; do
		$STRACE -e$n -h > /dev/null && syscall=$syscall,$n
	done
	run_strace -e "trace(syscall $syscall $filter)" $args > /dev/null

	if grep '^mmap(NULL, 0, PROT_NONE,' < "$LOG" > /dev/null; then
		mmap=mmap
	elif grep '^mmap2(NULL, 0, PROT_NONE,' < "$LOG" > /dev/null; then
		mmap=mmap2
	else
		dump_log_and_fail_with "mmap/mmap2 not found in $STRACE $args output"
	fi
}
