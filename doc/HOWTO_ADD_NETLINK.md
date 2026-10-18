# How to Add Netlink Decoders to strace

This guide explains how to add support for decoding new netlink protocol
families in strace.

## Overview

A netlink decoder is a function that decodes and prints the **payload**
of netlink messages for a specific protocol family. The decoder is
**not** responsible for:
- Printing the netlink message header (`struct nlmsghdr`)
- Determining the netlink protocol family
- Handling `NLMSG_ERROR` messages
- Handling `NLMSG_DONE` messages (framework handles integer payload)

These are handled automatically by the netlink framework in
`src/netlink.c` before your decoder is called. Your decoder's sole job
is to decode and print the message payload based on the message type.

### Return Value

Netlink decoders return `bool`:
- `true` — The message was successfully decoded
- `false` — Unknown message type; framework will fall through to hex dump

## Netlink Decoding Pipeline

Understanding how strace processes netlink messages helps you write
effective decoders.

### How Netlink Decoding Gets Triggered

Netlink decoding is triggered from two call paths:

**Path 1: sendto/send/recv/recvfrom** (via `src/net.c`):
- `SYS_FUNC(sendto)`, `SYS_FUNC(recvfrom)`, etc. call `decode_sockbuf()`
- `decode_sockbuf()` calls `getfdproto(tcp, fd)` to check if the socket
  protocol is `SOCK_PROTO_NETLINK`
- If so, calls `decode_netlink(tcp, fd, addr, len)`

**Path 2: sendmsg/recvmsg** (via `src/msghdr.c`):
- `print_struct_msghdr()` checks if `family == AF_NETLINK`
- If so, selects `iov_decode_netlink` as the iov decode function
- `iov_decode_netlink()` calls `decode_netlink(tcp, fd, addr, len)`

### Protocol Family Detection

`decode_netlink()` in `src/netlink.c` determines the netlink protocol
family by calling `get_fd_nl_family(tcp, fd)`, which:
1. Gets the socket inode via `getfdinode(tcp, fd)`
2. Looks up socket address info via `get_sockaddr_by_inode(tcp, fd, inode)`
3. Parses the protocol family from `/proc` socket information
4. Returns the `NETLINK_*` protocol family constant

The protocol family comes from **`/proc` socket information**, not from
the syscall arguments.

### Message Processing

`decode_netlink()` processes messages in this order:

1. **Special case: NETLINK_KOBJECT_UEVENT** — If the protocol family is
   `NETLINK_KOBJECT_UEVENT`, calls `decode_netlink_kobject_uevent()`
   directly and returns. Kobject uevent messages do not follow the
   `struct nlmsghdr` structure.

2. **For all other protocols** — Iterates through a sequence of
   `struct nlmsghdr` messages:
   - Fetches each `nlmsghdr` using `fetch_nlmsghdr()`
   - Calls `decode_nlmsghdr_with_payload()` for each message

3. **`decode_nlmsghdr_with_payload()`** does:
   - Calls `print_nlmsghdr()` to print the header (nlmsg_len,
     nlmsg_type, nlmsg_flags, nlmsg_seq, nlmsg_pid)
   - If payload exists beyond `NLMSG_HDRLEN`, calls `decode_payload()`

4. **`decode_payload()`** handles:
   - `NLMSG_ERROR` — calls `decode_nlmsgerr()`
   - `NLMSG_DONE` with `len == sizeof(int)` — prints the integer
   - For `nlmsg_type >= NLMSG_MIN_TYPE` or `nlmsg_type == NLMSG_DONE` —
     dispatches to `netlink_decoders[family]`
   - If decoder returns `false` or doesn't exist — hex dumps payload

### Three Dispatch Arrays

The netlink framework maintains three parallel dispatch arrays, all
indexed by netlink protocol family:

1. **`netlink_decoders[]`** (in `src/netlink.c`) — Payload decoders
   - 6 entries: CRYPTO, GENERIC, NETFILTER, ROUTE, SELINUX, SOCK_DIAG
   - Note: NETLINK_KOBJECT_UEVENT is NOT in this array (special case)

