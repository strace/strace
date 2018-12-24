#!/bin/sh
# Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
# Copyright (c) 1995-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

awk '
$1 == "#define" && $2 ~ /^E[A-Z0-9_]+$/ && $3 ~ /^[0-9]+$/ {
	errno[$3] = $2
	if ($3 > max)
		max = $3
}
END {
	for (i = 0; i <= max; i++)
		if (errno[i])
			printf("[%3d] = \"%s\",\n", i, errno[i])
}
' "$@"
