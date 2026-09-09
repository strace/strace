#!/bin/gawk -f
#
# Copyright (c) 2018-2026 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#
# Extract fully qualified field paths from bpf_attr.h struct definitions,
# tracking nested struct/union members.
#
# Usage:
#   gawk -v struct=NAME -f gen_bpf_attr_fields.awk < bpf_attr.h
#     Process only the named struct, output bare field names.

# Match a top-level struct definition opening:
#   struct NAME_struct [/* subtype */] {
/^struct ([^[:space:]]+)_struct([[:space:]]+\/\* ([^[:space:]]+) \*\/)?[[:space:]]+{/ {
	match($0, /^struct ([^[:space:]]+)_struct([[:space:]]+\/\* ([^[:space:]]+) \*\/)?[[:space:]]+{/, a)

	if (a[1] "_struct" != struct)
		next

	in_struct = 1
	depth = 0
	nfields = 0
	next
}

# Top-level closing brace: emit all buffered fields and reset.
/^}( ATTRIBUTE_ALIGNED\(.*\))?;/ {
	if (in_struct) {
		for (i = 0; i < nfields; i++)
			print fields[i]
		nfields = 0
	}
	in_struct = 0
	next
}

!in_struct { next }

# Opening brace of a nested struct/union: increase nesting depth.
/^[[:space:]]+(struct|union)[[:space:]]*\{/ {
	depth++
	next
}

# Closing brace of a nested struct/union.
/^[[:space:]]+\}/ {
	name = ""
	if (match($0, /\}[[:space:]]+([A-Za-z_][A-Za-z_0-9]*)[[:space:]]*;/, a))
		name = a[1]

	if (name != "") {
		# Named member (e.g., "} raw_tracepoint;"): prepend the
		# member name to all fields buffered at the current depth,
		# building fully qualified paths like "raw_tracepoint.tp_name".
		for (i = 0; i < nfields; i++)
			if (field_depth[i] == depth) {
				fields[i] = name "." fields[i]
				field_depth[i] = depth - 1
			}

		# Emit the named member itself as a checkable field,
		# unless the line has trailing text after the semicolon
		# (the "/* skip check */" mechanism).
		if (match($0, /;[[:space:]]*$/)) {
			fields[nfields] = name
			field_depth[nfields] = depth - 1
			nfields++
		}
	} else {
		# Anonymous closing brace: promote all fields at the
		# current depth one level up without changing their paths.
		for (i = 0; i < nfields; i++)
			if (field_depth[i] == depth)
				field_depth[i] = depth - 1
	}
	depth--
	next
}

# Field declaration: extract the identifier name and buffer it at the
# current nesting depth.  Lines with text after the semicolon (e.g.,
# "/* skip check */") are excluded by the ";$" anchor.
{
	if (match($0, /^[[:space:]]+[^;:\[\]]+[[:space:]]+([^[:space:]\[\]};:]+)(\[[^;:]*\])?;$/, a)) {
		fields[nfields] = a[1]
		field_depth[nfields] = depth
		nfields++
	}
}
