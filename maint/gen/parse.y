/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

%define api.token.prefix {T_}
%define parse.lac full
%define parse.error detailed

%locations

%code requires {
#include "deflang.h"
#include "ast.h"
}

%{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"

static struct ast_node *root;

static void error_prev_decl(char *identifier, struct ast_node *prev);

int yylex(void);
%}

%union {
	char* str;
	struct ast_number number;

	struct ast_node *node;
	struct ast_type *type;
	struct ast_type_option *type_option;
	struct ast_type_option_list *type_option_list;
	struct ast_syscall_arg *syscall_arg;
	struct ast_struct_element *struct_element;
	struct ast_flag_values *flag_values;
}

%token NEWLINE
%token LPAREN "("
%token RPAREN ")"
%token LBRACKET "["
%token RBRACKET "]"
%token LCURLY "{"
%token RCURLY "}"
%token COMMA ","
%token EQUALS "="
%token COLON ":"
%token <str> DEFINE "#define"
%token <str> IFDEF "#ifdef"
%token ENDIF "#endif"
%token <str> IFNDEF "#ifndef"
%token <str> INCLUDE "include"
%token <str> IDENTIFIER
%token <number> TEMPLATE_IDENTIFIER
%token <number> NUMBER
%token <str> DECODER_SOURCE

%type <node> compound compound_stmt statement decoder define ifdef ifndef include syscall struct flags
%type <type> type syscall_return_type
%type <type_option_list> type_options
%type <type_option> type_option type_option_range
%type <syscall_arg> syscall_arglist syscall_arg
%type <struct_element> struct_element struct_elements
%type <flag_values> flag_elements

%destructor { free($$); } <str>
%destructor { free($$.raw); } <number>
%destructor { free_ast_tree($$); } <node>

%start start

%%

start: opt_linebreak compound_stmt
		{
			root = $2;
		}

opt_linebreak: linebreaks | %empty

linebreaks: NEWLINE linebreaks
	| NEWLINE

compound: linebreaks compound_stmt
		{
			$$ = $2;
		}

compound_stmt: statement linebreaks compound_stmt
		{
			$1->next = $3->compound.children;
			$3->compound.children = $1;
			$$ = $3;
		}
	| statement linebreaks
		{
			$$ = create_ast_node(AST_COMPOUND, &@$);
			$$->compound.children = $1;
		}
	| error linebreaks compound_stmt
		{
			$$ = $3;
		}

statement: define
	| ifdef
	| ifndef
	| include
	| syscall
	| struct
	| flags
	| decoder

decoder: ":" type DECODER_SOURCE
		{
			$$ = create_ast_node(AST_DECODER, &@$);
			$$->decoder.type = $2;
			$$->decoder.decoder = $3;
		}

syscall: IDENTIFIER "(" syscall_arglist ")" syscall_return_type syscall_attribute
		{
			$$ = create_ast_node(AST_SYSCALL, &@$);
			$$->syscall = (struct ast_syscall) {
				.name = $1,
				.args = $3,
				.return_type = $5
			};

			struct ast_node *prev_decl = symbol_add($1, $$);
			if (prev_decl) {
				error_prev_decl($1, prev_decl);
				YYERROR;
			}
		}
	| IDENTIFIER "(" ")" syscall_return_type syscall_attribute
			{
				$$ = create_ast_node(AST_SYSCALL, &@$);
				$$->syscall = (struct ast_syscall) {
					.name = $1,
					.args = NULL,
					.return_type = $4
				};

				struct ast_node *prev_decl = symbol_add($1, $$);
				if (prev_decl) {
					error_prev_decl($1, prev_decl);
					YYERROR;
				}
			}

syscall_return_type: type
		{
			$$ = $1;
		}
	| %empty
		{
			$$ = create_or_get_type(NULL, "void", NULL);
		}

syscall_attribute: "(" type_options ")"
	| %empty

syscall_arglist: syscall_arg
		{
			$$ = $1;
		}
	| syscall_arg "," syscall_arglist
		{
			$$ = $1;
			$1->next = $3;
		}

syscall_arg: IDENTIFIER type
		{
			$$ = create_ast_syscall_arg($1, $2, NULL);;
		}

type: IDENTIFIER
		{
			char *error = NULL;
			$$ = create_or_get_type(&error, $1, NULL);
			if (error) {
				yyerror("%s", error);
				YYERROR;
			}
		}
	| IDENTIFIER "[" type_options "]"
		{
			char *error = NULL;
			$$ = create_or_get_type(&error, $1, $3);
			if (error) {
				yyerror("%s", error);
				YYERROR;
			}
		}