2. **`nlmsg_types[]`** (in `src/netlink.c`) — Message type printing
   - 8 entries: AUDIT, CRYPTO, GENERIC, NETFILTER, ROUTE, SELINUX,
     SOCK_DIAG, XFRM
   - Some protocols (AUDIT, XFRM) have type xlats but no payload decoder

3. **`nlmsg_flags[]`** (in `src/netlink.c`) — Message flags decoding
   - 6 entries: CRYPTO, GENERIC, NETFILTER, ROUTE, SOCK_DIAG, XFRM

When adding a new protocol family, you may need to add entries to all
three arrays depending on what level of decoding you want to provide.

### NETLINK_KOBJECT_UEVENT Special Case

This protocol family has a unique decoder:
- Messages do NOT follow the `struct nlmsghdr` structure
- Decoder has a different signature: `void decode_netlink_kobject_uevent(struct tcb *, kernel_ulong_t addr, unsigned int len)`
- Called directly before the nlmsghdr iteration loop
- Declared in `src/defs.h` with its own declaration (not using
  `DECL_NETLINK`)

## Adding a New Protocol Family Decoder

### Step 1: Identify the Protocol Family

Find the protocol family constant in kernel headers:

```bash
grep -r "NETLINK_" /usr/include/linux/netlink.h
```

Example:

```c
#define NETLINK_ROUTE          0
#define NETLINK_FIREWALL       3
#define NETLINK_CRYPTO        21
#define NETLINK_YOUR_SUBSYS   ??  /* New family */
```

### Step 2: Create the Decoder File

Create `src/netlink_<family>.c`:

```c
/*
 * Copyright (c) YEAR YOUR NAME <your@email>
 * Copyright (c) YEAR-YEAR The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"
#include "nlattr.h"
#include <linux/netlink.h>
#include <linux/your_subsys.h>

bool
decode_netlink_your_subsys(struct tcb *const tcp,
                            const struct nlmsghdr *const nlmsghdr,
                            const kernel_ulong_t addr,
                            const unsigned int len)
{
	switch (nlmsghdr->nlmsg_type) {
	case YOUR_SUBSYS_MSG_GET:
		/* Decode YOUR_SUBSYS_MSG_GET payload */
		/* ... */
		break;
	case YOUR_SUBSYS_MSG_SET:
		/* Decode YOUR_SUBSYS_MSG_SET payload */
		/* ... */
		break;
	default:
		return false;  /* Unknown message type */
	}

	return true;
}
```

**Key points:**
- Include `netlink.h` and `nlattr.h`
- Decoder receives `nlmsghdr`, payload `addr`, and payload `len`
- Payload `addr` points AFTER the `nlmsghdr` (already skipped by framework)
- Return `true` for successfully decoded messages
- Return `false` for unknown message types (triggers hex dump fallback)

### Step 3: Declare with DECL_NETLINK

Add declaration to `src/defs.h` in the `DECL_NETLINK` section:

```c
DECL_NETLINK(your_subsys);
```

This macro expands to:

```c
extern bool
decode_netlink_your_subsys(struct tcb *, const struct nlmsghdr *,
                            kernel_ulong_t addr, unsigned int len);
```

### Step 4: Add to Dispatch Arrays

Edit `src/netlink.c`:

**Add to `netlink_decoders[]` array** (for payload decoding):

```c
static const netlink_decoder_t netlink_decoders[] = {
	[NETLINK_CRYPTO] = decode_netlink_crypto,
	/* ... existing entries ... */
	[NETLINK_YOUR_SUBSYS] = decode_netlink_your_subsys,
};
```

**Add to `nlmsg_types[]` array** (for message type printing):

```c
static const struct xlat_data nlmsg_types[] = {
	/* ... existing entries ... */
	[NETLINK_YOUR_SUBSYS] = { NULL, nl_your_subsys_types, "YOUR_SUBSYS_MSG_???" },
};
```

