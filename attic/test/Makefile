CFLAGS += -Wall

PROGS = \
    sig skodic clone leaderkill childthread \
    sigkill_rain wait_must_be_interruptible threaded_execve \
    mtd ubi seccomp sfd mmap_offset_decode x32_lseek x32_mmap

all: $(PROGS)

leaderkill: LDFLAGS += -pthread

childthread: LDFLAGS += -pthread

clean distclean:
	rm -f *.o core $(PROGS) *.gdb

.PHONY: all clean distclean
