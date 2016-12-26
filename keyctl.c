/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

typedef int32_t key_serial_t;

#include "xlat/key_spec.h"

struct keyctl_dh_params {
	int32_t private;
	int32_t prime;
	int32_t base;
};

static void
print_keyring_serial_number(key_serial_t id)
{
	const char *str = xlookup(key_spec, (unsigned int) id);

	if (str)
		tprints(str);
	else
		tprintf("%d", id);
}

SYS_FUNC(add_key)
{
	/* type */
	printstr(tcp, tcp->u_arg[0]);
	/* description */
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	/* payload */
	tprints(", ");
	printstrn(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	/* payload length */
	tprintf(", %" PRI_klu ", ", tcp->u_arg[3]);
	/* keyring serial number */
	print_keyring_serial_number(tcp->u_arg[4]);

	return RVAL_DECODED;
}

SYS_FUNC(request_key)
{
	/* type */
	printstr(tcp, tcp->u_arg[0]);
	/* description */
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	/* callout_info */
	tprints(", ");
	printstr(tcp, tcp->u_arg[2]);
	/* keyring serial number */
	tprints(", ");
	print_keyring_serial_number(tcp->u_arg[3]);

	return RVAL_DECODED;
}

static void
keyctl_get_keyring_id(struct tcb *tcp, key_serial_t id, int create)
{
	print_keyring_serial_number(id);
	tprintf(", %d", create);
}

static void
keyctl_update_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		  kernel_ulong_t len)
{
	print_keyring_serial_number(id);
	tprints(", ");
	printstrn(tcp, addr, len);
	tprintf(", %llu", zero_extend_signed_to_ull(len));
}

static void
keyctl_handle_key_key(struct tcb *tcp, key_serial_t id1, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprints(", ");
	print_keyring_serial_number(id2);
}

static void
keyctl_read_key(struct tcb *tcp, key_serial_t id, kernel_ulong_t addr,
		kernel_ulong_t len, bool has_nul)
{
	if (entering(tcp)) {
		print_keyring_serial_number(id);
		tprints(", ");
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
		tprintf(", %llu", zero_extend_signed_to_ull(len));
	}
}

static void
keyctl_keyring_search(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr1,
		      kernel_ulong_t addr2, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprints(", ");
	printstr(tcp, addr1);
	tprints(", ");
	printstr(tcp, addr2);
	tprints(", ");
	print_keyring_serial_number(id2);
}

static void
keyctl_chown_key(struct tcb *tcp, key_serial_t id, unsigned user,
		 unsigned group)
{
	print_keyring_serial_number(id);
	printuid(", ", user);
	printuid(", ", group);
}

static void
keyctl_instantiate_key(struct tcb *tcp, key_serial_t id1, kernel_ulong_t addr,
		       kernel_ulong_t len, key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprints(", ");
	printstrn(tcp, addr, len);
	tprintf(", %llu, ", zero_extend_signed_to_ull(len));
	print_keyring_serial_number(id2);
}

static void
keyctl_instantiate_key_iov(struct tcb *tcp, key_serial_t id1,
			   kernel_ulong_t addr, kernel_ulong_t len,
			   key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprints(", ");
	tprint_iov(tcp, len, addr, IOV_DECODE_STR);
	tprintf(", %llu, ", zero_extend_signed_to_ull(len));
	print_keyring_serial_number(id2);
}

static void
keyctl_negate_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  key_serial_t id2)
{
	print_keyring_serial_number(id1);
	tprintf(", %u, ", timeout);
	print_keyring_serial_number(id2);
}

static void
keyctl_reject_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  unsigned error, key_serial_t id2)
{
	const char *err_str = err_name(error);

	print_keyring_serial_number(id1);
	tprintf(", %u, ", timeout);

	if (err_str)
		tprintf("%s, ", err_str);
	else
		tprintf("%u, ", error);

	print_keyring_serial_number(id2);
}

static void
keyctl_set_timeout(struct tcb *tcp, key_serial_t id, unsigned timeout)
{
	print_keyring_serial_number(id);
	tprintf(", %u", timeout);
}

static void
keyctl_get_persistent(struct tcb *tcp, unsigned uid, key_serial_t id)
{
	printuid("", uid);
	tprints(", ");
	print_keyring_serial_number(id);
}

#include "xlat/key_perms.h"

static void
keyctl_setperm_key(struct tcb *tcp, key_serial_t id, uint32_t perm)
{
	print_keyring_serial_number(id);
	tprints(", ");
	printflags(key_perms, perm, "KEY_???");
}

static void
print_dh_params(struct tcb *tcp, kernel_ulong_t addr)
{
	struct keyctl_dh_params params;

	if (umove_or_printaddr(tcp, addr, &params))
		return;

	tprints("{private=");
	print_keyring_serial_number(params.private);
	tprints(", prime=");
	print_keyring_serial_number(params.prime);
	tprints(", base=");
	print_keyring_serial_number(params.base);
	tprints("}");
}

static void
keyctl_dh_compute(struct tcb *tcp, kernel_ulong_t params, kernel_ulong_t buf,
		  kernel_ulong_t len)
{
	if (entering(tcp)) {
		print_dh_params(tcp, params);
		tprints(", ");
	} else {
		if (syserror(tcp)) {
			printaddr(buf);
		} else {
			kernel_ulong_t rval = (tcp->u_rval >= 0) &&
				((kernel_ulong_t) tcp->u_rval > len) ? len :
				(kernel_ulong_t) tcp->u_rval;
			printstrn(tcp, buf, rval);
		}
		tprintf(", %llu", zero_extend_signed_to_ull(len));
	}
}

#include "xlat/key_reqkeys.h"
#include "xlat/keyctl_commands.h"

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
			tprints(", ");
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
		printxval(key_reqkeys, arg2, "KEY_REQKEY_DEFL_???");
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
		keyctl_dh_compute(tcp, arg2, arg3, arg4);
		return 0;

	default:
		tprintf("%#" PRI_klx ", %#" PRI_klx
			", %#" PRI_klx ", %#" PRI_klx,
			arg2, arg3, arg4, arg5);
		break;
	}

	return RVAL_DECODED;
}
