#include "defs.h"

typedef int32_t key_serial_t;

#include "xlat/key_spec.h"

static void
print_keyring_serial_number(key_serial_t id)
{
	const char *str = xlookup(key_spec, id);

	if (str)
		tprints(str);
	else
		tprintf("%d", id);
}

SYS_FUNC(add_key)
{
	if (entering(tcp)) {
		/* type */
		printstr(tcp, tcp->u_arg[0], -1);
		/* description */
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
		/* payload */
		tprints(", ");
		printstr(tcp, tcp->u_arg[2], tcp->u_arg[3]);
		/* payload length */
		tprintf(", %lu, ", tcp->u_arg[3]);
		/* keyring serial number */
		print_keyring_serial_number(tcp->u_arg[4]);
	}
	return 0;
}

SYS_FUNC(request_key)
{
	if (entering(tcp)) {
		/* type */
		printstr(tcp, tcp->u_arg[0], -1);
		/* description */
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
		/* callout_info */
		tprints(", ");
		printstr(tcp, tcp->u_arg[2], -1);
		/* keyring serial number */
		tprints(", ");
		print_keyring_serial_number(tcp->u_arg[3]);
	}
	return 0;
}

static int
keyctl_get_keyring_id(struct tcb *tcp, key_serial_t id, int create)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprintf(", %d", create);
	}
	return 0;
}

static int
keyctl_join_session_keyring(struct tcb *tcp, long addr)
{
	if (entering(tcp)) {
		tprints(", ");
		printstr(tcp, addr, -1);
	}
	return 0;
}

static int
keyctl_update_key(struct tcb *tcp, key_serial_t id, long addr, long len)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprints(", ");
		printstr(tcp, addr, len);
		tprintf(", %lu", len);
	}
	return 0;
}

static int
keyctl_handle_key(struct tcb *tcp, key_serial_t id)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
	}
	return 0;
}

static int
keyctl_handle_key_key(struct tcb *tcp, key_serial_t id1, key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprints(", ");
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_read_key(struct tcb *tcp, key_serial_t id, long addr, long len)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprints(", ");
	} else {
		if (addr && syserror(tcp))
			tprintf("%#lx", addr);
		else {
			long rval = tcp->u_rval > len ?
				    len : (tcp->u_rval ? -1 : 0);
			printstr(tcp, addr, rval);
		}
		tprintf(", %lu", len);
	}
	return 0;
}

static int
keyctl_keyring_search(struct tcb *tcp, key_serial_t id1, long addr1,
		      long addr2, key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprints(", ");
		printstr(tcp, addr1, -1);
		tprints(", ");
		printstr(tcp, addr2, -1);
		tprints(", ");
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_chown_key(struct tcb *tcp, key_serial_t id, int user, int group)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprintf(", %d, %d", user, group);
	}
	return 0;
}

static int
keyctl_instantiate_key(struct tcb *tcp, key_serial_t id1, long addr,
		       long len, key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprints(", ");
		printstr(tcp, addr, len);
		tprintf(", %lu, ", len);
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_instantiate_key_iov(struct tcb *tcp, key_serial_t id1,
			   long addr, long len, key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprints(", ");
		tprint_iov(tcp, len, addr, 1);
		tprintf(", %lu, ", len);
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_negate_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprintf(", %u, ", timeout);
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_reject_key(struct tcb *tcp, key_serial_t id1, unsigned timeout,
		  unsigned error, key_serial_t id2)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id1);
		tprintf(", %u, %u, ", timeout, error);
		print_keyring_serial_number(id2);
	}
	return 0;
}

static int
keyctl_set_timeout(struct tcb *tcp, key_serial_t id, unsigned timeout)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprintf(", %u", timeout);
	}
	return 0;
}

static int
keyctl_get_persistent(struct tcb *tcp, int uid, key_serial_t id)
{
	if (entering(tcp)) {
		tprintf(", %d, ", uid);
		print_keyring_serial_number(id);
	}
	return 0;
}

