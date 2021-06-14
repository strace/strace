Syscall Definitions
====

This syscall definition language is based on the [syzkaller description language](https://github.com/google/syzkaller/blob/master/docs/syscall_descriptions.md).

All non-syscall statements maintain their relative ordering and are placed
before syscall statements in the generated C code.

## Syntax

### Types

Types have the following format `type_name[type_option]`.
The `type_name` can include alphanumeric characters and `$_`.
The `type_option` can be another type or a number.

Numbers can be specified as a decimal number (`65`), as a hex number (`0x41`), or as a character constant (`'A'`).

The default types are the following:
 * standard C types: `void`, `int`, `char`, `long`, `uint`, `ulong`, `longlong`, `ulonglong`, `double`, `float`
 * `stddef.h` types: `size_t`, `ssize_t`, ...
 * `stdint.h` types: `uint8_t`, `int8_t`, `uint64_t`, `int64_t`, ...
 * kernel types: `kernel_long_t`, `kernel_ulong_t`, ...
 * `fd`: A file descriptor
 * `tid`: A thread id
 * `string`: A null terminated char buffer
 * `path` A null terminated path string
 * `stringnoz[n]`: A non-null terminated char buffer of length `n`
 * `const[x]`: A constant of value `x` that inherits its parent type
 * `const[x:y]`: A constant with a value between `x` and `y` (inclusive) that inherits its parent type
 * `ptr[dir, typ]`: A pointer to object of type `typ`; direction can be `in`, `out`, `inout`
 * `ref[argname]`: A reference to the value of another parameter with name `argname` or `@ret`
 * `xor_flags[xlat_name, ???, underlying_typ]`: A integer type (`underlying_typ`)
    containing mutually exclusive flags with xlat symbol name `xlat_name`
 * `or_flags[xlat_name, ???, underlying_typ]`: A integer type (`underlying_typ`)
    containing flags that are ORed together with xlat symbol name `xlat_name`

Constants (`const`) can only be used within variant syscalls.

### Syscalls
Syscall definitions have the format
```
syscall_name (arg_type1 arg_name1, arg_type2 arg_name2, ...) return_type
```

The `return_type` is optional if no special printing mode is needed.

Some system calls have various modes of operations. Consider the `fcntl` syscall.
Its second parameter determines the types of the remaining arguments. To
handle this, a variant syscall definition can be used:
```
fcntl(filedes fd, cmd xor_flags[fcntl_cmds, F_???, kernel_ulong_t], arg kernel_ulong_t) kernel_ulong_t
fcntl$F_DUPFD(filedes fd, cmd const[F_DUPFD], arg kernel_ulong_t) fd
fcntl$F_DUPFD_CLOEXEC(filedes fd, cmd const[F_DUPFD_CLOEXEC], arg kernel_ulong_t) fd
...
```

The `$` character is used to indicate that a syscall is a variant of another one.
The `const` parameters of a variant syscall will be used to determine which
variant to use. If no variant syscalls match, the base syscall will be used.

### Custom Decoders

Custom decoders have the format
```
:type[argname, arg2[$3], $1] %{
    do_something(tcp, $$, $1);
%}
```

The type following the `:` indicates which type this decoder should apply to.
Template variables (`$` followed by 1 or more numbers) can be used to reference
the value of a type option. These variables can be used within the body of the
custom decoder and will be substituted with the resolved value.

The special `$$` variable refers to the root argument.

For example, the syscall `example(arg1 type[test, type2[5], 1]` would have the
following decoder for the arg1 parameter:
```
do_something(tcp, tcp->u_arg[1], 1);
```

### #import

Import statements have the format
```
#import "filename.def"
```

The contents of the `filename.def` will be treated as if they were placed in the current file.

### #ifdef/#ifndef

Ifdef, ifndef statements have the format
```
#ifdef condition
#ifndef condition
#endif
#endif
```

Ifdef, ifndef, and define statements will be included as-is in the generated output.
Unlike C, these cannot be placed in the middle of another statement.

### define/include

Include and define statements have the format
```
define DEBUG 1
include "filename.h"
include <filename.h>
```

The contents of include and define statements will be included as-is in the generated output.
