/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "deflang.h"
#include "symbols.h"

#define MAX_PREPROCESSOR_NEST 16
#define MAX_SYSCALL_COUNT 4096


struct condition_stack {
	size_t idx;
	char *stack[MAX_PREPROCESSOR_NEST];
};

/*
 * Copies (pointers to) the strings stored in the condition stack into a
 * statement_condition struct.
 *
 * Returns NULL if the stack is empty (i.e. there are no conditions)
 */
static struct statement_condition *
create_statement_condition(struct condition_stack *stack)
{
	if (stack->idx == 0) {
		return NULL;
	}
	struct statement_condition *ret =
		xmalloc((sizeof *ret) + stack->idx * (sizeof(char *)));
	ret->count = stack->idx;
	memcpy(ret->values, stack->stack, stack->idx * (sizeof(char *)));
	return ret;
}

static char *
strip_whitespace(char *str)
{
	if (*str == '\0') {
		return str;
	}

	char *end = str + strlen(str) - 1;

	while (end > str && isspace(*end)) {
		end--;
	}

	end[1] = '\0';

	return str;
}


struct processing_state {
	struct preprocessor_statement_list *preprocessor_head;
	struct preprocessor_statement_list *preprocessor_tail;
	struct decoder_list *decoder_head;
	struct struct_def *struct_stmts;
	struct syscall **syscall_buffer;
	size_t syscall_index;
};

/*
 * Splits the AST into preprocessor definitions, struct definitions, and
 * syscall definitions while maintaining the necessary information about
 * ifdef/ifndef conditions.
 */
static void
preprocess_rec(struct ast_node *root, struct condition_stack *cur,
			   struct processing_state *state)
{
	if (root->type == AST_IFDEF) {
		assert(cur->idx < MAX_PREPROCESSOR_NEST);
		cur->stack[cur->idx] = root->ifdef.value;
		cur->idx++;
		preprocess_rec(root->ifdef.child, cur, state);
		cur->idx--;
		cur->stack[cur->idx] = NULL;
	} else if (root->type == AST_DECODER) {
		struct decoder_list *decoder = xmalloc(sizeof *decoder);
		*decoder = (struct decoder_list) {
			.decoder = {
				.loc = root->loc,
				.matching_type = root->decoder.type,
				.fmt_string = strip_whitespace(root->decoder.decoder)
			},
			.next = state->decoder_head
		};
		state->decoder_head = decoder;
	} else if (root->type == AST_DEFINE || root->type == AST_INCLUDE) {
		struct statement_condition *conditions = create_statement_condition(cur);
		struct preprocessor_statement_list *new = xmalloc(sizeof *new);

		new->next = NULL;
		new->stmt.conditions = conditions;
		new->stmt.loc = root->loc;
		if (root->type == AST_DEFINE) {
			new->stmt.value = root->define.value;
		} else {
			new->stmt.value = root->include.value;
		}
		if (state->preprocessor_tail) {
			state->preprocessor_tail->next = new;
			state->preprocessor_tail = new;
		} else {
			state->preprocessor_head = new;
			state->preprocessor_tail = new;
		}
	} else if (root->type == AST_COMPOUND) {
		for (struct ast_node *node = root->compound.children; node != NULL; node = node->next) {
			preprocess_rec(node, cur, state);
		}
	} else if (root->type == AST_SYSCALL) {
		size_t arg_count = 0;
		for (struct ast_syscall_arg *arg = root->syscall.args; arg != NULL; arg = arg->next) {
			arg_count++;
		}

		struct syscall *new =
			xmalloc(sizeof(*new) + sizeof(struct syscall_argument) * arg_count);
		*new = (struct syscall) {
			.name = root->syscall.name,
			.conditions = create_statement_condition(cur),
			.ret = *root->syscall.return_type,
			.arg_count = arg_count,
			.loc = root->loc,
			.is_ioctl = strncmp(root->syscall.name, "ioctl$", 6) == 0
		};

		size_t cur_count = 0;
		for (struct ast_syscall_arg *arg = root->syscall.args; arg != NULL; arg = arg->next) {
			new->args[cur_count] = (struct syscall_argument) {
				.name = arg->name,
				.type = arg->type
			};
			cur_count++;
		}

		state->syscall_buffer[state->syscall_index] = new;
		state->syscall_index++;
	}
}

/*
 * Create a group of variant syscalls from a name-sorted list of syscalls.

 * Returns the number of syscalls processed.
 *
 * For example, ["prctl" "prctl$PR_CAP_AMBIENT", "prctl$GET_FP_MODE", "ioctl"]
 * would group together the prctl variants, store a syscall_group in out[out_idx]
 * and returns 3.
 */