**Add to `nlmsg_flags[]` array** (for message flags decoding):

```c
static const nlmsg_flags_decoder_t nlmsg_flags[] = {
	/* ... existing entries ... */
	[NETLINK_YOUR_SUBSYS] = decode_nlmsg_flags_your_subsys,
};
```

If your protocol uses standard netlink flags, you can omit the
`nlmsg_flags[]` entry. If it has protocol-specific flags, implement a
custom flags decoder function.

### Step 5: Create Xlat Files

Create xlat files for message types and attributes:

**`src/xlat/nl_your_subsys_types.in`** (message types):

```
#From linux/your_subsys.h
YOUR_SUBSYS_MSG_GET
YOUR_SUBSYS_MSG_SET
YOUR_SUBSYS_MSG_NEW
YOUR_SUBSYS_MSG_DEL
```

**`src/xlat/nl_your_subsys_attrs.in`** (if using netlink attributes):

```
#From linux/your_subsys.h
YOUR_SUBSYS_ATTR_NAME
YOUR_SUBSYS_ATTR_VALUE
YOUR_SUBSYS_ATTR_FLAGS
```

See [README-xlat.md](README-xlat.md) for xlat file format details.

### Step 6: Update Build System

Edit `src/Makefile.am`:

Add your decoder to the `libstrace_a_SOURCES` list:

```makefile
libstrace_a_SOURCES = \
	...
	netlink_your_subsys.c \
	...
```

Xlat files (`src/xlat/*.in`) are automatically processed by the build
system.

### Step 7: Write Tests

Create `tests/netlink_your_subsys.c`:

```c
#include "tests.h"
#include "test_netlink.h"
#include <linux/netlink.h>
#include <linux/your_subsys.h>

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_YOUR_SUBSYS);

	/* Construct and send netlink message */
	struct {
		struct nlmsghdr nlh;
		struct your_subsys_msg msg;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = YOUR_SUBSYS_MSG_GET,
			.nlmsg_flags = NLM_F_REQUEST,
		},
		.msg = { /* ... */ },
	};

	/* Send message */
	sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);

	/* Print expected strace output */
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=YOUR_SUBSYS_MSG_GET, "
	       "nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}, ...], "
	       "%u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned)sizeof(req), (unsigned)sizeof(req),
	       sprintrc(-1));

	puts("+++ exited with 0 +++");
	return 0;
}
```

Register the test in `tests/gen_tests.in`:

```
netlink_your_subsys +netlink_sock_diag.test
```

The `+netlink_sock_diag.test` pattern uses the shared netlink test
script. See [HOWTO_TEST.md](HOWTO_TEST.md) for comprehensive testing
guidance.

### Step 8: Update NEWS

Add an entry to the `NEWS` file documenting the new netlink protocol
support.

## Decoder Implementation Patterns

### Simple Message Type Switch

The simplest pattern — decode based on message type:

```c
bool
decode_netlink_your_subsys(struct tcb *const tcp,
                            const struct nlmsghdr *const nlmsghdr,
                            const kernel_ulong_t addr,
                            const unsigned int len)
{
	switch (nlmsghdr->nlmsg_type) {
	case YOUR_SUBSYS_MSG_SET: {
		struct your_subsys_config cfg;

		if (len < sizeof(cfg))
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		else if (!umove_or_printaddr(tcp, addr, &cfg)) {
			tprint_struct_begin();
			PRINT_FIELD_U(cfg, version);
			PRINT_FIELD_X(cfg, flags);
			tprint_struct_end();
		}
		break;
	}
	default:
		return false;
	}

	return true;
}
```

See `src/netlink_selinux.c` for a real example.

### Fixed Header + Attributes

Decode a fixed-size protocol-specific header, then netlink attributes:

