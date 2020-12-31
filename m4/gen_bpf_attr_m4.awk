#!/bin/gawk -f
#
# Copyright (c) 2018-2019 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

/^struct ([^[:space:]]+)_struct([[:space:]]+\/\* ([^[:space:]]+) \*\/)?[[:space:]]+{/ {
	match($0, /^struct ([^[:space:]]+)_struct([[:space:]]+\/\* ([^[:space:]]+) \*\/)?[[:space:]]+{/, a)

	struct_name = a[1]
	subtype_name = a[3]

	if (struct_name ~ /^BPF_/)
		prefix = "union bpf_attr"
	else
		prefix = "struct " struct_name

	if (subtype_name != "")
		prefix = prefix "." subtype_name

	in_struct = 1
	next
}

/^}( ATTRIBUTE_ALIGNED\(.*\))?;/ {
	in_struct = 0
	next
}

(in_struct == 1) {
	if (match($0, /^[[:space:]]+[^;:\[\]]+[[:space:]]+([^[:space:]\[\];:]+)(\[[^;:]*\])?;$/, a)) {
		print "\t\t" prefix "." a[1] ","
	}
}
