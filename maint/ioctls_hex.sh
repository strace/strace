#!/bin/sh
# Copyright (c) 2001 Wichert Akkerman <wichert@cistron.nl>
# Copyright (c) 2004-2018 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

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
