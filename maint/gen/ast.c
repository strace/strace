/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "deflang.h"
#include "symbols.h"
#include "parse.tab.h"

struct ast_node *
create_ast_node(enum ast_node_type type, void *loc)
{
	struct ast_node *node = xmalloc(sizeof *node);
	*node = (struct ast_node) {
		.type = type,
		.loc = {
			.lineno = ((YYLTYPE *) loc)->first_line,
			.colno = ((YYLTYPE *) loc)->first_column,
			.file = xstrdup(cur_filename)
		},
		.next = NULL
	};
	return node;
}

struct ast_type_option_list *
create_ast_type_option_list(struct ast_type_option *cur,
			    struct ast_type_option_list *next)
{
	struct ast_type_option_list *list = xmalloc(sizeof *list);
	*list = (struct ast_type_option_list) {
		.next = next,
		.option = cur
	};
	return list;
}

struct ast_syscall_arg *
create_ast_syscall_arg(char *name, struct ast_type *type,
		       struct ast_syscall_arg *next)
{
	struct ast_syscall_arg *arg = xmalloc(sizeof *arg);
	*arg = (struct ast_syscall_arg) {
		.name = name,
		.type = type,
		.next = next
	};
	return arg;
}

struct ast_flag_values *
create_ast_flag_values(char *name, struct ast_flag_values *next)
{
	struct ast_flag_values *arg = xmalloc(sizeof *arg);
	*arg = (struct ast_flag_values) {
		.name = name,
		.next = next
	};
	return arg;
}

struct ast_struct_element *
create_ast_struct_element(char *name, struct ast_type *type,
			  struct ast_struct_element *next)
{
	struct ast_struct_element *struct_element =
		xmalloc(sizeof *struct_element);
	*struct_element = (struct ast_struct_element) {
		.name = name,
		.type = type,
		.next = next
	};
	return struct_element;
}

struct known_type {
	struct ast_type type;
	struct known_type *next;
};

static struct known_type *known_types = NULL;

struct known_type_option {
	struct ast_type_option type_option;
	struct known_type_option *next;
};

static struct known_type_option *known_type_options = NULL;

static bool
compare_type_option_list(struct ast_type_option_list *a,
			 struct ast_type_option_list *b,
			 bool match_templates)
{
	struct ast_type_option_list *cur_a = a;
	struct ast_type_option_list *cur_b = b;

	while (cur_a != NULL) {
		if (cur_b == NULL) {
			return false;
		}

		if (cur_a->option->child_type == AST_TYPE_CHILD_TEMPLATE_ID ||
		    cur_b->option->child_type == AST_TYPE_CHILD_TEMPLATE_ID) {
			if (match_templates) {
				/* templates can match all other type options */
				cur_a = cur_a->next;
				cur_b = cur_b->next;
				continue;
			}
			return false;
		}

		if (cur_a->option->child_type != cur_b->option->child_type) {
			return false;
		}

		if (cur_a->option->child_type == AST_TYPE_CHILD_NUMBER &&
		    cur_a->option->number.val != cur_b->option->number.val) {
			return false;
		}

		if (cur_a->option->child_type == AST_TYPE_CHILD_TYPE) {
			if (!(strcmp(cur_a->option->type->name,
				     cur_b->option->type->name) == 0 &&
			      compare_type_option_list(cur_a->option->type->options,
						       cur_b->option->type->options,
						       match_templates))) {
				return false;
			}
		}

		cur_a = cur_a->next;
		cur_b = cur_b->next;
	}

	if (cur_b != NULL) {
		return false;
	}

	return true;
}

bool
ast_type_matching(struct ast_type *a, struct ast_type *b)
{
	return strcmp(a->name, b->name) == 0 &&
	       compare_type_option_list(a->options, b->options, true);
}

