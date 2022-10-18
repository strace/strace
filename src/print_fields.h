/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PRINT_FIELDS_H
# define STRACE_PRINT_FIELDS_H

#include <stdarg.h>

# include "static_assert.h"







struct structured_output {
	const char *structured_output_NULL_VALUE   ;
	const char *structured_output_NEWLINE	   ;
	const char *structured_output_STRING_BEGIN   ;
	const char *structured_output_STRING_END     ;
	const char *structured_output_PARTIAL_STRING_BEGIN  ;
	const char *structured_output_PARTIAL_STRING_END     ;
	const char *structured_output_ARRAY_BEGIN    ;
	const char *structured_output_ARRAY_NEXT       ;
	const char *structured_output_ARRAY_END	     ;
	const char *structured_output_FIELD_SET	     ;
	const char *structured_output_STRUCT_BEGIN   ;
	const char *structured_output_STRUCT_NEXT    ;
	const char *structured_output_STRUCT_END     ;
	const char *structured_output_INT_BEGIN	     ;
	const char *structured_output_INT_END	     ;
	const char *structured_output_FLAGS_BEGIN    ;
	const char *structured_output_FLAGS_NEXT     ;
	const char *structured_output_FLAGS_END	     ;
	const char *structured_output_CALL_BEGIN     ;
	const char *structured_output_CALL_ARG_NEXT  ;
	const char *structured_output_CALL_END	     ;
	const char *structured_output_COMMENT_BEGIN  ;
	const char *structured_output_COMMENT_END    ;
	const char *structured_output_ARG_NAME_BEGIN ;
	const char *structured_output_ARG_NAME_END   ;
	const char *structured_output_INDIRECT_BEGIN ;
	const char *structured_output_INDIRECT_END   ;
	const char *structured_output_SHIFT_BEGIN    ;
	const char *structured_output_SHIFT          ;
	const char *structured_output_SHIFT_END      ;
	const char *structured_output_ARRAY_INDEX_BEGIN ;
	const char *structured_output_ARRAY_INDEX_EQUAL ;
	const char *structured_output_ARRAY_INDEX_END   ;
	int structured_output_STRUCT_NEEDS_ENDING_NEXT ;
	int structured_output_ACCEPTS_COMMENTS ;
	int structured_output_ESCAPES_WITH_U_XXXX ;
};



# ifdef IN_STRACE

#  define STRACE_PRINTS(state_, s_) tprints_nodelim(state_, s_)

#  define STRACE_PRINTV(state_, fmt_, args_) tvprintf(state_, fmt_ , args_)

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
#  define STRACE_PRINTF tprintf_nodelim

#  define STRUCTURED_OUTPUT(x) structured_output->structured_output_##x
#  define STRUCTURED_OUTPUTF(x) structured_output->structured_output_##x
#  define STRUCTURED_OUTPUTI(x) structured_output->structured_output_##x

# else /* !IN_STRACE */

#  include <stdio.h>

#  define STRACE_PRINTS(state_, s_) fputs((s_), stdout)

#  define STRACE_PRINTV(state_, fmt_, args_) vprintf(fmt_ , args_)

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
#  define STRACE_PRINTF(state_, ...) printf (__VA_ARGS__)

#  define STRUCTURED_OUTPUT(x_) "."
#  define STRUCTURED_OUTPUTF(fmt_) "%s"
#  define STRUCTURED_OUTPUTI(fmt_) 0

#  define structured_output 0
#  define get_tcp_state()  0
#  define set_tcp_state(state_)

# endif /* !IN_STRACE */

enum tcp_state {
/* The state provided if we are printing something only for 'raw'
 * formatting, that should not appear in structured format. */
	TCP_STATE_DUMMY,

/* Last thing we printed was a delimiter */
	TCP_STATE_DELIM,

/* Last thing we printed was a value */
	TCP_STATE_VALUE,

/* Last thing we printed was a field assignment, without a
 * separator */
	TCP_STATE_FIELD,

/* We must handle comments */
	TCP_STATE_COMMENTED,
	TCP_STATE_COMMENT_BEGIN,
	TCP_STATE_COMMENT_END,
	TCP_STATE_COMMENTED_OUT = 8
};


/* This function does not print anything in structured output */
static inline void
tprints_dummy(const char* s)
{
	if(structured_output){
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, s);
	}
}

