# Copyright (c) 2021 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

CPPFLAGS = -std=gnu99 -O2 -D_GNU_SOURCE=1 -Wall -Wextra
TARGET = gen
OBJ = parse.tab.o lex.yy.o ast.o codegen.o symbols.o preprocess.o xmalloc.o
DEPS = $(OBJ:.o=.d)

GEN_IN = hdio.def
GEN_OUT_DIR=../../src/gen
GEN_OUT = $(patsubst %.def,$(GEN_OUT_DIR)/gen_%.c,$(GEN_IN))

all: $(TARGET) $(GEN_OUT)

$(GEN_OUT_DIR)/gen_%.c: defs/%.def $(TARGET)
	mkdir -p $(GEN_OUT_DIR)
	./gen $< $@

gen: $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

lex.yy.c: lex.l parse.tab.h
	flex $<

parse.tab.c parse.tab.h: parse.y
	bison -d $<

clean:
	$(RM) $(TARGET) $(OBJ) $(DEPS) \
		parse.tab.c parse.tab.h lex.yy.c

%.d: %.c
	$(CC) -MM $(CPPFLAGS) $< -o $@

$(DEPS): parse.tab.h

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
