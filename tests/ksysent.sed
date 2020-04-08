#!/bin/sed -rnf
#
# Copyright (c) 2015-2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

# should not have been exported at all
/#define[[:space:]]+__NR_(sys_epoll_|arch_specific_syscall|arm_sync_file_range|syscalls|syscall_count|syscall_max|available|reserved|unused)/d

# remove new aliases to traditional names on alpha
/#define[[:space:]]+__NR_get[gup]id[[:space:]]+__NR_getx[gup]id$/d

# should not have been named this way
s/__NR_(arm|xtensa)_fadvise64_64/__NR_fadvise64_64/

# legacy names
s/__NR_get_cpu/__NR_getcpu/
s/__NR_madvise1/__NR_madvise/
s/__NR_paccept/__NR_accept4/

# generate

# prioritize __NR_umount over __NR_umount2
s/#define[[:space:]]+__NR_(umount)2([[:space:]].*)?$/#if defined __NR_\12 \&\& (!defined __NR_\1 || __NR_\1 != __NR_\12)\n[__NR_\12 \& 0xffff] = "\12",\n#endif/p

# prioritize __NR_osf_shmat over __NR_shmat
s/#define[[:space:]]+__NR_(shmat)([[:space:]].*)?$/#if defined __NR_\1 \&\& (!defined __NR_osf_\1 || __NR_osf_\1 != __NR_\1)\n[__NR_\1 \& 0xffff] = "\1",\n#endif/p

# generic
s/#define[[:space:]]+__NR_([a-z_][^[:space:]]+)([[:space:]].*)?$/#ifdef __NR_\1\n[__NR_\1 \& 0xffff] = "\1",\n#endif/p