static inline void
tprintf_dummy(const char* fmt, ...)
{
	if(structured_output){

	} else {
		va_list args;
		va_start(args, fmt);
		STRACE_PRINTV(TCP_STATE_DUMMY, fmt, args);
		va_end(args);
	}
}

static inline void
tprint_space(void)
{
	tprints_dummy(" ");
}

static inline void
tprint_null(void)
{
	if(structured_output){
		STRACE_PRINTS(TCP_STATE_VALUE,
			      STRUCTURED_OUTPUT(NULL_VALUE));
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, "NULL");
	}
}

static inline void
tprint_shift_int(const int n)
{
	STRACE_PRINTF(TCP_STATE_DUMMY, "<<%d",n);
}

static inline void
tprint_newline(void)
{
	if(structured_output){
		STRACE_PRINTS(TCP_STATE_DELIM,
			      STRUCTURED_OUTPUT(NEWLINE));
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, "\n");
	}
}

// PRINT A QUOTED STRING VALUE

static inline void
tprint_quoted_string_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(STRING_BEGIN));
	}
}

static inline void
tprint_quoted_string_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(STRING_END));
	}
}

/* string is already quoted, no need to add quotes */
static inline void
tprints_quoted_string(const char *str)
{
	tprint_quoted_string_begin();
	STRACE_PRINTS( TCP_STATE_VALUE, str);
	tprint_quoted_string_end();
}

/* string is already quoted, no need to add quotes */
static inline void
tprintv_quoted_string(const char *fmt, va_list args)
{
	tprint_quoted_string_begin();
	STRACE_PRINTV(TCP_STATE_VALUE, fmt, args);
	tprint_quoted_string_end();
}

/* string is already quoted, no need to add quotes */
static inline void
tprintf_quoted_string(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprintv_quoted_string(fmt, args);
	va_end(args);
}

// PRINT A STRING VALUE

static inline void
tprint_string_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(STRING_BEGIN));
		STRACE_PRINTS( TCP_STATE_DUMMY, "\"");
	}
}

static inline void
tprint_string_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY, "\"");
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(STRING_END));
	}
}

/* string is not quoted, we need to add quotes */
static inline void
tprints_string(const char *str)
{
	tprint_string_begin();
	STRACE_PRINTS( TCP_STATE_VALUE, str);
	tprint_string_end();
}

/* string is not quoted, we need to add quotes */
static inline void
tprintv_string(const char *fmt, va_list args)
{
	tprint_string_begin();
	STRACE_PRINTV( TCP_STATE_VALUE, fmt, args);
	tprint_string_end();
}

/* string is not quoted, we need to add quotes */
static inline void
tprintf_string(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprintv_string( fmt, args);
	va_end(args);
}

// PRINT A QUOTED PARTIAL STRING VALUE

static inline void
tprint_quoted_partial_string_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(PARTIAL_STRING_BEGIN));
	}
}

static inline void
tprint_quoted_partial_string_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(PARTIAL_STRING_END));
	}
}

/* partial_string is already quoted, no need to add quotes */
static inline void
tprints_quoted_partial_string(const char *str)
{
	tprint_quoted_partial_string_begin();
	STRACE_PRINTS( TCP_STATE_VALUE, str);
	tprint_quoted_partial_string_end();
}

/* string is already quoted, no need to add quotes */
static inline void
tprintv_quoted_partial_string(const char *fmt, va_list args)
{
	tprint_quoted_partial_string_begin();
	STRACE_PRINTV( TCP_STATE_VALUE, fmt, args);
	tprint_quoted_partial_string_end();
}

/* string is already quoted, no need to add quotes */
static inline void
tprintf_quoted_partial_string(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprintv_quoted_partial_string(fmt, args);
	va_end(args);
}

// PRINT A PARTIAL STRING VALUE

static inline void
tprint_partial_string_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(PARTIAL_STRING_BEGIN));
		STRACE_PRINTS( TCP_STATE_DUMMY, "\"");
	}
}

static inline void
tprint_partial_string_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY, "\"");
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(PARTIAL_STRING_END));
	}
}

/* partial_string is not quoted, we need to add quotes */
static inline void
tprints_partial_string(const char *str)
{
	tprint_partial_string_begin();
	STRACE_PRINTS( TCP_STATE_VALUE, str);
	tprint_partial_string_end();
}

