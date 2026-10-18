# strace Decoder API Reference

This document provides the definitive API reference for writing syscall
and ioctl decoders in strace.

For conceptual information and architecture, see
[INTERNALS.md](INTERNALS.md). For step-by-step tutorials with examples,
see [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) and
[HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md). For term definitions, see
[GLOSSARY.md](GLOSSARY.md).

## Decoder Framework

### SYS_FUNC and Decoder Return Values

Syscall decoders are defined with the `SYS_FUNC()` macro:

```c
SYS_FUNC(syscall_name)
{
	/* Decoder implementation */
	return RVAL_DECODED;
}
```

Decoder return values indicate decoding status and return value type:

- `RVAL_DECODED` -- Fully decoded, no need to call decoder on exit
- `RVAL_FD` -- Return value is a file descriptor
- `RVAL_TID` -- Return value is a thread ID
- `RVAL_SID` -- Return value is a session ID
- `RVAL_TGID` -- Return value is a thread group ID
- `RVAL_PGID` -- Return value is a process group ID
- `RVAL_STR` -- Print `auxstr` field after return value
- `RVAL_NONE` -- Print nothing (syscall has no return value)
- `RVAL_IOCTL_DECODED` -- Ioctl sub-decoder successfully decoded
  (converted to `RVAL_DECODED` by ioctl dispatcher)

Return values can be combined: `return RVAL_DECODED | RVAL_FD;`

For one-phase decoders, return `RVAL_DECODED`. For two-phase decoders,
return `0` from `entering()` to get called again at `exiting()`.

See [GLOSSARY.md](GLOSSARY.md) for detailed return value semantics.

### Syscall State Queries

```c
bool entering(struct tcb *tcp)
    /* True if at syscall-enter-stop */

bool exiting(struct tcb *tcp)
    /* True if at syscall-exit-stop */

bool syserror(struct tcb *tcp)
    /* True if syscall failed (negative return value) */

bool traced(struct tcb *tcp)
    /* True if this syscall is being traced */
```

### Verbosity Queries

```c
bool verbose(struct tcb *tcp)
    /* True if QUAL_VERBOSE set (full structure decoding) */

bool abbrev(struct tcb *tcp)
    /* True if QUAL_ABBREV set (abbreviated output) */

bool raw(struct tcb *tcp)
    /* True if QUAL_RAW set (raw numeric values) */
```

### State Storage

For two-phase decoders, store state between entry and exit:

```c
int set_tcb_priv_data(struct tcb *tcp, void *priv_data,
                      void (*free_func)(void *))
    /* Store pointer (freed on syscall end) */
    /* Returns 0 on success, -1 on failure */

void *get_tcb_priv_data(struct tcb *tcp)
    /* Retrieve pointer */

int set_tcb_priv_ulong(struct tcb *tcp, unsigned long val)
    /* Store unsigned long value */
    /* Returns 0 on success, -1 on failure */

unsigned long get_tcb_priv_ulong(struct tcb *tcp)
    /* Retrieve unsigned long value */

void free_tcb_priv_data(struct tcb *tcp)
    /* Explicitly free private data */
```

## Value Printing

### Direct Value Macros

Print values without field names:

```c
PRINT_VAL_D(value)        /* Signed decimal */
PRINT_VAL_U(value)        /* Unsigned decimal */
PRINT_VAL_X(value)        /* Hexadecimal with 0x prefix */
PRINT_VAL_0X(value)       /* Hexadecimal, padded to the value's width */
PRINT_VAL_03O(value)      /* Octal with leading zeros */
PRINT_VAL_ID(value)       /* Unsigned, except -1 printed as signed */
```

### Argument Printing

```c
unsigned int print_arg_lld(struct tcb *, unsigned int arg_no);
    /* Print next argument as signed */

unsigned int print_arg_llu(struct tcb *, unsigned int arg_no);
    /* Print next argument as unsigned */
```

## Structure Field Printing

