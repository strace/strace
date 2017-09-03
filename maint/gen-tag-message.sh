#!/bin/sh -efu
# Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

get_commit_id()
{
	git rev-parse --verify "$1^{commit}"
}

id="$(get_commit_id "${1:-@}")"

tmpf=
cleanup()
{
	trap - EXIT
	[ -z "$tmpf" ] ||
		rm -f -- "$tmpf"
	exit "$@"
}

trap 'cleanup $?' EXIT
trap 'cleanup 1' HUP PIPE INT QUIT TERM
tmpf="$(mktemp -t "${0##*/}.XXXXXX")"

git show "$id:NEWS" > "$tmpf"
marker='^Noteworthy changes in release \([^ ]\+\) ([^)]\+)$'
vers="$(sed -n "s/$marker/\\1/p;q" "$tmpf")"

msg="NEWS for strace version $vers"
sep="$(echo "$msg" |sed s/./=/g)"
echo "$msg"
echo "$sep"

sed "0,/^$sep/d;/$marker/,\$d" "$tmpf"

cat <<'__EOF__'
Contributors
============

This release was made possible by the contributions of many people.
The maintainers are grateful to everyone who has contributed
changes or bug reports.  These include:

__EOF__

"$(dirname "$0")"/gen-contributors-list.sh
