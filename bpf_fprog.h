#ifndef STRACE_BPF_FPROG_H
#define STRACE_BPF_FPROG_H

struct bpf_fprog {
	unsigned short len;
	kernel_ulong_t filter;
};

#endif /* !STRACE_BPF_FPROG_H */
