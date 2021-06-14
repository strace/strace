CFLAGS += -ggdb -std=gnu99 -Wall -Wextra

all: gen

gen: parse.tab.o lex.yy.o ast.o codegen.o symbols.o parse.tab.h lex.yy.c preprocess.o
	$(CC) $(CFLAGS) parse.tab.o lex.yy.o ast.o codegen.o symbols.o preprocess.o -o ./gen

lex.yy.c: lex.l parse.tab.h
	flex lex.l

parse.tab.c parse.tab.h: parse.y
	bison -d parse.y

clean:
	rm -f lex.yy.o ast.o parse.tab.o codegen.o preprocess.o symbols.o
	rm -f gen parse.tab.c parse.tab.h lex.yy.c lex.yy.h
