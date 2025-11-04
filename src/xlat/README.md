# Xlat File Format

## Introduction

The xlat (translation) system in strace translates numeric constants to their
well-known symbolic names as defined in system header files.  Translation is
performed in context - the same numeric value can translate to different
symbolic names depending on the system call and argument position.  For
example, `2` translates to `SIGINT` in signal-related contexts, but to
`SEEK_END` in lseek contexts, making system call arguments recognizable
and meaningful.

The `gen.sh` script processes `*.in` xlat files to generate `*.h` header
files containing constant-to-string translation tables used throughout strace
for decoding system calls.

The generation workflow is: `*.in` → `gen.sh` → `*.h`

For the xlat structure definitions, see `src/xlat.h`.

## Basic Template Format

- **File naming convention**: `<name>.in` → `<name>.h`
- **Line-based format**: Each line represents a directive or constant entry
- **Comment syntax**: `/* ... */` (inline comments are stripped during
  processing)
- **Blank lines**: Allowed and ignored

## Constant Entry Formats

### Symbolic Constants (Most Common)

```
CONSTANT_NAME
CONSTANT_NAME   FALLBACK_VALUE
```

**`CONSTANT_NAME` alone**: References either a preprocessor constant that
evaluates to a numeric constant at compile time, or a enum constant.
**The xlat entry is generated conditionally** - only when the constant is
defined.  The generator wraps it in conditional compilation checks
(`#if defined(CONSTANT_NAME) || (defined(HAVE_DECL_CONSTANT_NAME) &&
HAVE_DECL_CONSTANT_NAME)`) unless `#unconditional` is active.  For enum
constants, there must be a corresponding configure check (typically generated
with `#enum` directive) for conditional generation to work properly.
Generates `XLAT(CONSTANT_NAME)` which expands to
`{ (unsigned)(CONSTANT_NAME), "CONSTANT_NAME" }`.  Use this format when
the xlat entry should only appear if the constant exists in system headers.

**`CONSTANT_NAME   FALLBACK_VALUE`** (with whitespace separator):
**The xlat entry is generated unconditionally** - always included in the table.
Provides a fallback value if the constant is not defined in system headers.
The fallback can be any expression that evaluates to a numeric constant at
compile time (numeric literals, macro invocations, expressions, etc.).  The
generator creates a conditional definition: if the constant isn't defined,
it defines it with the fallback value.  Then generates the xlat entry
unconditionally.  This is useful for constants that may not exist on all
platforms or kernel versions, but the xlat entry has to be present.

**Key distinction:**
- **Conditional xlat entry** (only when constant exists): Use `CONSTANT_NAME`
  alone
- **Unconditional xlat entry** (always present): Use `CONSTANT_NAME
  FALLBACK_VALUE` or the `#unconditional` directive

**Examples with numeric literals:**
- `O_CREAT` - expects O_CREAT to be defined in system headers (no fallback)
- `FUTEX_NO_NODE	-1` - uses -1 if FUTEX_NO_NODE isn't defined

**Examples with expressions:**
- `EFD_CLOEXEC  O_CLOEXEC` - falls back to `O_CLOEXEC` if `EFD_CLOEXEC`
  isn't defined
- `WDIOC_GETSUPPORT _IOR('W', 0, struct watchdog_info)` - falls back
  to the ioctl macro expression if WDIOC_GETSUPPORT isn't defined

### Symbolic Constants with Shift

```
1<<SHIFT_CONSTANT
```

Represents bit-shifted constants (e.g., `1<<_UFFDIO_REGISTER`).  The generator
recognizes the `1<<` prefix and treats it specially:
- Generates `XLAT_PAIR(1ULL<<SHIFT_CONSTANT, "1<<SHIFT_CONSTANT")` to store
  both the computed value and the symbolic representation
- Uses `1ULL` (unsigned long long) to avoid overflow for high bit positions
- The string representation preserves the shift notation for readability
- Useful for flag bits defined as shifts rather than hex values

**Example**: `1<<_UFFDIO_API` generates an entry that maps the bit value to the
string "1<<_UFFDIO_API"

### Verbatim Lines

Lines not matching the above patterns are passed through as-is to the generated
code.  This allows:
- Custom preprocessor directives (e.g., `#ifndef`, `#undef`, `#define`,
  `#endif`)
- Complex conditional compilation logic
- Multi-line constructs
- Special handling for edge cases and ABI workarounds

**Example from `wait4_options.in`:**
```
#ifndef WSTOPPED
WUNTRACED
#endif
```

These verbatim preprocessor directives are passed through to the generated
header file, allowing for complex conditional logic and workarounds for
platform-specific issues.

## Directives Reference

### `#unconditional`

Makes constants always defined (no conditional compilation).  Opposite:
`#conditional` (restores conditional behavior).