All `PRINT_FIELD_*` macros print `field_name=value`.

### Numeric Fields

```c
PRINT_FIELD_D(where, field)          /* Signed decimal */
PRINT_FIELD_U(where, field)          /* Unsigned decimal */
PRINT_FIELD_U_CAST(where, field, type) /* Unsigned with cast */
PRINT_FIELD_X(where, field)          /* Hexadecimal */
PRINT_FIELD_X_CAST(where, field, type) /* Hexadecimal with cast */
PRINT_FIELD_0X(where, field)         /* Hexadecimal with 0x prefix */
PRINT_FIELD_U64(where, field)        /* 64-bit unsigned */
PRINT_FIELD_ADDR64(where, field)     /* 64-bit address */
```

### Special Numeric Types

```c
PRINT_FIELD_ID(where, field)         /* UID/GID/PID */
PRINT_FIELD_ERR_D(where, field)      /* Error code (signed) */
PRINT_FIELD_ERR_U(where, field)      /* Error code (unsigned) */
PRINT_FIELD_CLOCK_T(where, field)    /* clock_t value */
PRINT_FIELD_TICKS(where, field, freq, precision)
    /* Tick count conversion */
PRINT_FIELD_TICKS_D(where, field, freq, precision)
    /* Ticks as decimal */
PRINT_FIELD_DEV(where, field)        /* Device number (major:minor) */
```

### Flags and Enums

```c
PRINT_FIELD_FLAGS(where, field, xlat, dflt)
    /* OR-able flags with xlat translation */

PRINT_FIELD_FLAGS_VERBOSE(where, field, xlat, dflt)
    /* Flags, verbose output (always show numeric value) */

PRINT_FIELD_XVAL(where, field, xlat, dflt)
    /* Exclusive value (enum) with xlat translation */

PRINT_FIELD_XVAL_VERBOSE(where, field, xlat, dflt)
    /* Enum, verbose output */

PRINT_FIELD_XVAL_D(where, field, xlat, dflt)
    /* Enum as signed decimal if not found in xlat */

PRINT_FIELD_XVAL_U(where, field, xlat, dflt)
    /* Enum as unsigned decimal if not found in xlat */

PRINT_FIELD_XVAL_U_VERBOSE(where, field, xlat, dflt)
    /* Enum, unsigned verbose */
```

### Strings

```c
PRINT_FIELD_CSTRING(where, field)
    /* C string (null-terminated) in fixed-size char array */

PRINT_FIELD_CSTRING_SZ(where, field, size)
    /* C string with explicit size limit */

PRINT_FIELD_STRING(where, field, len, style)
    /* String with length and quote style */

PRINT_FIELD_CHAR(where, field, flags)
    /* Character field with quote style flags */
```

### Pointers and Addresses

```c
PRINT_FIELD_PTR(where, field)
    /* Pointer field (prints address) */
```

### File Descriptors and PIDs

```c
PRINT_FIELD_FD(where, field, tcp)
    /* File descriptor (with -y path resolution) */

PRINT_FIELD_DIRFD(where, field, tcp)
    /* Directory file descriptor */

PRINT_FIELD_TGID(where, field, tcp)
    /* Thread group ID */

PRINT_FIELD_TID(where, field, tcp)
    /* Thread ID */
```

### Syscall Names

```c
PRINT_FIELD_SYSCALL_NAME(where, field, audit_arch)
    /* Syscall name field (decodes syscall number to name) */
```

### Network Fields

```c
PRINT_FIELD_INET_ADDR(where, field, af)
    /* IPv4/IPv6 address (af is AF_INET or AF_INET6) */

PRINT_FIELD_NET_PORT(where, field)
    /* Network port (converts from network byte order) */

PRINT_FIELD_IFINDEX(where, field)
    /* Network interface index (resolves to name) */

PRINT_FIELD_SOCKADDR(where, field, tcp)
    /* Socket address structure */
```

