# How to Add Ioctl Decoders to strace

This guide explains how to add support for decoding new ioctl commands
in strace.

## Overview

An ioctl decoder is a function that decodes and prints the third
argument (`argp`) of an `ioctl()` system call. The decoder is **not**
responsible for:
- Printing the file descriptor (first argument)
- Printing the ioctl command name (second argument)
- Looking up the command name in symbol tables

These are handled automatically by `SYS_FUNC(ioctl)` before your decoder
is called. Your decoder's sole job is to decode and print the `argp`
parameter based on the ioctl command.

## Ioctl Processing Pipeline

Understanding how strace processes ioctl calls helps you write effective
decoders.

### Ioctl Command Structure

Ioctl commands are 32-bit values with the following structure (on most
architectures):

```
 bits 31-30: Direction (_IOC_NONE, _IOC_READ, _IOC_WRITE, _IOC_READ|_IOC_WRITE)
 bits 29-16: Size of the argument structure
 bits 15-8:  Type (magic number identifying the subsystem)
 bits 7-0:   Number (command number within the subsystem)
```

Macros to extract these fields:
- `_IOC_DIR(cmd)` - Direction bits
- `_IOC_TYPE(cmd)` - Type byte (subsystem identifier)
- `_IOC_NR(cmd)` - Command number within the subsystem
- `_IOC_SIZE(cmd)` - Size field

### SYS_FUNC(ioctl) Flow

When strace intercepts an `ioctl()` call, `SYS_FUNC(ioctl)` in
`src/ioctl.c` handles it in this order:

1. **Print fd**: Prints the file descriptor argument
2. **Print command name**: Calls `ioctl_decode_command_number()` for
   parametric commands (e.g., `EVIOCGABS(0)`), then `ioctl_lookup()`
   for table-based name resolution
3. **Decode argp**: Calls `ioctl_decode()` to dispatch to your
   subsystem-specific decoder
4. **Post-process return value**: Converts `RVAL_IOCTL_DECODED` to
   `RVAL_DECODED`; if the decoder returned `RVAL_DECODED` without
   `RVAL_IOCTL_DECODED`, prints `argp` as a raw hex value

### Command Name Resolution

**Parametric commands** (command number encodes variable data):
- Handled by `ioctl_decode_command_number()` in `src/ioctl.c`
- Examples: `EVIOCGABS(axis)`, `SPI_IOC_MESSAGE(n)`, `JSIOCGNAME(len)`
- Returns `IOCTL_NUMBER_HANDLED` if recognized, `IOCTL_NUMBER_UNKNOWN`
  otherwise
- Can return `IOCTL_NUMBER_STOP_LOOKUP` to skip table lookup

**Table-based lookup** (most commands):
- `ioctl_lookup()` performs binary search in `ioctlent0.h` (or
  `ioctlent1.h`, `ioctlent2.h` for multi-personality builds)
- These tables are generated from kernel headers via `maint/` scripts
- New ioctl commands become available for name lookup when `ioctls_*.h`
  files are updated (see `HACKING-scripts`)
- Your decoder does not need to handle name lookup

### Arg Decoding Dispatch

`ioctl_decode()` in `src/ioctl.c` dispatches based on `_IOC_TYPE(code)`:

```c
switch (_IOC_TYPE(code)) {
    case 0x03:
        return hdio_ioctl(tcp, code, arg);
    case 0x12:
        return block_ioctl(tcp, code, arg);
    case 'K':
        return kd_ioctl(tcp, code, arg);
    case 'V':
        return v4l2_ioctl(tcp, code, arg);
    /* ... more cases ... */
    default:
        return ioctl_decode_unknown_type(tcp, code, arg);
}
```

Each subsystem decoder receives:
- `struct tcb *tcp` - Traced process control block
- `unsigned int code` (or `request`) - Full ioctl command
- `kernel_ulong_t arg` - Third argument to `ioctl()` (usually a pointer)

**Unknown types** fall through to `ioctl_decode_unknown_type()`, which
uses `_IOC_DIR` to decide whether to hex-dump the arg data on entering,
exiting, or both.

### Architecture-Specific Dispatch

