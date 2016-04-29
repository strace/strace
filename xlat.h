#ifndef STRACE_XLAT_H

struct xlat {
	unsigned int val;
	const char *str;
};

# define XLAT(val)			{ (unsigned)(val), #val }
# define XLAT_PAIR(val, str)		{ (unsigned)(val), str  }
# define XLAT_TYPE(type, val)		{     (type)(val), #val }
# define XLAT_TYPE_PAIR(val, str)	{     (type)(val), str  }
# define XLAT_END			{		0, 0    }

#endif
