#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
	static const char data[] = "data";
	const size_t size = sizeof(data) - 1;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	pid_t pid;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	close(0);
	close(1);

	if (socket(PF_INET, SOCK_STREAM, 0)) {
		perror("socket");
		return 77;
	}
	if (bind(0, (struct sockaddr *) &addr, len)) {
		perror("bind");
		return 77;
	}
	assert(listen(0, 5) == 0);

	memset(&addr, 0, sizeof(addr));
	assert(getsockname(0, (struct sockaddr *) &addr, &len) == 0);

	assert((pid = fork()) >= 0);

	if (pid) {
		char buf[sizeof(data)];
		int status;

		assert(accept(0, (struct sockaddr *) &addr, &len) == 1);
		assert(close(0) == 0);
		assert(recv(1, buf, sizeof(buf), MSG_WAITALL) == (int) size);
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
		assert(close(1) == 0);
	} else {
		assert(close(0) == 0);
		assert(socket(PF_INET, SOCK_STREAM, 0) == 0);
		assert(connect(0, (struct sockaddr *) &addr, len) == 0);
		assert(send(0, data, size, MSG_DONTROUTE) == (int) size);
		assert(close(0) == 0);
	}

	return 0;
}