### MAC and Hardware Addresses

```c
PRINT_FIELD_MAC(where, field)
    /* MAC address (6-byte array) */

PRINT_FIELD_MAC_SZ(where, field, size)
    /* MAC address with explicit size */

PRINT_FIELD_HWADDR_SZ(where, field, size, hwtype)
    /* Hardware address with device type */
```

### Arrays

```c
PRINT_FIELD_ARRAY(where, field, tcp, print_func)
    /* Print array using print_func callback */

PRINT_FIELD_ARRAY_INDEXED(where, field, tcp, print_func,
                           ind_xlat, ind_dflt)
    /* Print array with xlat-based index labels */

PRINT_FIELD_ARRAY_UPTO(where, field, upto, tcp, print_func)
    /* Print array up to 'upto' elements */

PRINT_FIELD_D_ARRAY(where, field)
    /* Array of signed decimals */

PRINT_FIELD_U_ARRAY(where, field)
    /* Array of unsigned decimals */

PRINT_FIELD_X_ARRAY(where, field)
    /* Array of hex values */

PRINT_FIELD_X_ARRAY2D(where, field)
    /* 2D array of hex values */

PRINT_FIELD_HEX_ARRAY(where, field)
    /* Byte array as hex dump */

PRINT_FIELD_HEX_ARRAY_UPTO(where, field, count)
    /* Hex dump up to count bytes */

PRINT_FIELD_COOKIE(where, field)
    /* Cookie (two-element array printed as unsigned decimals) */

PRINT_FIELD_UUID(where, field)
    /* UUID (RFC 4122 format) */

PRINT_FIELD_VAL_ARRAY(where, field, print_val_fn)
    /* Generic array with custom value printer */

PRINT_FIELD_VAL_ARRAY2D(where, field, print_val_fn)
    /* Generic 2D array with custom value printer */
```

### Generic Object Delegation

Call custom print functions on struct fields:

```c
PRINT_FIELD_OBJ_PTR(where, field, print_func, ...)
    /* Calls print_func(&field, ...) */

PRINT_FIELD_OBJ_TCB_PTR(where, field, tcp, print_func, ...)
    /* Calls print_func(tcp, &field, ...) */

PRINT_FIELD_OBJ_VAL(where, field, print_func, ...)
    /* Calls print_func(field, ...) */

PRINT_FIELD_OBJ_U(where, field, print_func, ...)
    /* Calls print_func(zero_extend(field), ...) */

PRINT_FIELD_OBJ_TCB_VAL(where, field, tcp, print_func, ...)
    /* Calls print_func(tcp, field, ...) */
```

### Conditional Field Printing

```c
MAYBE_PRINT_FIELD_LEN(prefix, where, field, len, print_func, ...)
    /* Print field only if len covers it; print hex if partial */
```

## Memory Access

### Fetching from Tracee Memory

```c
int umoven(struct tcb *tcp, kernel_ulong_t addr,
           unsigned int len, void *our_addr)
    /* Fetch len bytes from tracee memory */
    /* Returns 0 on success, -1 on failure */

int umove(struct tcb *tcp, kernel_ulong_t addr, void *our_addr)
    /* Macro wrapper for fetching objects */

int umove_or_printaddr(struct tcb *tcp, kernel_ulong_t addr,
                       void *our_addr)
    /* Fetch data; if fails, print address and return true */
    /* Returns 0 on success, 1 on failure (address printed) */
    /* Typical usage:
     *   if (umove_or_printaddr(tcp, addr, &st))
     *       return RVAL_DECODED;
     */

bool tfetch_obj(struct tcb *tcp, kernel_ulong_t addr, void *our_addr)
    /* Boolean variant: true on success */

bool tfetch_mem(struct tcb *tcp, kernel_ulong_t addr,
                unsigned int len, void *our_addr)
    /* Boolean fetch: true on success */

bool tfetch_mem64(struct tcb *tcp, uint64_t addr,
                  unsigned int len, void *laddr)
    /* 64-bit address variant */

bool tfetch_to_uint64(struct tcb *tcp, kernel_ulong_t addr,
                      unsigned int len, uint64_t *laddr)
    /* Fetch and convert to uint64 */

bool tfetch_to_uint64_64(struct tcb *tcp, uint64_t addr,
                         unsigned int len, uint64_t *laddr)
    /* 64-bit address variant */

int umovestr(struct tcb *tcp, kernel_ulong_t addr,
             unsigned int len, void *our_addr)
    /* Fetch null-terminated string (up to len bytes) */
```