/* string is not quoted, we need to add quotes */
static inline void
tprintv_partial_string(const char *fmt, va_list args)
{
	tprint_partial_string_begin();
	STRACE_PRINTV( TCP_STATE_VALUE, fmt, args);
	tprint_partial_string_end();
}

/* string is not quoted, we need to add quotes */
static inline void
tprintf_partial_string(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprintv_partial_string( fmt, args);
	va_end(args);
}


// PRINT AN INT VALUE

static inline void
tprint_int_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(INT_BEGIN));
	} else {
	}
}

static inline void
tprint_int_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(INT_END));
	} else {
	}
}

static inline void
tprints_int(const char *str)
{
	tprint_int_begin();
	STRACE_PRINTS( TCP_STATE_VALUE, str);
	tprint_int_end();
}

static inline void
tprintv_int(const char *fmt, va_list args)
{
	tprint_int_begin();
	STRACE_PRINTV( TCP_STATE_VALUE, fmt, args);
	tprint_int_end();
}

static inline void
tprintf_int(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprintv_int( fmt, args);
	va_end(args);
}

static inline void
tprint_flags_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(FLAGS_BEGIN));
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, "");
	}
}

static inline void
tprint_flags_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(FLAGS_END));
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, "");
	}
}



// PRINT FIELDS

static inline void
tprint_field_end(void)
{
	if(structured_output){
		set_tcp_state( TCP_STATE_FIELD);
	}
}

static inline void
tprint_struct_next(void)
{
	if(structured_output){
		tprint_field_end();
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, ", ");
	}
}

static inline void
tprints_field_set(const char* field)
{
	if(structured_output){
		if(get_tcp_state() == TCP_STATE_FIELD)
			STRACE_PRINTS( TCP_STATE_DELIM,
				       STRUCTURED_OUTPUT(STRUCT_NEXT));

		STRACE_PRINTF( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUTF(FIELD_SET), field);
	}
}



static inline void
tprintf_field_int(const char* field, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprints_field_set(field);
	tprintv_int( fmt, args);
	tprint_field_end();
	va_end(args);
}

static inline void
tprints_field_int(const char* field, const char* s)
{
	tprints_field_set(field);
	tprints_int(s);
	tprint_field_end();
}

static inline void
tprintf_field_string(const char* field, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprints_field_set(field);
	tprintv_string( fmt, args);
	tprint_field_end();
	va_end(args);
}

static inline void
tprints_field_string(const char* field, const char* s)
{
	tprints_field_set(field);
	tprints_string(s);
	tprint_field_end();
}

static inline void
tprintf_field_quoted_string(const char* field, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tprints_field_set(field);
	tprintv_quoted_string( fmt, args);
	tprint_field_end();
	va_end(args);
}

static inline void
tprints_field_quoted_string(const char* field, const char* s)
{
	tprints_field_set(field);
	tprints_quoted_string(s);
	tprint_field_end();
}

static inline void
tprints_field_int_begin(const char* field)
{
	tprints_field_set(field);
	tprint_int_begin();
}

static inline void
tprint_field_int_end(void)
{
	tprint_int_end();
	tprint_field_end();
}

static inline void
tprints_field_string_begin(const char* field)
{
	tprints_field_set(field);
	tprint_string_begin();
}

static inline void
tprint_field_string_end(void)
{
	tprint_string_end();
	tprint_field_end();
}


static inline void
tprint_struct_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(STRUCT_BEGIN));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "{");
	}
}

static inline void
tprint_struct_end(void)
{
	if(structured_output){
		if(get_tcp_state() == TCP_STATE_FIELD &&
		    STRUCTURED_OUTPUTI(STRUCT_NEEDS_ENDING_NEXT))
			STRACE_PRINTS( TCP_STATE_DELIM,
				       STRUCTURED_OUTPUT(STRUCT_NEXT));

		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(STRUCT_END));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "}");
	}
}

static inline void
tprint_union_begin(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, "{");
}

static inline void
tprint_union_next(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, ", ");
}

static inline void
tprint_union_end(void)
{
	STRACE_PRINTS( TCP_STATE_VALUE, "}");
}

static inline void
tprint_array_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(ARRAY_BEGIN));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "[");
	}
}

