#!/bin/sed -rnf

# should not have been exported at all
/#define[[:space:]]+__NR_(sys_epoll_|arch_specific_syscall|syscalls|syscall_count|syscall_max|available|reserved|unused)/d

# should not  have been named this way
s/__NR_(arm|xtensa)_fadvise64_64/__NR_fadvise64_64/

# legacy names
s/__NR_get_cpu/__NR_getcpu/
s/__NR_madvise1/__NR_madvise/
s/__NR_paccept/__NR_accept4/

# generate
s/#define[[:space:]]+__NR_([a-z_][^[:space:]]+)([[:space:]].*)?$/#ifdef __NR_\1\n[__NR_\1 \& 0xffff] = "\1",\n#endif/p