```c
bool
decode_netlink_your_subsys(struct tcb *const tcp,
                            const struct nlmsghdr *const nlmsghdr,
                            const kernel_ulong_t addr,
                            const unsigned int len)
{
	struct your_subsys_msg msg;

	if (len < sizeof(msg))
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	else if (!umove_or_printaddr(tcp, addr, &msg)) {
		tprint_struct_begin();
		PRINT_FIELD_XVAL(msg, family, addrfams, "AF_???");
		PRINT_FIELD_U(msg, version);
		tprint_struct_end();

		const size_t offset = NLMSG_ALIGN(sizeof(msg));
		if (len > offset) {
			tprint_array_next();
			decode_nlattr(tcp, addr + offset, len - offset,
			              nl_your_subsys_attrs, "YOUR_SUBSYS_ATTR_???",
			              your_subsys_nla_decoders,
			              ARRAY_SIZE(your_subsys_nla_decoders), NULL);
		}
	}

	return true;
}
```

See `src/netlink_netfilter.c` for a real example.

### Sub-Dispatch by Message Type

For protocol families with many message types, use a decoder array
indexed by message type:

```c
/* Declare sub-decoders with a custom macro */
#define DECL_YOUR_SUBSYS_DECODER(name) \
void name(struct tcb *tcp, const struct nlmsghdr *nlmsghdr, \
          uint8_t family, kernel_ulong_t addr, unsigned int len)

DECL_YOUR_SUBSYS_DECODER(decode_get_msg);
DECL_YOUR_SUBSYS_DECODER(decode_set_msg);

static const struct {
	void (*decoder)(struct tcb *, const struct nlmsghdr *,
	                uint8_t, kernel_ulong_t, unsigned int);
} your_subsys_decoders[] = {
	[YOUR_SUBSYS_MSG_GET - YOUR_SUBSYS_BASE] = { decode_get_msg },
	[YOUR_SUBSYS_MSG_SET - YOUR_SUBSYS_BASE] = { decode_set_msg },
};

bool
decode_netlink_your_subsys(struct tcb *const tcp,
                            const struct nlmsghdr *const nlmsghdr,
                            const kernel_ulong_t addr,
                            const unsigned int len)
{
	uint8_t family;
	if (len < sizeof(family))
		return false;

	if (umove(tcp, addr, &family))
		return true;

	const unsigned int index = nlmsghdr->nlmsg_type - YOUR_SUBSYS_BASE;
	if (index < ARRAY_SIZE(your_subsys_decoders)
	    && your_subsys_decoders[index].decoder) {
		your_subsys_decoders[index].decoder(tcp, nlmsghdr, family,
		                                     addr, len);
		return true;
	}

	return false;
}
```

See `src/netlink_route.c` and `src/netlink_route.h` for a real example
with `DECL_NETLINK_ROUTE_DECODER` macro.

### Sub-Dispatch by Address Family

For protocol families that further dispatch by address family (like
SOCK_DIAG):

```c
#define DECL_YOUR_SUBSYS_DIAG_DECODER(name) \
void name(struct tcb *tcp, const struct nlmsghdr *nlmsghdr, \
          uint8_t family, kernel_ulong_t addr, unsigned int len)

DECL_YOUR_SUBSYS_DIAG_DECODER(decode_af_inet_request);
DECL_YOUR_SUBSYS_DIAG_DECODER(decode_af_inet_response);

static const struct {
	DECL_YOUR_SUBSYS_DIAG_DECODER((*request));
	DECL_YOUR_SUBSYS_DIAG_DECODER((*response));
} diag_decoders[] = {
	[AF_INET] = { decode_af_inet_request, decode_af_inet_response },
	[AF_INET6] = { decode_af_inet_request, decode_af_inet_response },
	/* ... */
};

bool
decode_netlink_your_subsys(struct tcb *const tcp,
                            const struct nlmsghdr *const nlmsghdr,
                            const kernel_ulong_t addr,
                            const unsigned int len)
{
	uint8_t family;
	if (len < sizeof(family))
		return false;

	if (umove(tcp, addr, &family))
		return true;

	if (family >= ARRAY_SIZE(diag_decoders))
		return false;

	const bool is_request = nlmsghdr->nlmsg_flags & NLM_F_REQUEST;
	const DECL_YOUR_SUBSYS_DIAG_DECODER((*decoder)) =
		is_request ? diag_decoders[family].request
		           : diag_decoders[family].response;

	if (decoder) {
		decoder(tcp, nlmsghdr, family, addr, len);
		return true;
	}

	return false;
}
```

