#!/bin/sh
# Copyright (c) 2001 Wichert Akkerman <wichert@cistron.nl>
# Copyright (c) 2004-2015 Dmitry V. Levin <ldv@altlinux.org>
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

set -efu

me="${0##*/}"

[ $# -ge 2 ] || {
	echo >&2 "usage: $me include-directory type [files...]"
	exit 1
}

dir="$1"; shift # dir must exist
type="$1"; shift # type might be a regexp

cd "$dir"

regexp='[[:space:]]*#[[:space:]]*define[[:space:]]\+\([A-Z][A-Z0-9_]*\)[[:space:]]\+\(0x'"$type"'..\)\>'

for f; do
	grep "^$regexp" "$f" "uapi/$f" 2>/dev/null ||:;
done |
	sed 's|^uapi/||' |
	sed -n 's/^\([^:]*\):'"$regexp"'.*/{ "\1", "\2", 0, \3, 0 },/p' |
	LC_COLLATE=C sort -u
