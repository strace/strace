# How to Add Support for New Syscalls in strace

This document describes the process of adding support for decoding new
Linux syscalls in strace, based on analysis of recent implementation
commits.

## Overview

Adding support for a new syscall typically involves:
1. Wiring up the syscall in the syscall table
2. Implementing the decoding function
3. Adding path tracing support (if applicable)
4. Creating xlat files for constants/flags (if needed)
5. Updating NEWS
6. Writing tests
7. Updating build system (if creating new source files)

## Step-by-Step Guide

### 1. Determine the Syscall Number

First, identify:
- The syscall number (relative to BASE_NR for common syscalls, or
  architecture-specific)
- The kernel commit that introduced the syscall
- The syscall signature (number of arguments, types)

### 2. Wire Up the Syscall in syscallent-common.h

Add an entry to `src/linux/generic/syscallent-common.h`:

```c
[BASE_NR + <number>] = { <nargs>, <flags>, SEN(<syscall_name>),
                         "<syscall_name>" },
```

**Flags:**
- `TD` - Trace file descriptor operations
- `TF` - Trace file operations
- `TM` - Trace memory operations
- `TS` - Trace signal operations
- `TP` - Trace process operations
- `0` - No special flags

**Example:**
```c
[BASE_NR + 467] = { 5, TD|TF, SEN(open_tree_attr), "open_tree_attr"},
```

For architecture-specific syscalls (like x86_64), add to
`src/linux/<arch>/syscallent.h` instead.

### 3. Implement the Decoding Function

#### 3.1. Choose the Right Source File

- If a related syscall file exists (e.g., `mount_setattr.c` for
  mount-related), add to that file
- If it's a new category, create a new file (e.g., `lsm.c` for LSM
  syscalls)
- For simple syscalls, add to an appropriate existing file (e.g.,
  `mem.c` for memory operations)

#### 3.2. Write the SYS_FUNC Implementation

Use the `SYS_FUNC` macro to define the decoding function:

```c
SYS_FUNC(syscall_name)
{
    // Access arguments via tcp->u_arg[0], tcp->u_arg[1], etc.
    // Print arguments using appropriate print functions
    // Return RVAL_DECODED or 0
}
```

**Common Print Functions:**
- `printaddr(addr)` - Print a pointer address
- `PRINT_VAL_D(value)` - Print a decimal integer
- `PRINT_VAL_U(value)` - Print an unsigned integer
- `PRINT_VAL_X(value)` - Print a hexadecimal value
- `printstr(tcp, addr)` - Print a string from address
- `printpath(tcp, addr)` - Print a file path
- `print_dirfd(tcp, fd)` - Print a directory file descriptor
- `printflags(xlat, flags, "PREFIX_???")` - Print flags using xlat
- `printxval(xlat, value, "PREFIX_???")` - Print a value using xlat
- `printxval64(xlat, value, "PREFIX_???")` - Print a 64-bit value using xlat
- `tprint_arg_next()` - Print argument separator
- `tprints_arg_name("name")` - Print explicit argument name
- `tprints_arg_next_name("name")` - Print next argument with name
- `tprint_value_changed()` - Indicate value changed between phases
- `print_timespec64(tcp, addr)` - Print timespec64 structure
- `printnum_int(tcp, addr, format)` - Print integer from address

**Example (simple syscall):**
```c
SYS_FUNC(mseal)
{
    /* addr */
    printaddr(tcp->u_arg[0]);
    tprint_arg_next();

    /* length */
    PRINT_VAL_U(tcp->u_arg[1]);
    tprint_arg_next();

    /* flags */
    PRINT_VAL_X(tcp->u_arg[2]);

    return RVAL_DECODED;
}
```

**Example (complex syscall with structs):**
```c
SYS_FUNC(open_tree_attr)
{
    decode_dfd_file_flags_attr(tcp,
                               tcp->u_arg[0],  // dfd
                               tcp->u_arg[1],  // pathname
                               open_tree_flags,
                               tcp->u_arg[2],  // flags
                               "OPEN_TREE_???",
                               tcp->u_arg[3],  // attr
                               tcp->u_arg[4]); // size

    return RVAL_DECODED | RVAL_FD;
}
```