Some ioctl type values are dispatched conditionally:
- `'s'` (sock_ioctl): ALPHA, MIPS, SH, XTENSA only
- `'t'` (term_ioctl): ALPHA, MIPS, POWERPC, SPARC only
- `0xae` (kvm_ioctl): conditional on `HAVE_LINUX_KVM_H`

### Overlapping Ioctl Ranges

Certain ioctl command numbers are used by multiple subsystems. Two
special cases:

**Type `'f'`**: Has architecture-dependent overlaps. `f_ioctl()` in
`src/ioctl.c` chains fallbacks: `fs_f_ioctl()` → `sock_ioctl()` →
`term_ioctl()` depending on architecture and whether the previous
decoder recognized the command.

**Terminal/soundcard overlap**: Commands in a specific range overlap
between terminal ioctls and soundcard ioctls. `SYS_FUNC(ioctl)` resolves
the file descriptor path for these ranges to provide `finfo` to the
decoder, allowing context-dependent decoding.

## Adding a New Ioctl Decoder

### Step 1: Identify the Ioctl Type

Find the ioctl commands in the Linux kernel headers:

```bash
# Example: GPIO ioctls
grep -r "define.*GPIO.*_IO" /usr/include/linux/gpio.h
```

Look for the `_IOC_TYPE` value:

```c
#define GPIO_GET_LINEINFO_IOCTL _IOWR(0xB4, 0x02, struct gpio_v2_line_info)
/*                                    ^^^^
 *                                    This is the type byte
 */
```

The type byte (`0xB4` in this example) is what you'll use in the
dispatch switch in `src/ioctl.c`.

### Step 2: Create the Decoder File

Create `src/<subsystem>_ioctl.c`:

```c
/*
 * Copyright (c) YEAR YOUR NAME <your@email>
 * Copyright (c) YEAR-YEAR The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include <linux/subsystem.h>  /* The relevant kernel header */

#include "xlat/subsystem_ioctl_cmds.h"

int
subsystem_ioctl(struct tcb *const tcp, const unsigned int request,
                const kernel_ulong_t arg)
{
	switch (request) {
	case SUBSYS_CMD_1:
		/* Decode SUBSYS_CMD_1 */
		tprints_arg_next_name("argp");
		tprint_struct_begin();
		/* decode arg structure */
		tprint_struct_end();
		break;

	case SUBSYS_CMD_2:
		/* Decode SUBSYS_CMD_2 */
		tprints_arg_next_name("argp");
		/* decode arg value */
		break;

	default:
		return RVAL_DECODED;
	}
	return RVAL_IOCTL_DECODED;
}
```

**Key points:**
- Use parameter names `request` and `arg` to match `DECL_IOCTL` expansion
- Call `tprints_arg_next_name("argp")` before printing the argument value
- Use `break` for recognized commands, then return `RVAL_IOCTL_DECODED`
  after the switch
- Return `RVAL_DECODED` in the `default` case for unknown commands

### Step 3: Declare with DECL_IOCTL

Add declaration to `src/defs.h` in the `DECL_IOCTL` section:

```c
DECL_IOCTL(subsystem);
```

This macro expands to:

```c
int subsystem_ioctl(struct tcb *, unsigned int request, kernel_ulong_t arg);
```

### Step 4: Add Dispatch Case in ioctl.c

Edit `src/ioctl.c`, add a case to the `ioctl_decode()` switch:

```c
switch (_IOC_TYPE(code)) {
	/* ... existing cases ... */
	case 0xb4:  /* or 'S' for ASCII type bytes */
		return subsystem_ioctl(tcp, code, arg);
	/* ... */
}
```

### Step 5: Create Xlat Files

Create `src/xlat/subsystem_ioctl_cmds.in` for ioctl command names:

```
#From linux/subsystem.h
SUBSYS_CMD_1
SUBSYS_CMD_2
SUBSYS_CMD_3
```

If the ioctl uses flags or enums, create additional xlat files:

```
src/xlat/subsystem_flags.in
src/xlat/subsystem_modes.in
```

See [README-xlat.md](README-xlat.md) for xlat file format details.

### Step 6: Update Build System

Edit `src/Makefile.am`:

