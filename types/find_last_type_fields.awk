#!/bin/gawk -f
#
# Copyright (c) 2018-2020 Dmitry V. Levin <ldv@altlinux.org>
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

/^typedef struct {$/ {
	in_struct = 1
	last_field = ""
	next
}

(in_struct == 1) {
	if (match($0, /^} struct_([a-z][a-z_0-9]*);$/, a)) {
		in_struct = 0
		print a[1] "." last_field
		next
	}
	if (match($0, /^[[:space:]]+[^];[:space:]:\/[][^];:[]*[[:space:]]+([^][:space:];:[]+)(\[[^];:[]*\])?;.*$/, a)) {
		last_field = a[1]
		next
	}
}
