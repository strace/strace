/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PRINT_FIELDS_H
# define STRACE_PRINT_FIELDS_H

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
# ifndef STRACE_PRINTF
#  define STRACE_PRINTF tprintf
# endif

# define PRINT_FIELD_D(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%lld", (prefix_), #field_,			\
		      sign_extend_unsigned_to_ll((where_).field_))

# define PRINT_FIELD_U(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%llu", (prefix_), #field_,			\
		      zero_extend_signed_to_ull((where_).field_))

# define PRINT_FIELD_U_CAST(prefix_, where_, field_, type_)		\
	STRACE_PRINTF("%s%s=%llu", (prefix_), #field_,			\
		      zero_extend_signed_to_ull((type_) (where_).field_))

# define PRINT_FIELD_X(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%#llx", (prefix_), #field_,			\
		      zero_extend_signed_to_ull((where_).field_))

# define PRINT_FIELD_ADDR(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printaddr((where_).field_);				\
	} while (0)

# define PRINT_FIELD_ADDR64(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printaddr64((where_).field_);				\
	} while (0)

# define PRINT_FIELD_0X(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%#0*llx", (prefix_), #field_,		\
		      (int) sizeof((where_).field_) * 2,		\
		      zero_extend_signed_to_ull((where_).field_))

# define PRINT_FIELD_COOKIE(prefix_, where_, field_)			\
	STRACE_PRINTF("%s%s=[%llu, %llu]", (prefix_), #field_,		\
		      zero_extend_signed_to_ull((where_).field_[0]),	\
		      zero_extend_signed_to_ull((where_).field_[1]))

# define PRINT_FIELD_FLAGS(prefix_, where_, field_, xlat_, dflt_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printflags64((xlat_),					\
			     zero_extend_signed_to_ull((where_).field_),\
			     (dflt_));					\
	} while (0)

# define PRINT_FIELD_XVAL(prefix_, where_, field_, xlat_, dflt_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printxval64((xlat_),					\
			    zero_extend_signed_to_ull((where_).field_),	\
			    (dflt_));		\
	} while (0)

# define PRINT_FIELD_XVAL_U(prefix_, where_, field_, xlat_, dflt_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printxvals_ex(zero_extend_signed_to_ull((where_).field_), \
			      (dflt_), XLAT_STYLE_FMT_U,		\
			      (xlat_), NULL);				\
	} while (0)

# define PRINT_FIELD_ERR_D(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_err(sign_extend_unsigned_to_ll((where_).field_),	\
			  true);					\
	} while (0)

# define PRINT_FIELD_ERR_U(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_err(zero_extend_signed_to_ull((where_).field_),	\
			  false);					\
	} while (0)

/*
 * Generic "ID" printing. ID is considered unsigned except for the special value
 * of -1.
 */
# define PRINT_FIELD_ID(prefix_, where_, field_)					\
	do {										\
		if (sign_extend_unsigned_to_ll((where_).field_) == -1LL)		\
			STRACE_PRINTF("%s%s=-1", (prefix_), #field_);			\
		else									\
			STRACE_PRINTF("%s%s=%llu", (prefix_), #field_,			\
				      zero_extend_signed_to_ull((where_).field_));	\
	} while (0)

# define PRINT_FIELD_UID PRINT_FIELD_ID

# define PRINT_FIELD_UUID(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_uuid((const unsigned char *) ((where_).field_));	\
	} while (0)

# define PRINT_FIELD_U64(prefix_, where_, field_)					\
	do {										\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);				\
		if (zero_extend_signed_to_ull((where_).field_) == UINT64_MAX)		\
			print_xlat_u(UINT64_MAX);					\
		else									\
			STRACE_PRINTF("%llu",						\
				      zero_extend_signed_to_ull((where_).field_));	\
	} while (0)

# define PRINT_FIELD_STRING(prefix_, where_, field_, len_, style_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_quoted_string((const char *)(where_).field_,	\
				    (len_), (style_));			\
	} while (0)

