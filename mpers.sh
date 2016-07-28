#!/bin/sh -e
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
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

export LC_ALL=C

MPERS_AWK="${0%/*}/mpers.awk"
ARCH_FLAG=$1
PARSER_FILE=$2

CC="${CC-gcc}"
CFLAGS="$CFLAGS -gdwarf-2 -c"
CPP="${CPP-$CC -E}"
CPPFLAGS="$CPPFLAGS -MM -MG"

VAR_NAME='mpers_target_var'
BITS_DIR="mpers${ARCH_FLAG}"

mkdir -p ${BITS_DIR}
set -- $(sed -r -n \
	's/^#[[:space:]]*include[[:space:]]+DEF_MPERS_TYPE\(([^)[:space:]]*)\)$/\1/p' \
		"${PARSER_FILE}")
for m_type; do
	f_h="${BITS_DIR}/${m_type}.h"
	f_c="${BITS_DIR}/${m_type}.c"
	f_i="${BITS_DIR}/${m_type}.i"
	f_o="${BITS_DIR}/${m_type}.o"
	f_d1="${BITS_DIR}/${m_type}.d1"
	f_d2="${BITS_DIR}/${m_type}.d2"
	sed -e '
		/DEF_MPERS_TYPE('"${m_type}"')$/n
		/DEF_MPERS_TYPE/d
		/^[[:space:]]*#[[:space:]]*include[[:space:]]*"xlat\//d
		/^#[[:space:]]*include[[:space:]][[:space:]]*MPERS_DEFS$/ {s//'"${m_type} ${VAR_NAME}"';/;q}
		' "${PARSER_FILE}" > "${f_c}"
	$CPP $CPPFLAGS "${f_c}" > "${f_i}"
	grep -F -q "${m_type}.h" "${f_i}" ||
		continue
	sed -i -e '/DEF_MPERS_TYPE/d' "${f_c}"
	$CC $CFLAGS $ARCH_FLAG "${f_c}" -o "${f_o}"
	readelf --debug-dump=info "${f_o}" > "${f_d1}"
	sed -r -n '
		/^[[:space:]]*<1>/,/^[[:space:]]*<1><[^>]+>: Abbrev Number: 0/!d
		/^[[:space:]]*<[^>]*><[^>]*>: Abbrev Number: 0/d
		s/^[[:space:]]*<[[:xdigit:]]+>[[:space:]]+//
		s/^[[:space:]]*((<[[:xdigit:]]+>){2}):[[:space:]]+/\1\n/
		s/[[:space:]]+$//
		p' "${f_d1}" > "${f_d2}"
	gawk -v VAR_NAME="$VAR_NAME" -v ARCH_FLAG="${ARCH_FLAG#-}" \
		-f "$MPERS_AWK" "${f_d2}" > "${f_h}"
done
