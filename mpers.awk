function compare_indices(i1, v1, i2, v2) {
	c1 = strtonum(sprintf("%s", i1))
	c2 = strtonum(sprintf("%s", i2))
	if (c1 < c2)
		return -1
	return (c1 != c2)
}
function what_is(what_idx, type_idx, special, item)
{
	type_idx = array[what_idx]["type"]
	special = array[what_idx]["special"]
	switch (special) {
	case "base_type":
		switch (array[what_idx]["encoding"]) {
		case 5: # signed
			printf("%s ", "int" \
			8*array[what_idx]["byte_size"] "_t")
			break
		case 7: # unsigned
			printf("%s ", "uint" \
			8*array[what_idx]["byte_size"] "_t")
			break
		default: # float, signed/unsigned char
			printf("%s ", array[what_idx]["name"])
			break
		}
		break
	case "enumeration_type":
		printf("%s ", "uint" 8*array[type_idx]["byte_size"] "_t")
		break
	case "pointer_type":
		printf("%s", "mpers_ptr_t ")
		break
	case "array_type":
		what_is(type_idx)
		to_return = array[what_idx]["upper_bound"]
		return to_return
		break
	case "structure_type":
	case "union_type":
		if (special == "structure_type") {
			print "struct {"
		} else {
			print "union {"
		}
		for (item in array) {
			if ("parent" in array[item] && array[item]["parent"] \
				== what_idx) {
				returned = what_is(item)
				printf("%s", array[item]["name"])
				if (returned) {
					printf("%s", "[" returned "]")
				}
				print ";"
			}
		}
		printf("%s", "} ")
		break
	case "typedef":
		return what_is(type_idx)
		break
	case "member":
		return what_is(type_idx)
		break
	default:
		what_is(type_idx)
		break
	}
	return 0
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
	array[idx]["location"] = temparray[1]
}
/^DW_AT_name/ {
	match($0, /:[[:space:]]+([[:alpha:]_][[:alnum:]_[:space:]]*)/, temparray)
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
				8*array[item]["byte_size"] "_t mpers_ptr_t;"
			break
		}
	}
	for (item in array) {
		if (array[item]["name"] == VAR_NAME) {
			type=array[item]["type"]
			print "typedef"
			what_is(array[item]["type"])
			print ARCH_FLAG "_" array[type]["name"] ";"
			print "#define MPERS_" \
				ARCH_FLAG "_" array[type]["name"] " " \
				ARCH_FLAG "_" array[type]["name"]
			break
		}
	}
}
