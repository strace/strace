#define sys_semtimedop sys_semtimedop_time32
#define sys_recvmmsg sys_recvmmsg_time32
#include "subcall.h"
#undef sys_recvmmsg
#undef sys_semtimedop
