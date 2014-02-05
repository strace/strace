#include "defs.h"
#include <linux/keyctl.h>

typedef int32_t key_serial_t;

static const struct xlat key_spec[] = {
	XLAT(KEY_SPEC_THREAD_KEYRING),
	XLAT(KEY_SPEC_PROCESS_KEYRING),
	XLAT(KEY_SPEC_SESSION_KEYRING),
	XLAT(KEY_SPEC_USER_KEYRING),
	XLAT(KEY_SPEC_USER_SESSION_KEYRING),
	XLAT(KEY_SPEC_GROUP_KEYRING),
	XLAT(KEY_SPEC_REQKEY_AUTH_KEY),
	XLAT(KEY_SPEC_REQUESTOR_KEYRING),
	XLAT_END
};

static void
print_keyring_serial_number(key_serial_t id)
{
	const char *str = xlookup(key_spec, id);

	if (str)
		tprints(str);
	else
		tprintf("%d", id);
}

int
sys_add_key(struct tcb *tcp)
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

int
sys_request_key(struct tcb *tcp)
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

#define KEY_POS_VIEW	0x01000000
#define KEY_POS_READ	0x02000000
#define KEY_POS_WRITE	0x04000000
#define KEY_POS_SEARCH	0x08000000
#define KEY_POS_LINK	0x10000000
#define KEY_POS_SETATTR	0x20000000
#define KEY_POS_ALL	0x3f000000
#define KEY_USR_VIEW	0x00010000
#define KEY_USR_READ	0x00020000
#define KEY_USR_WRITE	0x00040000
#define KEY_USR_SEARCH	0x00080000
#define KEY_USR_LINK	0x00100000
#define KEY_USR_SETATTR	0x00200000
#define KEY_USR_ALL	0x003f0000
#define KEY_GRP_VIEW	0x00000100
#define KEY_GRP_READ	0x00000200
#define KEY_GRP_WRITE	0x00000400
#define KEY_GRP_SEARCH	0x00000800
#define KEY_GRP_LINK	0x00001000
#define KEY_GRP_SETATTR	0x00002000
#define KEY_GRP_ALL	0x00003f00
#define KEY_OTH_VIEW	0x00000001
#define KEY_OTH_READ	0x00000002
#define KEY_OTH_WRITE	0x00000004
#define KEY_OTH_SEARCH	0x00000008
#define KEY_OTH_LINK	0x00000010
#define KEY_OTH_SETATTR	0x00000020
#define KEY_OTH_ALL	0x0000003f

static const struct xlat key_perms[] = {
	XLAT(KEY_POS_VIEW),
	XLAT(KEY_POS_READ),
	XLAT(KEY_POS_WRITE),
	XLAT(KEY_POS_SEARCH),
	XLAT(KEY_POS_LINK),
	XLAT(KEY_POS_SETATTR),
	XLAT(KEY_POS_ALL),
	XLAT(KEY_USR_VIEW),
	XLAT(KEY_USR_READ),
	XLAT(KEY_USR_WRITE),
	XLAT(KEY_USR_SEARCH),
	XLAT(KEY_USR_LINK),
	XLAT(KEY_USR_SETATTR),
	XLAT(KEY_USR_ALL),
	XLAT(KEY_GRP_VIEW),
	XLAT(KEY_GRP_READ),
	XLAT(KEY_GRP_WRITE),
	XLAT(KEY_GRP_SEARCH),
	XLAT(KEY_GRP_LINK),
	XLAT(KEY_GRP_SETATTR),
	XLAT(KEY_GRP_ALL),
	XLAT(KEY_OTH_VIEW),
	XLAT(KEY_OTH_READ),
	XLAT(KEY_OTH_WRITE),
	XLAT(KEY_OTH_SEARCH),
	XLAT(KEY_OTH_LINK),
	XLAT(KEY_OTH_SETATTR),
	XLAT(KEY_OTH_ALL),
	XLAT_END
};

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

static const struct xlat key_reqkeys[] = {
	XLAT(KEY_REQKEY_DEFL_NO_CHANGE),
	XLAT(KEY_REQKEY_DEFL_DEFAULT),
	XLAT(KEY_REQKEY_DEFL_THREAD_KEYRING),
	XLAT(KEY_REQKEY_DEFL_PROCESS_KEYRING),
	XLAT(KEY_REQKEY_DEFL_SESSION_KEYRING),
	XLAT(KEY_REQKEY_DEFL_USER_KEYRING),
	XLAT(KEY_REQKEY_DEFL_USER_SESSION_KEYRING),
	XLAT(KEY_REQKEY_DEFL_GROUP_KEYRING),
	XLAT(KEY_REQKEY_DEFL_REQUESTOR_KEYRING),
	XLAT_END
};

static int
keyctl_set_reqkey_keyring(struct tcb *tcp, int reqkey)
{
	if (entering(tcp)) {
		tprints(", ");
		printxval(key_reqkeys, reqkey, "KEY_REQKEY_DEFL_???");
	}
	return 0;
}

static const struct xlat keyctl_commands[] = {
	XLAT(KEYCTL_GET_KEYRING_ID),
	XLAT(KEYCTL_JOIN_SESSION_KEYRING),
	XLAT(KEYCTL_UPDATE),
	XLAT(KEYCTL_REVOKE),
	XLAT(KEYCTL_CHOWN),
	XLAT(KEYCTL_SETPERM),
	XLAT(KEYCTL_DESCRIBE),
	XLAT(KEYCTL_CLEAR),
	XLAT(KEYCTL_LINK),
	XLAT(KEYCTL_UNLINK),
	XLAT(KEYCTL_SEARCH),
	XLAT(KEYCTL_READ),
	XLAT(KEYCTL_INSTANTIATE),
	XLAT(KEYCTL_NEGATE),
	XLAT(KEYCTL_SET_REQKEY_KEYRING),
	XLAT(KEYCTL_SET_TIMEOUT),
	XLAT(KEYCTL_ASSUME_AUTHORITY),
	XLAT(KEYCTL_GET_SECURITY),
	XLAT(KEYCTL_SESSION_TO_PARENT),
	XLAT(KEYCTL_REJECT),
	XLAT(KEYCTL_INSTANTIATE_IOV),
	XLAT(KEYCTL_INVALIDATE),
	XLAT(KEYCTL_GET_PERSISTENT),
	XLAT_END
};

int
sys_keyctl(struct tcb *tcp)
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
