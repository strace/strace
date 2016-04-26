#ifndef STRACE_XLAT_H

struct xlat {
	unsigned int val;
	const char *str;
};

# define XLAT(x) { x, #x }
# define XLAT_END { 0, 0 }

#endif
