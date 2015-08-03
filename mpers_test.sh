#!/bin/sh -efu

mpers_name="$1"; shift
srcdir=${0%/*}
mpers_sh="${srcdir}/mpers.sh"

mpers_dir="mpers-$mpers_name"
mkdir -p "$mpers_dir"

sample="$mpers_dir/sample.c"
cat > "$sample" <<EOF
#include "mpers_type.h"
#include DEF_MPERS_TYPE(int)
#include MPERS_DEFS
EOF

expected="$mpers_dir/sample.expected"
cat > "$expected" <<EOF
#include <inttypes.h>
typedef
int32_t ${mpers_name}_int;
#define MPERS_${mpers_name}_int ${mpers_name}_int
EOF

CFLAGS="$CPPFLAGS -I${srcdir}" \
CPPFLAGS="$CPPFLAGS -I${srcdir} -DIN_MPERS -DMPERS_IS_${mpers_name}" \
"$mpers_sh" "-$mpers_name" "$sample"
cmp "$expected" "$mpers_dir"/int.h > /dev/null
