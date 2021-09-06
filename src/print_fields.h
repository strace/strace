/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PRINT_FIELDS_H
# define STRACE_PRINT_FIELDS_H

# include "static_assert.h"

# ifdef IN_STRACE

static inline void
tprint_struct_begin(void)
{
	tprints("{");
}

static inline void
tprint_struct_next(void)
{
	tprints(", ");
}

static inline void
tprint_struct_end(void)
{
	tprints("}");
}

static inline void
tprint_array_begin(void)
{
	tprints("[");
}

static inline void
tprint_array_next(void)
{
	tprints(", ");
}

static inline void
tprint_array_end(void)
{
	tprints("]");
}

static inline void
tprint_array_index_begin(void)
{
	tprints("[");
}

static inline void
tprint_array_index_end(void)
{
	tprints("] = ");
}

static inline void
tprint_arg_next(void)
{
	tprints(", ");
}

static inline void
tprint_arg_end(void)
{
	tprints(")");
}

static inline void
tprint_bitset_begin(void)
{
	tprints("[");
}

static inline void
tprint_bitset_next(void)
{
	tprints(" ");
}

static inline void
tprint_bitset_end(void)
{
	tprints("]");
}

static inline void
tprint_comment_begin(void)
{
	tprints(" /* ");
}

static inline void
tprint_comment_end(void)
{
	tprints(" */");
}

static inline void
tprint_indirect_begin(void)
{
	tprints("[");
}

static inline void
tprint_indirect_end(void)
{
	tprints("]");
}

static inline void
tprint_more_data_follows(void)
{
	tprints("...");
}

static inline void
tprint_value_changed(void)
{
	tprints(" => ");
}

static inline void
tprint_alternative_value(void)
{
	tprints(" or ");
}

static inline void
tprint_unavailable(void)
{
	tprints("???");
}

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
#  define STRACE_PRINTF tprintf

# else /* !IN_STRACE */

#  include <stdio.h>

static inline void
tprint_struct_begin(void)
{
	fputs("{", stdout);
}

static inline void
tprint_struct_next(void)
{
	fputs(", ", stdout);
}

static inline void
tprint_struct_end(void)
{
	fputs("}", stdout);
}

static inline void
tprint_array_begin(void)
{
	fputs("[", stdout);
}

static inline void
tprint_array_next(void)
{
	fputs(", ", stdout);
}

static inline void
tprint_array_end(void)
{
	fputs("]", stdout);
}

static inline void
tprint_array_index_begin(void)
{
	fputs("[", stdout);
}

static inline void
tprint_array_index_end(void)
{
	fputs("] = ", stdout);
}

static inline void
tprint_arg_next(void)
{
	fputs(", ", stdout);
}

static inline void
tprint_arg_end(void)
{
	fputs(")", stdout);
}

static inline void
tprint_bitset_begin(void)
{
	fputs("[", stdout);
}

static inline void
tprint_bitset_next(void)
{
	fputs(" ", stdout);
}

static inline void
tprint_bitset_end(void)
{
	fputs("]", stdout);
}

static inline void
tprint_comment_begin(void)
{
	fputs(" /* ", stdout);
}

static inline void
tprint_comment_end(void)
{
	fputs(" */", stdout);
}

static inline void
tprint_indirect_begin(void)
{
	fputs("[", stdout);
}

static inline void
tprint_indirect_end(void)
{
	fputs("]", stdout);
}

static inline void
tprint_more_data_follows(void)
{
	fputs("...", stdout);
}

static inline void
tprint_value_changed(void)
{
	fputs(" => ", stdout);
}

static inline void
tprint_alternative_value(void)
{
	fputs(" or ", stdout);
}

static inline void
tprint_unavailable(void)
{
	fputs("???", stdout);
}

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
#  define STRACE_PRINTF printf

# endif /* !IN_STRACE */

static inline void
tprints_field_name(const char *name)
{
	STRACE_PRINTF("%s=", name);
}

static inline void
tprints_arg_name(const char *name)
{
	STRACE_PRINTF("%s=", name);
}

static inline void
tprints_arg_begin(const char *name)
{
	STRACE_PRINTF("%s(", name);
}

# define PRINT_VAL_D(val_)	\
	STRACE_PRINTF("%lld", sign_extend_unsigned_to_ll(val_))

