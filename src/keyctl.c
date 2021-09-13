/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "keyctl_kdf_params.h"

#include "xlat/key_perms.h"
#include "xlat/key_reqkeys.h"
#include "xlat/key_spec.h"
#include "xlat/keyctl_caps0.h"
#include "xlat/keyctl_caps1.h"
#include "xlat/keyctl_commands.h"
#include "xlat/keyctl_move_flags.h"
#include "xlat/keyctl_pkey_ops.h"

typedef int32_t key_serial_t;

static void
print_keyring_serial_number(key_serial_t id)
{
	printxval_d(key_spec, id, NULL);
}

SYS_FUNC(add_key)
{
	/* type */
	printstr(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* description */
	printstr(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* payload */
	printstrn(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	tprint_arg_next();

	/* payload length */
	PRINT_VAL_U(tcp->u_arg[3]);
	tprint_arg_next();

	/* keyring serial number */
	print_keyring_serial_number(tcp->u_arg[4]);

	return RVAL_DECODED;
}

SYS_FUNC(request_key)
{
	/* type */
	printstr(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* description */
	printstr(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* callout_info */
	printstr(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* keyring serial number */
	print_keyring_serial_number(tcp->u_arg[3]);

	return RVAL_DECODED;
}

static void
keyctl_get_keyring_id(struct tcb *tcp, key_serial_t id, int create)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	PRINT_VAL_D(create);
}

static void
keyctl_update_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		  kernel_ulong_t len)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	printstrn(tcp, addr, len);
	tprint_arg_next();

	PRINT_VAL_U(len);
}

static void
keyctl_handle_key_key(struct tcb *tcp, key_serial_t id1, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_read_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		kernel_ulong_t len, bool has_nul)
{
	if (entering(tcp)) {
		print_keyring_serial_number(id);
		tprint_arg_next();
	} else {
		if (syserror(tcp))
			printaddr(addr);
		else {
			kernel_ulong_t rval = (tcp->u_rval >= 0) &&
				((kernel_ulong_t) tcp->u_rval > len) ? len :
				(kernel_ulong_t) tcp->u_rval;
			printstr_ex(tcp, addr, rval, has_nul ?
				    QUOTE_OMIT_TRAILING_0 : 0);
		}
		tprint_arg_next();

		PRINT_VAL_U(len);
	}
}

static void
keyctl_keyring_search(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr1,
		      kernel_ulong_t addr2, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	printstr(tcp, addr1);
	tprint_arg_next();

	printstr(tcp, addr2);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_chown_key(struct tcb *tcp, key_serial_t id, unsigned user,
		 unsigned group)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	printuid(user);
	tprint_arg_next();

	printuid(group);
}

static void
keyctl_instantiate_key(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr,
		       kernel_ulong_t len, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	printstrn(tcp, addr, len);
	tprint_arg_next();

	PRINT_VAL_U(len);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_instantiate_key_iov(struct tcb *tcp, key_serial_t id1,
			   kernel_ulong_t addr, kernel_ulong_t len,
			   key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	tprint_iov(tcp, len, addr, iov_decode_str);
	tprint_arg_next();

	PRINT_VAL_U(len);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_negate_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	PRINT_VAL_U(timeout);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_reject_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  unsigned error, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprint_arg_next();

	PRINT_VAL_U(timeout);
	tprint_arg_next();

	print_err(error, false);
	tprint_arg_next();

	print_keyring_serial_number(id2);
}

static void
keyctl_set_timeout(struct tcb *tcp, key_serial_t id, unsigned timeout)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	PRINT_VAL_U(timeout);
}

static void
keyctl_get_persistent(struct tcb *tcp, unsigned uid, key_serial_t id)
{
	printuid(uid);
	tprint_arg_next();

	print_keyring_serial_number(id);
}

static void
keyctl_setperm_key(struct tcb *tcp, key_serial_t id, uint32_t perm)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	printflags(key_perms, perm, "KEY_???");
}

static void
print_dh_params(struct tcb *tcp, kernel_ulong_t addr)
{
	struct keyctl_dh_params params;

	if (umove_or_printaddr(tcp, addr, &params))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_VAL(params, private, print_keyring_serial_number);
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(params, prime, print_keyring_serial_number);
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(params, base, print_keyring_serial_number);
	tprint_struct_end();
}

static void
keyctl_dh_compute(struct tcb *tcp, kernel_ulong_t params, kernel_ulong_t buf,
		  kernel_ulong_t len, kernel_ulong_t kdf_addr)
{
	if (entering(tcp)) {
		print_dh_params(tcp, params);
		tprint_arg_next();
	} else {
		struct strace_keyctl_kdf_params kdf;

		if (syserror(tcp)) {
			printaddr(buf);
		} else {
			kernel_ulong_t rval = (tcp->u_rval >= 0) &&
				((kernel_ulong_t) tcp->u_rval > len) ? len :
				(kernel_ulong_t) tcp->u_rval;
			printstrn(tcp, buf, rval);
		}
		tprint_arg_next();

		PRINT_VAL_U(len);
		tprint_arg_next();

		if (fetch_keyctl_kdf_params(tcp, kdf_addr, &kdf)) {
			printaddr(kdf_addr);
		} else {
			tprint_struct_begin();
			PRINT_FIELD_OBJ_TCB_VAL(kdf, hashname, tcp,
						printstr);

			/*
			 * Kernel doesn't touch otherinfo
			 * if otherinfolen is zero.
			 */
			if (kdf.otherinfolen) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_TCB_VAL(kdf, otherinfo,
					tcp, printstrn, kdf.otherinfolen);
			} else {
				tprint_struct_next();
				PRINT_FIELD_PTR(kdf, otherinfo);
			}

			tprint_struct_next();
			PRINT_FIELD_U(kdf, otherinfolen);

			if (!IS_ARRAY_ZERO(kdf.__spare)) {
				tprint_struct_next();
				PRINT_FIELD_ARRAY(kdf, __spare, tcp,
						  print_xint_array_member);
			}

			tprint_struct_end();
		}
	}
}

static void
print_pkey_query(struct tcb *tcp, kernel_ulong_t addr)
{
	struct keyctl_pkey_query query;

	if (umove_or_printaddr(tcp, addr, &query))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(query, supported_ops, keyctl_pkey_ops,
			  "KEYCTL_SUPPORTS_???");
	tprint_struct_next();
	PRINT_FIELD_U(query, key_size);
	tprint_struct_next();
	PRINT_FIELD_U(query, max_data_size);
	tprint_struct_next();
	PRINT_FIELD_U(query, max_sig_size);
	tprint_struct_next();
	PRINT_FIELD_U(query, max_enc_size);
	tprint_struct_next();
	PRINT_FIELD_U(query, max_dec_size);

	if (!IS_ARRAY_ZERO(query.__spare)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(query, __spare, tcp,
				  print_xint_array_member);
	}

	tprint_struct_end();
}

static void
keyctl_pkey_query(struct tcb *const tcp,
		  const key_serial_t id,
		  const kernel_ulong_t reserved,
		  const kernel_ulong_t /* char * */ info,
		  const kernel_ulong_t /* keyctl_pkey_query * */ res)
{
	if (entering(tcp)) {
		print_keyring_serial_number(id);
		tprint_arg_next();

		PRINT_VAL_X(reserved);
		tprint_arg_next();

		printstr(tcp, info);
		tprint_arg_next();
	} else {
		print_pkey_query(tcp, res);
	}
}

static bool
fetch_print_pkey_params(struct tcb *tcp, kernel_ulong_t addr,
			struct keyctl_pkey_params *params, bool out)
{
	if (umove_or_printaddr(tcp, addr, params))
		return false;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_VAL(*params, key_id, print_keyring_serial_number);
	tprint_struct_next();
	PRINT_FIELD_U(*params, in_len);

	if (out) {
		tprint_struct_next();
		PRINT_FIELD_U(*params, out_len);
	} else {
		tprint_struct_next();
		PRINT_FIELD_U(*params, in2_len);
	}

	if (!IS_ARRAY_ZERO(params->__spare)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(*params, __spare, tcp,
				  print_xint_array_member);
	}

	tprint_struct_end();

	return true;
}

static int
keyctl_pkey_op(struct tcb *const tcp,
	       const kernel_ulong_t /* keyctl_pkey_params * */ params_addr,
	       const kernel_ulong_t /* char * */ info,
	       const kernel_ulong_t /* void * */ op1,
	       const kernel_ulong_t /* void * */ op2,
	       bool out)
{
	if (entering(tcp)) {
		struct keyctl_pkey_params params;
		bool ret;

		ret = fetch_print_pkey_params(tcp, params_addr, &params, out);
		if (ret && out)
			set_tcb_priv_ulong(tcp, params.out_len);

		tprint_arg_next();
		printstr(tcp, info);
		tprint_arg_next();

		if (ret)
			printstrn(tcp, op1, params.in_len);
		else
			printaddr(op1);
		tprint_arg_next();

		if (!out || !ret) {
			if (ret)
				printstrn(tcp, op2, params.in2_len);
			else
				printaddr(op2);
		}

		return ret && out ? 0 : RVAL_DECODED;
	} else {
		unsigned long out_len = get_tcb_priv_ulong(tcp);

		if (syserror(tcp))
			printaddr(op2);
		else
			printstrn(tcp, op2, out_len);
	}

	return 0;
}

static void
keyctl_restrict_keyring(struct tcb *const tcp,
			const key_serial_t id,
			const kernel_ulong_t addr1,
			const kernel_ulong_t addr2)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	printstr(tcp, addr1);
	tprint_arg_next();

	printstr(tcp, addr2);
}

