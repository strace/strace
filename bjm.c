#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <signal.h>
#include <linux/module.h>

/* WTA: #define these here: since Debian uses glibc2's includefiles
 * instead of the kernel includes we miss these otherwise.
 */

#if !defined(QM_MODULES)
#define QM_MODULES	1
#define QM_DEPS		2
#define QM_REFS		3
#define QM_SYMBOLS	4
#define QM_INFO		5
#endif

static struct xlat which[] = {
	{ 0,			"0"				},
	{ QM_MODULES,	"QM_MODULES"	},
	{ QM_DEPS,		"QM_DEPS"		},
	{ QM_REFS,		"QM_REFS"		},
	{ QM_SYMBOLS,	"QM_SYMBOLS"	},
	{ QM_INFO,		"QM_INFO"		},
	{ 0,			NULL			},
};

int
sys_query_module(tcp)
struct tcb *tcp;
{

	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", ");
		printxval(which, tcp->u_arg[1], "L_???");
		tprintf(", ");
		printstr(tcp, tcp->u_arg[2], tcp->u_arg[3]);
		tprintf(", %#lx", tcp->u_arg[4]);
	}
	return 0;
}

int
sys_delete_module(tcp)
struct tcb *tcp;
{

	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
	}
	return 0;
}