struct ast_type *
create_or_get_type(char **error, char *name, struct ast_type_option_list *options)
{
	/* check if we've seen this type before */
	for (struct known_type *cur = known_types;
	     cur != NULL; cur = cur->next) {
		if (strcmp(cur->type.name, name) == 0 &&
		    compare_type_option_list(cur->type.options, options, false)) {
			return &cur->type;
		}
	}

	/* allocate a new type */
	struct known_type *type = xmalloc(sizeof *type);

	char *status = resolve_type(&type->type, name, options);
	type->next = known_types;

	if (error) {
		*error = status;
	}

	if (status != NULL) {
		free(type);
		return NULL;
	}

	known_types = type;

	return &type->type;
}

struct ast_type_option *
create_or_get_type_option_number(struct ast_number number)
{
	/* check if we've seen this type option before */
	for (struct known_type_option *cur = known_type_options;
	     cur != NULL; cur = cur->next) {
		if (cur->type_option.child_type == AST_TYPE_CHILD_NUMBER &&
		    cur->type_option.number.val == number.val) {
			return &cur->type_option;
		}
	}

	/* allocate a new type option */
	struct known_type_option *option = xmalloc(sizeof *option);
	*option = (struct known_type_option) {
		.type_option = {
			.child_type = AST_TYPE_CHILD_NUMBER,
			.number = number
		},
		.next = known_type_options
	};

	known_type_options = option;

	return &option->type_option;
}

struct ast_type_option *
create_type_template_identifier(struct ast_number number)
{
	struct ast_type_option *option = xmalloc(sizeof *option);
	*option = (struct ast_type_option) {
		.child_type = AST_TYPE_CHILD_TEMPLATE_ID,
		.template = {
			.id = number.val
		}
	};

	return option;
}

struct ast_type_option *
create_or_get_type_option_nested(struct ast_type *child)
{
	/* check if we've seen this type option before */
	for (struct known_type_option *cur = known_type_options;
	     cur != NULL; cur = cur->next) {
		/*
		 * Since all types are allocated by create_or_get_type,
		 * types that are equal have the same address.
		 */
		if (cur->type_option.child_type == AST_TYPE_CHILD_TYPE &&
		    cur->type_option.type == child) {
			return &cur->type_option;
		}
	}

	/* allocate a new type option */
	struct known_type_option *option = xmalloc(sizeof *option);
	*option = (struct known_type_option) {
		.type_option = {
			.child_type = AST_TYPE_CHILD_TYPE,
			.type = child
		},
		.next = known_type_options
	};

	known_type_options = option;

	return &option->type_option;
}

struct ast_type_option *
create_type_option_range(struct ast_type_option *min,
			 struct ast_type_option *max)
{
	struct ast_type_option *ret = xmalloc(sizeof *ret);
	*ret = (struct ast_type_option) {
		.child_type = AST_TYPE_CHILD_RANGE,
		.range = {
			.min = min,
			.max = max
		}
	};
	return ret;
}


void
free_ast_tree(struct ast_node *root)
{
	switch (root->type) {
		case AST_IFDEF:
			free(root->ifdef.value);
			break;
		case AST_DEFINE:
			free(root->define.value);
			break;
		case AST_INCLUDE:
			free(root->include.value);
			break;
		case AST_STRUCT: {
			struct ast_struct_element *cur =
				root->ast_struct.elements;
			while (cur != NULL) {
				struct ast_struct_element *tmp = cur;
				cur = tmp->next;
				free(tmp->name);
				free(tmp);
			}
			break;
		}
		case AST_COMPOUND: {
			struct ast_node *cur = root->compound.children;
			while (cur != NULL) {
				struct ast_node *tmp = cur;
				cur = tmp->next;
				free_ast_tree(tmp);
			}
			break;
		}
		case AST_SYSCALL: {
			struct ast_syscall_arg *cur = root->syscall.args;
			while (cur != NULL) {
				struct ast_syscall_arg *tmp = cur;
				cur = tmp->next;
				free(tmp->name);
				free(tmp);
			}
			break;
		}
		case AST_FLAGS: {
			struct ast_flag_values *cur = root->flags.values;
			while (cur != NULL) {
				struct ast_flag_values *tmp = cur;
				cur = tmp->next;
				free(tmp->name);
				free(tmp);
			}
			break;
		}
		default:
			break;
	}

	free(root);
}