#include "xlat/key_perms.h"

static int
keyctl_setperm_key(struct tcb *tcp, key_serial_t id, uint32_t perm)
{
	if (entering(tcp)) {
		tprints(", ");
		print_keyring_serial_number(id);
		tprints(", ");
		printflags(key_perms, perm, "KEY_???");
	}
	return 0;
}

#include "xlat/key_reqkeys.h"

static int
keyctl_set_reqkey_keyring(struct tcb *tcp, int reqkey)
{
	if (entering(tcp)) {
		tprints(", ");
		printxval(key_reqkeys, reqkey, "KEY_REQKEY_DEFL_???");
	}
	return 0;
}

#include "xlat/keyctl_commands.h"

SYS_FUNC(keyctl)
{
	int cmd = tcp->u_arg[0];

	if (entering(tcp))
		printxval(keyctl_commands, cmd, "KEYCTL_???");

	switch (cmd) {
	case KEYCTL_GET_KEYRING_ID:
		return keyctl_get_keyring_id(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	case KEYCTL_JOIN_SESSION_KEYRING:
		return keyctl_join_session_keyring(tcp, tcp->u_arg[1]);

	case KEYCTL_UPDATE:
		return keyctl_update_key(tcp, tcp->u_arg[1],
					 tcp->u_arg[2], tcp->u_arg[3]);

	case KEYCTL_REVOKE:
	case KEYCTL_CLEAR:
	case KEYCTL_INVALIDATE:
	case KEYCTL_ASSUME_AUTHORITY:
		return keyctl_handle_key(tcp, tcp->u_arg[1]);

	case KEYCTL_LINK:
	case KEYCTL_UNLINK:
		return keyctl_handle_key_key(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	case KEYCTL_DESCRIBE:
	case KEYCTL_READ:
	case KEYCTL_GET_SECURITY:
		return keyctl_read_key(tcp, tcp->u_arg[1],
				       tcp->u_arg[2], tcp->u_arg[3]);

	case KEYCTL_SEARCH:
		return keyctl_keyring_search(tcp, tcp->u_arg[1], tcp->u_arg[2],
					     tcp->u_arg[3], tcp->u_arg[4]);

	case KEYCTL_CHOWN:
		return keyctl_chown_key(tcp, tcp->u_arg[1],
					tcp->u_arg[2], tcp->u_arg[3]);

	case KEYCTL_SETPERM:
		return keyctl_setperm_key(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	case KEYCTL_INSTANTIATE:
		return keyctl_instantiate_key(tcp, tcp->u_arg[1], tcp->u_arg[2],
					      tcp->u_arg[3], tcp->u_arg[4]);

	case KEYCTL_NEGATE:
		return keyctl_negate_key(tcp, tcp->u_arg[1],
					 tcp->u_arg[2], tcp->u_arg[3]);

	case KEYCTL_SET_REQKEY_KEYRING:
		return keyctl_set_reqkey_keyring(tcp, tcp->u_arg[1]);

	case KEYCTL_SET_TIMEOUT:
		return keyctl_set_timeout(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	case KEYCTL_SESSION_TO_PARENT:
		return 0;

	case KEYCTL_REJECT:
		return keyctl_reject_key(tcp, tcp->u_arg[1], tcp->u_arg[2],
					 tcp->u_arg[3], tcp->u_arg[4]);

	case KEYCTL_INSTANTIATE_IOV:
		return keyctl_instantiate_key_iov(tcp, tcp->u_arg[1],
						  tcp->u_arg[2], tcp->u_arg[3],
						  tcp->u_arg[4]);

	case KEYCTL_GET_PERSISTENT:
		return keyctl_get_persistent(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	default:
		if (entering(tcp))
			tprintf(", %#lx, %#lx, %#lx, %#lx",
				tcp->u_arg[1], tcp->u_arg[2],
				tcp->u_arg[3], tcp->u_arg[4]);
	}

	return 0;
}
