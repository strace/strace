#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define text_string "STRACE_STRING"
#define msgsz sizeof(text_string)

int
main (void)
{
	const long mtype = 0xdefaced;
	struct {
		long mtype;
		char mtext[msgsz];
	} msg = {
		.mtype = mtype,
		.mtext = text_string
	};
	int msqid = msgget(IPC_PRIVATE, IPC_CREAT | S_IRWXU);
	if (msqid == -1)
		return 77;
	if (msgsnd(msqid, &msg, msgsz, 0) == -1)
		goto cleanup;
	if (msgrcv(msqid, &msg, msgsz, mtype, 0) != msgsz)
		goto cleanup;
	if (msgctl(msqid, IPC_RMID, 0) == -1)
		return 77;
	return 0;

cleanup:
	msgctl(msqid, IPC_RMID, 0);
	return 77;
}
