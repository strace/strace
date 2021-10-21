#include "defs.h"
#include "print_utils.h"

void
print_pixelformat(uint32_t fourcc, const struct xlat *xlat)
{
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		PRINT_VAL_X(fourcc);
		return;
	}

	unsigned char a[] = {
		(unsigned char) fourcc,
		(unsigned char) (fourcc >> 8),
		(unsigned char) (fourcc >> 16),
		(unsigned char) (fourcc >> 24),
	};

	tprints_arg_begin("v4l2_fourcc");
	/* Generic char array printing routine.  */
	for (unsigned int i = 0; i < ARRAY_SIZE(a); ++i) {
		unsigned char c = a[i];

		if (i)
			tprint_arg_next();

		print_char(c, SCF_QUOTES);
	}
	tprint_arg_end();

	if (xlat) {
		const char *pixfmt_name = xlookup(xlat, fourcc);

		if (pixfmt_name)
			tprints_comment(pixfmt_name);
	}
}
