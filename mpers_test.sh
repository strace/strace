#!/bin/sh -efu

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
		long asl[3];
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
		short ss;
		unsigned short us;
		char sc;
		unsigned char uc;
	} u[3];
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
int${size}_t asl[3];
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
int16_t ss;
uint16_t us;
char sc;
unsigned char uc;
} u[3];
int16_t f[0];
} ATTRIBUTE_PACKED ${mpers_name}_sample_struct;
#define MPERS_${mpers_name}_sample_struct ${mpers_name}_sample_struct
EOF

CFLAGS="$CPPFLAGS -I${srcdir}" \
CPPFLAGS="$CPPFLAGS -I${srcdir} -DIN_MPERS -DMPERS_IS_${mpers_name}" \
"$mpers_sh" "-$mpers_name" "$sample"
cmp "$expected" "$mpers_dir"/sample_struct.h > /dev/null