static size_t
find_matching(struct syscall **syscall_buffer, size_t syscall_count,
	      struct syscall_group *out)
{
	struct syscall *base = syscall_buffer[0];
	assert(base != NULL);

	size_t base_name_len = strlen(base->name);
	size_t matching = 0;
	for (size_t i = 1; i < syscall_count; i++) {
		struct syscall *cur = syscall_buffer[i];
		/* all variants start with the same name as the base */
		if (strncmp(cur->name, base->name, base_name_len) != 0) {
			break;
		}
		/* and their last '$' is immediately after the base name */
		char *last_dollar = strrchr(cur->name, '$');
		if (last_dollar == cur->name + base_name_len) {
			matching++;
		}
	}

	if (matching == 0) {
		out[0] = (struct syscall_group) {
			.base = base,
			.child_count = 0,
			.children = NULL,
		};
		return 1;
	}

	struct syscall_group *children =
		xmalloc(sizeof(struct syscall_group) * matching);
	size_t children_idx = 0;

	size_t i = 1;
	while (i < syscall_count) {
		struct syscall *cur = syscall_buffer[i];
		if (strncmp(cur->name, base->name, base_name_len) != 0) {
			break;
		}
		char *last_dollar = strrchr(cur->name, '$');
		if (last_dollar != cur->name + base_name_len) {
			/* not a direct subvariant */
			fprintf(stderr, "not subvariant %s -> %s \n",
				base->name, cur->name);
			i += 1;
			continue;
		}
		i += find_matching(syscall_buffer + i, syscall_count - i,
				   children + children_idx);
		children_idx++;
	}

	assert(children_idx == matching);

	out[0] = (struct syscall_group) {
		.base = base,
		.child_count = children_idx,
		.children = children
	};
	return i;
}

static int
syscall_comparator(const void *a, const void *b)
{
	const struct syscall *syscall_a = *(const struct syscall **) a;
	const struct syscall *syscall_b = *(const struct syscall **) b;

	return strcmp(syscall_a->name, syscall_b->name);
}

static struct syscall_group *
group_syscall_variants(struct processing_state *state, size_t *out_count)
{
	/*
	 * The idea is to sort the syscalls by name:
	 * "prctl" "prctl$GET_FP_MODE"
	 *         "prctl$PR_CAP_AMBIENT"
	 *         "prctl$PR_CAP_AMBIENT$PR_CAP_AMBIENT_LOWER"
	 * This way, every variant will immediately follow the base syscall
	 * and will be grouped into a syscall_group 'find_matching'.
	 */

	qsort(state->syscall_buffer, state->syscall_index,
	      sizeof(struct syscall *), syscall_comparator);

	/*
	 * In the worst case (no variants),
	 * there can be MAX_SYSCALL_COUNT syscall groups.
	 */
	struct syscall_group *scratch =
		xcalloc(MAX_SYSCALL_COUNT, sizeof(*scratch));

	size_t groups = 0;
	size_t i = 0;
	while (i < state->syscall_index) {
		i += find_matching(state->syscall_buffer + i,
				   state->syscall_index - i,
				   scratch + groups);
		groups++;
	}

	struct syscall_group *ret =
		realloc(scratch, sizeof(*scratch) * (groups + 1));

	if (ret == NULL) {
		fprintf(stderr, "realloc failed for %zu bytes\n",
			sizeof(*scratch) * groups);
		exit(1);
	}

	*out_count = groups;
	return ret;
}

struct processed_ast *
preprocess(struct ast_node *root)
{
	struct processed_ast *ret = xmalloc(sizeof *ret);

	struct processing_state state = (struct processing_state) {
		.syscall_buffer =
			xcalloc(MAX_SYSCALL_COUNT, sizeof(struct syscall *)),
		.syscall_index = 0,
		.struct_stmts = NULL,
		.preprocessor_head = NULL,
		.preprocessor_tail = NULL,
		.decoder_head = NULL
	};

	struct condition_stack conditions = {.idx = 0, .stack = {0}};
	preprocess_rec(root, &conditions, &state);

	ret->preprocessor_stmts = state.preprocessor_head;
	ret->struct_stmts = state.struct_stmts;
	ret->syscall_groups =
		group_syscall_variants(&state, &ret->syscall_group_count);
	ret->decoders = state.decoder_head;

	return ret;
}