# define PRINT_FIELD_CSTRING(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_quoted_cstring((const char *) (where_).field_,	\
				     sizeof((where_).field_) +		\
					MUST_BE_ARRAY((where_).field_)); \
	} while (0)

# define PRINT_FIELD_CSTRING_SZ(prefix_, where_, field_, size_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_quoted_cstring((const char *) (where_).field_,	\
				     (size_));				\
	} while (0)

# define PRINT_FIELD_ARRAY(prefix_, where_, field_, tcp_, print_func_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_local_array((tcp_), (where_).field_,		\
				  (print_func_));			\
	} while (0)

# define PRINT_FIELD_ARRAY_UPTO(prefix_, where_, field_, tcp_, max_,	\
				print_func_)				\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_local_array_upto((tcp_), (where_).field_, (max_),	\
				  (print_func_));			\
	} while (0)

# define PRINT_FIELD_HEX_ARRAY(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_quoted_string((const char *)(where_).field_,	\
				    sizeof((where_).field_) +		\
					    MUST_BE_ARRAY((where_).field_), \
				    QUOTE_FORCE_HEX); \
	} while (0)

# define PRINT_FIELD_INET_ADDR(prefix_, where_, field_, af_)		\
	do {								\
		STRACE_PRINTF(prefix_);					\
		print_inet_addr((af_), &(where_).field_,		\
				sizeof((where_).field_), #field_);	\
	} while (0)

# define PRINT_FIELD_AX25_ADDR(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_ax25_addr(&(where_).field_);			\
	} while (0)

# define PRINT_FIELD_X25_ADDR(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_x25_addr(&(where_).field_);			\
	} while (0)

# define PRINT_FIELD_NET_PORT(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
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
			STRACE_PRINTF(" /* ");				\
									\
		STRACE_PRINTF("htons(%u)", ntohs((where_).field_));	\
									\
		if (xlat_verbose(xlat_verbosity)			\
				== XLAT_STYLE_VERBOSE)			\
			STRACE_PRINTF(" */");				\
	} while (0)

# define PRINT_FIELD_IFINDEX(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_ifindex((where_).field_);				\
	} while (0)

# define PRINT_FIELD_SOCKADDR(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_sockaddr(&(where_).field_,			\
			       sizeof((where_).field_));		\
	} while (0)

# define PRINT_FIELD_DEV(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_dev_t((where_).field_);				\
	} while (0)

# define PRINT_FIELD_PTR(prefix_, where_, field_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printaddr((mpers_ptr_t) (where_).field_);		\
	} while (0)

# define PRINT_FIELD_FD(prefix_, where_, field_, tcp_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printfd((tcp_), (where_).field_);			\
	} while (0)

# define PRINT_FIELD_STRN(prefix_, where_, field_, len_, tcp_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printstrn((tcp_), (where_).field_, (len_));		\
	} while (0)


# define PRINT_FIELD_STR(prefix_, where_, field_, tcp_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printstr((tcp_), (where_).field_);			\
	} while (0)

# define PRINT_FIELD_PATH(prefix_, where_, field_, tcp_)			\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printpath((tcp_), (where_).field_);			\
	} while (0)

# define PRINT_FIELD_MAC(prefix_, where_, field_)			\
	PRINT_FIELD_MAC_SZ((prefix_), (where_), field_,			\
			   ARRAY_SIZE((where_).field_))

# define PRINT_FIELD_MAC_SZ(prefix_, where_, field_, size_)		\
	do {								\
		static_assert(sizeof(((where_).field_)[0]) == 1,	\
			      "MAC address is not a byte array");	\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_mac_addr("", (const uint8_t *) ((where_).field_),	\
			       (size_));				\
	} while (0)

# define PRINT_FIELD_HWADDR_SZ(prefix_, where_, field_, size_, hwtype_)	\
	do {								\
		static_assert(sizeof(((where_).field_)[0]) == 1,	\
			      "hwaddress is not a byte array");	\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_hwaddr("", (const uint8_t *) ((where_).field_),	\
			       (size_), (hwtype_));			\
	} while (0)

#endif /* !STRACE_PRINT_FIELDS_H */
