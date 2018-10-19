#ifndef STRACE_GLIBC_COMPAT_H
#define STRACE_GLIBC_COMPAT_H

#if defined __GLIBC__
# ifndef __GLIBC_MINOR__
#  warning "__GLIBC__ is defined, but __GLIBC_MINOR__ isn't"
#  define __GLIBC_MINOR__ 0
# endif /* __GLIBC_MINOR__ */

# ifdef __GLIBC_PREREQ
#  define GLIBC_PREREQ __GLIBC_PREREQ
# else
#  define GLIBC_PREREQ(maj, min) \
	((((__GLIBC__) << 16) + (__GLIBC_MINOR__)) >= (((maj) << 16) + (min)))
# endif /* __GLIBC_PREREQ */

# define GLIBC_OLDER(maj, min) (!GLIBC_PREREQ((maj), (min)))
#else /* !__GLIBC__ */
# define GLIBC_PREREQ(maj, min)	0
# define GLIBC_OLDER(maj, min)	0
#endif

#endif /* STRACE_GLIBC_COMPAT_H */