**Should only be used in xlat files where all constants are defined in bundled
headers** (headers included in strace's source tree, not system headers).

### `#conditional`

Restores conditional compilation (default behavior).  Constants wrapped in
`#if defined(CONST) || (defined(HAVE_DECL_CONST) && HAVE_DECL_CONST)`.

### `#nocheckval`

Disables value validation with static_assert.  Opposite: `#checkval` (restores
validation).

### `#checkval`

Restores value validation (default behavior).  When a constant has a fallback
value (format: `CONSTANT_NAME   FALLBACK_VALUE`), generates a static_assert to
verify that if the constant is defined in system headers, its value matches the
expected fallback value.

The validation: `static_assert((CONSTANT_NAME) == (FALLBACK_VALUE),
"CONSTANT_NAME != FALLBACK_VALUE")`

This catches cases where system headers provide an unexpected or incorrect
value for the constant.

### `#sorted [sort command]`

Marks xlat table as sorted by value (enables binary search).  Optional: specify
sort command (e.g., `#sorted sort -k2,2n`).  Generates compile-time sort order
validation using static_assert.  Sets xlat type to `XT_SORTED`.

### `#value_indexed`

Marks xlat table as indexed by value (direct array access).  Constants must
have sequential numeric values starting from 0.  Sets xlat type to
`XT_INDEXED`.

### `#val_type <type>`

Specifies the C type for constant values (default: `unsigned`).  Example:
`#val_type uint64_t`.  Used in XLAT_TYPE() and XLAT_TYPE_PAIR() macros.

### `#enum`

Generates autoconf m4 macro for checking constant declarations.  Creates
`<name>.m4` file with AC_CHECK_DECLS.  Cannot be combined with
`#unconditional`.

### `#include <header.h>`

Adds header includes to the generated m4 file.  Used with `#enum` for autoconf
checks.  Multiple includes allowed.

### `#Generated`

Marks the xlat file as automatically generated.  This directive is intended
for other tools and is currently ignored by `gen.sh`.

### `#From`

Annotates the xlat file with the source of the xlat constants.  There could be
several `#From` directives in a single xlat file.  This directive is intended
for other tools and is currently ignored by `gen.sh`.

### `#Prefix PREFIX1_ ... PREFIXn_`

Annotates the xlat file with the prefix pattern(s) to use when searching for
constants in Linux headers.  Multiple prefixes can be specified, separated by
spaces.  This directive is intended for other tools and is currently ignored
by `gen.sh`.

**Example:**
```
#From include/uapi/linux/io_uring.h
#Prefix IORING_REGISTER_ IORING_UNREGISTER_
IORING_REGISTER_BUFFERS
IORING_UNREGISTER_BUFFERS
...
```

### `#<anything else>`

Any other line starting with `#` is passed through verbatim.  Allows custom
preprocessor directives in generated code.

## Generated Output Structure

### Header File Structure

- Include guards and standard headers
- Conditional definitions with fallback values
- Static const `struct xlat_data <name>_xdata[]` array
- Static const `struct xlat <name>[1]` object

### M4 File (with `#enum`)

- Autoconf macro `st_CHECK_ENUMS_<name>`
- AC_CHECK_DECLS for all constants

### Supporting Files (Directory Mode)

- `Makemodule.am`: Make rules for all xlat files
- `.gitignore`: Ignore generated files
- `st_check_enums.m4`: Master autoconf macro

## Xlat Types and Lookup Behavior

### XT_NORMAL (Default)

- Linear search through array
- No ordering requirement

### XT_SORTED (with `#sorted`)

- Binary search (faster for large tables)
- Must be sorted by numeric value
- Compile-time validation of sort order using static_assert

### XT_INDEXED (with `#value_indexed`)

- Direct array indexing by value
- Fastest lookup
- Values must be sequential 0, 1, 2, ...

## Common Patterns and Examples

### Simple Unconditional Flags

```
src/xlat/clone_flags.in
```

### Value-Indexed Enum

```
src/xlat/bpf_map_types.in
```

### Sorted Table

```
src/xlat/nl_audit_types.in
```

### With Enum Checking

```
src/xlat/waitid_types.in
```

### Custom Value Type

```
src/xlat/uffd_api_flags.in
```

### With ioctl Definitions

```
src/xlat/watchdog_ioctl_cmds.in
```

## Best Practices

- Use `#unconditional` only in xlat files where all constants are defined
  in bundled headers (not system headers)
- Use `#sorted` for large tables (>20 entries) to enable binary search
- Use `#value_indexed` only when values are truly sequential
- Use `#enum` to generate autoconf checks for portability
- Add comments to explain unusual constants or historical changes
- Maintain consistent ordering within files
- Use `#val_type` for non-standard integer types (uint64_t, etc.)

## Generation Process

### Single File Mode

```bash
src/xlat/gen.sh input.in output.h [output.m4]
```

### Directory Mode

```bash
src/xlat/gen.sh src/xlat src/xlat
```

- Processes all `*.in` files in parallel
- Generates corresponding `*.h` and `*.m4` files
- Creates Makemodule.am and .gitignore

### Integration with Build System

- Makefile rules auto-generated in Makemodule.am
- Each `.h` depends on corresponding `.in` and gen.sh
- Regenerated automatically when template changes

## Technical Details

### Two-Pass Processing

- **Pass 1**: Output directives and conditional definitions
- **Pass 2**: Output xlat_data array entries

### Macro Definitions

- `XLAT(val)`: Standard entry
- `XLAT_PAIR(val, str)`: Custom string representation
- `XLAT_TYPE(type, val)`: Typed entry
- `XLAT_TYPE_PAIR(type, val, str)`: Typed with custom string

## Files to Reference

- `src/xlat/gen.sh` - The generator script (primary source)
- `src/xlat.h` - xlat structure definitions
- xlat files in `src/xlat/*.in`
