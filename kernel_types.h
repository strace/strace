#ifndef STRACE_KERNEL_TYPES_H

# define STRACE_KERNEL_TYPES_H

# if defined HAVE___KERNEL_LONG_T && defined HAVE___KERNEL_ULONG_T

# include <asm/posix_types.h>

typedef __kernel_long_t kernel_long_t;
typedef __kernel_ulong_t kernel_ulong_t;

# elif defined __x86_64__ && defined __ILP32__

typedef long long kernel_long_t;
typedef unsigned long long kernel_ulong_t;

# else

typedef long kernel_long_t;
typedef unsigned long kernel_ulong_t;

# endif

typedef struct {
	kernel_ulong_t	d_ino;
	kernel_ulong_t	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
} kernel_dirent;

#endif