type_options: type_option_range "," type_options
		{
			$$ = create_ast_type_option_list($1, $3);
		}
	| type_option_range
		{
			$$ = create_ast_type_option_list($1, NULL);
		}

type_option_range: type_option ":" type_option
		{
			$$ = create_type_option_range($1, $3);
		}
	| type_option
		{
			$$ = $1;
		}

type_option: type
		{
			$$ = create_or_get_type_option_nested($1);
		}
	| NUMBER
		{
			$$ = create_or_get_type_option_number($1);
		}
	| TEMPLATE_IDENTIFIER
		{
			$$ = create_type_template_identifier($1);
		}

define: DEFINE
		{
		   $$ = create_ast_node(AST_DEFINE, &@$);
		   $$->define.value = $1;
		}

ifdef: IFDEF compound ENDIF
		{
			$$ = create_ast_node(AST_IFDEF, &@$);
			$$->ifdef.value = $1;
			$$->ifdef.invert = false;
			$$->ifdef.child = $2;
		}

ifndef: IFNDEF compound ENDIF
		{
			$$ = create_ast_node(AST_IFDEF, &@$);
			$$->ifdef.value = $1;
			$$->ifdef.invert = true;
			$$->ifdef.child = $2;
		}

include: INCLUDE
		{
			$$ = create_ast_node(AST_INCLUDE, &@$);
			$$->include.value = $1;
		}

struct: IDENTIFIER "{" linebreaks struct_elements "}" struct_attr
		{
			$$ = create_ast_node(AST_STRUCT, &@$);
			$$->ast_struct.name = $1;
			$$->ast_struct.elements = $4;

			struct ast_node *prev_decl = symbol_add($1, $$);
			if (prev_decl) {
				error_prev_decl($1, prev_decl);
				YYERROR;
			}
		}
	| IDENTIFIER "{" linebreaks "}" struct_attr
		{
			yyerror("struct '%s' has no members", $1);
			$$ = NULL;
			YYERROR;
		}

struct_elements: struct_element struct_elements
		{
			$$ = $1;
			$$->next = $2;
		}
	| struct_element
		{
			$$ = $1;
		}

struct_element: IDENTIFIER type linebreaks
		{
			$$ = create_ast_struct_element($1, $2, NULL);
		}

struct_attr: "[" type "]"
	| %empty

flags: IDENTIFIER "=" flag_elements
		{
			$$ = create_ast_node(AST_FLAGS, &@$);
			$$->flags.name = $1;
			$$->flags.values = $3;

			struct ast_node *prev_decl = symbol_add($1, $$);
			if (prev_decl) {
				error_prev_decl($1, prev_decl);
				YYERROR;
			}
		}

flag_elements: IDENTIFIER "," flag_elements
		{
			$$ = create_ast_flag_values($1, $3);
		}
	| IDENTIFIER
		{
			$$ = create_ast_flag_values($1, NULL);
		}

%%

static void error_prev_decl(char *identifier, struct ast_node *prev)
{
	yyerror("Previous declaration of %s at line %d col %d",
		identifier, prev->loc.lineno, prev->loc.colno);
}

void
yyerror(const char* fmt, ...)
{
	fprintf(stderr, "error %d: %s: line %d column %d\n",
		yynerrs, cur_filename, yylloc.first_line, yylloc.first_column);
	if (yyin) {
		char buffer[257];
		long int saved = ftell(yyin);
		if (saved == -1 ||
		    fseek(yyin, last_line_location, SEEK_SET) != 0 ||
		    fgets(buffer, sizeof(buffer) - 1, yyin) == NULL) {
			buffer[0] = '\0';
		}
		if (saved != -1) {
			fseek(yyin, saved, SEEK_SET);
		}

		/* add a new line if necessary */
		size_t len = strlen(buffer);
		if (len > 0) {
			if (buffer[len - 1] != '\n') {
				buffer[len] = '\n';
				buffer[len + 1] = '\0';
			}
			fprintf(stderr, "\t%s", buffer);
		}
	}
	fprintf(stderr, "\t%*s ", yylloc.first_column, "^");

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");
}

int
main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s [input file] [output file]\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	if (!lexer_init_newfile(argv[1])) {
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (yyparse() != 0) {
		return EXIT_FAILURE;
	}

	if (!generate_code(argv[1], argv[2], preprocess(root))) {
		free_ast_tree(root);
		return EXIT_FAILURE;
	}

	free_ast_tree(root);

	return EXIT_SUCCESS;
}