static inline void
tprint_array_next(void)
{
	if(structured_output){
		if(get_tcp_state() != TCP_STATE_DELIM)
			STRACE_PRINTS( TCP_STATE_DELIM,
				       STRUCTURED_OUTPUT(ARRAY_NEXT));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, ", ");
	}
}

static inline void
tprint_array_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(ARRAY_END));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "]");
	}
}

static inline void
tprint_array_index_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(ARRAY_INDEX_BEGIN));
	} else {
		STRACE_PRINTS( TCP_STATE_DELIM, "[");
	}
}

static inline void
tprint_array_index_equal(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(ARRAY_INDEX_EQUAL));
		STRACE_PRINTS( TCP_STATE_DELIM, "");
	} else {
		STRACE_PRINTS( TCP_STATE_DELIM, "]=");
	}
}

static inline void
tprint_array_index_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(ARRAY_INDEX_END));
	}
}

static inline void
tprint_arg_next(void)
{
	if(structured_output){
		if(get_tcp_state() != TCP_STATE_DELIM)
			STRACE_PRINTS( TCP_STATE_DELIM,
				       STRUCTURED_OUTPUT(ARRAY_NEXT));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, ", ");
	}
}

static inline void
tprint_arg_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(CALL_END));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, ")");
	}
}

static inline void
tprint_bitset_begin(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, "[");
}

static inline void
tprint_bitset_next(void)
{
	STRACE_PRINTS(TCP_STATE_DELIM, " ");
}

static inline void
tprint_bitset_end(void)
{
	STRACE_PRINTS(TCP_STATE_VALUE, "]");
}

static inline void
tprint_comment_begin(void)
{
	if(structured_output){
		set_tcp_state( TCP_STATE_COMMENT_BEGIN);
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(COMMENT_BEGIN));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, " /* ");
	}
}

static inline void
tprint_comment_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY,
			       STRUCTURED_OUTPUT(COMMENT_END));
		set_tcp_state( TCP_STATE_COMMENT_END);
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, " */");
	}
}

static inline void
tprint_indirect_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(INDIRECT_BEGIN));
	} else {
		STRACE_PRINTS(TCP_STATE_DUMMY, "[");
	}
}

static inline void
tprint_indirect_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(INDIRECT_END));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "]");
	}
}

static inline void
tprint_attribute_begin(void)
{
	tprints_dummy("[");
}

static inline void
tprint_attribute_end(void)
{
	tprints_dummy("]");
}

static inline void
tprint_associated_info_begin(void)
{
	tprints_dummy("<");
}

static inline void
tprint_associated_info_end(void)
{
	tprints_dummy(">");
}

static inline void
tprint_more_data_follows(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DUMMY, "");
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "...");
	}
}

static inline void
tprint_value_changed(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, " => ");
}

static inline void
tprint_alternative_value(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, " or ");
}

static inline void
tprint_unavailable(void)
{
	STRACE_PRINTS(TCP_STATE_VALUE, "???");
}

static inline void
tprint_shift_begin(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT (SHIFT_BEGIN));
	}
}

static inline void
tprint_shift(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT (SHIFT));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "<<");
	}
}

static inline void
tprint_shift_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT (SHIFT_END));
	}
}

static inline void
tprint_flags_or(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(FLAGS_NEXT));
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, "|");
	}
}

static inline void
tprint_plus(void)
{
	STRACE_PRINTS( TCP_STATE_DELIM, "+");
}

static inline void
tprints_field_name(const char *name)
{
	if(structured_output){
		tprints_field_set(name);
	} else {
		STRACE_PRINTF( TCP_STATE_DUMMY, "%s=", name);
	}
}

static inline void
tprints_arg_name_begin(const char *name)
{
	if(structured_output){
		STRACE_PRINTF( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUTF(ARG_NAME_BEGIN), name);
	} else {
		STRACE_PRINTF( TCP_STATE_DUMMY, "%s=", name);
	}
}

static inline void
tprint_arg_name_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(ARG_NAME_END));
	} else {
	}
}

static inline void
tprints_arg_begin(const char *name)
{
	if(structured_output){
		STRACE_PRINTF( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUTF(CALL_BEGIN) , name);

	} else {
		STRACE_PRINTF( TCP_STATE_DUMMY, "%s(", name);
	}
}