See `src/netlink_sock_diag.c` and `src/netlink_sock_diag.h` for a real
example with `DECL_NETLINK_DIAG_DECODER` macro.

### Generic Netlink Family Dispatch

For NETLINK_GENERIC, dispatch is based on dynamically-assigned generic
netlink family IDs:

```c
#define DECL_GENL_DECODER(name) \
void name(struct tcb *tcp, const struct genlmsghdr *hdr, \
          kernel_ulong_t addr, unsigned int len)

DECL_GENL_DECODER(decode_your_genl_family);

static const struct genl_family genl_families[] = {
	{ "your_family_name", decode_your_genl_family, 0 },
};

/* In decode_netlink_generic: */
const struct genl_decoder *decoder = lookup_genl_decoder(nlmsghdr->nlmsg_type);
if (decoder)
	decoder->decoder(tcp, &ghdr, addr + offset, len - offset);
```

See `src/netlink_generic.c`, `src/netlink_nlctrl.c`, and
`src/netlink_generic.h` for real examples with
`DECL_NETLINK_GENERIC_DECODER` macro.

### Handling Short/Truncated Messages

Always check message length before accessing structures:

```c
struct your_msg msg;

if (len < sizeof(msg))
	printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
else if (!umove_or_printaddr(tcp, addr, &msg)) {
	/* Decode msg fields */
}
```

The `printstr_ex()` fallback hex-dumps truncated data. The
`umove_or_printaddr()` prints the address if the memory read fails.

### Unknown Message Types

For unrecognized message types within your protocol family, return
`false` to trigger the framework's hex dump fallback:

```c
default:
	return false;
```

The framework will hex-dump the payload.

## Decoding Netlink Attributes

Many netlink protocols use TLV-encoded attributes (`struct nlattr`).
The `decode_nlattr()` function in `src/nlattr.c` provides the decoding
engine.

### decode_nlattr() Usage

**Signature:**

```c
void decode_nlattr(struct tcb *tcp,
                   kernel_ulong_t addr,
                   unsigned int len,
                   const struct xlat *table,
                   const char *dflt,
                   const nla_decoder_t *decoders,
                   unsigned int size,
                   const void *opaque_data);
```

**Parameters:**
- `addr` — Address of first nlattr
- `len` — Length of nlattr sequence
- `table` — Xlat table for attribute type names (can be `NULL`)
- `dflt` — Default string for unknown types (e.g., `"ATTR_???"`)
- `decoders` — Array of decoder function pointers indexed by attribute
  type
- `size` — Size of decoders array
- `opaque_data` — Passed through to decoder functions

**Example:**

```c
static const nla_decoder_t your_subsys_nla_decoders[] = {
	[YOUR_SUBSYS_ATTR_NAME] = decode_nla_str,
	[YOUR_SUBSYS_ATTR_VALUE] = decode_nla_u32,
	[YOUR_SUBSYS_ATTR_FLAGS] = NULL,  /* Use default hex decoder */
};

/* In your decoder: */
decode_nlattr(tcp, addr, len,
              nl_your_subsys_attrs, "YOUR_SUBSYS_ATTR_???",
              your_subsys_nla_decoders,
              ARRAY_SIZE(your_subsys_nla_decoders), NULL);
```

The `decoders` array is indexed by `(nla->nla_type & NLA_TYPE_MASK)`.
If a decoder returns `false` or is `NULL`, the attribute data is
hex-dumped.

### Writing Custom Attribute Decoders

**nla_decoder_t signature:**