**Return Values:**
- `RVAL_DECODED` - Syscall was fully decoded
- `RVAL_DECODED | RVAL_FD` - Syscall returns a file descriptor
- `0` - Continue decoding (for two-phase syscalls using `entering(tcp)`)

**Two-Phase Decoding:**
For syscalls that modify arguments (like `lsm_get_self_attr`), use
`entering(tcp)`:

```c
SYS_FUNC(lsm_get_self_attr)
{
    if (entering(tcp)) {
        // Print input arguments
        // Store state if needed
        set_tcb_priv_ulong(tcp, size);  // Store for later comparison
        return 0;
    }

    // Print output arguments
    // Compare with stored state
    uint32_t saved_size = get_tcb_priv_ulong(tcp);
    if (saved_size != size) {
        tprint_value_changed();  // Indicate value changed
    }
    return 0;
}
```

**State Storage Between Phases:**
- `set_tcb_priv_ulong(tcp, value)` / `get_tcb_priv_ulong(tcp)` -
  Store/retrieve unsigned long
- `set_tcb_priv_data(tcp, ptr, free_func)` / `get_tcb_priv_data(tcp)` -
  Store/retrieve pointer with cleanup
- Use `xobjdup()` to duplicate objects for storage

**Using Return Value in Exit Phase:**
When the syscall returns a value that affects output decoding, use
`tcp->u_rval`:

```c
SYS_FUNC(getxattrat)
{
    if (entering(tcp)) {
        // Store input size
        set_tcb_priv_ulong(tcp, args.size);
        return 0;
    }

    // Use return value for actual size
    kernel_ulong_t val_size = entering(tcp) ?
        (kernel_long_t) args.size : tcp->u_rval;
    // ...
}
```

**Error Handling in Exit Phase:**
Use `tfetch_obj_ignore_syserror()` when fetching objects that might fail
if the syscall returned an error:

```c
if (!tfetch_obj_ignore_syserror(tcp, p_size, &size)) {
    // Handle error case
    printaddr(p_ctx);
    return print_args_size_flags(tcp, p_size, flags);
}
```

### 4. Create Xlat Files (if needed)

If the syscall uses constants or flags, create xlat files in `src/xlat/`:

**File format (`src/xlat/<name>.in`):**
```
#unconditional
#From include/uapi/linux/header.h  (optional comment)
#Prefix PREFIX_  (optional prefix)
CONSTANT_NAME1   0x01
CONSTANT_NAME2   0x02
```

**Special directives:**
- `#unconditional` - Always include these constants. When using this
  directive, list only symbolic names without numeric values (the build
  system handles values automatically).
- `#value_indexed` - Values are indexed (0, 1, 2, ...)
- `#val_type uint64_t` - Specify value type. Required for 64-bit xlat
  values to be properly decoded symbolically (e.g., when using
  `printxval64()`).
- `#Prefix PREFIX_` - Add prefix to constant names
- `#Pattern PATTERN_` - Match constants using a pattern (e.g., `#Pattern
  *_NS` matches constants ending in `_NS` like `TIME_NS`, `MNT_NS`)

**Example (`src/xlat/futex2_flags.in`):**
```
#unconditional
#From include/uapi/linux/futex.h
#Prefix FUTEX2_
FUTEX2_NUMA    0x04
FUTEX2_PRIVATE 0x80
```

**Usage in code:**
```c
#include "xlat/futex2_flags.h"

printflags(futex2_flags, flags, "FUTEX2_???");
```

The build system automatically generates header files from `.in` files.

**External Declarations in defs.h:**
If you create new xlat files that need to be accessible from other
files, add an external declaration to `src/defs.h`:

```c
extern const struct xlat fsmagic[];
extern const struct xlat mount_attr_attr[];
```

