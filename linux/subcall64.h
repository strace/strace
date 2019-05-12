#define sys_semtimedop sys_semtimedop_time64
#define sys_recvmmsg sys_recvmmsg_time64
#include "subcall.h"
#undef sys_recvmmsg
#undef sys_semtimedop