static inline void
tprints_argspace_begin(const char *name)
{
	if(structured_output){
		tprints_field_string("cmd", name);
		tprints_field_set("args");
		tprint_array_begin();
	} else {
		STRACE_PRINTF( TCP_STATE_DUMMY, "%s(", name);
	}
}

static inline void
tprint_argspace_end(void)
{
	if(structured_output){
		STRACE_PRINTS( TCP_STATE_VALUE,
			       STRUCTURED_OUTPUT(ARRAY_END));
		tprint_field_end();
	} else {
		STRACE_PRINTS( TCP_STATE_DUMMY, ")");
	}
}

static inline void
tprint_unfinished(int at_end)
{
	if(structured_output){
		if ( at_end){
			/* we need to close argspace here */
			STRACE_PRINTS( TCP_STATE_VALUE,
				       STRUCTURED_OUTPUT(ARRAY_END));
			tprint_field_end();
			tprints_field_string("return", "unfinished");
		} else {
			tprints_arg_name_begin("return");
			tprints_string("unfinished");
			tprint_arg_name_end();
		}
	} else {
		tprints_field_string("return", "unfinished");
	}
}

static inline void
tprint_syscall_begin(void)
{
	if(structured_output){
		tprint_struct_begin();
	} else {

	}
}


static inline void
tprint_syscall_end(void)
{
	if(structured_output){
		tprint_struct_end();
		STRACE_PRINTS( TCP_STATE_DELIM,
			       STRUCTURED_OUTPUT(STRUCT_NEXT));
	} else {

	}
}

# define PRINT_VAL_D(val_)					\
	tprintf_int("%lld", sign_extend_unsigned_to_ll(val_))

# define PRINT_VAL_D_FIELD(field_, val_)				\
	do {								\
		tprints_field_int_begin(field_);			\
		STRACE_PRINTF(TCP_STATE_VALUE, "%lld", sign_extend_unsigned_to_ll(val_)); \
		tprint_field_int_end();					\
	} while(0)