This is typically needed when the xlat is used in multiple source files
or when it's part of a public API.

### 5. Add Path Tracing Support (if applicable)

If the syscall operates on file paths (takes a pathname argument), add
it to `src/pathtrace.c` in the `pathtrace_match_set` function:

```c
case SEN_open_tree_attr:
    // Add to appropriate switch case group
    break;
```

**Common groups:**
- `fd, path` - Takes file descriptor and pathname
- `path, path` - Takes two paths
- `path` - Takes single path

**Example:**
```c
case SEN_open_tree:
case SEN_open_tree_attr:
case SEN_openat:
    /* fd, path */
    return fdmatch(tcp, tcp->u_arg[0], set, fdset) ||
           upathmatch(tcp, tcp->u_arg[1], set);
```

### 6. Update Build System (if creating new files)

If you created a new source file (e.g., `src/lsm.c`), add it to
`src/Makefile.am`:

```makefile
libstrace_a_SOURCES = \
    ...
    lsm.c \
    ...
```

**Important:** After adding new files (source files, xlat files, or test
files), you must run `./bootstrap` to regenerate autotools files and xlat
headers. This is required for the build system to recognize new files.

### 7. Update NEWS

Add an entry to `NEWS`:

```
* Improvements
  * Implemented decoding of <syscall_name> syscall.
```

**Note:** If you're adding a related syscall to an existing family, you
may update an existing NEWS entry instead of creating a new one:

```
* Improvements
  * Implemented decoding of getxattrat and setxattrat syscalls.
```

Later, when adding more related syscalls:
```
* Improvements
  * Implemented decoding of getxattrat, setxattrat, listxattrat,
    and removexattrat syscalls.
```

### 8. Write Tests

Create test files in `tests/`:

#### 8.1. Basic Test File (`tests/<syscall_name>.c`)

```c
/*
 * Check decoding of <syscall_name> syscall.
 *
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
    // Test cases
    long rc = syscall(__NR_<syscall_name>, ...);
    printf("<syscall_name>(...) = %s\n", sprintrc(rc));

    puts("+++ exited with 0 +++");
    return 0;
}
```

#### 8.2. Path Tracing Test (if applicable)

Create `tests/<syscall_name>-P.c`:

```c
#define PATH_TRACING
#include "<syscall_name>.c"
```

#### 8.3. Success Test Variant (optional)

For testing successful return values using syscall injection, create
`tests/<syscall_name>-success.c`:

```c
#define INJECT_RETVAL
#include "<syscall_name>.c"
```

This pattern allows the same test code to be used for both error and
success cases, with the `INJECT_RETVAL` macro controlling the output
format.

#### 8.4. Register Tests

Add to `tests/gen_tests.in`:
```
<syscall_name> -a<num>
<syscall_name>-P --decode-fds --trace-path=/dev/full \
  -e trace=<syscall_name> 9>>/dev/full
<syscall_name>-success --inject=<syscall_name>:retval=42 -a<num> \
  --trace=<syscall_name>
```

