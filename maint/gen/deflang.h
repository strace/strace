#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "preprocess.h"

extern int yylineno;
extern FILE *yyin;

extern int last_line_location;
extern char *cur_filename;

void *
xmalloc(size_t n);

void *
xcalloc(size_t n);

extern int
yylex_destroy(void);

bool
lexer_init_newfile(char *filename);

void
yyerror(const char *s, ...) __attribute__ ((format (printf, 1, 2)));

bool
generate_code(const char *in_filename, const char *out_filename, struct processed_ast *ast);