#ifndef STRACE_SECCOMP_FPROG_H
#define STRACE_SECCOMP_FPROG_H

struct seccomp_fprog {
	unsigned short len;
	unsigned long filter;
};

#endif /* !STRACE_SECCOMP_FPROG_H */
