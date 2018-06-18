#include <sys/cdefs.h>
#if defined(__BIONIC__)
/*
 * Bionic's <linux/signal.h> is the UAPI one, and <signal.h> requires it.
 */
#include_next <linux/signal.h>
#else
/*
 * Workaround the infamous incompatibility between <linux/signal.h>
 * and many libc headers by overriding <linux/signal.h> with <signal.h>.
 */
#include <signal.h>
#endif
