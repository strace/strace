Multi-Personality Support (MPERS)
=================================

## What Is MPERS

MPERS (Multi-Personality Support) enables a single strace binary to correctly
decode system calls from processes running different ABIs (Application Binary
Interfaces) on the same kernel.

For example, on x86_64:
- 64-bit native processes (x86_64 ABI)
- 32-bit processes (i386 ABI, via compat layer)
- x32 processes (x32 ABI - 32-bit pointers with 64-bit syscalls)

The challenge: structure layouts differ between these ABIs. A `struct stat`
in i386 has different sizes and field offsets than in x86_64. To decode
syscall arguments correctly, strace needs to know which personality the
traced process is using and apply the corresponding structure definitions.

MPERS solves this by compiling certain decoder files multiple times - once
for each personality - with different compiler flags that produce different
structure layouts. The resulting object files are linked into libmpers-m32.a
(for 32-bit personality) and libmpers-mx32.a (for x32 personality) libraries.

## Which Architectures Use MPERS

Architecture support for multiple personalities:

| Architecture | Personalities | Description                             |
|--------------|---------------|-----------------------------------------|
| x86_64       | 3             | native (64-bit), m32 (i386), mx32 (x32) |
| aarch64      | 2             | native (64-bit), m32 (aarch32/arm)      |
| powerpc64    | 2             | native (64-bit), m32 (powerpc)          |
| s390x        | 2             | native (64-bit), m32 (s390)             |
| sparc64      | 2             | native (64-bit), m32 (sparc)            |
| tile         | 2             | native (64-bit), m32 (tile32)           |
| x32          | 2             | native (x32), m32 (i386)                |
| Most others  | 1             | Single personality only                 |

The personality count is determined by the SUPPORTED_PERSONALITIES constant
defined in architecture-specific headers (src/linux/*/arch_defs_.h), with
fallback defaults in src/arch_defs.h. You can check which architectures
support multiple personalities:

```bash
grep SUPPORTED_PERSONALITIES src/linux/*/arch_defs_.h
```

## Build Pipeline

The MPERS build pipeline involves several stages:

1. **Preprocessing** (mpers.sh): For each .c.mpers.i file, the source is
   preprocessed with personality-specific compiler flags (-m32 or -mx32)
   to extract type definitions.

2. **Parsing** (mpers.awk): The preprocessed output is parsed to extract
   structure size and offset information, generating personality-specific
   headers.

3. **Compilation**: The source files are compiled multiple times:
   - Once for the native personality (linked directly into strace)
   - Once for m32 personality (if SUPPORTED_PERSONALITIES > 1)
   - Once for mx32 personality (if SUPPORTED_PERSONALITIES > 2)

4. **Library Creation**: The m32 and mx32 object files are collected into
   libmpers-m32.a and libmpers-mx32.a static libraries.

5. **Linking**: strace links against all personality libraries, selecting the
   correct decoder function at runtime based on the traced process's
   personality.

Build artifacts:
- mpers-m32/ directory: Intermediate files for 32-bit personality
- mpers-mx32/ directory: Intermediate files for x32 personality
- m32_printer_decls.h, mx32_printer_decls.h: Function declarations
- m32_printer_defs.h, mx32_printer_defs.h: Type definitions
- libmpers-m32.a, libmpers-mx32.a: Compiled libraries

## Which Files Need MPERS

Files that decode structures whose layout changes between personalities need
MPERS support. Common examples:

**Data structures**: stat, stat64, statfs, statfs64, flock, flock64, iovec,
mmsghdr, msghdr, siginfo_t, timespec, timeval, rusage, sysinfo, ustat

**Ioctl structures**: v4l2 structures, kd (console) structures, evdev
structures, loop device structures

**Other**: bpf_fprog, mq_attr, group_req, sg_req_info, xfs_quotastat

You can identify MPERS-enabled files by looking for .c.mpers.i files in src/:

```bash
ls src/*.c.mpers.i
```

Or by searching for DEF_MPERS_TYPE usage:

```bash
grep -l "DEF_MPERS_TYPE" src/*.c
```

## How to Write MPERS-Aware Code

To use MPERS functionality in your decoder:

1. **Include defs.h**:
   ```c
   #include "defs.h"
   ```

2. **Declare MPERS types** for each structure that varies by personality:
   ```c
   #include DEF_MPERS_TYPE(struct_stat)
   ```

3. **Include relevant headers** and typedef compound types that are not
   already typedefed:
   ```c
   #include <sys/stat.h>
   typedef struct stat struct_stat;
   ```

4. **Include MPERS_DEFS once** after all DEF_MPERS_TYPE declarations:
   ```c
   #include MPERS_DEFS
   ```

5. **Define printer functions** using MPERS_PRINTER_DECL:
   ```c
   MPERS_PRINTER_DECL(void, print_struct_stat, struct tcb *tcp,
                      kernel_ulong_t addr)
   {
       struct_stat st;
       if (umove_or_printaddr(tcp, addr, &st))
           return;
       /* decode st fields */
   }
   ```

6. **Call MPERS functions** from other decoders:
   - Call directly as `function_name(args)`
   - The build system generates the dispatch logic automatically

## Example: print_timespec.c

```c
/* print_timespec.c */
#include "defs.h"