Add your decoder to the `libstrace_a_SOURCES` list:

```makefile
libstrace_a_SOURCES = \
	...
	subsystem_ioctl.c \
	...
```

Xlat files (`src/xlat/*.in`) are automatically processed by the build
system.

### Step 7: Write Tests

Create `tests/ioctl_subsystem.c`:

```c
#include "tests.h"
#include "scno.h"
#include <linux/ioctl.h>
#include <linux/subsystem.h>

int
main(void)
{
	/* Use invalid fd to test decoding without hardware */
	struct subsys_struct arg = { .field = 42 };
	long rc = ioctl(-1, SUBSYS_CMD_1, &arg);
	printf("ioctl(-1, SUBSYS_CMD_1, {field=42}) = %s\n",
	       sprintrc(rc));

	/* Test error cases */
	ioctl(-1, SUBSYS_INVALID_CMD, NULL);
	printf("ioctl(-1, SUBSYS_INVALID_CMD, NULL) = %s\n",
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
```

Register the test in `tests/gen_tests.in`:

```
ioctl_subsystem +ioctl.test
```

The `+ioctl.test` pattern runs the test with standard ioctl tracing
options. See [HOWTO_TEST.md](HOWTO_TEST.md) for comprehensive testing
guidance.

### Step 8: Update NEWS

Add an entry to the `NEWS` file documenting the new ioctl support.

## Decoder Return Values

Ioctl decoders return `int` values that control decoding flow and whether
the generic handler should print the argument. Constants are defined in
`src/defs.h`.

**Return Value Constants:**

- **`RVAL_IOCTL_DECODED`**
  - **Meaning**: The decoder has decoded and printed the ioctl argument
  - **When to use**: Your decoder printed the `argp` structure or value
  - **Effect**: `SYS_FUNC(ioctl)` skips generic arg printing
  - **Most common**: This is what most ioctl decoders return on success

- **`RVAL_DECODED`**
  - **Meaning**: Decoding is complete, but generic handler should print arg
  - **When to use**: For unknown/unsupported commands within your subsystem
  - **Effect**: Generic handler prints `argp` as raw hex pointer
  - **Typical use**: `default:` case in your switch statement

- **`0`** (no flags)
  - **Meaning**: Continue to syscall exit-stop for two-phase decoding
  - **When to use**: Kernel will modify the structure; need to show both
    input and output
  - **Effect**: Decoder will be called again on syscall exit
  - **Common pattern**: `entering(tcp) ? 0 : RVAL_IOCTL_DECODED`

- **`RVAL_IOCTL_DECODED | RVAL_FD`**
  - **Meaning**: Decoded, and return value is a file descriptor
  - **When to use**: Ioctl returns a new file descriptor
  - **Effect**: Enables `-y` (print paths) and `--trace-fds` features
  - **Rare**: Most ioctls don't return file descriptors

**Semantics:**

The distinction between `RVAL_IOCTL_DECODED` and `RVAL_DECODED` tells
`SYS_FUNC(ioctl)` whether the decoder printed the argument:

- `RVAL_IOCTL_DECODED` → "I printed it, don't print it again"
- `RVAL_DECODED` → "I didn't print it, use the generic handler"

`SYS_FUNC(ioctl)` converts `RVAL_IOCTL_DECODED` to `RVAL_DECODED` before
returning, so the distinction only matters within the ioctl decoding
machinery.

**On entering (syscall entry):**
- Return `RVAL_IOCTL_DECODED` if you printed `argp` and no exit-stop needed
- Return `0` if you need to print output values on syscall exit
- Return `RVAL_DECODED` for unknown commands (generic prints argp)

**On exiting (syscall exit, after returning 0 on entry):**
- Return `RVAL_IOCTL_DECODED` if you printed `argp`
- Return `RVAL_DECODED` if you didn't print it (generic prints argp)

## Decoder Implementation Patterns

### One-Phase Decoding (entering only)

Most ioctls are decoded in a single phase:

```c
case SUBSYS_GET_INFO:
	tprints_arg_next_name("argp");
	print_subsys_info(tcp, arg);
	return RVAL_IOCTL_DECODED;
```

Or using the `break` pattern:

