#include <linux/fcntl.h>

#if defined HAVE_STRUCT_FLOCK
typedef struct flock struct_kernel_flock;
#elif defined HAVE_STRUCT___KERNEL_FLOCK
typedef struct __kernel_flock struct_kernel_flock;
#else
# error struct flock definition not found in <linux/fcntl.h>
#endif

#if defined HAVE_STRUCT_FLOCK64
typedef struct flock64 struct_kernel_flock64;
#elif defined HAVE_STRUCT___KERNEL_FLOCK64
typedef struct __kernel_flock64 struct_kernel_flock64;
#else
# error struct flock64 definition not found in <linux/fcntl.h>
#endif