#include DEF_MPERS_TYPE(struct_timespec)

#include <time.h>
typedef struct timespec struct_timespec;

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, print_timespec, struct tcb *tcp,
                   kernel_ulong_t addr)
{
    struct_timespec ts;

    if (umove_or_printaddr(tcp, addr, &ts))
        return;

    tprintf("{tv_sec=%lld, tv_nsec=%lld}",
            (long long) ts.tv_sec, (long long) ts.tv_nsec);
}
```

This file will be compiled three times on x86_64 (native, m32, mx32), with
each version having different sizeof(struct_timespec) and field offsets.

## MPERS for Struct Size Only

When a struct's size differs between personalities but the fields you
need are at personality-agnostic offsets, you can MPERS just the
**size** rather than the whole fetch operation.

For example, `struct ifreq` is 40 bytes on 64-bit and 32 bytes on
32-bit (due to a pointer in the union), but `ifr_name` (offset 0) and
`ifr_flags` (offset 16) are identical in both layouts.

If an existing MPERS translation unit already has the type (e.g.,
`sock.c` has `DEF_MPERS_TYPE(struct_ifreq)`), add a size function
there rather than creating a new translation unit:

```c
MPERS_PRINTER_DECL(unsigned int, get_ifreq_size, void)
{
    return sizeof(struct_ifreq);
}
```

Then in the decoder, use `umoven_or_printaddr(tcp, arg, get_ifreq_size(), &buf)`
to read the personality-correct number of bytes, and access the
fixed-offset fields normally.

See also `get_sock_fprog_size()` in `src/fetch_bpf_fprog.c` for another
example of this pattern.

## Macro Reference

### DEF_MPERS_TYPE(type_name)

Marks a type for MPERS processing. Expands differently depending on build
context:
- During MPERS preprocessing: expands to a header file path that triggers
  type extraction
- In normal builds: expands to `"empty.h"` (no-op)

The `type_name` must be a valid C identifier (underscores instead of spaces).

### MPERS_DEFS

Includes generated personality-specific type definitions. Expands to:
- `"m32_type_defs.h"` during m32 personality build
- `"mx32_type_defs.h"` during mx32 personality build
- `"native_defs.h"` during native build

Must appear after all `DEF_MPERS_TYPE()` declarations and after including
headers that define the structures.

### MPERS_PRINTER_DECL(return_type, function_name, parameters)

Declares an MPERS-aware function with personality-specific naming. Expands to:
- `return_type function_name(parameters)` in native builds
- `return_type m32_function_name(parameters)` in m32 builds
- `return_type mx32_function_name(parameters)` in mx32 builds

The main decoder calls these functions, and the runtime selects the correct
personality variant based on `tcp->currpers`.

### MPERS_FUNC_NAME(function_name)

Generates the personality-specific function name. Rarely needed in handwritten
code - the build system auto-generates dispatch wrappers. Use only when
calling MPERS functions from within other MPERS files.
