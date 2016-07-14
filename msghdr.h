#ifndef MSGHDR_H_
# define MSGHDR_H_

/* For definitions of struct msghdr and struct mmsghdr. */
# include <sys/socket.h>

# ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned msg_len;
};
# endif

struct tcb;
extern void print_struct_msghdr(struct tcb *, const struct msghdr *, const int *, unsigned long);

#endif /* MSGHDR_H_ */