# define PRINT_VAL_U(val_)	\
	STRACE_PRINTF("%llu", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_X(val_)	\
	STRACE_PRINTF("%#llx", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_03O(val_)	\
	STRACE_PRINTF("%#03llo", zero_extend_signed_to_ull(val_))

# define PRINT_VAL_0X(val_)						\
	STRACE_PRINTF("%#0*llx", (int) sizeof(val_) * 2,		\
		      zero_extend_signed_to_ull(val_))

# define PRINT_VAL_ID(val_)						\
	do {								\
		if (sign_extend_unsigned_to_ll(val_) == -1LL)		\
			PRINT_VAL_D(-1);				\
		else							\
			PRINT_VAL_U(val_);				\
	} while (0)

# define PRINT_FIELD_D(where_, field_)					\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_D((where_).field_);				\
	} while (0)

# define PRINT_FIELD_U(where_, field_)					\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_U((where_).field_);				\
	} while (0)

# define PRINT_FIELD_U_CAST(where_, field_, type_)			\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_U((type_)((where_).field_));			\
	} while (0)

# define PRINT_FIELD_X(where_, field_)					\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_X((where_).field_);				\
	} while (0)

# define PRINT_FIELD_X_CAST(where_, field_, type_)			\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_X((type_)((where_).field_));			\
	} while (0)

# define PRINT_FIELD_ADDR64(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		printaddr64((where_).field_);				\
	} while (0)

# define PRINT_FIELD_0X(where_, field_)					\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_0X((where_).field_);				\
	} while (0)

# define PRINT_FIELD_VAL_ARRAY(where_, field_, print_val_)		\
	do {								\
		tprints_field_name(#field_);				\
		for (size_t i_ = 0;					\
		     i_ < ARRAY_SIZE((where_).field_);			\
		     ++i_) {						\
			if (i_)						\
				tprint_array_next();			\
			else						\
				tprint_array_begin();			\
			print_val_((where_).field_[i_]);		\
		}							\
		tprint_array_end();					\
	} while (0)

# define PRINT_FIELD_D_ARRAY(where_, field_)				\
	PRINT_FIELD_VAL_ARRAY((where_), field_, PRINT_VAL_D)

# define PRINT_FIELD_U_ARRAY(where_, field_)				\
	PRINT_FIELD_VAL_ARRAY((where_), field_, PRINT_VAL_U)

# define PRINT_FIELD_X_ARRAY(where_, field_)				\
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

# define PRINT_FIELD_X_ARRAY2D(where_, field_)				\
	PRINT_FIELD_VAL_ARRAY2D((where_), field_, PRINT_VAL_X)

# define PRINT_FIELD_COOKIE(where_, field_)				\
	do {								\
		static_assert(ARRAY_SIZE((where_).field_) == 2,		\
			      "unexpected array size");			\
		PRINT_FIELD_U_ARRAY((where_), field_);			\
	} while (0)

# define PRINT_FIELD_FLAGS(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printflags64((xlat_),					\
			     zero_extend_signed_to_ull((where_).field_),\
			     (dflt_));					\
	} while (0)

# define PRINT_FIELD_XVAL(where_, field_, xlat_, dflt_)			\
	do {								\
		tprints_field_name(#field_);				\
		printxval64((xlat_),					\
			    zero_extend_signed_to_ull((where_).field_),	\
			    (dflt_));		\
	} while (0)

# define PRINT_FIELD_XVAL_U(where_, field_, xlat_, dflt_)		\
	do {								\
		tprints_field_name(#field_);				\
		printxval64_u((xlat_),					\
			      zero_extend_signed_to_ull((where_).field_), \
			      (dflt_));					\
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
# define PRINT_FIELD_ID(where_, field_)					\
	do {								\
		tprints_field_name(#field_);				\
		PRINT_VAL_ID((where_).field_);				\
	} while (0)

# define PRINT_FIELD_UUID(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_uuid((const unsigned char *) ((where_).field_));	\
	} while (0)

# define PRINT_FIELD_U64(where_, field_)					\
	do {									\
		tprints_field_name(#field_);					\
		if (zero_extend_signed_to_ull((where_).field_) == UINT64_MAX)	\
			print_xlat_u(UINT64_MAX);				\
		else								\
			PRINT_VAL_U((where_).field_);				\
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

# define PRINT_FIELD_ARRAY(where_, field_, tcp_, print_func_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_local_array((tcp_), (where_).field_,		\
				  (print_func_));			\
	} while (0)

# define PRINT_FIELD_ARRAY_UPTO(where_, field_,				\
				upto_, tcp_, print_func_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_local_array_upto((tcp_), (where_).field_,		\
				       (upto_), (print_func_));		\
	} while (0)

# define PRINT_FIELD_HEX_ARRAY(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_quoted_string((const char *)(where_).field_,	\
				    sizeof((where_).field_) +		\
					    MUST_BE_ARRAY((where_).field_), \
				    QUOTE_FORCE_HEX); \
	} while (0)

