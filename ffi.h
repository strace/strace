#ifndef STRACE_FFI_H
#define STRACE_FFI_H

#include "macros.h"

#define FFI_CONCAT(a, b) a ## b
#define FFI_CONCAT2(a, b) FFI_CONCAT(a, b)

/*
 * FFI_CONTENT expands to FFI_CONTENT_ (which strigifies its arguments) when
 * FFI_CDEF is defined, and to FFI_CONTENT_FFI_CDEF (which simply expands to its
 * arguments) when it is not.
 */
#define FFI_CONTENT FFI_CONCAT2(FFI_CONTENT_, FFI_CDEF)

#define FFI_CONTENT_(...)         STRINGIFY(__VA_ARGS__)
#define FFI_CONTENT_FFI_CDEF(...) __VA_ARGS__

#endif /* !STRACE_FFI_H */
