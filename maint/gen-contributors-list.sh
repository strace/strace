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

print_help()
{
	cat <<__EOF__
$0 [-h|--help] [-e|--include-email] [-|--stdin] [LAST_COMMIT [FIRST_COMMIT|--initial]]"

Prints list of contributors for the specified range of commits.

Options:
  -h, --help    Print this message and exit.
  -e, --include-email
                Include e-mail address of each contributer in the output.
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
