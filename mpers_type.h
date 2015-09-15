#ifdef IN_MPERS
# define STRINGIFY(a) #a
# define DEF_MPERS_TYPE(args) STRINGIFY(args.h)
# ifdef MPERS_IS_m32
#  define MPERS_PREFIX m32_
#  define MPERS_DEFS "m32_defs.h"
# elif defined MPERS_IS_mx32
#  define MPERS_PREFIX mx32_
#  define MPERS_DEFS "mx32_defs.h"
# endif
#else
# define MPERS_PREFIX
# define DEF_MPERS_TYPE(args) "empty.h"
# define MPERS_DEFS "native_defs.h"
#endif
