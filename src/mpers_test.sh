#!/bin/sh -efu
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2015-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

[ "x${D:-0}" != x1 ] || set -x

mpers_name="$1"; shift
mpers_cc_flags="$1"; shift
size="$(printf %s "$mpers_name" |tr -cd '[0-9]')"
[ "$size" -gt 0 ]

srcdir=${0%/*}
mpers_sh="${srcdir}/mpers.sh"

mpers_dir="mpers-$mpers_name"
mkdir -p "$mpers_dir"

sample="$mpers_dir/sample.c"
cat > "$sample" <<EOF
#include <stdint.h>
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
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	} u[3][2];
	short f[0];
} sample_struct;
#include MPERS_DEFS
EOF

expected="$mpers_dir/sample.expected"
mpers_ptr_t="uint${size}_t"
cat > "$expected" <<EOF
#include <stdint.h>
#ifndef mpers_ptr_t_is_${mpers_ptr_t}
typedef ${mpers_ptr_t} mpers_ptr_t;
#define mpers_ptr_t_is_${mpers_ptr_t}
#endif
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
signed char i8;
int16_t i16;
int32_t i32;
int64_t i64;
unsigned char u8;
uint16_t u16;
uint32_t u32;
uint64_t u64;
} u[3][2];
int16_t f[0];
} ATTRIBUTE_PACKED ${mpers_name}_sample_struct;
#define MPERS_${mpers_name}_sample_struct ${mpers_name}_sample_struct
EOF

CFLAGS="$CPPFLAGS -I${srcdir} -DMPERS_IS_${mpers_name}" \
CPPFLAGS="$CPPFLAGS -I${srcdir} -DIN_MPERS -DMPERS_IS_${mpers_name}" \
"$mpers_sh" "$mpers_name" "$mpers_cc_flags" "$sample"
cmp "$expected" "$mpers_dir"/sample_struct.h > /dev/null