# define PRINT_FIELD_INET_ADDR(where_, field_, af_)			\
	print_inet_addr((af_), &(where_).field_,			\
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
				== XLAT_STYLE_VERBOSE)			\
			tprint_comment_begin();				\
									\
		tprints_arg_begin("htons");				\
		unsigned short us_ = ntohs((where_).field_);		\
		PRINT_VAL_U(us_);					\
		tprint_arg_end();					\
									\
		if (xlat_verbose(xlat_verbosity)			\
				== XLAT_STYLE_VERBOSE)			\
			tprint_comment_end();				\
	} while (0)

# define PRINT_FIELD_IFINDEX(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_ifindex((where_).field_);				\
	} while (0)

# define PRINT_FIELD_SOCKADDR(where_, field_, tcp_)			\
	do {								\
		tprints_field_name(#field_);				\
		print_sockaddr(tcp_, &(where_).field_,			\
			       sizeof((where_).field_));		\
	} while (0)

# define PRINT_FIELD_DEV(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		print_dev_t((where_).field_);				\
	} while (0)

# define PRINT_FIELD_PTR(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		printaddr((mpers_ptr_t) (where_).field_);		\
	} while (0)

# define PRINT_FIELD_FD(where_, field_, tcp_)				\
	do {								\
		tprints_field_name(#field_);				\
		printfd((tcp_), (where_).field_);			\
	} while (0)

# define PRINT_FIELD_TGID(where_, field_, tcp_)				\
	do {								\
		tprints_field_name(#field_);				\
		printpid((tcp_), (where_).field_, PT_TGID);		\
	} while (0)

# define PRINT_FIELD_SYSCALL_NAME(where_, field_, audit_arch_)		\
	do {								\
		tprints_field_name(#field_);				\
		const char *nr_prefix_ = NULL;				\
		const char *name = syscall_name_arch((where_).field_,	\
					(audit_arch_), &nr_prefix_);	\
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
			tprints(nr_prefix_);				\
		tprints(name);						\
		if (!nr_prefix_ ||					\
		    xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)	\
			tprint_comment_end();				\
	} while (0)

# define PRINT_FIELD_MAC(where_, field_)				\
	PRINT_FIELD_MAC_SZ((where_), field_, ARRAY_SIZE((where_).field_))

# define PRINT_FIELD_MAC_SZ(where_, field_, size_)			\
	do {								\
		static_assert(sizeof(((where_).field_)[0]) == 1,	\
			      "MAC address is not a byte array");	\
		tprints_field_name(#field_);				\
		print_mac_addr("", (const uint8_t *) ((where_).field_),	\
			MIN((size_), ARRAY_SIZE((where_).field_)));	\
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

# define PRINT_FIELD_OBJ_TCB_PTR(where_, field_,			\
				 tcp_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)((tcp_), &((where_).field_),		\
			      ##__VA_ARGS__);				\
	} while (0)

# define PRINT_FIELD_OBJ_VAL(where_, field_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)((where_).field_, ##__VA_ARGS__);		\
	} while (0)

# define PRINT_FIELD_OBJ_U(where_, field_, print_func_, ...)		\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)(zero_extend_signed_to_ull((where_).field_),\
			      ##__VA_ARGS__);				\
	} while (0)

# define PRINT_FIELD_OBJ_TCB_VAL(where_, field_,			\
			     tcp_, print_func_, ...)			\
	do {								\
		tprints_field_name(#field_);				\
		(print_func_)((tcp_), (where_).field_, ##__VA_ARGS__);	\
	} while (0)

#endif /* !STRACE_PRINT_FIELDS_H */
