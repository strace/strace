/*
 * Should only be included without FFI_CDEF from defs.h, so no include guards.
 */

#include "ffi.h"

FFI_CONTENT(
struct syscall_class {
	const char *name;
	unsigned int value;
};
)

FFI_CONTENT(
typedef struct ioctlent {
	const char *symbol;
	unsigned int code;
} struct_ioctlent;
)

/* Trace Control Block */
FFI_CONTENT(
struct tcb {
	int flags;		/* See below for TCB_ values */
	int pid;		/* If 0, this tcb is free */
	int qual_flg;		/* qual_flags[scno] or DEFAULT_QUAL_FLAGS + RAW */
	unsigned long u_error;	/* Error code */
	kernel_ulong_t scno;	/* System call number */
	kernel_ulong_t u_arg[MAX_ARGS];	/* System call arguments */
	kernel_long_t u_rval;	/* Return value */
)

#if SUPPORTED_PERSONALITIES > 1
FFI_CONTENT(
	unsigned int currpers;	/* Personality at the time of scno update */
)
#endif

#ifndef FFI_CDEF
	int sys_func_rval;	/* Syscall entry parser's return value */
	int curcol;		/* Output column for this process */
	FILE *outf;		/* Output file for this process */
	const char *auxstr;	/* Auxiliary info from syscall (see RVAL_STR) */
	void *_priv_data;	/* Private data for syscall decoding functions */
	void (*_free_priv_data)(void *); /* Callback for freeing priv_data */
	const struct_sysent *s_ent; /* sysent[scno] or dummy struct for bad scno */
	const struct_sysent *s_prev_ent; /* for "resuming interrupted SYSCALL" msg */
	struct inject_opts *inject_vec[SUPPORTED_PERSONALITIES];
	struct timeval stime;	/* System time usage as of last process wait */
	struct timeval dtime;	/* Delta for system time usage */
	struct timeval etime;	/* Syscall entry time */
# ifdef USE_LIBUNWIND
	struct UPT_info *libunwind_ui;
	struct mmap_cache_t *mmap_cache;
	unsigned int mmap_cache_size;
	unsigned int mmap_cache_generation;
	struct queue_t *queue;
# endif
#endif /* !FFI_CDEF */

FFI_CONTENT(
};
)