static void
keyctl_move(struct tcb *const tcp,
	    const key_serial_t id,
	    const key_serial_t from,
	    const key_serial_t to,
	    const unsigned int flags)
{
	print_keyring_serial_number(id);
	tprint_arg_next();

	print_keyring_serial_number(from);
	tprint_arg_next();

	print_keyring_serial_number(to);
	tprint_arg_next();

	printflags(keyctl_move_flags, flags, "KEYCTL_MOVE_???");
}

static bool
print_keyctl_caps(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	static const struct {
		const struct xlat *xlat;
		const char *dflt;
	} caps[] = {
		{ keyctl_caps0, "KEYCTL_CAPS0_???" },
		{ keyctl_caps1, "KEYCTL_CAPS1_???" },
	};

	kernel_ulong_t *pos = data;
	unsigned char *elem = elem_buf;

	if (*pos < ARRAY_SIZE(caps))
		printflags(caps[*pos].xlat, *elem, caps[*pos].dflt);
	else
		PRINT_VAL_X(*elem);

	(*pos)++;

	return true;
}

static void
keyctl_capabilities(struct tcb *const tcp,
		    const kernel_ulong_t /* char * */ buf,
		    const kernel_ulong_t buflen)
{
	kernel_ulong_t pos = 0;
	unsigned char elem;

	if (entering(tcp))
		return;

	if (syserror(tcp)) {
		printaddr(buf);
	} else {
		print_array(tcp, buf, MIN(buflen, (kernel_ulong_t) tcp->u_rval),
			    &elem, sizeof(elem),
			    tfetch_mem, print_keyctl_caps, &pos);
	}
	tprint_arg_next();

	PRINT_VAL_U(buflen);
}

