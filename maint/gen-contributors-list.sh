#!/bin/sh -efu
# Copyright (c) 2017 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2017-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

[ "x${D:-0}" != x1 ] || set -x

print_help()
{
	cat <<__EOF__
$0 [-h|--help] [-e|--include-email] [-|--stdin] [LAST_COMMIT [FIRST_COMMIT|--initial]]"

Prints list of contributors for the specified range of commits.

Options:
  -h, --help    Print this message and exit.
  -e, --include-email
                Include e-mail address of each contributor in the output.
  -, --stdin    Read the list of additional contributors from stdin.
  LAST_COMMIT   Last commit of the range. Default value is HEAD.
  FIRST_COMMIT  First commit in the range. Default is the last commit tagged
                with a tag started with the character 'v'. If "--initial"
                is specified, the first commit of the branch is used.
__EOF__
}

get_commit_id()
{
	git rev-parse --verify "$1^{commit}"
}

SCRIPT='s/^[^:@<]\+:[[:space:]]*"\?\([^"<@[:space:]][^"<@]*\)"\?[[:space:]]\(<[^<@]\+@[^>]\+>\).*/\1 \2/p'
# Script for adding angle brackets to e-mail addresses in case they are absent
SCRIPT_NORM_EMAILS='s/[^[:space:]<@]\+@[^>]\+$/<\0>/'
MATCH_OUT='^\([^<@[:space:]][^<@]*\)[[:space:]]\(<[^<@]\+@[^>]\+>\)$'
OUT_EMAILS='\1 \2'
OUT_NO_EMAILS='\1'

read_stdin=0
include_email=0

while true; do
	case "${1:-}" in
	-h|--help)
		print_help
		exit 1
		;;
	-e|--include-email)
		include_email=1
		;;
	-|--stdin)
		read_stdin=1
		;;
	*)
		break
		;;
	esac

	shift
done

what="$(get_commit_id "${1:-@}")"
since="${2:-$(git describe --abbrev=0 --match='v*' "$what")}"

case "$since" in
--initial)
	since="$(git rev-list --max-parents=0 $what)"
	;;
*)
	since="$(get_commit_id "$since")"
	;;
esac

if [ 0 = "$include_email" ]; then
	out="$OUT_NO_EMAILS"
else
	out="$OUT_EMAILS"
fi

{
	git log "^$since" "$what" | sed -n "$SCRIPT"
	[ 0 = "$read_stdin" ] || sed "$SCRIPT_NORM_EMAILS"
} \
	| git check-mailmap --stdin \
	| LC_COLLATE=C sort -u \
	| sed "s/$MATCH_OUT/$out/"
