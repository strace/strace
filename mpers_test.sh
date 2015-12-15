#!/bin/sh -efu

mpers_name="$1"; shift
srcdir=${0%/*}
mpers_sh="${srcdir}/mpers.sh"

mpers_dir="mpers-$mpers_name"
mkdir -p "$mpers_dir"

sample="$mpers_dir/sample.c"
cat > "$sample" <<EOF
#include "mpers_type.h"
#include DEF_MPERS_TYPE(sample_struct)
typedef struct { int i; unsigned short s[0]; } sample_struct;
#include MPERS_DEFS
EOF

expected="$mpers_dir/sample.expected"
cat > "$expected" <<EOF
#include <inttypes.h>
typedef
struct {
int32_t i;
uint16_t s[00];
} ATTRIBUTE_PACKED ${mpers_name}_sample_struct;
#define MPERS_${mpers_name}_sample_struct ${mpers_name}_sample_struct
EOF

CFLAGS="$CPPFLAGS -I${srcdir}" \
CPPFLAGS="$CPPFLAGS -I${srcdir} -DIN_MPERS -DMPERS_IS_${mpers_name}" \
"$mpers_sh" "-$mpers_name" "$sample"
cmp "$expected" "$mpers_dir"/sample_struct.h > /dev/null