**Important rules for `gen_tests.in`:**
- When the test name exactly matches the syscall name, you don't need to
  specify `--trace=` (it's added automatically). For example, `listns -a23`
  instead of `listns -a23 -e trace=listns`.
- For variant test names (like `-success`, `-P`), you must explicitly
  include `--trace=<syscall_name>` since the test name doesn't match the
  syscall name.

**Determining the `-a` alignment value:**
- Start with `-a32` (or a reasonable default based on similar tests)
- If the alignment is incorrect, try decreasing the value
- The goal is for the shortest line in the test output to have exactly 1
  space before the `=` sign
- Run the test and compare output to determine the correct value

Add to `tests/.gitignore`:
```
<syscall_name>
<syscall_name>-P
<syscall_name>-success
```

Add to `tests/pure_executables.list`:
```
<syscall_name>
<syscall_name>-P
<syscall_name>-success
```

### 9. Special Cases

#### 9.1. Architecture-Specific Syscalls

For syscalls that only exist on specific architectures (e.g., `uretprobe`
on x86_64):

1. Add to architecture-specific syscallent file: `src/linux/<arch>/syscallent.h`
2. If no decoding needed, add to `src/syscall_dummy.h`:
   ```c
   # define sys_uretprobe printargs
   ```

#### 9.2. Creating Reusable Helper Functions

When implementing syscall families (like the xattr family), create
helper functions that can be shared:

```c
// Helper for dirfd, pathname, flags
static void
decode_dirfd_pathname_flags(struct tcb *tcp)
{
    print_dirfd(tcp, tcp->u_arg[0]);
    tprint_arg_next();
    printpath(tcp, tcp->u_arg[1]);
    tprint_arg_next();
    printflags(xattrat_flags, tcp->u_arg[2], "AT_???");
}

// Helper that extends the above
static void
decode_dirfd_pathname_flags_name(struct tcb *tcp)
{
    decode_dirfd_pathname_flags(tcp);
    tprint_arg_next();
    printstr(tcp, tcp->u_arg[3]);
}
```

This pattern allows multiple syscalls (`setxattrat`, `getxattrat`,
`listxattrat`, `removexattrat`) to share common decoding logic.

#### 9.3. Syscalls Using Existing Helpers

Many syscalls can reuse existing helper functions:
- `decode_dfd_file_flags_attr()` - For syscalls with dfd, pathname,
  flags, attr, size
- `print_dirfd()` - For directory file descriptors
- `printpath()` - For pathnames
- `printstr()` - For strings
- `print_waiter_array()` - For futex waiter arrays
- `print_timespec64()` - For timespec64 structures

Look at similar syscalls for reusable patterns.

#### 9.4. Printing Arrays with Callbacks

For syscalls that return arrays, use `print_array()` with a callback function:

```c
static bool
print_lsm_id_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
                          void *data)
{
    const uint64_t *p_id = elem_buf;
    printxval64(lsm_ids, *p_id, "LSM_ID_???");
    return true;
}

SYS_FUNC(lsm_list_modules)
{
    // ...
    print_array(tcp, p_ids, (kernel_ulong_t) tcp->u_rval, &elem,
                sizeof(elem), tfetch_mem, print_lsm_id_array_member, 0);
    // ...
}
```

#### 9.5. Syscalls with Complex Structures

For syscalls that pass complex structures:

1. Define helper functions to fetch and print structures
2. Use `tfetch_obj()` or `umoven_or_printaddr()` to fetch data
3. Use `PRINT_FIELD_*` macros to print struct fields:
   - `PRINT_FIELD_X()` - Hexadecimal
   - `PRINT_FIELD_U()` - Unsigned integer
   - `PRINT_FIELD_XVAL()` - Using xlat
   - `PRINT_FIELD_FLAGS()` - Flags using xlat
   - `PRINT_FIELD_OBJ_TCB_VAL()` - Complex object

**Example:**
```c
struct lsm_ctx ctx;
if (tfetch_obj(tcp, addr, &ctx)) {
    tprint_struct_begin();
    PRINT_FIELD_XVAL(ctx, id, lsm_ids, NULL);
    PRINT_FIELD_X(ctx, flags);
    PRINT_FIELD_U(ctx, len);
    tprint_struct_end();
}
```

**Custom Print Macros:**
For special cases, you may need to define custom print macros:

```c
#define PRINT_FIELD_CSTRING_OFFSET(where_, field_, str_buf_, str_size_) \
    do { \
        tprints_field_name(#field_); \
        if ((where_).field_ < (str_size_)) \
            print_quoted_cstring((str_buf_) + (where_).field_, \
                                (str_size_) - (where_).field_); \
        else \
            PRINT_VAL_X((where_).field_); \
    } while (0)
```

**Fetching Structures:**
- `tfetch_obj(tcp, addr, obj)` - Fetch object, returns true on success
- `tfetch_obj_ignore_syserror(tcp, addr, obj)` - Fetch object, ignores
  syscall errors (use in exit phase)
- `umoven_or_printaddr(tcp, addr, size, obj)` - Fetch memory or print
  address on failure
- `umove_or_printaddr(tcp, addr, obj)` - Fetch object or print address
  on failure

## Checklist

- [ ] Determine syscall number and signature
- [ ] Add entry to `syscallent-common.h` (or arch-specific file)
- [ ] Implement `SYS_FUNC()` in appropriate source file
- [ ] Create xlat files for constants/flags (if needed)
- [ ] Add external xlat declarations to `defs.h` (if needed)
- [ ] Add path tracing support (if applicable)
- [ ] Update `Makefile.am` (if creating new source file)
- [ ] Update `NEWS` (or update existing entry if part of family)
- [ ] Write test file(s)
- [ ] Register tests in `gen_tests.in`, `.gitignore`, `pure_executables.list`
- [ ] Test the implementation
- [ ] Commit with message: "Implement decoding of <syscall_name> syscall"

## Example Commit Messages

```
Implement decoding of open_tree_attr syscall

This new syscall was introduced by Linux kernel commit
v6.15-rc1~252^2~4^2~2.

* src/linux/generic/syscallent-common.h [BASE_NR + 467]: Wire up
open_tree_attr.
* src/mount_setattr.c (SYS_FUNC(open_tree_attr)): New function.
* src/pathtrace.c (pathtrace_match_set): Handle SEN_open_tree_attr.
* NEWS: Mention this change.
```

## Testing

Run the test:
```bash
make check TESTS=<syscall_name>
```

Or run strace on a program that uses the syscall:
```bash
strace -e trace=<syscall_name> <program>
```

## References

- Recent implementation commits analyzed:
  - 8003bbc88 tests: check decoding of open_tree_attr syscall
  - 545a57f20 Implement decoding of open_tree_attr syscall
  - 43b15a30f tests: check decoding of removexattrat syscall
  - d7c5ffdcc Implement decoding of removexattrat syscall
  - 460e12203 tests: check decoding of listxattrat syscall
  - d39909c6a Implement decoding of listxattrat syscall
  - d51a75dae tests: check decoding of getxattrat syscall
  - 0134cdff4 Implement decoding of getxattrat syscall
  - b0028ab09 tests: check decoding of setxattrat syscall
  - a33af41e1 Implement decoding of setxattrat syscall
  - 34c493f17 Implement decoding of uretprobe syscall
  - 52e1d0aa6 Implement decoding of mseal syscall
  - 3c2396054 tests: check decoding of lsm_list_modules syscall
  - ac2d189ca tests: check decoding of lsm_set_self_attr syscall
  - 52d8446bb tests: check decoding of lsm_get_self_attr syscall
  - 669d1a749 Implement decoding of lsm_list_modules syscall
  - 2f16c6562 Implement decoding of lsm_set_self_attr syscall
  - 84712ee25 Implement decoding of lsm_get_self_attr syscall
  - 823b5e0c3 tests: check decoding of listmount syscall
  - d5df646a5 Implement decoding of listmount syscall
  - f1817c4ae tests: check decoding of statmount syscall
  - 17406a59d Implement decoding of statmount syscall
  - 8409c05ac tests: check decoding of futex_requeue syscall
  - a4d7b80af Implement decoding of futex_requeue syscall
  - 0e9993592 tests: check decoding of futex_wait syscall
  - 8b48aaa2f Implement decoding of futex_wait syscall
  - 02c85334d tests: check decoding of futex_wake syscall
  - d990ccaae Implement decoding of futex_wake syscall

## Notes

- Always check the kernel source for the exact syscall signature
- Look at similar existing syscalls for implementation patterns
- Use appropriate print functions for each argument type
- Handle error cases (invalid addresses, etc.) gracefully
- Follow the existing code style and conventions
- Test with both valid and invalid inputs
- Line length limit: all lines must be 80 characters or less
