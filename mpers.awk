#!/bin/gawk
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

function compare_indices(i1, v1, i2, v2) {
	c1 = strtonum(sprintf("%s", i1))
	c2 = strtonum(sprintf("%s", i2))
	if (c1 < c2)
		return -1
	return (c1 != c2)
}
function array_get(array_idx, array_member, array_return)
{
	array_return = array[array_idx][array_member]
	if ("" == array_return) {
		printf("%s: index [%s] without %s\n",
		       FILENAME, array_idx, array_member) > "/dev/stderr"
		exit 1
	}
	return array_return
}
function array_seq(array_idx)
{
	if ("seq" in array[array_idx])
		return array[array_idx]["seq"]
	index_seq++
	array[array_idx]["seq"] = index_seq
	return index_seq
}
function enter(array_idx)
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
function update_upper_bound(idx, val, count)
{
	count = array[idx]["count"]
	if (count == "")
		count = 1
	array[idx]["count"] = count * val
	array[idx]["upper_bound"] = array[idx]["upper_bound"] "[" val "]"
}
function what_is(what_idx, type_idx, special, item, \
		 location, prev_location, prev_returned_size)
{
	enter(what_idx)
	special = array_get(what_idx, "special")
	switch (special) {
	case "base_type":
		switch (array_get(what_idx, "encoding")) {
		case 5: # signed
			printf("int%s_t ",
			       8 * array_get(what_idx, "byte_size"))
			break
		case 7: # unsigned
			printf("uint%s_t ",
			       8 * array_get(what_idx, "byte_size"))
			break
		default: # float, signed/unsigned char
			printf("%s ", array_get(what_idx, "name"))
			break
		}
		returned_size = array_get(what_idx, "byte_size")
		break
	case "enumeration_type":
		returned_size = array_get(what_idx, "byte_size")
		printf("uint%s_t ", 8 * returned_size)
		break
	case "pointer_type":
		printf("mpers_ptr_t ")
		returned_size = array_get(what_idx, "byte_size")
		break
	case "array_type":
		type_idx = array_get(what_idx, "type")
		what_is(type_idx)
		to_return = array[what_idx]["upper_bound"]
		if ("" == to_return)
			to_return = "[0]"
		returned_size = array[what_idx]["count"] * returned_size
		return leave(what_idx, to_return)
		break
	case "structure_type":
		print "struct {"
		prev_location = 0
		location = 0
		returned_size = 0
		prev_returned_size = 0
		for (item in array) {
			if ("parent" in array[item] && \
				array_get(item, "parent") == what_idx) {
				location = array_get(item, "location")
				loc_diff = location - prev_location - \
					prev_returned_size
				if (loc_diff != 0) {
					printf("unsigned char mpers_%s_%s[%s];\n",
					       "filler", array_seq(item), loc_diff)
				}
				prev_location = location
				returned = what_is(item)
				prev_returned_size = returned_size
				printf("%s%s;\n", array[item]["name"], returned)
			}
		}
		returned_size = array_get(what_idx, "byte_size")
		loc_diff = returned_size - prev_location - prev_returned_size
		if (loc_diff != 0) {
			printf("unsigned char mpers_%s_%s[%s];\n",
			       "end_filler", array_seq(item), loc_diff)
		}
		printf("} ATTRIBUTE_PACKED ")
		break
	case "union_type":
		print "union {"
		for (item in array) {
			if ("parent" in array[item] && \
				array_get(item, "parent") == what_idx) {
				returned = what_is(item)
				printf("%s%s;\n", array[item]["name"], returned)
			}
		}
		printf("} ")
		returned_size = array_get(what_idx, "byte_size")
		break
	case "typedef":
		type_idx = array_get(what_idx, "type")
		return leave(what_idx, what_is(type_idx))
		break
	case "member":
		type_idx = array_get(what_idx, "type")
		return leave(what_idx, what_is(type_idx))
		break
	default:
		type_idx = array_get(what_idx, "type")
		what_is(type_idx)
		break
	}
	return leave(what_idx, "")
}
BEGIN {
	match(ARCH_FLAG, /[[:digit:]]+/, temparray)
	default_pointer_size = temparray[0] / 8
	print "#include <inttypes.h>"
}
/^<[[:xdigit:]]+>/ {
	match($0, /([[:alnum:]]+)><([[:alnum:]]+)/, matches)
	level = matches[1]
	idx = "0x" matches[2]
	array[idx]["idx"] = idx
	parent[level] = idx
}
/^DW_AT_data_member_location/ {
	if (!match($0, /\(DW_OP_plus_uconst:[[:space:]]+([[:digit:]]+)\)/, temparray))
		match($0, /([[:digit:]]+)/, temparray)
	array[idx]["location"] = temparray[1]
}
/^DW_AT_name/ {
	match($0, /:[[:space:]]+([[:alpha:]_][[:alnum:]_[:space:]]*)/, \
		temparray)
	array[idx]["name"] = temparray[1]
}
/^DW_AT_byte_size/ {
	match($0, /[[:digit:]]+/, temparray)
	array[idx]["byte_size"] = temparray[0]
}
/^DW_AT_encoding/ {
	match($0, /[[:digit:]]+/, temparray)
	array[idx]["encoding"] = temparray[0]
}
/^DW_AT_type/ {
	match($0, /:[[:space:]]+<(0x[[:xdigit:]]*)>$/, temparray)
	array[idx]["type"] = temparray[1]
}
/^DW_AT_upper_bound/ {
	match($0, /[[:digit:]]+/, temparray)
	update_upper_bound(parent[level - 1], temparray[0] + 1)
}
/^DW_AT_count/ {
	match($0, /[[:digit:]]+/, temparray)
	update_upper_bound(parent[level - 1], temparray[0])
}
/^Abbrev Number:[^(]+\(DW_TAG_/ {
	if (match($0, /typedef|union_type|structure_type|pointer_type\
|enumeration_type|array_type|base_type|member/, temparray)) {
		array[idx]["special"] = temparray[0]
		if ("pointer_type" == temparray[0])
			array[idx]["byte_size"] = default_pointer_size
		if (level > 1 && "member" == temparray[0])
			array[idx]["parent"] = parent[level-1]
	}
}
END {
	PROCINFO["sorted_in"] = "compare_indices"
	for (item in array) {
		if (array[item]["special"] == "pointer_type") {
			print "typedef uint" \
				8 * array_get(item, "byte_size") "_t mpers_ptr_t;"
			break
		}
	}
	for (item in array) {
		if (array[item]["name"] == VAR_NAME) {
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