### Ignore-Syserror Variants

For two-phase decoders, use these in `exiting()` when the syscall may
have failed but you still need to read memory:

```c
bool tfetch_obj_ignore_syserror(struct tcb *tcp, kernel_ulong_t addr,
                                void *our_addr)

int umove_or_printaddr_ignore_syserror(struct tcb *tcp,
                                       kernel_ulong_t addr,
                                       void *our_addr)
```

### Printing from Tracee Memory

```c
void printaddr(kernel_ulong_t addr)
    /* Print address in hexadecimal */

void printaddr64(uint64_t addr)
    /* Print 64-bit address */

int printstr(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print null-terminated string */

int printstrn(struct tcb *tcp, kernel_ulong_t addr,
              kernel_ulong_t len)
    /* Fetch and print string with length */

int printstr_ex(struct tcb *tcp, kernel_ulong_t addr,
                kernel_ulong_t len, unsigned int flags)
    /* Print string with options (quote style, etc.) */

int printpath(struct tcb *tcp, kernel_ulong_t addr)
    /* Print filesystem path (resolves with -y if applicable) */

int printpathn(struct tcb *tcp, kernel_ulong_t addr,
               unsigned int n)
    /* Print path with max length */

void printfd(struct tcb *tcp, int fd)
    /* Print file descriptor (shows path with -y) */

int print_quoted_string(const char *str, unsigned int size,
                        unsigned int flags)
    /* Print quoted string (already in tracer memory) */

int print_quoted_string_ex(const char *str, unsigned int size,
                           unsigned int flags,
                           const char *escape_chars)
    /* Extended variant with custom escape chars */

int print_quoted_cstring(const char *str, unsigned int size)
    /* Print C string with quotes */
```

### Numeric Value Fetch-and-Print

Fetch single numeric value from tracee memory and print it:

```c
void printnum_int(struct tcb *tcp, kernel_ulong_t addr,
                  const char *fmt)
    /* Fetch and print int with format string */

void printnum_int64(struct tcb *tcp, kernel_ulong_t addr,
                    const char *fmt)
    /* Fetch and print int64 with format string */

void printnum_fd(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print file descriptor */

void printnum_pid(struct tcb *tcp, kernel_ulong_t addr,
                  enum pid_type type)
    /* Fetch and print PID with type (PT_TID/PT_TGID/etc.) */

void printnum_slong(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print signed long (personality-aware) */

void printnum_ulong(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print unsigned long (personality-aware) */

void printnum_ptr(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print pointer (personality-aware) */

void printnum_kptr(struct tcb *tcp, kernel_ulong_t addr)
    /* Fetch and print kernel pointer (klongsize-aware) */
```

### Numeric Pair Fetch-and-Print

```c
void printpair_int(struct tcb *tcp, kernel_ulong_t addr,
                   const char *fmt)
    /* Fetch and print pair of ints */

void printpair_int64(struct tcb *tcp, kernel_ulong_t addr,
                     const char *fmt)
    /* Fetch and print pair of int64s */
```

## Xlat (Translation) Printing

### Exclusive Values (Enums)

