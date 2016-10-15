#include "tests.h"

#include <asm/unistd.h>

#ifdef __NR_process_vm_readv

# define OP     process_vm_readv
# define OP_NR  __NR_process_vm_readv
# define OP_STR "process_vm_readv"
# define OP_WR  0

# include "process_vm_readv_writev.c"

#else

SKIP_MAIN_UNDEFINED("__NR_process_vm_readv");

#endif
