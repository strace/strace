/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <string.h>

#include "deflang.h"
#include "symbols.h"

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

struct symbol_entry {
	char *name;
	struct ast_node *source;
	struct symbol_entry *next;
};

struct symbol_entry *symbol_table;

struct ast_node *
symbol_get(char *name)
{
	for (struct symbol_entry *cur = symbol_table;
	     cur != NULL; cur = cur->next) {
		if (strcmp(cur->name, name) == 0) {
			return cur->source;
		}
	}

	return NULL;
}

struct ast_node *
symbol_add(char *name, struct ast_node *source)
{
	struct ast_node *previous_def = symbol_get(name);
	if (previous_def != NULL) {
		return previous_def;
	}

	struct symbol_entry *entry = xmalloc(sizeof *entry);
	*entry = (struct symbol_entry) {
		.name = name,
		.source = source,
		.next = symbol_table
	};

	symbol_table = entry;

	return NULL;
}


char *
resolve_type(struct ast_type *out, char *name,
	     struct ast_type_option_list *options)
{
	out->name = name;
	out->options = options;
	out->type = TYPE_BASIC;

	struct {
		char *name;
		size_t expected_args;
	} expected_options_len[] = {
		{"const", 1},
		{"ptr", 2},
		{"ref", 1},
		{"xor_flags", 3},
		{"or_flags", 3},
	};

	size_t options_len = 0;
	for (struct ast_type_option_list *cur = options;
	     cur != NULL; cur = cur->next) {
		if (cur->option->child_type == AST_TYPE_CHILD_TEMPLATE_ID) {
			return NULL;
		}

		options_len++;
	}

	for (size_t i = 0; i < ARRAY_LEN(expected_options_len); ++i) {
		if (strcmp(name, expected_options_len[i].name) == 0) {
			if (options_len != expected_options_len[i].expected_args) {
				return xasprintf("type '%s' expects %zu "
						 "type options; got %zu",
						 name,
						 expected_options_len[i].expected_args,
						 options_len);
			}
		}
	}

	if (strcmp(name, "const") == 0) {
		out->type = TYPE_CONST;
		out->constt.value = options->option;
		out->constt.real_type = NULL;
	} else if (strcmp(name, "ptr") == 0) {
		out->type = TYPE_PTR;
		if (options->option->child_type != AST_TYPE_CHILD_TYPE) {
			return "first type option for ptr must be"
			       " 'in', 'out' or 'inout'";
		}
		if (strcmp(options->option->type->name, "in") == 0) {
			out->ptr.dir = PTR_DIR_IN;
		} else if (strcmp(options->option->type->name, "out") == 0) {
			out->ptr.dir = PTR_DIR_OUT;
		} else if (strcmp(options->option->type->name, "inout") == 0) {
			out->ptr.dir = PTR_DIR_INOUT;
		} else {
			return "first type option for ptr must be"
			       " 'in', 'out' or 'inout'";
		}
		out->ptr.type = options->next->option->type;
	} else if (strcmp(name, "ref") == 0) {
		out->type = TYPE_REF;
		if (options->option->child_type != AST_TYPE_CHILD_TYPE) {
			return "first type option for len must be"
			       " the name of another argument or $ret";
		}
		if (strcmp(options->option->type->name, "@ret") == 0) {
			out->ref.return_value = true;
		} else {
			out->ref.return_value = false;
			out->ref.argname = options->option->type->name;
		}
	} else if (strcmp(name, "xor_flags") == 0) {
		out->type = TYPE_XORFLAGS;
		out->xorflags.flag_type = options->option;
		if (options->option->child_type != AST_TYPE_CHILD_TYPE) {
			return "first type option for ptr must be a string";
		}
		out->xorflags.dflt = options->next->option->type->name;
		if (options->next->next->option->child_type !=
		    AST_TYPE_CHILD_TYPE) {
			return "third type option for xor_flags must be"
			       " the underlying flag type";
		}
		out->xorflags.underlying = options->next->next->option->type;
	} else if (strcmp(name, "or_flags") == 0) {
		out->type = TYPE_ORFLAGS;
		out->orflags.flag_type = options->option;
		if (options->option->child_type != AST_TYPE_CHILD_TYPE) {
			return "first type option for ptr must be a string";
		}
		out->orflags.dflt = options->next->option->type->name;
		if (options->next->next->option->child_type !=
		    AST_TYPE_CHILD_TYPE) {
			return "third type option for or_flags must be"
			       " the underlying flag type";
		}
		out->orflags.underlying = options->next->next->option->type;
	}

	return NULL;
}
