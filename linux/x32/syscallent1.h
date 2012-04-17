/* Our second set comes from the i386 files.
   Only a couple of calls we cannot support without the i386 headers.  */

#define sys_oldstat printargs
#define sys_oldfstat printargs
#define sys_oldlstat printargs
#define sys_lseek sys_lseek32
#define sys_lstat64 sys_stat64
#define sys_truncate64 sys_truncate
#define sys_ftruncate64 sys_ftruncate
#include "i386/syscallent.h"
