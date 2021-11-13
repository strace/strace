/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "deflang.h"
#include "symbols.h"

struct {
	char *name;
	char *ctype;
} basic_types[] = {
	{"uchar", "unsigned char"},
	{"ushort", "unsigned short"},
	{"uint", "unsigned int"},
	{"ulong", "unsigned long"},
	{"longlong", "long long"},
	{"ulonglong", "unsigned long long"},
	{"longdouble", "long double"},
	{"string", "char"},
	{"path", "char"},
	{"size", "kernel_size_t"},
	{"size_t", "kernel_size_t"},
	{"gid", "gid_t"}
};

char *signed_int_types[] = {
	"char",
	"short",
	"int",
	"long",
	"longlong",
	"kernel_long_t",
	"ssize_t"
};

char *unsigned_int_types[] = {
	"uchar",
	"ushort",
	"uint",
	"ulong",
	"ulonglong",
	"kernel_ulong_t",
	"size_t",
	"size"
};

static struct decoder_list *decoders = NULL;

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

/* convenience macros */

#define OUTFI(...) outf_indent(indent_level, out, __VA_ARGS__)

#define OUTF(...) outf(out, __VA_ARGS__)

#define OUTC(c) outc(out, c)

#define OUTSI(s) outs_indent(indent_level, out, s)

#define OUTS(s) outs(out, s)

static void
outf_indent(int indent_level, FILE *out, const char *fmt,
			...) __attribute__((format(printf, 3, 4)));

