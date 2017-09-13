/*
 * Should only be included from strace.c, so no include guards.
 */

#include <lualib.h>
#include <lauxlib.h>

#define L script_L

static struct tcb *
func_next_sc(void)
{
	static bool first = true;
	if (!first) {
		if (!current_tcp)
			return NULL;
		if (!dispatch_event(TE_SYSCALL_STOP_HOOK_EXIT, NULL, NULL,
		    true))
			goto term;
	}
	first = false;

	while (1) {
		int status;
		siginfo_t si;
		enum trace_event ret = next_event(&status, &si);
		if (!dispatch_event(ret, &status, &si, true))
			goto term;
		if (ret == TE_SYSCALL_STOP && (current_tcp->flags & TCB_HOOK)) {
			current_tcp->flags &= ~TCB_HOOK;
			return current_tcp;
		}
	}

term:
	current_tcp = NULL;
	return NULL;
}

static bool
func_monitor(unsigned int scno, unsigned int pers, bool entry_hook,
	     bool exit_hook)
{
	if (pers >= SUPPORTED_PERSONALITIES)
		return false;
	set_hook_qual(scno, pers, entry_hook, exit_hook);
	return true;
}

static void
prepare_ad_hoc_inject(void)
{
	if (!(current_tcp->flags & TCB_AD_HOC_INJECT)) {
		current_tcp->ad_hoc_inject_data.flags = 0;
		current_tcp->qual_flg |= QUAL_INJECT;
		current_tcp->flags |= TCB_AD_HOC_INJECT;
	}
}

static bool
func_inject_signo(int signo)
{
	if (!current_tcp || exiting(current_tcp))
		return false;
	if (signo <= 0 || signo > SIGRTMAX)
		return false;
	prepare_ad_hoc_inject();
	current_tcp->ad_hoc_inject_data.flags |= INJECT_F_SIGNAL;
	current_tcp->ad_hoc_inject_data.signo = signo;
	return true;
}

static bool
func_inject_retval(int retval)
{
	if (!current_tcp || exiting(current_tcp))
		return false;
	if (retval < -MAX_ERRNO_VALUE)
		return false;
	prepare_ad_hoc_inject();
	current_tcp->ad_hoc_inject_data.flags |= INJECT_F_RETVAL;
	current_tcp->ad_hoc_inject_data.rval = retval;
	return true;
}

static int
func_umove(kernel_ulong_t addr, size_t len, void *laddr)
{
	return current_tcp ? umoven(current_tcp, addr, len, laddr) : -1;
}

static int
func_umove_str(kernel_ulong_t addr, size_t len, char *laddr)
{
	return current_tcp ? umovestr(current_tcp, addr, len, laddr) : -1;
}

static bool
func_path_match(const char **set, size_t nset)
{
	if (!current_tcp)
		return false;
	struct path_set s = {set, nset};
	return pathtrace_match_set(current_tcp, &s);
}

static const char *
get_lua_msg(int pos)
{
	const char *msg = lua_tostring(L, pos);
	return msg ? msg : "(error object cannot be converted to string)";
}

static void
assert_lua_impl(int ret, const char *expr, const char *file, int line)
{
	if (ret == 0)
		return;
	error_msg_and_die("assert_lua(%s) failed at %s:%d: %s", expr, file,
		line, get_lua_msg(-1));
}

#define assert_lua(expr) assert_lua_impl(expr, #expr, __FILE__, __LINE__)

static void
check_lua(int ret)
{
	switch (ret) {
	case 0:
		break;
	case LUA_ERRERR:
		error_msg_and_die("lua: error while running error handler: %s",
			get_lua_msg(-1));
		break;
	default:
		error_msg_and_die("lua: %s", get_lua_msg(-1));
	}
}

#ifdef LUA_FFILIBNAME
# define FFILIBNAME LUA_FFILIBNAME
#else
/* non-LuaJIT */
# define FFILIBNAME "ffi"
#endif

#ifdef LUA_BITLIBNAME
# define BITLIBNAME LUA_BITLIBNAME
#else
/* Lua <= 5.1 (non-LuaJIT) */
# define BITLIBNAME "bit"
#endif

#ifdef LUA_DBLIBNAME
# define DBLIBNAME LUA_DBLIBNAME
#else
/* ? */
# define DBLIBNAME "debug"
#endif

