#!/bin/sh -efu
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

mpers_name="$1"; shift
size="$(printf %s "$mpers_name" |tr -cd '[0-9]')"
[ "$size" -gt 0 ]

srcdir=${0%/*}
mpers_sh="${srcdir}/mpers.sh"

mpers_dir="mpers-$mpers_name"
mkdir -p "$mpers_dir"

sample="$mpers_dir/sample.c"
cat > "$sample" <<EOF
#include "mpers_type.h"
#include DEF_MPERS_TYPE(sample_struct)
typedef struct {
	struct {
		void *p;
		char sc;
		/* unsigned char mpers_filler_1[1]; */
		short ss;
		unsigned char uc;
		/* unsigned char mpers_filler_2[3]; */
		int si;
		unsigned ui;
		long sl;
		unsigned short us;
		/* unsigned char mpers_filler_3[6]; */
		long long sll __attribute__((__aligned__(8)));
		unsigned long long ull;
		unsigned long ul;
		long asl[3][5][7];
		char f;
		/* unsigned char mpers_end_filler_4[7]; */
	} s;
	union {
		long long sll;
		unsigned long long ull;
		void *p;
		long sl;
		unsigned long ul;
		int si;
		unsigned ui;
		short ss[7][9];
		unsigned short us[4];
		char sc;
		unsigned char uc;
	} u[3][2];
	short f[0];
} sample_struct;
#include MPERS_DEFS
EOF

expected="$mpers_dir/sample.expected"
cat > "$expected" <<EOF
#include <inttypes.h>
typedef uint${size}_t mpers_ptr_t;
typedef
struct {
struct {
mpers_ptr_t p;
char sc;
unsigned char mpers_filler_1[1];
int16_t ss;
unsigned char uc;
unsigned char mpers_filler_2[3];
int32_t si;
uint32_t ui;
int${size}_t sl;
uint16_t us;
unsigned char mpers_filler_3[6];
int64_t sll;
uint64_t ull;
uint${size}_t ul;
int${size}_t asl[3][5][7];
char f;
unsigned char mpers_end_filler_4[7];
} ATTRIBUTE_PACKED s;
union {
int64_t sll;
uint64_t ull;
mpers_ptr_t p;
int${size}_t sl;
uint${size}_t ul;
int32_t si;
uint32_t ui;
int16_t ss[7][9];
uint16_t us[4];
char sc;
unsigned char uc;
} u[3][2];
int16_t f[0];
} ATTRIBUTE_PACKED ${mpers_name}_sample_struct;
#define MPERS_${mpers_name}_sample_struct ${mpers_name}_sample_struct
EOF

CFLAGS="$CPPFLAGS -I${srcdir} -DMPERS_IS_${mpers_name}" \
CPPFLAGS="$CPPFLAGS -I${srcdir} -DIN_MPERS -DMPERS_IS_${mpers_name}" \
"$mpers_sh" "-$mpers_name" "$sample"
cmp "$expected" "$mpers_dir"/sample_struct.h > /dev/null
