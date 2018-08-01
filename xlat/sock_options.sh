#!/bin/sh -eu
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# Generate fallback definitions of SO_* constants in xlat/sock_options.in file.
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

linux="$1"; shift
in="${0%.sh}.in"

sed -n 's/^\(SO_[^[:space:]]*\).*/\1/p' < "$in" |
uniq |
while read name; do
	sed -rn 's/#define[[:space:]]+('"$name"')[[:space:]]+([[:digit:]]+)$/\2\t\1/p' \
		"$linux"/include/uapi/asm-generic/socket.h
done |
sort -n |
while read def name; do
	grep -EH '#define[[:space:]]+'"$name"'[[:space:]]+(0x[[:xdigit:]]+|[[:digit:]]+)' \
		"$linux"/arch/*/include/uapi/asm/socket.h |
	sed -rn 's|^[^#]*/arch/([^/]+)/include/uapi/asm/socket\.h:#define[[:space:]]+'"$name"'[[:space:]]+([^[:space:]]+)([[:space:]].*)?$|\1\t\2|p' |
	sed s/parisc/hppa/ |sort |
	awk -vname="$name" -vdef="$def" '
{
	i = strtonum($2)
	if (i == def) next
	if (a[i])
		a[i] = a[i] " || defined __" $1 "__"
	else
		a[i] = "defined __" $1 "__"
}
END {
	iftext = "#if"
	for (i in a) {
		printf("%s %s\n%s %u\n", iftext, a[i], name, i)
		iftext = "#elif"
	}
	if (iftext != "#if")
		print "#else"
	printf("%s %s\n", name, def)
	if (iftext != "#if")
		print "#endif"
	print ""
}
	'
done