# define PRINT_VAL_U(val_)					\
	tprintf_int("%llu", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_X(val_)					\
	tprintf_int("%#llx", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_03O(val_)					\
	tprintf_int("%#03llo", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_0X(val_)				\
	tprintf_int("%#0*llx", (int) sizeof(val_) * 2,	\
		    zero_extend_signed_to_ull(val_))

# define PRINT_VAL_ID(val_)					\
	do {							\
		if (sign_extend_unsigned_to_ll(val_) == -1LL)	\
			PRINT_VAL_D(-1);			\
		else						\
			PRINT_VAL_U(val_);			\
	} while (0)

# define PRINT_FIELD_D(where_, field_)		\
	do {					\
		tprints_field_name(#field_);	\
		PRINT_VAL_D((where_).field_);	\
	} while (0)

# define PRINT_FIELD_U(where_, field_)		\
	do {					\
		tprints_field_name(#field_);	\
		PRINT_VAL_U((where_).field_);	\
	} while (0)

# define PRINT_FIELD_U_CAST(where_, field_, type_)	\
	do {						\
		tprints_field_name(#field_);		\
		PRINT_VAL_U((type_)((where_).field_));	\
	} while (0)

# define PRINT_FIELD_X(where_, field_)		\
	do {					\
		tprints_field_name(#field_);	\
		PRINT_VAL_X((where_).field_);	\
	} while (0)

# define PRINT_FIELD_X_CAST(where_, field_, type_)	\
	do {						\
		tprints_field_name(#field_);		\
		PRINT_VAL_X((type_)((where_).field_));	\
	} while (0)

# define PRINT_FIELD_ADDR64(where_, field_)	\
	do {					\
		tprints_field_name(#field_);	\
		printaddr64((where_).field_);	\
	} while (0)

# define PRINT_FIELD_0X(where_, field_)		\
	do {					\
		tprints_field_name(#field_);	\
		PRINT_VAL_0X((where_).field_);	\
	} while (0)

# define PRINT_FIELD_VAL_ARRAY(where_, field_, print_val_)	\
	do {							\
		tprints_field_name(#field_);			\
		for (size_t i_ = 0;				\
		     i_ < ARRAY_SIZE((where_).field_);		\
		     ++i_) {					\
			if (i_)					\
				tprint_array_next();		\
			else					\
				tprint_array_begin();		\
			print_val_((where_).field_[i_]);	\
		}						\
		tprint_array_end();				\
	} while (0)

# define PRINT_FIELD_D_ARRAY(where_, field_)			\
	PRINT_FIELD_VAL_ARRAY((where_), field_, PRINT_VAL_D)

# define PRINT_FIELD_U_ARRAY(where_, field_)			\
	PRINT_FIELD_VAL_ARRAY((where_), field_, PRINT_VAL_U)

# define PRINT_FIELD_X_ARRAY(where_, field_)			\
	PRINT_FIELD_VAL_ARRAY((where_), field_, PRINT_VAL_X)

# define PRINT_FIELD_VAL_ARRAY2D(where_, field_, print_val_)		\
	do {								\
		tprints_field_name(#field_);				\
		for (size_t i_ = 0;					\
		     i_ < ARRAY_SIZE((where_).field_);			\
		     ++i_) {						\
			if (i_)						\
				tprint_array_next();			\
			else						\
				tprint_array_begin();			\
			for (size_t j_ = 0;				\
			     j_ < ARRAY_SIZE((where_).field_[i_]);	\
			     ++j_) {					\
				if (j_)					\
					tprint_array_next();		\
				else					\
					tprint_array_begin();		\
				print_val_((where_).field_[i_][j_]);	\
			}						\
			tprint_array_end();				\
		}							\
		tprint_array_end();					\
	} while (0)

# define PRINT_FIELD_X_ARRAY2D(where_, field_)			\
	PRINT_FIELD_VAL_ARRAY2D((where_), field_, PRINT_VAL_X)

# define PRINT_FIELD_COOKIE(where_, field_)			\
	do {							\
		static_assert(ARRAY_SIZE((where_).field_) == 2,	\
			      "unexpected array size");		\
		PRINT_FIELD_U_ARRAY((where_), field_);		\
	} while (0)

# define PRINT_FIELD_FLAGS(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printflags64((xlat_),					\
			     zero_extend_signed_to_ull((where_).field_), \
			     (dflt_));					\
	} while (0)

# define PRINT_FIELD_FLAGS_VERBOSE(where_, field_, xlat_, dflt_)	\
	do {								\
		tprints_field_name(#field_);				\
		printflags_ex(zero_extend_signed_to_ull((where_).field_), \
			      (dflt_),					\
			      xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW \
			      ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,	\
			      (xlat_), NULL);				\
	} while (0)

# define PRINT_FIELD_XVAL(where_, field_, xlat_, dflt_)			\
	do {								\
		tprints_field_name(#field_);				\
		printxval64((xlat_),					\
			    zero_extend_signed_to_ull((where_).field_),	\
			    (dflt_));					\
	} while (0)

# define PRINT_FIELD_XVAL_VERBOSE(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printxval_ex((xlat_),					\
			     zero_extend_signed_to_ull((where_).field_), \
			     (dflt_),					\
			     xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW \
			     ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE);	\
	} while (0)

# define PRINT_FIELD_XVAL_D(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printxval64_d((xlat_),					\
			      sign_extend_unsigned_to_ll((where_).field_), \
			      (dflt_));					\
	} while (0)

# define PRINT_FIELD_XVAL_U(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printxval64_u((xlat_),					\
			      zero_extend_signed_to_ull((where_).field_), \
			      (dflt_));					\
	} while (0)

# define PRINT_FIELD_XVAL_U_VERBOSE(where_, field_, xlat_, dflt_)	\
	do {								\
		tprints_field_name(#field_);				\
		printxval_ex((xlat_),					\
			     zero_extend_signed_to_ull((where_).field_), \
			     (dflt_),					\
			     (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW \
			      ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE)	\
			     | XLAT_STYLE_FMT_U);			\
	} while (0)

# define PRINT_FIELD_ERR_D(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_err(sign_extend_unsigned_to_ll((where_).field_),	\
			  true);					\
	} while (0)

# define PRINT_FIELD_ERR_U(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_err(zero_extend_signed_to_ull((where_).field_),	\
			  false);					\
	} while (0)

/*
 * Generic "ID" printing. ID is considered unsigned except for the special value
 * of -1.
 */
# define PRINT_FIELD_ID(where_, field_)		\
	do {					\
		tprints_field_name(#field_);	\
		PRINT_VAL_ID((where_).field_);	\
	} while (0)

# define PRINT_FIELD_UUID(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_uuid((const unsigned char *) ((where_).field_));	\
	} while (0)

# define PRINT_FIELD_U64(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		if (zero_extend_signed_to_ull((where_).field_) == UINT64_MAX) \
			print_xlat_u(UINT64_MAX);			\
		else							\
			PRINT_VAL_U((where_).field_);			\
	} while (0)

# define PRINT_FIELD_CLOCK_T(where_, field_)	\
	do {					\
		tprints_field_name(#field_);	\
		print_clock_t((where_).field_);	\
	} while (0)

# define PRINT_FIELD_STRING(where_, field_, len_, style_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_string((const char *)(where_).field_,	\
				    (len_), (style_));			\
	} while (0)

# define PRINT_FIELD_CSTRING(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_cstring((const char *) (where_).field_,	\
				     sizeof((where_).field_) +		\
				     MUST_BE_ARRAY((where_).field_));	\
	} while (0)