static int
traceback_and_die(lua_State *arg_L)
{
	/* L: error */
	const char *msg = get_lua_msg(1);
	lua_getglobal(L, DBLIBNAME); /* L: error debug */
	lua_getfield(L, -1, "traceback"); /* L: error debug traceback */
	lua_pushstring(L, msg); /* L: error debug traceback msg */
	lua_pushinteger(L, 2); /* L: error debug traceback msg level */
	lua_call(L, 2, 1); /* L: error debug result */
	const char *traceback = lua_tostring(L, -1);
	error_msg_and_die("lua: %s", traceback ? traceback : msg);
}

static void
init_luajit(const char *scriptfile)
{
	if (L)
		/* already initialized? */
		error_msg_and_help("multiple -l arguments");

	if (!(L = luaL_newstate()))
		error_msg_and_die("luaL_newstate failed (out of memory?)");

	luaL_openlibs(L);

	lua_getglobal(L, "require"); /* L: require */
	lua_pushstring(L, FFILIBNAME); /* L: require str */
	assert_lua(lua_pcall(L, 1, 1, 0)); /* L: ffi */
	lua_getfield(L, -1, "cdef"); /* L: ffi cdef */
	luaL_Buffer b;
	luaL_buffinit(L, &b); /* L: ffi cdef ? */
	{
		char buf[128];
		snprintf(buf, sizeof(buf),
			"typedef int%d_t kernel_long_t;"
			"typedef uint%d_t kernel_ulong_t;",
			(int) sizeof(kernel_long_t) * 8,
			(int) sizeof(kernel_ulong_t) * 8);
		luaL_addstring(&b, buf); /* L: ffi cdef ? */
	}
	const char *defs =
#define FFI_CDEF
#include "sysent.h"
#include "defs_shared.h"
#undef FFI_CDEF
	;
	luaL_addstring(&b, defs); /* L: ffi cdef ? */
	luaL_pushresult(&b); /* L: ffi cdef str */
	assert_lua(lua_pcall(L, 1, 0, 0)); /* L: ffi */

	lua_getfield(L, 1, "cast"); /* L: ffi cast */
	lua_remove(L, 1); /* L: cast */

	/*
	 * Assigns a FFI cdata function pointer of type rettype (*)(__VA_ARGS__)
	 * with value ptr, to the table on top of L's stack, under given name.
	 * Assumes the ffi.cast function is at the bottom of the stack.
	 */
#define EXPOSE_FUNC(rettype, ptr, name, ...)				\
	do {								\
		rettype (*fptr_)(__VA_ARGS__) = ptr;			\
		/* L: cast ? table */					\
		lua_pushvalue(L, 1); /* L: cast ? table cast */		\
		lua_pushstring(L, #rettype " (*)(" #__VA_ARGS__ ")");	\
		/* L: cast ? table cast str */				\
		lua_pushlightuserdata(L, * (void **) (&fptr_));		\
		/* L: cast ? table cast str ptr */			\
		assert_lua(lua_pcall(L, 2, 1, 0));			\
		/* L: cast ? table value */				\
		lua_setfield(L, -2, name); /* L: cast ? table */	\
	} while (0)

	/*
	 * Assigns a FFI cdata pointer of given type, to the table on top of L's
	 * stack, under given name.
	 * Assumes the ffi.cast function is at the bottom of the stack.
	 */
#define EXPOSE(type, ptr, name)						\
	do {								\
		/* Get a compilation error/warning on type mismatch */	\
		type tmp_ = ptr;					\
		(void) tmp_;						\
		/* L: cast ? table */					\
		lua_pushvalue(L, 1); /* L: cast ? table cast */		\
		lua_pushstring(L, #type);				\
		/* L: cast ? table cast str */				\
		lua_pushlightuserdata(L, (void *) ptr);			\
		/* L: cast ? table cast str ptr */			\
		assert_lua(lua_pcall(L, 2, 1, 0));			\
		/* L: cast ? table cast value */			\
		lua_setfield(L, -2, name); /* L: cast ? table */	\
	} while (0)

	lua_newtable(L); /* L: cast strace */
	lua_newtable(L); /* L: cast strace C */

	EXPOSE_FUNC(bool, func_monitor, "monitor",
		unsigned int, unsigned int, bool, bool);
	EXPOSE_FUNC(void, set_hook_qual_all, "monitor_all",
		bool, bool);
	EXPOSE_FUNC(bool, func_inject_signo, "inject_signo",
		int);
	EXPOSE_FUNC(bool, func_inject_retval, "inject_retval",
		int);
	EXPOSE_FUNC(int, func_umove, "umove",
		kernel_ulong_t, size_t, void *);
	EXPOSE_FUNC(int, func_umove_str, "umove_str",
		kernel_ulong_t, size_t, char *);
	EXPOSE_FUNC(bool, func_path_match, "path_match",
		const char **, size_t);

	EXPOSE(const struct_sysent *const *, sysent_vec, "sysent_vec");
	EXPOSE(const char *const **, errnoent_vec, "errnoent_vec");
	EXPOSE(const char *const **, signalent_vec, "signalent_vec");
	EXPOSE(const struct_ioctlent *const *, ioctlent_vec, "ioctlent_vec");

	EXPOSE(const unsigned int *, nsyscall_vec, /*(!)*/ "nsysent_vec");
	EXPOSE(const unsigned int *, nerrnoent_vec, "nerrnoent_vec");
	EXPOSE(const unsigned int *, nsignalent_vec, "nsignalent_vec");
	EXPOSE(const unsigned int *, nioctlent_vec, "nioctlent_vec");

	EXPOSE(const struct syscall_class *, syscall_classes,
		"syscall_classes");

#if SUPPORTED_PERSONALITIES == 1
	static const char *const personality_names[] = {#__WORDSIZE " bit"};
#endif
	EXPOSE(const char *const *, personality_names, "pers_name");
	EXPOSE(const int *, personality_wordsize, "pers_wordsize");
	EXPOSE(const int *, personality_klongsize, "pers_klongsize");

	lua_setfield(L, -2, "C"); /* L: cast strace */

	lua_pushinteger(L, SUPPORTED_PERSONALITIES); /* L: cast strace int */
	lua_setfield(L, -2, "npersonalities"); /* L: cast strace */

	lua_pushinteger(L, MAX_ARGS); /* L: cast strace int */
	lua_setfield(L, -2, "max_args"); /* L: cast strace */

	lua_pushinteger(L, PATH_MAX); /* L: cast strace int */
	lua_setfield(L, -2, "path_max"); /* L: cast strace */

	lua_setglobal(L, "strace"); /* L: cast */

	const char *code =
#include "luajit_lib.h"
	;
	assert_lua(luaL_loadstring(L, code)); /* L: cast chunk */

	lua_newtable(L); /* L: cast chunk table */

	lua_pushstring(L, FFILIBNAME); /* L: cast chunk table str */
	lua_setfield(L, -2, "ffilibname"); /* L: cast chunk table */
	lua_pushstring(L, BITLIBNAME); /* L: cast chunk table str */
	lua_setfield(L, -2, "bitlibname"); /* L: cast chunk table */
	lua_pushinteger(L, TCB_INSYSCALL); /* L: cast chunk table int */
	lua_setfield(L, -2, "tcb_insyscall"); /* L: cast chunk table */
	lua_pushinteger(L, QUAL_TRACE); /* L: cast chunk table int */
	lua_setfield(L, -2, "qual_trace"); /* L: cast chunk table */
	lua_pushinteger(L, QUAL_ABBREV); /* L: cast chunk table int */
	lua_setfield(L, -2, "qual_abbrev"); /* L: cast chunk table */
	lua_pushinteger(L, QUAL_VERBOSE); /* L: cast chunk table int */
	lua_setfield(L, -2, "qual_verbose"); /* L: cast chunk table */
	lua_pushinteger(L, QUAL_RAW); /* L: cast chunk table int */
	lua_setfield(L, -2, "qual_raw"); /* L: cast chunk table */

	EXPOSE_FUNC(struct tcb *, func_next_sc, "next_sc",
		void); /* L: cast chunk table */

	lua_pushcfunction(L, traceback_and_die);
	/* L: cast chunk table traceback_and_die */
	lua_replace(L, 1); /* L: traceback_and_die chunk table */

	assert_lua(lua_pcall(L, 1, 1, 1)); /* L: traceback_and_die func */

	check_lua(luaL_loadfile(L, scriptfile));
	/* L: traceback_and_die func chunk */
#undef EXPOSE_FUNC
#undef EXPOSE
}

static void ATTRIBUTE_NORETURN
run_luajit(void)
{
	/* L: traceback_and_die func chunk */
	check_lua(lua_pcall(L, 0, 0, 1)); /* L: traceback_and_die func */
	check_lua(lua_pcall(L, 0, 0, 1)); /* L: traceback_and_die */
	terminate();
}

#undef FFILIBNAME
#undef BITLIBNAME
#undef assert_lua
#undef L
