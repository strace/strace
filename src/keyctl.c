/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
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
	tprints_arg_name("type");
	printstr(tcp, tcp->u_arg[0]);

	/* description */
	tprints_arg_next_name("description");
	printstr(tcp, tcp->u_arg[1]);

	/* payload */
	tprints_arg_next_name("payload");
	printstrn(tcp, tcp->u_arg[2], tcp->u_arg[3]);

	/* payload length */
	tprints_arg_next_name("plen");
	PRINT_VAL_U(tcp->u_arg[3]);

	/* keyring serial number */
	tprints_arg_next_name("keyring");
	print_keyring_serial_number(tcp->u_arg[4]);

	return RVAL_DECODED;
}

SYS_FUNC(request_key)
{
	/* type */
	tprints_arg_name("type");
	printstr(tcp, tcp->u_arg[0]);

	/* description */
	tprints_arg_next_name("description");
	printstr(tcp, tcp->u_arg[1]);

	/* callout_info */
	tprints_arg_next_name("callout_info");
	printstr(tcp, tcp->u_arg[2]);

	/* keyring serial number */
	tprints_arg_next_name("dest_keyring");
	print_keyring_serial_number(tcp->u_arg[3]);

	return RVAL_DECODED;
}

static void
keyctl_get_keyring_id(struct tcb *tcp, key_serial_t id, int create)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("create");
	PRINT_VAL_D(create);
}

static void
keyctl_update_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		  kernel_ulong_t len)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("payload");
	printstrn(tcp, addr, len);

	tprints_arg_next_name("plen");
	PRINT_VAL_U(len);
}

static void
keyctl_handle_key_key(struct tcb *tcp, key_serial_t id1, key_serial_t id2)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_read_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		kernel_ulong_t len, bool has_nul)
{
	if (entering(tcp)) {
		tprints_arg_next_name("key");
		print_keyring_serial_number(id);
	} else {
		tprints_arg_next_name("buffer");
		if (syserror(tcp))
			printaddr(addr);
		else {
			kernel_ulong_t rval = (tcp->u_rval >= 0) &&
				((kernel_ulong_t) tcp->u_rval > len) ? len :
				(kernel_ulong_t) tcp->u_rval;
			printstr_ex(tcp, addr, rval, has_nul ?
				    QUOTE_OMIT_TRAILING_0 : 0);
		}

		tprints_arg_next_name("buflen");
		PRINT_VAL_U(len);
	}
}

static void
keyctl_keyring_search(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr1,
		      kernel_ulong_t addr2, key_serial_t id2)
{
	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("type");
	printstr(tcp, addr1);

	tprints_arg_next_name("description");
	printstr(tcp, addr2);

	tprints_arg_next_name("dest_keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_chown_key(struct tcb *tcp, key_serial_t id, unsigned user,
		 unsigned group)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("uid");
	printuid(user);

	tprints_arg_next_name("gid");
	printuid(group);
}

static void
keyctl_instantiate_key(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr,
		       kernel_ulong_t len, key_serial_t id2)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("payload");
	printstrn(tcp, addr, len);

	tprints_arg_next_name("plen");
	PRINT_VAL_U(len);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_instantiate_key_iov(struct tcb *tcp, key_serial_t id1,
			   kernel_ulong_t addr, kernel_ulong_t len,
			   key_serial_t id2)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("payload");
	tprint_iov(tcp, len, addr, iov_decode_str);

	tprints_arg_next_name("plen");
	PRINT_VAL_U(len);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_negate_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  key_serial_t id2)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("timeout");
	PRINT_VAL_U(timeout);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_reject_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  unsigned error, key_serial_t id2)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id1);

	tprints_arg_next_name("timeout");
	PRINT_VAL_U(timeout);

	tprints_arg_next_name("error");
	print_err(error, false);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id2);
}

static void
keyctl_set_timeout(struct tcb *tcp, key_serial_t id, unsigned timeout)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("timeout");
	PRINT_VAL_U(timeout);
}

static void
keyctl_get_persistent(struct tcb *tcp, unsigned uid, key_serial_t id)
{
	tprints_arg_next_name("uid");
	printuid(uid);

	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id);
}