```c
typedef bool (*nla_decoder_t)(struct tcb *, kernel_ulong_t addr,
                              unsigned int len, const void *opaque_data);
```

Returns `true` if successfully decoded, `false` for hex dump fallback.

**Example:**

```c
static bool
decode_your_custom_attr(struct tcb *const tcp,
                        const kernel_ulong_t addr,
                        const unsigned int len,
                        const void *const opaque_data)
{
	struct your_attr_data data;

	if (len < sizeof(data))
		return false;

	if (umove_or_printaddr(tcp, addr, &data))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(data, field1);
	PRINT_FIELD_X(data, field2);
	tprint_struct_end();

	return true;
}
```

For the full list of built-in attribute decoders (`decode_nla_str`,
`decode_nla_u32`, `decode_nla_in_addr`, etc.), see
[DECODER_API.md](DECODER_API.md).

## Testing Netlink Decoders

### The sendto-and-match Pattern

Most netlink tests use this pattern:
1. Create a real netlink socket via `create_nl_socket()`
2. Construct netlink messages in userspace
3. Send messages via `sendto()` with `MSG_DONTWAIT`
4. Print expected strace output to stdout
5. Test framework compares strace's actual output with expected output

Tests do **not** use injection. They send real messages to real sockets.
The messages typically return errors (since they're nonsensical), but
the test only verifies strace's decoding of the message content.

**Example:**

```c
#include "tests.h"
#include "test_netlink.h"
#include <linux/netlink.h>
#include <linux/your_subsys.h>

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_YOUR_SUBSYS);

	struct {
		struct nlmsghdr nlh;
		struct your_subsys_msg msg;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = YOUR_SUBSYS_MSG_GET,
			.nlmsg_flags = NLM_F_REQUEST,
		},
		.msg = { .field = 42 },
	};

	sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=YOUR_SUBSYS_MSG_GET, "
	       "nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}, "
	       "{field=42}], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned)sizeof(req), (unsigned)sizeof(req),
	       sprintrc(-1));

	puts("+++ exited with 0 +++");
	return 0;
}
```

### Shared Test Script: netlink_sock_diag.test

Almost all netlink tests use `+netlink_sock_diag.test` as their test
runner in `tests/gen_tests.in`:

```
netlink_your_subsys +netlink_sock_diag.test
```

This shared script:
1. Runs `../netlink_netlink_diag` to verify netlink socket availability
2. Calls `run_strace_match_diff -e trace=sendto "$@"`

The `run_strace_match_diff` function runs the test program, captures its
stdout as expected output, runs strace, and compares the outputs.

### Test Helpers

**`create_nl_socket(NETLINK_FAMILY)`** (in `tests/create_nl_socket.c`):
- Creates a netlink socket
- Calls `perror_msg_and_skip()` if socket creation or bind fails
- Returns the file descriptor

**`skip_if_unavailable(path)`** (in `tests/tests.h`):
- Skips the test if the specified path does not exist
- Used to check for `/proc/self/fd/` availability

**`test_netlink.h` macros**:
- `TEST_NETLINK_()` — Construct nlmsghdr, call sendto, print expected
  output
- `TEST_NETLINK_OBJECT_EX_()` — Test three scenarios: truncated, short
  read, full object
- Many convenience wrappers

**`test_nlattr.h` macros**:
- `TEST_NLATTR_()` — Send message with an nlattr and verify decoding
- `TEST_NLATTR_OBJECT()` — Test nlattr with object payload
- `TEST_NESTED_NLATTR_*()` — Test nested nlattr structures
- Many pre-defined integer checkers

See [HOWTO_TEST.md](HOWTO_TEST.md) for comprehensive testing guidance.

### gen_tests.in Registration

Register your test in `tests/gen_tests.in`:

```
netlink_your_subsys +netlink_sock_diag.test
```

For tests with special options:

```
netlink_your_subsys +netlink_sock_diag.test -a22
```

### Common Test Guards

**Skip if /proc unavailable:**

```c
skip_if_unavailable("/proc/self/fd/");
```

**Skip if socket creation fails:**

```c
int fd = create_nl_socket(NETLINK_YOUR_SUBSYS);
/* create_nl_socket() calls perror_msg_and_skip() on failure */
```

## Real-World Examples

Study existing decoder files for reference (ordered from simple to
complex):

- **Simple decoder**: `src/netlink_selinux.c`
  - Message type switch
  - Fixed-size message structures
  - Good starting point

- **Fixed header + attributes**: `src/netlink_netfilter.c`
  - Decodes `struct nfgenmsg` header
  - Generic nlattr dump

- **Simple decoder with attributes**: `src/netlink_crypto.c`
  - Message type switch
  - Uses `decode_nlattr()` for attributes

- **Sub-dispatch by message type**: `src/netlink_route.c` +
  `src/netlink_route.h`
  - Large decoder array indexed by RTM_* type
  - `DECL_NETLINK_ROUTE_DECODER` macro
  - Many sub-decoder files (`src/rtnl_*.c`)

- **Sub-dispatch by address family**: `src/netlink_sock_diag.c` +
  `src/netlink_sock_diag.h`
  - Dispatches by AF_* family
  - Request vs response decoders
  - `DECL_NETLINK_DIAG_DECODER` macro
  - Sub-decoders: `src/netlink_inet_diag.c`, `src/netlink_unix_diag.c`,
    etc.

- **Generic netlink family dispatch**: `src/netlink_generic.c` +
  `src/netlink_nlctrl.c` + `src/netlink_generic.h`
  - Dynamic family ID lookup
  - `DECL_NETLINK_GENERIC_DECODER` macro

## Troubleshooting

### Decoder Not Called

Check:
1. Correct `NETLINK_*` protocol family in dispatch arrays
2. Declaration added to `src/defs.h` with `DECL_NETLINK()`
3. Function signature matches:
   `bool name(struct tcb *, const struct nlmsghdr *, kernel_ulong_t addr, unsigned int len)`
4. Decoder file added to `libstrace_a_SOURCES` in `src/Makefile.am`
5. Protocol family detected correctly — check `/proc/<pid>/fd/<fd>`
   socket info

### Message Header Not Printed

The netlink framework prints the `nlmsghdr` automatically. Your decoder
should only print the payload. If you see duplicate headers or missing
headers, check that you're not calling `print_nlmsghdr()` (it's a
static function in `src/netlink.c` — you cannot call it).

