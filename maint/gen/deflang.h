/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef DEFLANG_H
# define DEFLANG_H

# include <stdbool.h>
# include <stdio.h>

# include "preprocess.h"
# include "xmalloc.h"

extern FILE *yyin;

extern int last_line_location;
extern char *cur_filename;

bool
lexer_init_newfile(char *filename);

void
yyerror(const char *s, ...) ATTRIBUTE_FORMAT((printf, 1, 2));

bool
generate_code(const char *in_filename, const char *out_filename,
	      struct processed_ast *ast);

#endif /* DEFLANG_H */