```c
void printxval(const struct xlat *xlat, uint64_t val,
               const char *dflt)
    /* Print enum value (personality-aware) */

void printxval64(const struct xlat *xlat, uint64_t val,
                 const char *dflt)
    /* 64-bit variant */

void printxval_d(const struct xlat *xlat, int64_t val,
                 const char *dflt)
    /* Signed variant */

void printxval_u(const struct xlat *xlat, uint64_t val,
                 const char *dflt)
    /* Unsigned variant */
```

### Multi-Xlat Values

```c
void printxvals(uint64_t val, const char *dflt, ...)
    /* Print value using multiple xlat tables (sentinel-terminated) */

void printxval_ex(const struct xlat *xlat, uint64_t val,
                  const char *dflt, enum xlat_style style)
    /* Print value with explicit xlat_style */
```

### Flags (OR-able)

```c
int printflags(const struct xlat *xlat, uint64_t flags,
               const char *dflt)
    /* Print OR-able flags (personality-aware) */

int printflags64(const struct xlat *xlat, uint64_t flags,
                 const char *dflt)
    /* 64-bit variant */

int printflags_ex(uint64_t flags, const char *dflt,
                  enum xlat_style, const struct xlat *, ...)
                  ATTRIBUTE_SENTINEL
    /* Extended flags printer with sentinel-terminated xlat list */
```

### Raw Xlat Printing

```c
void print_xlat_ex(uint64_t val, const char *str,
                   enum xlat_style style)
    /* Print raw xlat with explicit style */

void print_xlat(uint64_t val)
    /* Print raw xlat (default style) */

void print_xlat32(uint32_t val)
    /* 32-bit variant */

void print_xlat_u(uint64_t val)
    /* Unsigned variant */

void print_xlat_d(int64_t val)
    /* Signed variant */
```

### Verbosity Control

The output style depends on `-Xabbrev`, `-Xverbose`, `-Xraw`:

- **Abbrev**: `SEEK_END`
- **Verbose**: `0x2 /* SEEK_END */`
- **Raw**: `0x2`

## Output Formatting

### Structure Delimiters

```c
void tprint_struct_begin(void)
    /* Print opening brace: { */

void tprint_struct_next(void)
    /* Print comma separator: , */

void tprint_struct_end(void)
    /* Print closing brace: } */
```

Typical usage:

```c
tprint_struct_begin();
PRINT_FIELD_U(st, value1);
tprint_struct_next();
PRINT_FIELD_X(st, value2);
tprint_struct_end();
/* Prints: {value1=42, value2=0xff} */
```

### Union Delimiters

```c
void tprint_union_begin(void)
void tprint_union_next(void)
void tprint_union_end(void)
```

### Array Delimiters

```c
void tprint_array_begin(void)
void tprint_array_next(void)
void tprint_array_end(void)
    /* Array delimiters: [ ] with comma separators */

void tprint_array_index_begin(void)
    /* Print [ for array index */

void tprint_array_index_equal(void)
    /* Print ]= for array index assignment */

void tprint_array_index_end(void)
    /* End array index (noop) */
```

### Argument Separators

```c
void tprint_arg_next(void)
    /* Print comma between syscall arguments */

void tprints_arg_name(const char *name)
    /* Print named argument (for --decode-pids=comm, etc.) */

void tprints_arg_next_name(const char *name)
    /* Combine: comma + named argument */
```

### Function-Call Delimiters

For call-style decoder output such as `htons(...)`, `_IOC(...)`,
`makedev(...)`, use the `fn` family of printers:

```c
void tprints_fn_begin(const char *name)
    /* Print "name(" for call-style decoder output */

void tprint_fn_next(void)
    /* Print comma between call-style arguments */

void tprint_fn_end(void)
    /* Print ")" */
```

The `arg` variants frame the top-level syscall arguments and should
not be used for nested call-style output:

```c
void tprints_arg_begin(const char *name)
    /* Print "name(" for syscall argument framing */

void tprint_arg_end(void)
    /* Print ")" */
```

### Field Names

```c
void tprints_field_name(const char *name)
    /* Print "name=" for field output */
```

