CFLAGS += -Wall

PROGS = \
    vfork fork sig skodic clone leaderkill childthread \
    sigkill_rain wait_must_be_interruptible threaded_execve \
    mtd ubi select sigreturn

all: $(PROGS)

leaderkill: LDFLAGS += -pthread

childthread: LDFLAGS += -pthread

clean distclean:
	rm -f *.o core $(PROGS) *.gdb

.PHONY: all clean distclean