SYS_FUNC(keyctl)
{
	int cmd = tcp->u_arg[0];
	kernel_ulong_t arg2 = tcp->u_arg[1];
	kernel_ulong_t arg3 = tcp->u_arg[2];
	kernel_ulong_t arg4 = tcp->u_arg[3];
	kernel_ulong_t arg5 = tcp->u_arg[4];

	if (entering(tcp)) {
		printxval(keyctl_commands, cmd, "KEYCTL_???");

		/*
		 * For now, KEYCTL_SESSION_TO_PARENT is the only cmd without
		 * arguments.
		 */
		if (cmd != KEYCTL_SESSION_TO_PARENT)
			tprint_arg_next();
	}

	switch (cmd) {
	case KEYCTL_GET_KEYRING_ID:
		keyctl_get_keyring_id(tcp, arg2, arg3);
		break;

	case KEYCTL_JOIN_SESSION_KEYRING:
		printstr(tcp, arg2);
		break;

	case KEYCTL_UPDATE:
		keyctl_update_key(tcp, arg2, arg3, arg4);
		break;

	case KEYCTL_REVOKE:
	case KEYCTL_CLEAR:
	case KEYCTL_INVALIDATE:
	case KEYCTL_ASSUME_AUTHORITY:
		print_keyring_serial_number(arg2);
		break;

	case KEYCTL_LINK:
	case KEYCTL_UNLINK:
		keyctl_handle_key_key(tcp, arg2, arg3);
		break;

	case KEYCTL_DESCRIBE:
	case KEYCTL_READ:
	case KEYCTL_GET_SECURITY:
		keyctl_read_key(tcp, arg2, arg3, arg4, cmd != KEYCTL_READ);
		return 0;

	case KEYCTL_SEARCH:
		keyctl_keyring_search(tcp, arg2, arg3, arg4, arg5);
		break;

	case KEYCTL_CHOWN:
		keyctl_chown_key(tcp, arg2, arg3, arg4);
		break;

	case KEYCTL_SETPERM:
		keyctl_setperm_key(tcp, arg2, arg3);
		break;

	case KEYCTL_INSTANTIATE:
		keyctl_instantiate_key(tcp, arg2, arg3, arg4, arg5);
		break;

	case KEYCTL_NEGATE:
		keyctl_negate_key(tcp, arg2, arg3, arg4);
		break;

	case KEYCTL_SET_REQKEY_KEYRING:
		printxvals_ex((int) arg2, "KEY_REQKEY_DEFL_???",
			      XLAT_STYLE_FMT_D, key_reqkeys, NULL);
		break;

	case KEYCTL_SET_TIMEOUT:
		keyctl_set_timeout(tcp, arg2, arg3);
		break;

	case KEYCTL_SESSION_TO_PARENT:
		break;

	case KEYCTL_REJECT:
		keyctl_reject_key(tcp, arg2, arg3, arg4, arg5);
		break;

	case KEYCTL_INSTANTIATE_IOV:
		keyctl_instantiate_key_iov(tcp, arg2, arg3, arg4, arg5);
		break;

	case KEYCTL_GET_PERSISTENT:
		keyctl_get_persistent(tcp, arg2, arg3);
		break;

	case KEYCTL_DH_COMPUTE:
		keyctl_dh_compute(tcp, arg2, arg3, arg4, arg5);
		return 0;

	case KEYCTL_PKEY_QUERY:
		keyctl_pkey_query(tcp, arg2, arg3, arg4, arg5);
		return 0;

	case KEYCTL_PKEY_ENCRYPT:
	case KEYCTL_PKEY_DECRYPT:
	case KEYCTL_PKEY_SIGN:
		return keyctl_pkey_op(tcp, arg2, arg3, arg4, arg5, true);

	case KEYCTL_PKEY_VERIFY:
		keyctl_pkey_op(tcp, arg2, arg3, arg4, arg5, false);
		break;

	case KEYCTL_RESTRICT_KEYRING:
		keyctl_restrict_keyring(tcp, arg2, arg3, arg4);
		break;

	case KEYCTL_MOVE:
		keyctl_move(tcp, arg2, arg3, arg4, arg5);
		break;

	case KEYCTL_CAPABILITIES:
		keyctl_capabilities(tcp, arg2, arg3);
		return 0;

	default:
		PRINT_VAL_X(arg2);
		tprint_arg_next();

		PRINT_VAL_X(arg3);
		tprint_arg_next();

		PRINT_VAL_X(arg4);
		tprint_arg_next();

		PRINT_VAL_X(arg5);
		break;
	}

	return RVAL_DECODED;
}