```c
case SUBSYS_GET_INFO:
	tprints_arg_next_name("argp");
	print_subsys_info(tcp, arg);
	break;

/* ... */

default:
	return RVAL_DECODED;
}
return RVAL_IOCTL_DECODED;
```

### Two-Phase Decoding (entering and exiting)

Use two-phase when the kernel writes to the structure and you need to
show both input and output values:

```c
case SUBSYS_QUERY:
	if (entering(tcp)) {
		/* Decode input on entry */
		tprints_arg_next_name("argp");
		tprint_struct_begin();
		printxval(subsys_query_types, arg, "QUERY_???");
		return 0;  /* Call us again on exit */
	} else {
		/* Decode output on exit */
		tprint_struct_next();
		/* decode result fields */
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}
```

### No-Parameter Ioctls

Some ioctl commands have no argument:

```c
case SUBSYS_RESET:
	/* No argument to decode */
	break;

/* ... */

default:
	return RVAL_DECODED;
}
return RVAL_IOCTL_DECODED;
```

### Direction-Based Dispatch

Some decoders dispatch based on `_IOC_DIR`:

```c
switch (_IOC_DIR(request)) {
case _IOC_READ:
	if (entering(tcp))
		return 0;
	tprints_arg_next_name("argp");
	return decode_read_ioctl(tcp, request, arg);
case _IOC_WRITE:
	tprints_arg_next_name("argp");
	return decode_write_ioctl(tcp, request, arg) | RVAL_DECODED;
default:
	return RVAL_DECODED;
}
```

### Unknown Commands Within a Subsystem

For unrecognized commands within your subsystem, return `RVAL_DECODED`
to let the generic handler print the raw command:

```c
default:
	return RVAL_DECODED;
```

The generic handler will print:

```
ioctl(3, _IOC(_IOC_READ|_IOC_WRITE, 0xb4, 0x99, 0x20), 0x7ffd...) = -1 ENOTTY
```

### Using tprints_arg_next_name("argp")

Always call `tprints_arg_next_name("argp")` before printing the argument
value. This ensures proper output formatting with the ioctl third
argument label:

```c
case SUBSYS_GET_VALUE:
	tprints_arg_next_name("argp");
	printnum_int(tcp, arg, "%d");
	break;
```

### break vs return Pattern

Both patterns are valid:

**Direct return:**
```c
case SUBSYS_CMD:
	tprints_arg_next_name("argp");
	/* ... */
	return RVAL_IOCTL_DECODED;
```

**Break pattern** (more common in actual decoders):
```c
case SUBSYS_CMD:
	tprints_arg_next_name("argp");
	/* ... */
	break;

/* ... more cases ... */

default:
	return RVAL_DECODED;
}
return RVAL_IOCTL_DECODED;
```

The break pattern is cleaner when you have many cases.

### XLAT_MACROS_ONLY Pattern

If you need ioctl command constant definitions but not the xlat array:

```c
#define XLAT_MACROS_ONLY
#include "xlat/subsystem_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY
```

This provides `#define` fallbacks for ioctl command constants without
pulling in the xlat array. Useful when the xlat is only needed in other
parts of the decoder.

### Decoding Structures

Use helpers from `DECODER_API.md`. Common pattern:

```c
struct subsys_config cfg;
if (umove_or_printaddr(tcp, arg, &cfg))
	return RVAL_IOCTL_DECODED;

tprint_struct_begin();
PRINT_FIELD_U(cfg, version);
PRINT_FIELD_X(cfg, flags);
PRINT_FIELD_CSTRING(cfg, name);
PRINT_FIELD_PTR(cfg, callback);
tprint_struct_end();
```

See [DECODER_API.md](DECODER_API.md) for the complete list of printing
helpers.

### ATTRIBUTE_FALLTHROUGH

Mark intentional fall-through between switch cases:

```c
case SUBSYS_READ_VALUE:
	if (entering(tcp))
		return 0;
	ATTRIBUTE_FALLTHROUGH;
case SUBSYS_WRITE_VALUE:
	tprints_arg_next_name("argp");
	printnum_int(tcp, arg, "%d");
	break;
```