### Attribute Decoding Errors

```
decode_nlattr: short read (got 12 of 16) @0x7ffd...
```

Solution: Attribute length mismatch. Check:
- `nla_len` field is correct
- Attributes are properly aligned (4-byte boundaries)
- Decoder array index matches attribute type

### Xlat Not Working

Check:
- Xlat file created in `src/xlat/`
- `#From` directive points to correct header
- Constants exist in bundled headers
- Xlat table included: `#include "xlat/nl_your_subsys_types.h"`
- Xlat table passed to `decode_nlattr()` or used in `printxval()`

### Test Failures

Common issues:
- Expected output doesn't match strace output
- Message field order wrong
- Missing `+++ exited with 0 +++` line
- Socket creation failed (test skipped by `create_nl_socket()`)
- `/proc/self/fd/` not available (use `skip_if_unavailable`)

### Protocol Family Not Detected

If your decoder is never called, the protocol family may not be
detected. Check:
- Socket was created with correct `NETLINK_*` protocol
- `/proc/<pid>/fd/<fd>` shows correct protocol name
- Protocol name in `/proc` matches xlat entry in
  `src/xlat/netlink_protocols.in`

## See Also

- [DECODER_API.md](DECODER_API.md) — Decoder helper functions, macros,
  and netlink attribute decoders
- [README-xlat.md](README-xlat.md) — Xlat file format
- [HOWTO_TEST.md](HOWTO_TEST.md) — Test framework guide
- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) — Similar process for
  syscalls