static void
keyctl_setperm_key(struct tcb *tcp, key_serial_t id, uint32_t perm)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("perms");
	printflags(key_perms, perm, "KEY_???");
}

static void
print_dh_params(struct tcb *tcp, kernel_ulong_t addr)
{
	struct keyctl_dh_params params;

	tprints_arg_next_name("dh_params");
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
	} else {
		struct strace_keyctl_kdf_params kdf;

		tprints_arg_next_name("buffer");
		if (syserror(tcp)) {
			printaddr(buf);
		} else {
			kernel_ulong_t rval = (tcp->u_rval >= 0) &&
				((kernel_ulong_t) tcp->u_rval > len) ? len :
				(kernel_ulong_t) tcp->u_rval;
			printstrn(tcp, buf, rval);
		}

		tprints_arg_next_name("buflen");
		PRINT_VAL_U(len);

		tprints_arg_next_name("kdf_params");
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
		tprints_arg_next_name("key");
		print_keyring_serial_number(id);

		tprints_arg_next_name("reserved");
		PRINT_VAL_X(reserved);

		tprints_arg_next_name("params");
		printstr(tcp, info);
	} else {
		tprints_arg_next_name("info");
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

		tprints_arg_next_name("params");
		ret = fetch_print_pkey_params(tcp, params_addr, &params, out);

		tprints_arg_next_name("info");
		printstr(tcp, info);

		tprints_arg_next_name("in");
		if (ret)
			printstrn(tcp, op1, params.in_len);
		else
			printaddr(op1);

		if (ret && out) {
			set_tcb_priv_ulong(tcp, params.out_len);
			return 0;
		}

		tprints_arg_next_name("in2");
		if (ret)
			printstrn(tcp, op2, params.in2_len);
		else
			printaddr(op2);

		return RVAL_DECODED;
	} else {
		unsigned long out_len = get_tcb_priv_ulong(tcp);

		tprints_arg_next_name("out");
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
	tprints_arg_next_name("keyring");
	print_keyring_serial_number(id);

	tprints_arg_next_name("type");
	printstr(tcp, addr1);

	tprints_arg_next_name("restriction");
	printstr(tcp, addr2);
}

static void
keyctl_move(struct tcb *const tcp,
	    const key_serial_t id,
	    const key_serial_t from,
	    const key_serial_t to,
	    const unsigned int flags)
{
	tprints_arg_next_name("key");
	print_keyring_serial_number(id);

	tprints_arg_next_name("from_keyring");
	print_keyring_serial_number(from);

	tprints_arg_next_name("to_keyring");
	print_keyring_serial_number(to);

	tprints_arg_next_name("flags");
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

	tprints_arg_next_name("buffer");
	if (syserror(tcp)) {
		printaddr(buf);
	} else {
		print_array(tcp, buf, MIN(buflen, (kernel_ulong_t) tcp->u_rval),
			    &elem, sizeof(elem),
			    tfetch_mem, print_keyctl_caps, &pos);
	}

	tprints_arg_next_name("buflen");
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
		tprints_arg_name("operation");
		printxval(keyctl_commands, cmd, "KEYCTL_???");
	}

	switch (cmd) {
	case KEYCTL_GET_KEYRING_ID:
		keyctl_get_keyring_id(tcp, arg2, arg3);
		break;

	case KEYCTL_JOIN_SESSION_KEYRING:
		tprints_arg_next_name("name");
		printstr(tcp, arg2);
		break;

	case KEYCTL_UPDATE:
		keyctl_update_key(tcp, arg2, arg3, arg4);
		break;

	case KEYCTL_REVOKE:
	case KEYCTL_CLEAR:
	case KEYCTL_INVALIDATE:
	case KEYCTL_ASSUME_AUTHORITY:
		tprints_arg_next_name("key");
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
		tprints_arg_next_name("reqkey_defl");
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
		tprints_arg_next_name("arg2");
		PRINT_VAL_X(arg2);

		tprints_arg_next_name("arg3");
		PRINT_VAL_X(arg3);

		tprints_arg_next_name("arg4");
		PRINT_VAL_X(arg4);

		tprints_arg_next_name("arg5");
		PRINT_VAL_X(arg5);
		break;
	}

	return RVAL_DECODED;
}
