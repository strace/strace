/*
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include <stdarg.h>
#include "filter.h"

struct expression_token {
	enum token_type {
		TOK_VARIABLE,
		TOK_OPERATOR
	} type;
	union token_data {
		unsigned int variable_id;
		enum operator_type {
			OP_NOT,
			OP_AND,
			OP_OR
		} operator_id;
	} data;
};

struct bool_expression {
	unsigned int ntokens;
	struct expression_token *tokens;
};

struct bool_expression *
create_expression(void)
{
	return xcalloc(1, sizeof(struct bool_expression));
}

static void
reallocate_expression(struct bool_expression *const expr,
		      const unsigned int new_ntokens)
{
	if (!expr)
		error_msg_and_die("invalid expression");
	expr->tokens = xreallocarray(expr->tokens, new_ntokens,
				     sizeof(*expr->tokens));
	if (new_ntokens > expr->ntokens)
		memset(expr->tokens + expr->ntokens, 0,
		       sizeof(*expr->tokens) * (new_ntokens - expr->ntokens));
	expr->ntokens = new_ntokens;
}

static void
add_variable_token(struct bool_expression *expr, unsigned int id)
{
	struct expression_token token;
	token.type = TOK_VARIABLE;
	token.data.variable_id = id;
	reallocate_expression(expr, expr->ntokens + 1);
	expr->tokens[expr->ntokens - 1] = token;
}

static void
add_operator_token(struct bool_expression *expr, int op) {
	struct expression_token token;
	token.type = TOK_OPERATOR;
	token.data.operator_id = op;
	reallocate_expression(expr, expr->ntokens + 1);
	expr->tokens[expr->ntokens - 1] = token;
}

void
expression_add_filter_and(struct bool_expression *expr, unsigned int filter_id)
{
	add_variable_token(expr, filter_id);
	add_operator_token(expr, OP_AND);
}

void
set_expression_qualify_mode(struct bool_expression *expr)
{
	if (!expr)
		error_msg_and_die("invalid expression");
	reallocate_expression(expr, 1);
	expr->tokens[0].type = TOK_VARIABLE;
	expr->tokens[0].data.variable_id = 0;
}

ATTRIBUTE_FORMAT((printf, 3, 4))
static int
printf_append(char **ptr, char *end, const char *fmt, ...)
	{
		int ret;
		va_list args;

		va_start(args, fmt);
		ret = vsnprintf(*ptr, end - *ptr, fmt, args);
		va_end(args);

		if (ret < 0)
			return ret;

		*ptr += MIN(ret, end - *ptr);
		return ret;
}

/* Print full diagnostics for corrupted expression */
ATTRIBUTE_NORETURN
static void
handle_corrupted_expression(struct bool_expression *expr, bool *stack,
			    unsigned int stack_size, unsigned int current_pos,
			    bool *variables, unsigned int variables_num)
{
	char *buf, *pos, *end;
	unsigned int buf_size;
	unsigned int i;

	/* Calculate buffer size. */
	buf_size = sizeof("corrupted filter expression:");
	buf_size += sizeof("expression (ntokens = ):")
		    + 3 * sizeof(unsigned int)
		    + (sizeof("op_") + 3 * sizeof(int)) * expr->ntokens;
	buf_size += sizeof("variables (nvariables = ):") + 3 * sizeof(int)
		    + sizeof("false") * variables_num;
	buf_size += sizeof("current position: ") + 3 * sizeof(int);
	buf_size += sizeof("stack (stack_size = ):") + 3 * sizeof(int)
		    + sizeof("false") * stack_size;

	buf = xcalloc(buf_size, 1);
	pos = buf;
	end = buf + buf_size;

	printf_append(&pos, end, "corrupted filter expression:\n");

	/* Print expression. */
	printf_append(&pos, end, "expression (ntokens = %u):", expr->ntokens);
	for (i = 0; i < expr->ntokens; ++i) {
		switch (expr->tokens[i].type) {
		case TOK_VARIABLE:
			printf_append(&pos, end, " v_%u",
				      expr->tokens[i].data.variable_id);
			break;
		case TOK_OPERATOR:
			switch (expr->tokens[i].data.operator_id) {
			case OP_NOT:
				printf_append(&pos, end, " not");
				break;
			case OP_AND:
				printf_append(&pos, end, " and");
				break;
			case OP_OR:
				printf_append(&pos, end, " or");
				break;
			default:
				printf_append(&pos, end, " op_%d",
					      expr->tokens[i].data.operator_id);
			}
			break;
		default:
			printf_append(&pos, end, " ?_%d", expr->tokens[i].type);
		}
	}
	printf_append(&pos, end, "\n");

	/* Print variables. */
	printf_append(&pos, end, "variables (nvariables = %u):", variables_num);
	for (i = 0; i < variables_num; ++i)
		printf_append(&pos, end, !variables[i] ? " false" : " true");
	printf_append(&pos, end, "\n");

	printf_append(&pos, end, "current position: %u\n", current_pos);

	/* Print current stack state. */
	printf_append(&pos, end, "stack (stack_size = %u):", stack_size);
	for (i = 0; i < stack_size; ++i)
		printf_append(&pos, end, !stack[i] ? " false" : " true");

	error_msg_and_die("%s", buf);
}

#define MAX_STACK_SIZE 32

bool
run_expression(struct bool_expression *expr, bool *variables,
	       unsigned int variables_num)
{
	bool stack[MAX_STACK_SIZE];
	unsigned int stack_size = 0;
	unsigned int i;

	for (i = 0; i < expr->ntokens; ++i) {
		struct expression_token *tok = &expr->tokens[i];

		switch (tok->type) {
		case TOK_VARIABLE:
			if (stack_size == MAX_STACK_SIZE)
				handle_corrupted_expression(expr, stack,
							    stack_size, i,
							    variables,
							    variables_num);

			if (tok->data.variable_id >= variables_num)
				handle_corrupted_expression(expr, stack,
							    stack_size, i,
							    variables,
							    variables_num);
			stack[stack_size++] = variables[tok->data.variable_id];
			break;
		case TOK_OPERATOR:
			switch (tok->data.operator_id) {
			case OP_NOT:
				if (stack_size == 0)
					handle_corrupted_expression(expr, stack,
								stack_size, i,
								variables,
								variables_num);
				stack[stack_size - 1] = !stack[stack_size - 1];
				break;
			case OP_AND:
				if (stack_size < 2)
					handle_corrupted_expression(expr, stack,
								stack_size, i,
								variables,
								variables_num);
				stack[stack_size - 2] = stack[stack_size - 2]
						     && stack[stack_size - 1];
				--stack_size;
				break;
			case OP_OR:
				if (stack_size < 2)
					handle_corrupted_expression(expr, stack,
								stack_size, i,
								variables,
								variables_num);
				stack[stack_size - 2] = stack[stack_size - 2]
						     || stack[stack_size - 1];
				--stack_size;
				break;
			default:
				handle_corrupted_expression(expr, stack,
							    stack_size, i,
							    variables,
							    variables_num);
			}
			break;
		}
	}

	if (stack_size != 1)
		handle_corrupted_expression(expr, stack, stack_size, i,
					    variables, variables_num);
	return stack[0];
}
