# Automake input for linux/ subdirectory of strace.

noinst_HEADERS = ioctlent.h errnoent.h signalent.h \
		 syscall.h syscallent.h dummy.h
BUILT_SOURCES = $(noinst_HEADERS)

DIST_SUBDIRS = mips sparc x86_64

EXTRA_DIST = ioctlsort.c \
	     */errnoent.h */ioctlent.h */signalent.h */syscallent.h

ioctlent.h: ioctlsort
	./ioctlsort >$@

ioctldefs.h ioctls.h: ioctlent.sh
	$(SHELL) $(srcdir)/ioctlent.sh

errnoent.h: ../errnoent.sh $(includedir)/linux/errno.h
	$(SHELL) $(srcdir)/../errnoent.sh $(includedir)/*/errno.h >$@

signalent.h: ../signalent.sh $(includedir)/linux/signal.h
	$(SHELL) $(srcdir)/../signalent.sh $(includedir)/*/signal.h >$@

#syscallent.h: ../syscallent.sh $(includedir)/sys/syscall.h
syscallent.h:
	$(SHELL) $(srcdir)/../syscallent.sh $(includedir)/sys/syscall.h >$@