### Comment Output

```c
void tprint_comment_begin(void)
    /* Print " /* " */

void tprint_comment_end(void)
    /* Print " */" */

void tprintf_comment(const char *fmt, ...)
    /* Print formatted comment */

void tprints_comment(const char *str)
    /* Print comment string */
```

### Flag Composition

For manual flag composition (when not using printflags):

```c
void tprint_flags_begin(void)
    /* Begin flag composition (noop) */

void tprint_flags_or(void)
    /* Print "|" between flags */

void tprint_flags_end(void)
    /* End flag composition (noop) */
```

### Bitset Delimiters

```c
void tprint_bitset_begin(void)
void tprint_bitset_next(void)
void tprint_bitset_end(void)
```

### Indicators and Decorators

```c
void tprint_value_changed(void)
    /* Print " => " to show value change */

void tprint_more_data_follows(void)
    /* Print "..." for truncated output */

void tprint_null(void)
    /* Print "NULL" */

void tprint_indirect_begin(void)
void tprint_indirect_end(void)
    /* Print [ ] for indirect values */

void tprint_attribute_begin(void)
void tprint_attribute_end(void)
    /* Print [ ] for attributes */

void tprint_associated_info_begin(void)
void tprint_associated_info_end(void)
    /* Print < > */
```

## Array Printing

### Generic Array Printer

```c
bool print_array(struct tcb *tcp,
                 kernel_ulong_t start_addr, size_t nmemb,
                 void *elem_buf, size_t elem_size,
                 tfetch_mem_fn tfetch_mem_func,
                 print_fn print_func, void *opaque_data)
    /* Generic array printer with callback */
    /* Returns true if all elements printed */

bool print_array_ex(struct tcb *tcp,
                    kernel_ulong_t addr, size_t nmemb,
                    void *elem_buf, size_t elem_size,
                    tfetch_mem_fn tfetch_fn, print_fn print_fn,
                    void *opaque, unsigned int flags,
                    const struct xlat *index_xlat,
                    const char *index_dflt)
    /* Extended array printer with index xlat and flags */
```

Array callback signatures:

```c
typedef bool (*print_fn)(struct tcb *, void *elem_buf,
                         size_t elem_size, void *opaque_data);

typedef bool (*tfetch_mem_fn)(struct tcb *, kernel_ulong_t addr,
                              unsigned int size, void *dest);
```

### Local Array Helpers

For arrays already in tracer memory:

```c
print_local_array(tcp, arr, print_func)
    /* Macro: print local array (size determined from array type) */

void print_local_array_upto(struct tcb *tcp, const void *arr,
                            size_t upto, print_fn print_func)
    /* Print local array up to 'upto' elements */

void print_local_array_ex(struct tcb *tcp, const void *arr,
                          size_t nmemb, size_t elem_size,
                          print_fn print_func, void *opaque,
                          unsigned int flags,
                          const struct xlat *index_xlat,
                          const char *index_dflt)
    /* Extended local array printer */
```

### Array Element Callbacks

Pre-defined callbacks for common array types:

```c
bool print_int_array_member(struct tcb *tcp, void *elem_buf,
                            size_t elem_size, void *data)

bool print_uint_array_member(struct tcb *tcp, void *elem_buf,
                             size_t elem_size, void *data)

bool print_xint_array_member(struct tcb *tcp, void *elem_buf,
                             size_t elem_size, void *data)

bool print_fd_array_member(struct tcb *tcp, void *elem_buf,
                           size_t elem_size, void *data)
```

## Special Value Printing

### File Descriptors

```c
void printfd(struct tcb *tcp, int fd)
    /* Print file descriptor (shows path with -y) */

void printfd_pid(struct tcb *tcp, int pid, int fd)
    /* Print fd for specific pid */

void print_dirfd(struct tcb *tcp, int fd)
    /* Print directory fd (AT_FDCWD special-cased) */
```