## MPERS Considerations

If your ioctl operates on structures whose layout changes between
personalities (32-bit vs 64-bit), you may need MPERS support.

### When MPERS Is Needed

MPERS is required when:
- The structure contains `long`, `size_t`, `time_t`, or pointer types
- Field offsets or structure size differ between 32-bit and 64-bit ABIs
- The ioctl is used with compat layer (32-bit processes on 64-bit kernel)

Common examples: structures with `struct timespec`, `struct timeval`,
pointers.

### Architecture-Dependent Ioctl Numbers

A distinct but related problem arises when ioctl commands encode a struct
size via `_IOW` / `_IOR` / `_IOWR` and that struct contains a pointer or
other type whose size differs between 32-bit and 64-bit. In this case,
the ioctl command **number itself** differs between architectures.

For example:

```c
/* sizeof(struct sock_fprog) is 16 on 64-bit, 8 on 32-bit */
#define TUNATTACHFILTER _IOW('T', 213, struct sock_fprog)
```

On 64-bit: `TUNATTACHFILTER` = `0x400854d5` (size field = 16).
On 32-bit: `TUNATTACHFILTER` = `0x400854d5` becomes `0x400454d5`
(size field = 8).

A plain `case TUNATTACHFILTER:` in a decoder compiled as 64-bit will
never match the 32-bit ioctl number, silently falling through to the
`default` case for compat tracees.

**Fix**: mask out the size bits when matching these commands:

```c
#define SUBSYS_IOC(cmd_) ((cmd_) & ~(_IOC_SIZEMASK << _IOC_SIZESHIFT))

/* In the decoder, use a nested switch for affected commands only: */
default:
    switch (SUBSYS_IOC(code)) {
    case SUBSYS_IOC(TUNATTACHFILTER):
    case SUBSYS_IOC(TUNDETACHFILTER):
        /* decode ... */
        return RVAL_IOCTL_DECODED;
    default:
        return RVAL_DECODED;
    }
```

Only apply this to ioctls whose size field actually varies across
architectures. Ioctls using `int`, `unsigned int`, or `_IO` (no size)
are safe with plain `case` matching.

This is not the same as parametric commands (where `_IOC_NR` encodes
variable data) — here the NR is fixed but the SIZE field changes.

### MPERS Mechanics

There is nothing ioctl-specific about MPERS support. The same
`DEF_MPERS_TYPE` / `MPERS_PRINTER_DECL` / `MPERS_DEFS` pattern applies
to all decoders that handle personality-dependent structures.

See [README-mpers.md](README-mpers.md) for complete MPERS documentation,
including how to create MPERS decoders and update the build system.

## Parametric Ioctl Commands

Some ioctl commands encode variable data in the command number itself.

### Examples

- `EVIOCGABS(axis)` - axis number encoded in command
- `SPI_IOC_MESSAGE(n)` - message count encoded in command
- `JSIOCGNAME(len)` - buffer length encoded in command

These are handled by `ioctl_decode_command_number()` in `src/ioctl.c`.

### Adding a Parametric Decoder

If you need to add a new parametric command decoder:

1. Add a case to `ioctl_decode_command_number()` in `src/ioctl.c`
2. Decode the variable part from `_IOC_NR(code)` or `_IOC_SIZE(code)`
3. Print the command name with the variable part
4. Return `IOCTL_NUMBER_HANDLED` if recognized
5. Return `IOCTL_NUMBER_STOP_LOOKUP` to skip table lookup
6. Return `IOCTL_NUMBER_UNKNOWN` to continue to table lookup

Example pattern:

```c
case 'k':
	if (_IOC_DIR(code) == _IOC_WRITE && _IOC_NR(code) == 0) {
		tprints_arg_begin("SPI_IOC_MESSAGE");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return IOCTL_NUMBER_HANDLED;
	}
	return IOCTL_NUMBER_UNKNOWN;
```

## Testing Ioctl Decoders

### Standard Pattern: ioctl(-1, CMD, ...)

Most ioctl tests use an invalid file descriptor to test decoding without
requiring any hardware:

```c
#include "tests.h"
#include "scno.h"
#include <linux/subsystem.h>

int
main(void)
{
	struct subsys_data data = { .value = 123 };
	ioctl(-1, SUBSYS_GET_DATA, &data);
	printf("ioctl(-1, SUBSYS_GET_DATA, {value=123}) = %s\n",
	       sprintrc(-1));

	puts("+++ exited with 0 +++");
	return 0;
}
```

Register in `tests/gen_tests.in`:

```
ioctl_subsystem +ioctl.test
```

The `+ioctl.test` script runs the test with `-a16 -eioctl` and filters
out stdin/stdout/stderr ioctl noise.

### Injection Pattern: ioctl-success.sh

To test decoding of output structures (that the kernel would fill on
success), use syscall injection:

```c
#include "tests.h"
#include "scno.h"
#include <linux/subsystem.h>

int
main(int argc, char *argv[])
{
	/* First two args are injection start index and return value */
	if (argc < 3)
		error_msg_and_fail("Usage: %s INJECT_START INJECT_RETVAL",
		                   argv[0]);

	/* Test with injection */
	struct subsys_data data = { .value = 123 };
	ioctl(-1, SUBSYS_GET_DATA, &data);
	/* Print expected output based on INJECT_RETVAL */

	puts("+++ exited with 0 +++");
	return 0;
}
```

Register in `tests/gen_tests.in`:

```
ioctl_subsystem-success +ioctl-success.sh -a30
```

The `ioctl-success.sh` script uses:
- `IOCTL_INJECT_START` (default 256)
- `IOCTL_INJECT_RETVAL` (default 42)
- Pattern: `-e inject=ioctl:retval=${IOCTL_INJECT_RETVAL}:when=${IOCTL_INJECT_START}+`

### Xlat Variants: -Xraw, -Xabbrev, -Xverbose

Tests commonly have variants for different xlat output modes. Create
wrapper `.c` files:

```c
/* ioctl_subsystem-Xraw.c */
#define XLAT_RAW 1
#include "ioctl_subsystem.c"
```

```c
/* ioctl_subsystem-Xverbose.c */
#define XLAT_VERBOSE 1
#include "ioctl_subsystem.c"
```

The main test file uses `XLAT_RAW` / `XLAT_VERBOSE` to conditionally
adjust output.

Register in `tests/gen_tests.in`:

```
ioctl_subsystem-Xraw     +ioctl.test -a18 -Xraw
ioctl_subsystem-Xverbose +ioctl.test -a47 -Xverbose
```

### Test Naming Convention

Ioctl test files follow the pattern:
- `ioctl_<subsystem>.c` - Main test
- `ioctl_<subsystem>-<variant>.c` - Variant (e.g., `-success`, `-Xraw`,
  `-v`)

See [HOWTO_TEST.md](HOWTO_TEST.md) for comprehensive testing guidance.

## Real-World Examples

Study existing decoder files for reference:

- **Simple decoder**: `src/random_ioctl.c`
  - Few commands, simple arguments
  - Uses `break` pattern
  - Shows no-parameter ioctls
  - Uses `XLAT_MACROS_ONLY` pattern

- **Complex decoder**: `src/v4l2.c`
  - Many commands, large structures
  - Extensive two-phase decoding

- **MPERS decoder**: `src/kd_mpers_ioctl.c`
  - Uses MPERS for console ioctls
  - Two-file pattern with `kd_ioctl.c`

- **Flags/enums**: `src/block.c`
  - Extensive use of xlat for flags
  - MPERS support

- **Two-phase**: `src/perf_ioctl.c`
  - Shows entering/exiting pattern
  - MPERS support

- **Direction-based**: `src/evdev.c` (evdev_ioctl function)
  - Dispatches based on `_IOC_DIR`
  - Parametric command handling

## See Also

- [DECODER_API.md](DECODER_API.md) - Decoder helper functions and macros
- [README-xlat.md](README-xlat.md) - Xlat file format
- [README-mpers.md](README-mpers.md) - Multi-personality support
- [HOWTO_TEST.md](HOWTO_TEST.md) - Test framework guide
- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) - Similar process for
  syscalls
- [HACKING-scripts](HACKING-scripts) - Maintenance scripts including
  ioctl table generation
