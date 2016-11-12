#if defined M68K
# include "32/ioctls_inc_align16.h"
#elif defined X86_64 || defined X32 \
 || SIZEOF_STRUCT_I64_I32 < SIZEOF_LONG_LONG * 2
# include "32/ioctls_inc_align32.h"
#else
# include "32/ioctls_inc_align64.h"
#endif
