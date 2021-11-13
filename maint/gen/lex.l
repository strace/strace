/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

%option noyywrap nounput yylineno warn

%{
#include <stdio.h>
#include <stdint.h>
#include "deflang.h"
#include "ast.h"
#include "parse.tab.h"

static void
update_yylloc();

#define YY_USER_ACTION update_yylloc();

#define MAX_IMPORT_LEVEL 10

struct saved_import_state {
	YYLTYPE location;
	char *filename;
	int cur_location;
	int last_line_location;
};

/* a stack to store state before an import */
static struct saved_import_state import_states[MAX_IMPORT_LEVEL];
/* the current index into import_states */
static int import_level = 0;

char *cur_filename;

static int cur_location;

int last_line_location;
%}

%x COMMENT_MULTI
%x COMMENT_LINE
%x IMPORT

%%
"," return T_COMMA;
"(" return T_LPAREN;
")" return T_RPAREN;
"[" return T_LBRACKET;
"]" return T_RBRACKET;
"{" return T_LCURLY;
"}" return T_RCURLY;
"=" return T_EQUALS;
":" return T_COLON;

(-)?"0x"[0-9A-Fa-f]+ {
	yylval.number.raw = xstrdup(yytext);
	yylval.number.val = strtol(yytext, NULL, 16);
	return T_NUMBER;
}

(-)?[0-9]+ {
	yylval.number.raw = xstrdup(yytext);
	yylval.number.val = strtol(yytext, NULL, 10);
	return T_NUMBER;
}

(-)?"0b"[01]+ {
	int sign = (yytext[0] == '-') ? -1 : +1;
	int offset = ((sign == -1) ? sizeof("-0b") : sizeof("0b")) - 1;

	/* binary literals are supported in C by GNU extension */
	yylval.number.raw = xstrdup(yytext);
	yylval.number.val = sign * strtol(yytext + offset, NULL, 2);
	return T_NUMBER;
}

\'.\' {
	yylval.number.raw = xstrdup(yytext);
	yylval.number.val = yytext[1];
	return T_NUMBER;
}

\$[0-9]+ {
	yylval.number.val = strtol(yytext + 1, NULL, 10);
	yylval.number.raw = NULL;
	return T_TEMPLATE_IDENTIFIER;
}

[A-Za-z_@\?][A-Za-z0-9_\?\$]* {
	if (yytext[0] == '@' && strcmp(yytext, "@ret") != 0) {
		yyerror("@ can only be used in @ret");
		yyterminate();
	}
	yylval.str = xstrdup(yytext);
	return T_IDENTIFIER;
}

(?x: "%{" ( [^%] | %+ [^}] )* %* "%}" ) {
	yylval.str = xstrdup(yytext + 2);
	yylval.str[strlen(yylval.str) - 2] ='\0';

	return T_DECODER_SOURCE;
}

"define".+ {
	yylval.str = xstrdup(yytext);
	return T_DEFINE;
}
"#ifdef".+ {
	yylval.str = xstrdup(yytext);
	return T_IFDEF;
}
"#ifndef".+ {
	yylval.str = xstrdup(yytext);
	return T_IFNDEF;
}
"include".+ {
	yylval.str = xstrdup(yytext);
	return T_INCLUDE;
}
"#endif".* {
	return T_ENDIF;
}

"#import \"" {
	BEGIN(IMPORT);
}
<IMPORT>[^\n\"]+ {
	if (import_level >= MAX_IMPORT_LEVEL) {
		fprintf(stderr, "imports are nested more than %d levels\n",
			MAX_IMPORT_LEVEL);
		yyterminate();
	}

	/* eat characters until newline */
	int c = input();
	cur_location++;
	while(c && c != '\n'){
		cur_location++;
		c = input();
	}
	/* update current location */
	yylloc.last_line++;
	yylloc.last_column = 1;
	last_line_location = cur_location;

	/* save current state */
	import_states[import_level++] = (struct saved_import_state) {
		.filename = cur_filename,
		.location = yylloc,
		.cur_location = cur_location,
		.last_line_location = last_line_location
	};

	cur_filename = xstrdup(yytext);

	yylloc = (struct YYLTYPE) {1, 1, 1, 1};
	cur_location = 0;
	last_line_location = 0;

	yyin = fopen(yytext, "r");

	if (yyin == NULL) {
		fprintf(stderr, "failed to import file '%s' on line %d\n",
			yytext, yylineno);
		yyterminate();
	}

	yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
	BEGIN(INITIAL);
}

<<EOF>> {
	/*
	 * Emit a newline at the end of a file before EOF
	 * to ensure the last statement in the file is terminated.
	 */
	static int emitted_newline;

	if (!emitted_newline) {
		emitted_newline = 1;
		return T_NEWLINE;
	}

	emitted_newline = 0;

	if (import_level > 0) {
		free(cur_filename);

		struct saved_import_state saved = import_states[--import_level];

		cur_filename = saved.filename;
		cur_location = saved.cur_location;
		last_line_location = saved.last_line_location;
		yylloc = saved.location;
	}
	yypop_buffer_state();
	if (!YY_CURRENT_BUFFER) {
		yyterminate();
	}
}

"/*" {
	BEGIN(COMMENT_MULTI);
}
<COMMENT_MULTI>"*/" {
	BEGIN(INITIAL);
}

"//"|"#" {
	BEGIN(COMMENT_LINE);
}
<COMMENT_LINE>\n {
	BEGIN(INITIAL);
}

<COMMENT_LINE,COMMENT_MULTI>.|\n {}

[ \t\r] {}

\n {
	return T_NEWLINE;
}

. {
	yyerror("unexpected character: %s", yytext);
	yyterminate();
}

%%

static void
update_yylloc()
{
	yylloc.first_line = yylloc.last_line;
	yylloc.first_column = yylloc.last_column;

	int i = 0;
	while (yytext[i] != '\0') {
		cur_location++;
		if (yytext[i] == '\n') {
			yylloc.last_line++;
			yylloc.last_column = 1;
			last_line_location = cur_location;
		} else {
			yylloc.last_column++;
		}
		i++;
	}
}

bool
lexer_init_newfile(char *filename)
{
	/* clean up internal state managed by flex */
	yylex_destroy();

	yyin = fopen(filename, "r");
	if (yyin == NULL) {
		return false;
	}

	cur_filename = filename;

	return true;
}
