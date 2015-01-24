#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>

int main(void)
{
	int fd;
	int data = 0;
	struct iovec iov = {
		.iov_base = &data,
		.iov_len = sizeof(iov)
	};

	while ((fd = open("/dev/null", O_RDWR)) < 3)
		assert(fd >= 0);
	(void) close(3);

	int sv[2];
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	int one = 1;
	assert(setsockopt(sv[0], SOL_SOCKET, SO_PASSCRED, &one, sizeof(one)) == 0);

	pid_t pid = fork();
	assert(pid >= 0);

	if (pid) {
		assert(close(sv[0]) == 0);
		assert(dup2(sv[1], 1) == 1);
		assert(close(sv[1]) == 0);

		int fds[2];
		assert((fds[0] = open("/dev/null", O_RDWR)) == 3);
		assert((fds[1] = open("/dev/zero", O_RDONLY)) == 4);

		union {
			struct cmsghdr cmsg;
			char buf[CMSG_LEN(sizeof(fds))];
		} control = {
			.cmsg = {
				.cmsg_level = SOL_SOCKET,
				.cmsg_type = SCM_RIGHTS,
				.cmsg_len = CMSG_LEN(sizeof(fds))
			}
		};

		memcpy(CMSG_DATA(&control.cmsg), fds, sizeof(fds));

		struct msghdr mh = {
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = &control,
			.msg_controllen = sizeof(control)
		};

		assert(sendmsg(1, &mh, 0) == sizeof(iov));
		assert(close(1) == 0);

                int status;
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
	} else {
		assert(close(sv[1]) == 0);
		assert(dup2(sv[0], 0) == 0);
		assert(close(sv[0]) == 0);

		struct cmsghdr control[4];

		struct msghdr mh = {
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = control,
			.msg_controllen = sizeof(control)
		};

		assert(recvmsg(0, &mh, 0) == sizeof(iov));
		assert(close(0) == 0);
	}

	return 0;
}
