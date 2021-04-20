#!/bin/gawk
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2015-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

function array_get(array_idx, array_member, \
		   array_return)
{
	array_return = array[array_idx, array_member]
	if ("" == array_return) {
		printf("%s: index [%s] without %s\n",
		       FILENAME, array_idx, array_member) > "/dev/stderr"
		exit 1
	}
	return array_return
}
function norm_idx(idx)
{
	return sprintf("%016s", idx)
}
function array_seq(array_idx)
{
	if ((array_idx, "seq") in array)
		return array[array_idx, "seq"]
	index_seq++
	array[array_idx, "seq"] = index_seq
	return index_seq
}
function enter(array_idx,
	       item)
{
	if (array_idx in called) {
		printf("%s: index loop detected:", FILENAME) > "/dev/stderr"
		for (item in called)
			printf(" %s", item) > "/dev/stderr"
		print "" > "/dev/stderr"
		exit 1
	}
	called[array_idx] = 1
}
function leave(array_idx, to_return)
{
	delete called[array_idx]
	return to_return
}
function update_upper_bound(idx, val, \
			    count)
{
	count = array[idx, "count"]
	if (count == "")
		count = 1
	array[idx, "count"] = count * val
	array[idx, "upper_bound"] = array[idx, "upper_bound"] "[" val "]"
}
function what_is(what_idx, \
		 item, loc_diff, location, prev_location, prev_returned_size, \
		 special, to_return, type_idx, enc, i)
{
	enter(what_idx)
	special = array_get(what_idx, "special")
	if (special == "base_type") {
		enc = array_get(what_idx, "encoding")
		if (enc == 5) { # signed
			printf("int%s_t ",
			       8 * array_get(what_idx, "byte_size"))
		} else if (enc == 7) { # unsigned
			printf("uint%s_t ",
			       8 * array_get(what_idx, "byte_size"))
		} else { # float, signed/unsigned char
			printf("%s ", array_get(what_idx, "name"))
		}
		returned_size = array_get(what_idx, "byte_size")
	} else if (special == "enumeration_type") {
		returned_size = array_get(what_idx, "byte_size")
		printf("uint%s_t ", 8 * returned_size)
	} else if (special == "pointer_type") {
		printf("mpers_ptr_t ")
		returned_size = array_get(what_idx, "byte_size")
	} else if (special == "array_type") {
		type_idx = array_get(what_idx, "type")
		what_is(type_idx)
		to_return = array[what_idx, "upper_bound"]
		if ("" == to_return)
			to_return = "[0]"
		returned_size = array[what_idx, "count"] * returned_size
		return leave(what_idx, to_return)
	} else if (special == "structure_type") {
		print "struct {"
		prev_location = 0
		location = 0
		returned_size = 0
		prev_returned_size = 0
		for (i = 1; i <= parents_cnt; i += 1) {
			if (array_parents[aparents_keys[i]] == what_idx) {
				location = array_get(aparents_keys[i], "location")
				loc_diff = location - prev_location - \
					prev_returned_size
				if (loc_diff != 0) {
					printf("unsigned char mpers_%s_%s[%s];\n",
					       "filler", array_seq(aparents_keys[i]), loc_diff)
				}
				prev_location = location
				returned = what_is(aparents_keys[i])
				prev_returned_size = returned_size
				printf("%s%s;\n", array[aparents_keys[i], "name"], returned)
			}
		}
		returned_size = array_get(what_idx, "byte_size")
		loc_diff = returned_size - prev_location - prev_returned_size
		if (loc_diff != 0) {
			printf("unsigned char mpers_%s_%s[%s];\n",
			       "end_filler", array_seq(item), loc_diff)
		}
		printf("} ATTRIBUTE_PACKED ")
	} else if (special == "union_type") {
		print "union {"
		for (i = 1; i <= parents_cnt; i += 1) {
			if (array_parents[aparents_keys[i]] == what_idx) {
				returned = what_is(aparents_keys[i])
				printf("%s%s;\n", array[aparents_keys[i], "name"], returned)
			}
		}
		printf("} ")
		returned_size = array_get(what_idx, "byte_size")
	} else if (special == "typedef") {
		type_idx = array_get(what_idx, "type")
		return leave(what_idx, what_is(type_idx))
	} else if (special == "member") {
		type_idx = array_get(what_idx, "type")
		return leave(what_idx, what_is(type_idx))
	} else {
		type_idx = array_get(what_idx, "type")
		what_is(type_idx)
	}
	return leave(what_idx, "")
}
BEGIN {
	match(ARCH_FLAG, /[[:digit:]]+/, temparray)
	default_pointer_size = temparray[0] / 8
	print "#include <stdint.h>"
}
/^<[[:xdigit:]]+>/ {
	match($0, /([[:alnum:]]+)><([[:alnum:]]+)/, matches)
	level = matches[1]
	idx = norm_idx(matches[2])
	array[idx, "idx"] = idx
	parent[level] = idx
}
/^DW_AT_data_member_location/ {
	if (!match($0, /\(DW_OP_plus_uconst:[[:space:]]+([[:digit:]]+)\)/, temparray))
		match($0, /([[:digit:]]+)/, temparray)
	array[idx, "location"] = temparray[1]
}
/^DW_AT_name/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?([[:alpha:]_][[:alnum:]_[:space:]]*)/,
	      temparray)
	array_names[idx] = 1
	array[idx, "name"] = temparray[2]
}
/^DW_AT_byte_size/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?([[:digit:]]+)/,
	      temparray)
	array[idx, "byte_size"] = temparray[2]
}
/^DW_AT_encoding/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?([[:digit:]]+)/,
	      temparray)
	array[idx, "encoding"] = temparray[2]
}
/^DW_AT_type/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?<0x([[:xdigit:]]*)>(, .*)?$/,
	      temparray)
	array[idx, "type"] = norm_idx(temparray[2])
}
/^DW_AT_upper_bound/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?([[:digit:]]+)/,
	      temparray)
	update_upper_bound(parent[level - 1], temparray[2] + 1)
}
/^DW_AT_count/ {
	match($0, /:[[:space:]]+(\([[:alpha:]][[:alnum:]]*\)[[:space:]]+)?([[:digit:]]+)/,
	      temparray)
	update_upper_bound(parent[level - 1], temparray[2])
}
/^Abbrev Number:[^(]+\(DW_TAG_/ {
	if (match($0, /typedef|union_type|structure_type|pointer_type\
|enumeration_type|array_type|base_type|member/, temparray)) {
		array_special[idx] = temparray[0]
		array[idx, "special"] = temparray[0]
		if ("pointer_type" == temparray[0])
			array[idx, "byte_size"] = default_pointer_size
		if (level > 1 && "member" == temparray[0])
			array_parents[idx] = parent[level-1]
	}
}
END {
	parents_cnt = asorti(array_parents, aparents_keys)

	for (item in array_special) {
		if (array[item, "special"] == "pointer_type") {
			mpers_ptr_t = \
				"uint" 8 * array_get(item, "byte_size") "_t"
			print "#ifndef mpers_ptr_t_is_" mpers_ptr_t
			print "typedef " mpers_ptr_t " mpers_ptr_t;"
			print "#define mpers_ptr_t_is_" mpers_ptr_t
			print "#endif"
			break
		}
	}
	for (item in array_names) {
		if (array[item, "name"] == VAR_NAME) {
			type = array_get(item, "type")
			print "typedef"
			what_is(type)
			name = array_get(type, "name")
			print ARCH_FLAG "_" name ";"
			print "#define MPERS_" \
				ARCH_FLAG "_" name " " \
				ARCH_FLAG "_" name
			break
		}
	}
}
