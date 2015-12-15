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
function enter(array_idx)
{
	if (called[array_idx]) {
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
function what_is(what_idx, type_idx, special, item, \
		 location, prev_location, prev_returned_size)
{
	enter(what_idx)
	special = array_get(what_idx, "special")
	switch (special) {
	case "base_type":
		switch (array_get(what_idx, "encoding")) {
		case 5: # signed
			printf("%s ", "int" \
			8 * array_get(what_idx, "byte_size") "_t")
			break
		case 7: # unsigned
			printf("%s ", "uint" \
			8 * array_get(what_idx, "byte_size") "_t")
			break
		default: # float, signed/unsigned char
			printf("%s ", array_get(what_idx, "name"))
			break
		}
		returned_size = array_get(what_idx, "byte_size")
		break
	case "enumeration_type":
		type_idx = array_get(what_idx, "type")
		returned_size = array_get(what_idx, "byte_size")
		printf("%s ", "uint" 8 * returned_size "_t")
		break
	case "pointer_type":
		printf("%s", "mpers_ptr_t ")
		returned_size = array_get(what_idx, "byte_size")
		break
	case "array_type":
		type_idx = array_get(what_idx, "type")
		what_is(type_idx)
		to_return = array[what_idx]["upper_bound"]
		returned_size = to_return * returned_size
		if ("" == to_return)
			to_return = "00"
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
					printf("%s", \
						"unsigned char mpers_filler_" \
						item "[" loc_diff "];\n")
				}
				prev_location = location
				returned = what_is(item)
				prev_returned_size = returned_size
				printf("%s", array[item]["name"])
				if (returned) {
					printf("[%s]", returned)
				}
				print ";"
			}
		}
		returned_size = array_get(what_idx, "byte_size")
		loc_diff = returned_size - prev_location - prev_returned_size
		if (loc_diff != 0) {
			printf("%s", "unsigned char mpers_end_filler_" \
				item "[" loc_diff "];\n")
		}
		printf("%s", "} ATTRIBUTE_PACKED ")
		break
	case "union_type":
		print "union {"
		for (item in array) {
			if ("parent" in array[item] && \
				array_get(item, "parent") == what_idx) {
				returned = what_is(item)
				printf("%s", array_get(item, "name"))
				if (returned) {
					printf("[%s]", returned)
				}
				print ";"
			}
		}
		printf("%s", "} ")
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
	return leave(what_idx, 0)
}
BEGIN {
	print "#include <inttypes.h>"
}
/^<[[:xdigit:]]+>/ {
	match($0, /([[:alnum:]]+)><([[:alnum:]]+)/, matches)
	level = matches[1]
	idx = "0x" matches[2]
	array[idx]["idx"] = idx
	parent[level] = idx
	if (level > 1) {
		array[idx]["parent"] = parent[level-1]
	}
}
/^DW_AT_data_member_location/ {
	match($0, /[[:digit:]]+/, temparray)
	array[idx]["location"] = temparray[0]
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
	array[parent[level-1]]["upper_bound"] = temparray[0] + 1
}
/^Abbrev Number:[^(]+\(DW_TAG_/ {
	if (match($0, /typedef|union_type|structure_type|pointer_type\
|enumeration_type|array_type|base_type|member/, temparray)) {
		array[idx]["special"] = temparray[0]
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
