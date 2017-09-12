#!/bin/gawk -f
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2017 Gleb Fotengauer-Malinovskiy <glebfm@altlinux.org>
# Copyright (c) 2015-2017 The strace developers.
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

BEGIN {
	dirmap[2^8] = "_IOC_NONE"
	dirmap[2^9] = "_IOC_READ"
	dirmap[2^10] = "_IOC_WRITE"
	dirmap[2^10 + 2^9] = "_IOC_READ|_IOC_WRITE"
}
function array_get(array_idx, array_member, \
		   array_return)
{
	array_return = array[array_idx][array_member]
	if ("" == array_return) {
		printf("%s: index [%s] without %s\n",
		       FILENAME, array_idx, array_member) > "/dev/stderr"
		exit 1
	}
	return array_return
}
function dir2str(dir, \
		 r) {
	r = dirmap[dir]
	if (r == "") {
		printf("%s: ioctl direction %d is not known\n",
		       FILENAME, dir) > "/dev/stderr"
		exit 1
	}
	return r
}
/^<[[:xdigit:]]+>/ {
	match($0, /([[:alnum:]]+)><([[:alnum:]]+)/, matches)
	level = matches[1]
	idx = "0x" matches[2]
	parent[level] = idx
}
/^DW_AT_name/ {
	match($0, /:[[:space:]]+([[:alpha:]_][[:alnum:]_[:space:]]*)/,
		temparray)
	array[idx]["name"] = temparray[1]
}
/^DW_AT_type/ {
	match($0, /:[[:space:]]+<(0x[[:xdigit:]]*)>$/, temparray)
	array[idx]["type"] = temparray[1]
}
/^DW_AT_upper_bound/ {
	match($0, /[[:digit:]]+/, temparray)
	array[array[idx]["parent"]]["size"] = temparray[0]
}
/^DW_AT_count/ {
	match($0, /[[:digit:]]+/, temparray)
	array[array[idx]["parent"]]["size"] = temparray[0] - 1
}
/^Abbrev Number:[^(]+\(DW_TAG_/ {
	if (match($0, /member|subrange_type|variable/, temparray)) {
		array[idx]["special"] = temparray[0]
		if (level > 1 && ("member" == temparray[0] ||
		    "subrange_type" == temparray[0]))
			array[idx]["parent"] = parent[level-1]
	}
}
END {
	for (i in array) {
		if (array[i]["special"] == "variable" &&
		    index(array_get(i, "name"), "ioc_") == 1) {
			ioc_name = substr(array_get(i, "name"),
					  length("ioc_") + 1)
			type = array_get(i, "type")
			delete sizemap
			for (j in array) {
				if ("parent" in array[j] &&
				    type == array_get(j, "parent")) {
					t = array_get(j, "type")

					field_name = array_get(j, "name")
					sizemap[field_name] = \
						array_get(t, "size")
				}
			}
			if (sizemap["d"] == "" ||
			    sizemap["n"] == "" ||
			    sizemap["s"] == "") {
				printf("%s: failed to parse %s ioctl info\n",
				       FILENAME, ioc_name) > "/dev/stderr"
				exit 1
			}
			printf("{ \"%s\", \"%s\", %s, 0x%04x, 0x%02x },\n",
				  HEADER_NAME, ioc_name, dir2str(sizemap["d"]),
				  sizemap["n"], sizemap["s"])
		}
	}
}
