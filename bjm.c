#include "defs.h"

#if defined(LINUX)

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

#if !defined(QM_MODULES)
#define QM_MODULES	1
#define QM_DEPS		2
#define QM_REFS		3
#define QM_SYMBOLS	4
#define QM_INFO		5
#endif

static struct xlat which[] = {
	{ 0,		"0"		},
	{ QM_MODULES,	"QM_MODULES"	},
	{ QM_DEPS,	"QM_DEPS"	},
	{ QM_REFS,	"QM_REFS"	},
	{ QM_SYMBOLS,	"QM_SYMBOLS"	},
	{ QM_INFO,	"QM_INFO"	},
	{ 0,		NULL		},
};

static struct xlat modflags[] = {
	{ MOD_UNINITIALIZED,	"MOD_UNINITIALIZED"	},
	{ MOD_RUNNING,		"MOD_RUNNING"		},
	{ MOD_DELETED,		"MOD_DELETED"		},
	{ MOD_AUTOCLEAN,	"MOD_AUTOCLEAN"		},
	{ MOD_VISITED,		"MOD_VISITED"		},
	{ MOD_USED_ONCE,	"MOD_USED_ONCE"		},
	{ MOD_JUST_FREED,	"MOD_JUST_FREED"	},
	{ 0,			NULL			},
};

void
printstringlist(addr,num)
char* addr;
int num;
{
	int first;

	first=1;
	tprintf("{");
	while (num--) {
		if (first)
			first=0;
		else
			tprintf(",");
		tprintf(addr);
		addr+=strlen(addr)+1;
	}
	tprintf("}");
}


int
sys_query_module(tcp)
struct tcb *tcp;
{

	if (exiting(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", ");
		printxval(which, tcp->u_arg[1], "QM_???");
		tprintf(", ");
		if (tcp->u_rval!=0) {
			size_t	ret;
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf("%#lx, %lu, %d", tcp->u_arg[2], tcp->u_arg[3], ret);
		} else if (tcp->u_arg[1]==QM_INFO) {
			struct module_info	mi;
			size_t			ret;
			umove(tcp, tcp->u_arg[2], &mi);
			tprintf("{address=%#lx, size=%lu, flags=", mi.addr, mi.size);
			printflags(modflags, mi.flags);
			tprintf(", usecount=%lu}", mi.usecount);
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf(", %d", ret);
		} else if ((tcp->u_arg[1]==QM_MODULES) ||
			       	(tcp->u_arg[1]==QM_DEPS) ||
			       	(tcp->u_arg[1]==QM_REFS)) {
			char*	data	= (char*)malloc(tcp->u_arg[3]);
			char*	mod	= data;
			size_t	ret;
			int	first	= 0;

			umoven(tcp, tcp->u_arg[2], tcp->u_arg[3], data);
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf("{");
			while (ret--) {
				if (first)
					first=0;
				else
					tprintf(",");
				tprintf(mod);
				mod+=strlen(mod)+1;
			}
			tprintf("}, %d", ret);
			free(data);
		} else if (tcp->u_arg[1]==QM_SYMBOLS) {
			char*			data	= (char *)malloc(tcp->u_arg[3]);
			struct module_symbol*	sym	= (struct module_symbol*)data;
			size_t			ret;
			umoven(tcp, tcp->u_arg[2], tcp->u_arg[3], data);
			tprintf("{");
			while (ret--) {
				tprintf("{name=%#lx, value=%lu} ", sym->name, sym->value);
				sym++;
			}
			tprintf("}, %d", ret);
			free(data);
		} else {
			printstr(tcp, tcp->u_arg[2], tcp->u_arg[3]);
			tprintf(", %#lx", tcp->u_arg[4]);
		}
	}
	return 0;
}

int
sys_create_module(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return RVAL_HEX;
}

int
sys_init_module(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lx", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* LINUX */

