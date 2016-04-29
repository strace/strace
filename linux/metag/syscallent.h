#include "32/syscallent.h"
/* [244 ... 259] are arch specific */
[245] = { 2,	0,	SEN(printargs),	"metag_setglobalbit"	},
[246] = { 1,	0,	SEN(printargs),	"metag_set_fpu_flags"	},
[247] = { 1,	0,	SEN(printargs),	"metag_set_tls"		},
[248] = { 0,	NF,	SEN(printargs),	"metag_get_tls"		},