# define PRINT_FIELD_CSTRING_SZ(where_, field_, size_)			\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_cstring((const char *) (where_).field_,	\
				     (size_));				\
	} while (0)

# define PRINT_FIELD_ARRAY(where_, field_, tcp_, print_func_)	\
	do {							\
		tprints_field_name(#field_);			\
		print_local_array((tcp_), (where_).field_,	\
				  (print_func_));		\
	} while (0)

# define PRINT_FIELD_ARRAY_INDEXED(where_, field_, tcp_, print_func_,	\
				   ind_xlat_, ind_dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_local_array_ex((tcp_), (where_).field_,		\
				     ARRAY_SIZE((where_).field_),	\
				     sizeof(((where_).field_)[0]),	\
				     (print_func_),			\
				     NULL, PAF_PRINT_INDICES | XLAT_STYLE_FMT_U, \
				     (ind_xlat_), (ind_dflt_));		\
	} while (0)

# define PRINT_FIELD_ARRAY_UPTO(where_, field_,			\
				upto_, tcp_, print_func_)	\
	do {							\
		tprints_field_name(#field_);			\
		print_local_array_upto((tcp_), (where_).field_,	\
				       (upto_), (print_func_));	\
	} while (0)

# define PRINT_FIELD_HEX_ARRAY(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_string((const char *)(where_).field_,	\
				    sizeof((where_).field_) +		\
				    MUST_BE_ARRAY((where_).field_),	\
				    QUOTE_FORCE_HEX);			\
	} while (0)

# define PRINT_FIELD_HEX_ARRAY_UPTO(where_, field_, upto_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_string((const char *)(where_).field_,	\
				    (upto_), QUOTE_FORCE_HEX);		\
	} while (0)

# define PRINT_FIELD_INET_ADDR(where_, field_, af_)		\
	print_inet_addr((af_), &(where_).field_,		\
			sizeof((where_).field_), #field_)

# define PRINT_FIELD_NET_PORT(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
									\
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)	\
			print_quoted_string((const char *)		\
					    &(where_).field_,		\
					    sizeof((where_).field_),	\
					    QUOTE_FORCE_HEX);		\
									\
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)	\
			break;						\
									\
		if (xlat_verbose(xlat_verbosity)			\
		    == XLAT_STYLE_VERBOSE)				\
			tprint_comment_begin();				\
									\
		tprints_arg_begin("htons");				\
		unsigned short us_ = ntohs((where_).field_);		\
		PRINT_VAL_U(us_);					\
		tprint_arg_end();					\
									\
		if (xlat_verbose(xlat_verbosity)			\
		    == XLAT_STYLE_VERBOSE)				\
			tprint_comment_end();				\
	} while (0)

