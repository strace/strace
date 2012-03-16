all: \
    vfork fork sig skodic clone leaderkill childthread \
    sigkill_rain wait_must_be_interruptible threaded_execve

leaderkill: LDFLAGS += -pthread

childthread: LDFLAGS += -pthread

clean distclean:
	rm -f *.o core \
    vfork fork sig skodic clone leaderkill childthread \
    sigkill_rain wait_must_be_interruptible threaded_execve
