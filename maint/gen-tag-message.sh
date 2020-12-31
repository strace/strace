#!/bin/sh -efu
# Copyright (c) 2017 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2017-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

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

msg_date=`LC_TIME=C date -u '+%Y-%m-%d'`
msg="Noteworthy changes in strace $vers ($msg_date)"
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

"$(dirname "$0")"/gen-contributors-list.sh |
	sed 's/^./* &/'

cat <<'__EOF__'

Please refer to the CREDITS file for the full list of strace contributors.
__EOF__