# define PRINT_FIELD_IFINDEX(where_, field_)	\
	do {					\
		tprints_field_name(#field_);	\
		print_ifindex((where_).field_);	\
	} while (0)

# define PRINT_FIELD_SOCKADDR(where_, field_, tcp_)		\
	do {							\
		tprints_field_name(#field_);			\
		print_sockaddr(tcp_, &(where_).field_,		\
			       sizeof((where_).field_));	\
	} while (0)

# define PRINT_FIELD_DEV(where_, field_)	\
	do {					\
		tprints_field_name(#field_);	\
		print_dev_t((where_).field_);	\
	} while (0)

# define PRINT_FIELD_PTR(where_, field_)			\
	do {							\
		tprints_field_name(#field_);			\
		printaddr((mpers_ptr_t) (where_).field_);	\
	} while (0)

# define PRINT_FIELD_FD(where_, field_, tcp_)		\
	do {						\
		tprints_field_name(#field_);		\
		printfd((tcp_), (where_).field_);	\
	} while (0)

# define PRINT_FIELD_CHAR(where_, field_, flags_)	\
	do {						\
		tprints_field_name(#field_);		\
		print_char((where_).field_, (flags_));	\
	} while (0)

# define PRINT_FIELD_TGID(where_, field_, tcp_)			\
	do {							\
		tprints_field_name(#field_);			\
		printpid((tcp_), (where_).field_, PT_TGID);	\
	} while (0)

# define PRINT_FIELD_SYSCALL_NAME(where_, field_, audit_arch_)		\
	do {								\
		tprints_field_name(#field_);				\
		const char *nr_prefix_ = NULL;				\
		const char *name = syscall_name_arch((where_).field_,	\
						     (audit_arch_), &nr_prefix_); \
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV	\
		    || !nr_prefix_)					\
			PRINT_VAL_U((where_).field_);			\
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW	\
		    || !name)						\
			break;						\
		if (!nr_prefix_ ||					\
		    xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)	\
			tprint_comment_begin();				\
		if (nr_prefix_)						\
			tprints_string(nr_prefix_);			\
		tprints_string(name);					\
		if (!nr_prefix_ ||					\
		    xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)	\
			tprint_comment_end();				\
	} while (0)

/* Fabrice: probably change on the tprints_string in the previous macro */


# define PRINT_FIELD_MAC(where_, field_)				\
	PRINT_FIELD_MAC_SZ((where_), field_, ARRAY_SIZE((where_).field_))

# define PRINT_FIELD_MAC_SZ(where_, field_, size_)			\
	do {								\
		static_assert(sizeof(((where_).field_)[0]) == 1,	\
			      "MAC address is not a byte array");	\
		tprints_field_name(#field_);				\
		print_mac_addr("", (const uint8_t *) ((where_).field_),	\
			       MIN((size_), ARRAY_SIZE((where_).field_))); \
	} while (0)

# define PRINT_FIELD_HWADDR_SZ(where_, field_, size_, hwtype_)		\
	do {								\
		static_assert(sizeof(((where_).field_)[0]) == 1,	\
			      "hwaddress is not a byte array");		\
		tprints_field_name(#field_);				\
		print_hwaddr("", (const uint8_t *) ((where_).field_),	\
			     (size_), (hwtype_));			\
	} while (0)


# define PRINT_FIELD_OBJ_PTR(where_, field_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)(&((where_).field_), ##__VA_ARGS__);	\
	} while (0)

# define PRINT_FIELD_OBJ_TCB_PTR(where_, field_,		\
				 tcp_, print_func_, ...)	\
	do {							\
		tprints_field_name(#field_);			\
		(print_func_)((tcp_), &((where_).field_),	\
			      ##__VA_ARGS__);			\
	} while (0)

# define PRINT_FIELD_OBJ_VAL(where_, field_, print_func_, ...)	\
	do {							\
		tprints_field_name(#field_);			\
		(print_func_)((where_).field_, ##__VA_ARGS__);	\
	} while (0)

# define PRINT_FIELD_OBJ_U(where_, field_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)(zero_extend_signed_to_ull((where_).field_), \
			      ##__VA_ARGS__);				\
	} while (0)

# define PRINT_FIELD_OBJ_TCB_VAL(where_, field_,			\
				 tcp_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)((tcp_), (where_).field_, ##__VA_ARGS__);	\
	} while (0)


#define MAYBE_PRINT_FIELD_LEN(print_prefix_, where_, field_,		\
			      len_, print_func_, ...)			\
	do {								\
		unsigned int start = offsetof(typeof(where_), field_);	\
		unsigned int end = start + sizeof((where_).field_);	\
		if (len_ > start) {					\
			print_prefix_;					\
			if (len_ >= end) {				\
				print_func_((where_), field_,		\
					    ##__VA_ARGS__);		\
			} else {					\
				tprints_field_name(#field_);		\
				print_quoted_string(			\
					(void *)&(where_).field_,	\
					len_ - start, QUOTE_FORCE_HEX);	\
			}						\
		}							\
	} while (0)

#endif /* !STRACE_PRINT_FIELDS_H */