### PIDs and UIDs

```c
void printpid(struct tcb *tcp, int pid, enum pid_type type)
    /* Print PID with optional comm resolution */

void printpid_tgid_pgid(struct tcb *tcp, int pid)
    /* Print TGID if positive, PGID if negative */

void printuid(uid_t uid)
    /* Print UID */
```

### Signals

```c
void printsignal(int sig)
    /* Print signal value */

void print_sigset_addr(struct tcb *tcp, kernel_ulong_t addr)
    /* Print signal set */

void print_wait_status(int status)
    /* Print wait status */
```

### Device Numbers and File Modes

```c
void print_dev_t(unsigned long long dev)
    /* Device number as major:minor */

void print_symbolic_mode_t(unsigned int mode)
    /* File mode bits (rwxr-xr-x) */

void print_numeric_umode_t(unsigned short mode)
    /* Numeric mode (0755) */
```

### I/O Vectors

```c
void tprint_iov(struct tcb *tcp, kernel_ulong_t len,
                kernel_ulong_t addr, print_fn print_func)
    /* Print I/O vector array */

void tprint_iov_upto(struct tcb *tcp, kernel_ulong_t len,
                     kernel_ulong_t addr, kernel_ulong_t data_size,
                     print_fn print_func, void *opaque_data)
    /* Print I/O vector array up to data_size bytes */
```

### Other Special Values

```c
void print_err(int64_t err, bool negated)
    /* Print error code with name */

void print_uuid(const unsigned char uuid[16])
    /* UUID in standard format */

void print_mac_addr(const char *prefix,
                    const unsigned char addr[6])
    /* MAC address */

void print_kernel_version(unsigned long version)
    /* Kernel version (major.minor.patch) */

void print_ioprio(unsigned int ioprio)
    /* I/O priority class and level */
```

## Timing Functions

```c
const char *sprinttime(long long sec)
    /* Format time_t as string */

const char *sprinttime_usec(long long sec, unsigned long long usec)
    /* Format time with microseconds */

const char *sprinttime_nsec(long long sec, unsigned long long nsec)
    /* Format time with nanoseconds */

void print_timespec64(struct tcb *tcp, kernel_ulong_t addr)
    /* Print timespec64 from tracee memory */

const char *sprint_timespec64(struct tcb *tcp, kernel_ulong_t addr)
    /* Sprint timespec64 from tracee memory */

void print_itimerspec64(struct tcb *tcp, kernel_ulong_t addr)
    /* Print itimerspec64 from tracee memory */
```

## Common Patterns

For decoder implementation patterns and examples, see
[HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) for syscall decoders,
[HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md) for ioctl decoders, and
[INTERNALS.md](INTERNALS.md) for architectural details.

Minimal example illustrating `RVAL_DECODED`:

```c
SYS_FUNC(getpid)
{
	/* No arguments, just return */
	return RVAL_DECODED;
}
```

## See Also

- **[INTERNALS.md](INTERNALS.md)** - Architecture and design
  overview: ptrace event loop, decoder dispatch, syscall tables
- **[HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md)** - Step-by-step
  guide with examples for adding syscall decoders
- **[HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md)** - Ioctl decoder guide
- **[GLOSSARY.md](GLOSSARY.md)** - Term definitions including RVAL
  semantics, decoder concepts, xlat system
- **src/print_fields.h** - PRINT_FIELD macros and printer declarations
- **src/print_fields.c** - Printer function implementations
- **src/defs.h** - Function prototypes and core definitions (2200+
  lines)

## Quick Tips

1. Always use `umove_or_printaddr()` to safely fetch structures from
   tracee memory
2. Check `syserror()` in `exiting()` before decoding output structures
3. Use xlat translations instead of raw numbers for constants and flags
4. Use `PRINT_FIELD_*` macros instead of manual formatting
5. Call `tprint_struct_next()` between structure fields
6. Call `tprint_arg_next()` between syscall arguments
