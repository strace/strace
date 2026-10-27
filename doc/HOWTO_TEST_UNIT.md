# How to Write Unit Tests for Internal Functions and Data Structures

This guide explains how to write function-level unit tests for strace's
internal data structures and utility functions (such as `trie`, `dyxlat`).

## Overview

Most tests in strace's test suite are **integration tests** that evaluate
system call decoders:
1. A test program executes system calls.
2. strace traces the test program and records output.
3. The test framework compares the trace log against expected output.

In contrast, **unit tests** directly exercise C functions and internal data
structures without running strace:
- The test binary links the module under test directly.
- Test assertions evaluate functions, memory state, and return values in-process.
- The test harness executes the compiled binary standalone via `run_prog`.
- An exit code of 0 indicates success, 77 indicates skipped, and any non-zero
  code indicates failure.

## Architecture and Patterns

Unit tests in strace rely on four key conventions:

### 1. Inclusion Wrapper (`tests/<module>_for_tests.c`)

In strace, source files under `src/` are compiled into the `strace` binary.
Rather than compiling `src/<module>.c` directly with custom compiler flags,
strace uses an **inclusion wrapper** file in `tests/`:

```c
#include "<module>.c"
```

Why an inclusion wrapper is needed:
- **Header isolation**: `src/defs.h` (the main header for `strace`) and
  `tests/tests.h` (the test framework header) declare conflicting prototypes
  for several formatting functions (such as `print_quoted_string` and
  `printxval`). Keeping `<module>_for_tests.c` in its own translation unit
  prevents these header conflicts.
- **Function/macro mocking**: Macro overrides can be injected before the
  `#include` directive without modifying production source code in `src/`.
  For example, `tests/xmalloc_for_tests.c` overrides die-on-error behavior:
  ```c
  #define error_msg_and_die error_msg_and_fail
  #include "xmalloc.c"
  ```
- **Static visibility**: Functions with `static` linkage in `src/<module>.c`
  are accessible within the wrapper's translation unit, but not from the
  separately compiled test driver. To exercise them directly, add test code
  or externally visible test helpers after the `#include` in the wrapper.
  Declare any such helpers in a test-only header shared with the driver.

### 2. Test Driver (`tests/<module>_test.c`)

The test driver contains `main()` and test functions:

```c
#include "tests.h"
#include "<module>.h"

#include <stdio.h>
#include <string.h>

static void
assert_str_equals(const char *msg, const char *expected, const char *actual)
{
	if (!expected && !actual)
		return;
	if (!expected || !actual || strcmp(expected, actual) != 0)
		error_msg_and_fail("%s: expected \"%s\", actual \"%s\"",
		                   msg, expected ? expected : "(null)",
		                   actual ? actual : "(null)");
}

int
main(void)
{
	test_basic_operations();
	test_edge_cases();
	return 0;
}
```

Key guidelines for test drivers:
- When the module has a dedicated header (e.g. `trie.h`, `dyxlat.h`), include
  `tests/tests.h` and `<module>.h`.
- If a module is declared only in `defs.h`, extract declarations into a
  dedicated header `<module>.h` and include it in the files that use the
  interface, or include `defs.h` directly and omit `tests/tests.h` to prevent
  prototype conflicts.
- Report assertion failures with descriptive messages and exit with non-zero
  (e.g., using `error_msg_and_fail()` with `tests.h`, or `fprintf(stderr, ...)`
  and `exit(1)` with `defs.h`).
- Return 0 on success.
- If prerequisites are unavailable, exit with code 77 (skipped).

### 3. Build System Registration (`tests/Makefile.am`)

Add the test program to `check_PROGRAMS` and define its build rules:

```makefile
check_PROGRAMS = ... <module>_test ...

<module>_test_SOURCES = <module>_test.c <module>_for_tests.c
<module>_test_CPPFLAGS = $(AM_CPPFLAGS) $(CODE_COVERAGE_CPPFLAGS)
<module>_test_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS)
<module>_test_LDADD = $(LDADD) $(CODE_COVERAGE_LIBS)
```

The test program links with `libtests.a` (`$(LDADD)`), which provides the
standard test assertion and runtime functions.

Remember to add `<module>_test` to `tests/.gitignore`.

### 4. Test Runner Registration (`tests/gen_tests.in`)

Register the test in `tests/gen_tests.in` with `run_prog`:

```text
<module>_test    run_prog
```

`gen_tests.sh` treats `run_prog` as an arbitrary command (Pattern 4).
The generated `tests/<module>_test.gen.test` script sources `tests/init.sh`
and runs `run_prog` without arguments.

When called without arguments, `run_prog` (defined in `tests/init-once.sh`)
executes `../$NAME` (`../<module>_test`) directly:
- Exit 0 -> `PASS`
- Exit 77 -> `SKIP`
- Any other exit code -> `FAIL`

## Running and Debugging

### Running the Test

Run only your unit test through the automake test harness:

```bash
make -C tests check check_PROGRAMS="<module>_test" TESTS="<module>_test.gen.test"
```

Or execute the compiled binary directly:

```bash
./tests/<module>_test
echo $?
```

### Sanitizers and Memory Debugging

Because unit tests execute as normal binaries without `ptrace` or complex
inter-process coordination, they can run directly under memory debugging tools.
For example, from the root of a clean source checkout, build and run the
dyxlat test with AddressSanitizer:

```bash
./bootstrap
./configure --disable-mpers \
  CFLAGS="-g -O1 -fsanitize=address -fno-omit-frame-pointer" \
  LDFLAGS="-fsanitize=address"
make -C src all
make -C tests check check_PROGRAMS="dyxlat_test" TESTS="dyxlat_test.gen.test"
```

Use a separate checkout for this build to avoid mixing instrumented and
uninstrumented objects. The compiler and runtime must support AddressSanitizer.
Building `src` first also creates headers needed by the inclusion wrapper.

A successful ordinary test run does not establish that memory accesses are
valid. For example, the dyxlat trailing-NUL regression test exercises an
out-of-bounds read in the original implementation, but its string and size
assertions can still pass. Run it under AddressSanitizer or a memory checker
to detect that defect reliably. When validating a memory-safety regression
test, check that it passes with the fix and reports the original invalid
access without the fix.

Alternatively, use a normal, non-sanitized build with Valgrind:

```bash
valgrind --leak-check=full --error-exitcode=1 ./tests/<module>_test
```

## Existing Examples

- `tests/trie_test.c`, `tests/trie_for_tests.c`:
  Unit test for `src/trie.c` (prefix trie implementation used by PID
  namespace translation).
- `tests/dyxlat_test.c`, `tests/dyxlat_for_tests.c`:
  Unit test for `src/dyxlat.c` (dynamic xlat table operations).