static void
outf(FILE *out, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
outc(FILE *out, int c)
{
	fputc(c, out);
}

static void
outs(FILE *out, const char *s)
{
	fputs(s, out);
}

static void
indent(FILE *out, int indent)
{
	for (int i = 0; i < indent; ++i) {
		outc(out, '\t');
	}
}

static void
outs_indent(int indent_level, FILE *out, const char *s)
{
	indent(out, indent_level);
	fprintf(out, "%s", s);
}

static void
outf(FILE *out, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(out, fmt, args);

	va_end(args);
}

static void
outf_indent(int indent_level, FILE *out, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	indent(out, indent_level);
	vfprintf(out, fmt, args);

	va_end(args);
}

static void
log_warning(char *fmt, struct ast_loc node, ...)
{
	va_list args;
	va_start(args, node);

	fprintf(stderr, "Codegen Warning: ");
	fprintf(stderr, "line %d, col %d: ", node.lineno, node.colno);

	vfprintf(stderr, fmt, args);

	fprintf(stderr, "\n");

	va_end(args);
}

static bool
is_signed_integer_typename(const char *name)
{
	for (size_t i = 0; i < ARRAY_LEN(signed_int_types); ++i) {
		if (strcmp(signed_int_types[i], name) == 0) {
			return true;
		}
	}

	return false;
}

static bool
is_unsigned_integer_typename(const char *name)
{
	for (size_t i = 0; i < ARRAY_LEN(unsigned_int_types); ++i) {
		if (strcmp(unsigned_int_types[i], name) == 0) {
			return true;
		}
	}

	return false;
}

/*
 * Stores a string referring to the i-th argument in the current syscall.
 */
static char *
get_syscall_arg_value(struct syscall *syscall, size_t i)
{
	if (syscall->is_ioctl) {
		if (i >= 1 && i <= 2) {
			const char *ioctl_args[3] = {"", "code", "arg"};
			return xstrdup(ioctl_args[i]);
		}

		log_warning("ioctl decoder referenced OOB argument %zu",
			    syscall->loc, i);
	}
	return xasprintf("tcp->u_arg[%zu]", i);
}

/*
 * Stores a string referring to the return value of the current syscall.
 */
static char *
get_syscall_ret_value()
{
	return xstrdup("tcp->u_rval");
}

/*
 * Converts a string containing the C equivalent of a given type.
 */
static char *
type_to_ctype(const struct ast_type *type)
{
	if (type->type == TYPE_BASIC) {
		for (size_t i = 0; i < ARRAY_LEN(basic_types); ++i) {
			if (strcmp(type->name, basic_types[i].name) == 0) {
				return basic_types[i].ctype;
			}
		}

		struct ast_node *def = symbol_get(type->name);
		if (def != NULL && def->type == AST_STRUCT) {
			return xasprintf("struct %s", type->name);
		}

		return type->name;
	}

	if (type->type == TYPE_PTR) {
		return xasprintf("%s *", type_to_ctype(type->ptr.type));
	}

	if (type->type == TYPE_XORFLAGS) {
		return type_to_ctype(type->xorflags.underlying);
	}

	if (type->type == TYPE_ORFLAGS) {
		return type_to_ctype(type->orflags.underlying);
	}

	return type->name;
}

static char *
type_variable_declaration(const struct ast_type *type, const char *var_name) {
	if (type->type == TYPE_BASIC && strcmp(type->name, "array") == 0) {
		if (type->options != NULL && type->options->next != NULL) {
			struct ast_type_option *underlying_type =
				type->options->option;
			struct ast_type_option *member_count_type =
				type->options->next->option;
			if (underlying_type->child_type == AST_TYPE_CHILD_TYPE &&
			    member_count_type->child_type == AST_TYPE_CHILD_NUMBER) {
				return xasprintf("%s %s[%s];",
						 type_to_ctype(underlying_type->type),
						 var_name, member_count_type->number.raw);
			}
		}
	}

	return xasprintf("%s %s;", type_to_ctype(type), var_name);

}
/*
 * Get flags to return from a SYS_FUNC.
 */
static char *
get_sys_func_return_flags(struct ast_type *type, bool is_ioctl)
{
	struct {
		char *type;
		char *flag;
	} flags[] = {
		{"fd", "RVAL_FD"},
		{"tid", "RVAL_TID"},
		{"sid", "RVAL_SID"},
		{"tgid", "RVAL_TGID"},
		{"pgid", "RVAL_PGID"}
	};

	char *base = "RVAL_DECODED";
	if (is_ioctl) {
		base = "RVAL_IOCTL_DECODED";
	}

	char *following = NULL;
	for (size_t i = 0; i < ARRAY_LEN(flags); ++i) {
		if (strcmp(flags[i].type, type->name) == 0) {
			following = flags[i].flag;
			break;
		}
	}

	if (following) {
		return xasprintf("%s | %s", base, following);
	} else {
		return xasprintf("%s", base);
	}
}

/*
 * Resolves a type option to a concrete value.
 *
 * For example, const[PATH_MAX] is resolved to PATH_MAX
 * and const[ref[argname]] is resolved to tcp->u_arg[2]
 * (where argname is the name of the 3rd syscall argument).
 *
 * The specified type option MUST NOT be a range or a template id.
 */
static char *
resolve_type_option_to_value(struct syscall *syscall,
			     struct ast_type_option *option)
{
	assert(option->child_type != AST_TYPE_CHILD_RANGE &&
	       option->child_type != AST_TYPE_CHILD_TEMPLATE_ID);

	if (option->child_type == AST_TYPE_CHILD_NUMBER) {
		/* return the number exactly as specified in the source file */
		return option->number.raw;
	} else if (option->child_type == AST_TYPE_CHILD_TYPE) {
		if (option->type->type == TYPE_REF) {
			/* identify which argument is being referred to */

			/* syscall return value */
			if (option->type->ref.return_value) {
				return get_syscall_ret_value();
			}

			/* find syscall argument by name */
			bool found = false;
			size_t index = 0;

			for (; index < syscall->arg_count; ++index) {
				if (strcmp(option->type->ref.argname,
					   syscall->args[index].name) == 0) {
					found = true;
					break;
				}
			}

			if (found) {
				return get_syscall_arg_value(syscall, index);
			}

			log_warning("Failed to resolve 'ref' type"
				    " with value \"%s\" to argument",
				    syscall->loc, option->type->ref.argname);
			return "#error FAILED TO RESOLVE REF TYPE TO VALUE";
		} else {
			/* assume the given value is a constant or from a #define */
			return option->type->name;
		}
	}

	assert(false);
}

/*
 * Stores the value of a given variable using set_tcb_priv_data.
 */
static void
store_single_value(FILE *out, struct ast_type *type, char *arg, int indent_level)
{
	OUTFI("{\n");
	indent_level++;

	OUTFI("%s\n", type_variable_declaration(type->ptr.type, "tmp_var"));
	OUTFI("if (!umove_or_printaddr(tcp, %s, &tmp_var)) {\n", arg);
	indent_level++;

	OUTFI("void *tmp_buffer = xmalloc(sizeof(%s));\n", "tmp_var");
	OUTFI("memcpy(tmp_buffer, tmp_var, sizeof(%s));\n", "tmp_var");
	OUTFI("set_tcb_priv_data(tcp, tmp_buffer, free);\n");

	indent_level--;
	OUTFI("}\n");

	indent_level--;
	OUTFI("}\n");
}

static void
generate_printer(FILE *out, struct syscall *syscall, const char *argname,
		 const char *arg, bool entering,
		 struct ast_type *type, int indent_level);

static void
generate_printer_ptr(FILE *out, struct syscall *syscall, const char *argname,
		     const char *arg, bool entering,
		     struct ast_type *type, int indent_level)
{
	/* copy from target memory and use decoder for resulting value */
	if ((IS_IN_PTR(type) && entering) || (IS_OUT_PTR(type) && !entering)) {
		CLEANUP_FREE char *var_name =
			xasprintf("tmpvar_%s", argname);
		OUTFI("%s\n",
		      type_variable_declaration(type->ptr.type, var_name));
		OUTFI("if (!umove_or_printaddr(tcp, %s, &%s)) {\n",
		      arg, var_name);
		indent_level++;

		OUTFI("tprint_indirect_begin();\n");
		generate_printer(out, syscall, argname, var_name, entering,
				 type->ptr.type, indent_level);
		OUTFI("tprint_indirect_end();\n");

		indent_level--;
		OUTSI("}\n");
	}
}

static void
generate_templated_printer(FILE *out, struct syscall *syscall,
			   const char *arg, struct ast_type *arg_type,
			   struct decoder templated_decoder)
{
	struct {
		char *value;
		intmax_t template_id;
	} substitutions[256];
	int subs_pos = 0;

	/* Do a DFS over the template type to find substitution markers.  */
	struct dfs_stack_entry {
		struct ast_type *template;
		struct ast_type *actual;
	};

	struct dfs_stack_entry dfs_stack[128] = {0};
	int stack_ptr = 0;

	dfs_stack[stack_ptr] = (struct dfs_stack_entry) {
		.template = templated_decoder.matching_type,
		.actual = arg_type
	};
	stack_ptr++;

	while (stack_ptr != 0) {
		stack_ptr--;
		struct dfs_stack_entry entry = dfs_stack[stack_ptr];

		if (entry.actual == NULL || entry.template == NULL) {
			continue;
		}

		if (strcmp(entry.actual->name, entry.template->name) != 0) {
			continue;
		}

		struct ast_type_option_list *template_option =
			entry.template->options;
		struct ast_type_option_list *actual_option =
			entry.actual->options;
		for (; actual_option != NULL && template_option != NULL;
		     actual_option = actual_option->next,
		     template_option = template_option->next) {
			if (template_option->option->child_type ==
			    AST_TYPE_CHILD_TEMPLATE_ID) {
				substitutions[subs_pos].value =
					resolve_type_option_to_value(syscall,
								     actual_option->option);
				substitutions[subs_pos].template_id =
					template_option->option->template.id;
				subs_pos++;
				continue;
			}

			if (actual_option->option->child_type !=
			    template_option->option->child_type) {
				break;
			}

			if (template_option->option->child_type ==
			    AST_TYPE_CHILD_TYPE) {
				dfs_stack[stack_ptr] = (struct dfs_stack_entry) {
					.template = template_option->option->type,
					.actual = actual_option->option->type
				};
				stack_ptr++;
			}
		}
	}

	/*
	 * Output the template string and replace substitution markers
	 * with real values.
	 */
	const char *template = templated_decoder.fmt_string;
	size_t template_len = strlen(template);

	intmax_t cur = 0;
	bool in_template_number = false;
	for (size_t i = 0; i < template_len; ++i) {
		if (template[i] == '$' && (template[i + 1] == '$')) {
			OUTF("(%s)", arg);
			i++;
			continue;
		}

		if (template[i] == '$' && (isdigit(template[i + 1]))) {
			cur = 0;
			in_template_number = true;
			continue;
		}

		if (!in_template_number) {
			OUTC(template[i]);
			continue;
		}

		if (isdigit(template[i])) {
			cur = cur * 10 + (template[i] - '0');
		}

		if (!isdigit(template[i]) || i == template_len - 1) {
			in_template_number = false;

			int found = -1;
			/* find matching substitution */
			for (int j = 0; j < subs_pos; ++j) {
				if (substitutions[j].template_id == cur) {
					found = j;
					break;
				}
			}

			if (found == -1) {
				log_warning("Template variable $%" PRIdMAX
					    " could not be resolved!",
					    syscall->loc, cur);
				continue;
			}

			OUTF("(%s)", substitutions[found].value);

			if (i != template_len - 1) {
				OUTC(template[i]);
			}
		}
	}
}

/*
 * Outputs a call to a function/macro to print out arg with the given type.
 */
static void
generate_printer(FILE *out, struct syscall *syscall,
		 const char *argname, const char *arg, bool entering,
		 struct ast_type *type, int indent_level)
{
	for (struct decoder_list *cur = decoders;
	     cur != NULL; cur = cur->next) {
		if (ast_type_matching(cur->decoder.matching_type, type)) {
			OUTFI("/* using decoder from %s:%d:%d */\n",
			      cur->decoder.loc.file,
			      cur->decoder.loc.lineno,
			      cur->decoder.loc.colno);
			generate_templated_printer(out, syscall, arg, type,
						   cur->decoder);
			OUTC('\n');
			return;
		}
	}

	if (type->type == TYPE_BASIC) {
		if (is_signed_integer_typename(type->name)) {
			OUTFI("PRINT_VAL_D((%s) %s);\n",
			      type_to_ctype(type), arg);
			return;
		} else if (is_unsigned_integer_typename(type->name)) {
			OUTFI("PRINT_VAL_U((%s) %s);\n",
			      type_to_ctype(type), arg);
			return;
		}

		log_warning("No known printer for basic type %s",
			    syscall->loc, type->name);
		outf_indent(indent_level, out,
			    "#error UNHANDLED BASIC TYPE: %s\n", type->name);
	} else if (type->type == TYPE_PTR) {
		generate_printer_ptr(out, syscall, argname, arg, entering, type,
				     indent_level);
	} else if (type->type == TYPE_ORFLAGS) {
		OUTFI("printflags64(%s, zero_extend_signed_to_ull(%s), \"%s\");\n",
		      type->orflags.flag_type->type->name, arg,
		      type->orflags.dflt);
	} else if (type->type == TYPE_XORFLAGS) {
		OUTFI("printxval64(%s, zero_extend_signed_to_ull(%s), \"%s\");\n",
		      type->xorflags.flag_type->type->name, arg,
		      type->xorflags.dflt);
	} else if (strcmp(type->name, "stringnoz") == 0 ||
		   strcmp(type->name, "string") == 0) {
		log_warning("Type '%s' should be wrapped in a ptr type"
			    " to indicate direction",
			    syscall->loc, type->name);
	} else if (type->type == TYPE_CONST) {
		if (!type->constt.real_type) {
			log_warning("Const type (%s) has no matching"
				    " parent syscall argument.",
				    syscall->loc, argname);
			return;
		}
		OUTFI("/* inherited parent type (%s) */\n",
		      type_to_ctype(type->constt.real_type));
		generate_printer(out, syscall, argname, arg, entering,
				 type->constt.real_type, indent_level);
	} else {
		log_warning("Type '%s' is currently unhandled",
			    syscall->loc, type->name);
		outf_indent(indent_level, out,
			    "#error UNHANDLED TYPE: %s\n", type->name);
	}
}

static void
generate_return_flags(FILE *out, struct syscall *syscall, int indent_level)
{
	struct ast_type ret = syscall->ret;
	if (ret.type == TYPE_ORFLAGS) {
		OUTFI("tcp->auxstr = sprintflags(\"%s\", %s"
		      ", (kernel_ulong_t) tcp->u_rval);\n",
		      ret.orflags.dflt, ret.orflags.flag_type->type->name);
		OUTFI("return RVAL_STR;\n");
	} else if (ret.type == TYPE_XORFLAGS) {
		OUTFI("tcp->auxstr = xlookup(%s"
		      ", (kernel_ulong_t) tcp->u_rval);\n",
		      ret.xorflags.flag_type->type->name);
		OUTFI("return RVAL_STR;\n");
	} else {
		CLEANUP_FREE char *flags =
			get_sys_func_return_flags(&ret, syscall->is_ioctl);
		OUTFI("return %s;\n", flags);
	}
}

/*
 * Transforms a variant syscall name (like fcntl$F_DUPFD) to a valid C function
 * name (like var_fcntl_F_DUPFD).
 *
 * The is_leaf parameter should be set if corresponding syscall is a leaf node,
 * i.e. has no sub syscalls.
 */
static char *
get_variant_function_name(char *variant_name, bool is_leaf)
{
	char *out = xasprintf("var_%s%s",
			      is_leaf ? "leaf_" : "", variant_name);
	for (char *p = out; *p; ++p) {
		if (*p == '$') {
			*p = '_';
		}
	}
	return out;
}

/*
 * Output the start of any preprocessor conditions.
 *
 * For example:
 * #ifdef linux
 */
void
out_statement_condition_start(FILE *out, struct statement_condition *condition)
{
	if (condition == NULL) {
		return;
	}
	for (size_t i = 0; i < condition->count; ++i) {
		OUTF("%s\n", condition->values[i]);
	}
}

/*
 * Output the end of the specified preprocessor conditions.
 *
 * For example:
 * #endif
 */
void
out_statement_condition_end(FILE *out, struct statement_condition *condition)
{
	if (condition == NULL) {
		return;
	}
	for (size_t i = 0; i < condition->count; ++i) {
		OUTS("#endif\n\n");
	}
}

static char *
get_decoder_prototype(bool internal, struct syscall *syscall, char *func_name)
{
	return xasprintf("%sint\n"
			 "%s(struct tcb *tcp%s)\n",
			 internal ? "static " : "",
			 func_name,
			 syscall->is_ioctl ?
				", unsigned int code, kernel_ulong_t arg" : "");
}

/*
 * Prints out a decoder for the given system call.
 */
static void
generate_decoder(FILE *out, struct syscall *syscall, bool is_variant,
		 bool ioctl_fallback)
{
	int indent_level = 0;

	out_statement_condition_start(out, syscall->conditions);

	int arg_offset = 0;
	int arg_index = 0;

	if (syscall->is_ioctl) {
		/* no need to decode code, or arg for ioctl variant syscalls */
		arg_offset = 2;
		arg_index = 2;
	}

	/*
	 * Determine which strategy to use depending on
	 * how many OUT ptrs there are.
	 */
	size_t out_ptrs = 0;
	for (size_t i = arg_offset; i < syscall->arg_count; i++) {
		if (IS_OUT_PTR(syscall->args[i].type)) {
			out_ptrs++;
		}
	}

	/* output function declaration */
	if (is_variant) {
		CLEANUP_FREE char *func_name =
			get_variant_function_name(syscall->name, true);
		CLEANUP_FREE char *decoder_prototype =
			get_decoder_prototype(true, syscall, func_name);
		OUTSI(decoder_prototype);
	} else {
		OUTFI("SYS_FUNC(%s)\n", syscall->name);
	}
	OUTSI("{\n");
	indent_level++;

	if (syscall->is_ioctl && ioctl_fallback) {
		OUTSI("return RVAL_DECODED;\n");
		indent_level--;
		OUTSI("}\n");
		return;
	}

	if (out_ptrs == 0) {
		if (syscall->is_ioctl) {
			OUTFI("tprint_arg_next();\n");
		}

		/* 0 out ptrs: print all args in sysenter */
		for (size_t i = arg_offset; i < syscall->arg_count; i++) {
			struct syscall_argument arg = syscall->args[i];
			OUTFI("/* arg: %s (%s) */\n",
			      arg.name, type_to_ctype(arg.type));
			CLEANUP_FREE char *arg_val =
				get_syscall_arg_value(syscall, arg_index++);
			generate_printer(out, syscall, arg.name, arg_val, true,
					 arg.type, indent_level);

			if (i < syscall->arg_count - 1) {
				OUTSI("tprint_arg_next();\n");
			}
			OUTC('\n');
		}
	} else if (out_ptrs == 1) {
		/*
		 * == 1 out ptrs: print args until the out ptr in sysenter,
		 * rest in sysexit
		 */
		size_t cur = arg_offset;

		OUTSI("if (entering(tcp)) {\n");
		indent_level++;

		if (syscall->is_ioctl) {
			OUTFI("tprint_arg_next();\n");
		}

		for (; cur < syscall->arg_count; ++cur) {
			struct syscall_argument arg = syscall->args[cur];
			if (IS_OUT_PTR(arg.type)) {
				break;
			}

			OUTFI("/* arg: %s (%s) */\n",
			      arg.name, type_to_ctype(arg.type));
			CLEANUP_FREE char *arg_val =
				get_syscall_arg_value(syscall, arg_index++);
			generate_printer(out, syscall, arg.name, arg_val, true,
					 arg.type, indent_level);

			if (cur < syscall->arg_count - 1) {
				OUTSI("tprint_arg_next();\n\n");
			}
		}

		if (cur < syscall->arg_count &&
		    IS_INOUT_PTR(syscall->args[cur].type)) {
			CLEANUP_FREE char *arg_val =
				get_syscall_arg_value(syscall, cur);
			store_single_value(out, syscall->args[cur].type,
					   arg_val, indent_level);
		}

		OUTSI("return 0;\n");
		indent_level--;
		OUTSI("}\n");

		if (cur < syscall->arg_count &&
		    IS_INOUT_PTR(syscall->args[cur].type)) {
			/*
			 * TODO:
			 * Compare the current value with the previous value
			 * and print only if changed.
			 */
		}

		for (; cur < syscall->arg_count; ++cur) {
			struct syscall_argument arg = syscall->args[cur];
			OUTFI("/* arg: %s (%s) */\n",
			      arg.name, type_to_ctype(arg.type));
			CLEANUP_FREE char *arg_val =
				get_syscall_arg_value(syscall, arg_index++);

			if (IS_INOUT_PTR(syscall->args[cur].type)) {
				generate_printer(out, syscall, arg.name,
						 "get_tcb_priv_data(tcp)",
						 false, arg.type, indent_level);
			}

			generate_printer(out, syscall, arg.name, arg_val,
					 false, arg.type, indent_level);

			if (cur < syscall->arg_count - 1) {
				OUTSI("tprint_arg_next();\n");
			}
			OUTC('\n');
		}
	} else {
		/*
		 * TODO
		 * > 1 out ptrs;
		 * store necessary ptr values using set_tcb_priv_data
		 */
		OUTSI("#error TODO\n");
	}

	generate_return_flags(out, syscall, indent_level);

	indent_level--;
	OUTSI("}\n");

	out_statement_condition_end(out, syscall->conditions);
}

/*
 * Write out the specified #define statements.
 */
void
output_defines(FILE *out, struct preprocessor_statement_list *defines)
{
	struct preprocessor_statement_list *cur = defines;
	while (cur != NULL) {
		out_statement_condition_start(out, cur->stmt.conditions);
		OUTF("#%s\n", cur->stmt.value);
		out_statement_condition_end(out, cur->stmt.conditions);
		cur = cur->next;
	}
}

/*
 * Outputs a function which delegates to the child syscalls based on the
 * values of the child's const-typed arguments.
 *
 * The is_variant flag indicates whether the group's base syscall is a child
 * of a variant syscall itself.
 */
void
output_variant_syscall_group(FILE *out, struct syscall_group *group,
			     bool is_variant)
{
	int indent_level = 0;
	if (is_variant) {
		/* variant system call */
		CLEANUP_FREE char *func_name =
			get_variant_function_name(group->base->name, false);
		CLEANUP_FREE char *decoder_prototype =
			get_decoder_prototype(false, group->base, func_name);
		OUTSI(decoder_prototype);
	} else {
		/* base system call */
		OUTFI("SYS_FUNC(%s) {\n", group->base->name);
	}
	OUTSI("{\n");
	indent_level++;

	OUTSI("");
	for (size_t child = 0; child < group->child_count; child++) {
		struct syscall_group *cur_child_grp = &group->children[child];
		struct syscall *cur_child = cur_child_grp->base;

		out_statement_condition_start(out, cur_child->conditions);

		OUTS("if (");

		bool first = true;
		for (size_t arg_idx = 0;
		     arg_idx < cur_child->arg_count; ++arg_idx) {
			struct syscall_argument arg = cur_child->args[arg_idx];

			if (arg.type->type != TYPE_CONST) {
				continue;
			}

			if (first) {
				first = false;
			} else {
				OUTS(" && ");
			}

			CLEANUP_FREE char *arg_str =
				get_syscall_arg_value(cur_child, arg_idx);

			if (arg.type->constt.value->child_type ==
			    AST_TYPE_CHILD_RANGE) {
				OUTF("((%s) <= (%s) && (%s) <= (%s))",
				     arg_str,
				     resolve_type_option_to_value(cur_child,
					arg.type->constt.value->range.min),
				     arg_str,
				     resolve_type_option_to_value(cur_child,
					arg.type->constt.value->range.max)
				);
			} else {
				OUTF("(%s) == (%s)",
				     arg_str,
				     resolve_type_option_to_value(cur_child,
					arg.type->constt.value));
			}
		}
		OUTS(") {\n");

		indent_level++;

		CLEANUP_FREE char *func_name =
			get_variant_function_name(cur_child->name,
						  cur_child_grp->child_count == 0);
		OUTFI("return %s(tcp%s);\n", func_name,
		      cur_child->is_ioctl ? ", code, arg" : "");

		indent_level--;
		OUTSI("} else ");
	}

	OUTS("{\n");
	indent_level++;

	CLEANUP_FREE char *func_name =
		get_variant_function_name(group->base->name, true);
	OUTFI("return %s(tcp%s);\n", func_name,
	      group->base->is_ioctl ? ", code, arg" : "");

	indent_level--;
	OUTSI("}\n");

	indent_level--;
	OUTSI("}\n");
}

/*
 * Outputs a syscall group and syscall variants.
 */
void
output_syscall_groups(FILE *out, struct syscall_group *groups,
		      size_t group_count, struct syscall_group *parent)
{
	for (size_t i = 0; i < group_count; ++i) {
		struct syscall_group *cur = &groups[i];

		if (parent) {
			/*
			 * Store the real type of const parameters
			 * based on their parent.
			 */
			for (size_t j = 0;
			     j < cur->base->arg_count && j < parent->base->arg_count;
			     ++j) {
				struct syscall_argument *cur_arg =
					&cur->base->args[j];
				struct syscall_argument *parent_arg =
					&parent->base->args[j];
				if (cur_arg->type->type == TYPE_CONST) {
					if (parent_arg->type->type == TYPE_CONST) {
						cur_arg->type->constt.real_type =
							parent_arg->type->constt.real_type;
					} else {
						cur_arg->type->constt.real_type =
							parent_arg->type;
					}
				}
			}
		}

		if (groups[i].child_count == 0) {
			generate_decoder(out, groups[i].base, parent != NULL,
					 false);
			continue;
		}

		output_syscall_groups(out, groups[i].children,
				      groups[i].child_count, &groups[i]);

		if (strcmp(groups[i].base->name, "ioctl") != 0) {
			generate_decoder(out, groups[i].base, true, true);

			output_variant_syscall_group(out, &groups[i],
						     parent != NULL);
		}
	}
}

bool
generate_code(const char *in_filename, const char *out_filename,
	      struct processed_ast *ast)
{
	FILE *out = fopen(out_filename, "w");

	if (out == NULL) {
		return false;
	}

	outf(out, "/* Generated by ./maint/gen/generate.sh"
		  " from ./maint/gen/%s; do not edit. */\n\n", in_filename);
	outf(out, "%s",
		 "#include <stddef.h>\n"
		 "#include \"generated.h\"\n\n"
		 "typedef kernel_ulong_t kernel_size_t;\n\n"
	);

	decoders = ast->decoders;

	output_defines(out, ast->preprocessor_stmts);
	output_syscall_groups(out, ast->syscall_groups,
			      ast->syscall_group_count, NULL);

	fclose(out);

	return true;
}
