# How to Write and Run Tests for strace

This guide explains strace's test framework and how to write tests for
syscall decoders and other functionality.

## Overview

strace tests work by:

1. Running a test program that invokes system calls
2. Capturing strace's output for those syscalls
3. Comparing the output against expected output

Tests are designed to be deterministic and self-contained. Most test
programs print their own expected output to stdout, which the test
framework compares against strace's actual log output.

## Test File Types

### .c files (test programs)

Test program source code. These programs:

- Invoke the syscalls being tested
- Print expected strace output to stdout (or fd 3 via `tprintf()`)
- Use macros and helpers from `tests/tests.h`
- Exit with code 0 on success, 77 to skip

Example: `tests/openat.c`

All test programs are compiled and linked against `libtests.a`, which
contains common helper functions.

### .gen.test files (auto-generated shell scripts)

Auto-generated shell scripts created by `tests/gen_tests.sh` from entries
in `tests/gen_tests.in`. These are the most common type of test. The vast
majority of strace tests use this mechanism.

Generated test scripts source `tests/init.sh` (which sources
`tests/init-once.sh`), run the test program under strace, and compare
output.

### .test files (hand-written shell scripts)

Hand-written shell scripts for tests requiring custom logic beyond what
`gen_tests.in` supports. Examples include tests with complex setup,
multiple execution phases, or dynamic expected output generation.

Example: `tests/ioctl.test`

These tests are registered in `tests/Makefile.am` in either the
`DECODER_TESTS` or `MISC_TESTS` lists.

### .expected files

Pre-generated files used by `match_diff()` or `match_grep()`:

- **For `match_diff()`**: Contains literal expected output that is
  compared byte-for-byte with strace's log via `diff -u`
- **For `match_grep()`**: Contains grep patterns (one per line) that are
  matched against strace output using `grep -E -x`. Patterns can be
  negated with a `!` prefix

Example: `tests/eventfd.expected` contains:
```
eventfd2(4294967295, EFD_SEMAPHORE|EFD_CLOEXEC|EFD_NONBLOCK) = 0
+++ exited with 0 +++
```

### .awk files

AWK programs used with `match_awk()` for flexible output validation when
exact string matching is insufficient. The AWK program is executed with
strace's log as input; if it returns non-zero, the test fails.

### Wrapper .c files (test variants)

The most common way to create test variants (-P, -v, -success, -Xraw,
etc.) is a simple wrapper .c file that defines a preprocessor macro and
includes the main test:

**tests/bpf-v.c** (verbose variant):
```c
#define VERBOSE 1
#include "bpf.c"
```

**tests/fsconfig-P.c** (path-tracing variant):
```c
#define PATH_TRACING
#include "fsconfig.c"
```

**tests/bpf-success.c** (injection variant):
```c
#define INJECT_RETVAL 42
#include "bpf.c"
```

The main test file (e.g., `bpf.c`) uses `#ifdef VERBOSE`, `#ifdef
PATH_TRACING`, or `#ifdef INJECT_RETVAL` to conditionally adjust its
output for the variant.

This pattern is used by hundreds of test variants throughout the test
suite.

## gen_tests.in Format

The `tests/gen_tests.in` file is the primary test registration mechanism.
Each line defines one test. Lines beginning with `#` are comments.

### Format Overview

Each line has the format:

```
NAME    ARG0    ARGS...
```

The `ARG0` field determines which of four code generation patterns
`gen_tests.sh` uses:

1. **Empty or starts with `-`** (standard strace flags) — generates
   `run_strace_match_diff $ARG0 $ARGS`
2. **Starts with `+`** (delegate to script) — sources the named script
   with arguments
3. **`-einject=` or `--inject=`** (injection test) — sources
   `scno_tampering.sh`, then runs strace with injection
4. **Anything else** (arbitrary command) — runs the command verbatim
   (e.g., `test_trace_expr`, `test_pidns`)

### Standard Tests (Pattern 1)

Most tests use this pattern. `ARG0` is either empty or begins with `-`.

**Bare test name** (no extra strace flags):
```
_newselect
```

Generates a script that calls `run_strace_match_diff` with no arguments.
The function auto-adds `-e trace=_newselect` at runtime.

**Test with strace flags**:
```
accept -a22
openat -a36 -P $NAME.sample
access -a30 --trace-path=access_sample
```

Generates scripts that call `run_strace_match_diff -a22`,
`run_strace_match_diff -a36 -P openat.sample`, etc.

Note: The `--trace=<name>` flag is NOT added by `gen_tests.sh`. It is
added at runtime by `run_strace_match_diff()` (in `init-once.sh`) if no
`-e trace=`, `-etrace=`, or `--trace=` is present in the arguments.

### Script Delegation (Pattern 2)

Delegate to a shared test script. `ARG0` starts with `+`.

**Example**:
```
arch_prctl +arch_prctl.sh -a27
btrfs +ioctl.test
```

Generates a script that does:
```sh
set -- -a27
. "${srcdir=.}/arch_prctl.sh"
```

The script receives arguments via `$@`. Both `.sh` helper scripts and
`.test` files can be used.

### Injection Tests (Pattern 3)

Test fault injection with `--inject=`. `ARG0` is `-einject=...` or
`--inject=...`.

**Example**:
```
bpf-success -einject=bpf:retval=42 -etrace=bpf -a20
```

Generated script sources `scno_tampering.sh` (which checks if the
architecture supports syscall number tampering), then runs strace with
the injection arguments.

If the injection arguments include `-y` (fd path decoding), the generated
script uses `run_strace_match_diff`. Otherwise, it uses explicit
`run_strace`/`match_diff` calls.

Note: `scno_tampering.sh` calls `check_scno_tampering()`, which skips the
test on architectures or kernel versions that don't support injection.

### Arbitrary Commands (Pattern 4)

Run a custom shell function. `ARG0` is anything else.

**Example**:
```
clock test_trace_expr 'times|times-.*|fcntl.*' -e/clock
fcntl--pidns-translation test_pidns -a8 -e trace=fcntl
```

Generates a script that sources `init.sh` and runs the command verbatim:
```sh
. "${srcdir=.}/init.sh"
test_trace_expr 'times|times-.*|fcntl.*' -e/clock
```

`test_trace_expr()` and `test_pidns()` are functions defined in
`init-once.sh`.

## Writing a Test Program

### Basic Structure

A minimal test program:

```c
#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	/* Invoke the syscall */
	long rc = syscall(__NR_read, 0, NULL, 0);

	/* Print expected strace output */
	printf("read(0, NULL, 0) = %s\n", sprintrc(rc));

	/* Indicate successful test completion */
	puts("+++ exited with 0 +++");
	return 0;
}
```

**Key points**:
- Include `tests.h` (test framework) and `scno.h` (syscall numbers)
- Call syscalls via `syscall(__NR_...)` for architecture independence
- Print expected strace output to stdout
- End with `puts("+++ exited with 0 +++")` — this matches strace's exit
  message

For syscalls that might not be available on all architectures, add guards:

```c
#include "tests.h"
#include "scno.h"

#ifdef __NR_openat

# include <asm/fcntl.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	/* ... test code ... */
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_openat")

#endif
```

All tests that `#include "scno.h"` can rely on the fact that all
`__NR_xxx` constants known for this architecture are defined. Guards are
only needed for syscalls that are not available on all architectures (e.g.,
architecture-specific syscalls, or newer syscalls not yet available
everywhere)

### Printing Expected Output: printf vs tprintf

Most tests use standard `printf()` / `puts()` to print expected output to
stdout. The test framework captures this and compares it against strace's
log.

For tests that trace their own I/O operations (read, write, etc.), use
`tprintf()` instead:

```c
#include "tests.h"

int
main(void)
{
	tprintf("read(0, \"\", 0) = 0\n");
	/* ... */
	tprintf("+++ exited with 0 +++\n");
	return 0;
}
```

`tprintf()` (defined in `tests/tprintf.c`) closes stdin, moves stdout to
fd 3, and writes there. This prevents test output from being interleaved
with traced I/O on stdout.

### Using sprintrc() for Return Values

`sprintrc()` formats a syscall return value the way strace does:

```c
#include "tests.h"

long rc = syscall(__NR_openat, AT_FDCWD, "/nonexistent", O_RDONLY);

/* For rc = -1, errno = ENOENT: */
printf("openat(AT_FDCWD, \"/nonexistent\", O_RDONLY) = %s\n", sprintrc(rc));
/* Prints: openat(...) = -1 ENOENT (No such file or directory) */

/* For rc = 3: */
printf("openat(AT_FDCWD, \"/tmp/test\", O_RDONLY) = %s\n", sprintrc(rc));
/* Prints: openat(...) = 3 */
```

`sprintrc()` signature:
```c
const char *sprintrc(long rc);
```

Returns:
- `"0"` if `rc == 0`
- `"-1 ERRNO (description)"` if `rc == -1` (uses `errno`)
- `"N"` otherwise (decimal value)

For `match_grep()` tests, use `sprintrc_grep()` instead — it escapes
parentheses for grep pattern matching.

### Skipping Unavailable Syscalls

Tests that `#include "scno.h"` can rely on all `__NR_xxx` constants for
syscalls wired up to the current architecture being defined. The
`tests/scno.h` file is generated from strace's syscallent tables and
provides fallback definitions for all syscalls present in those tables.

Guards are only needed for syscalls NOT yet wired up to all supported
architectures.

**When to use `#ifdef __NR_xxx` guards:**
- Architecture-specific syscalls (e.g., `__NR_arch_prctl` on x86)
- Newer syscalls not yet wired up to all supported architectures — i.e.,
  not yet present (directly or via `#include "generic/syscallent-common.h"`)
  in syscallent tables for all architectures
- Syscalls with complex availability (e.g., `__NR_getpid` which might be
  aliased to `__NR_getxpid` on some architectures)

**When guards are NOT needed:**
- Universal syscalls available on all architectures (e.g., `__NR_read`,
  `__NR_write`, `__NR_close`)
- Newer syscalls already wired up to all supported architectures — i.e.,
  already present in syscallent tables for all architectures (directly or
  indirectly via generic/syscallent-common.h). Once a syscall is wired up
  everywhere, `scno.h` guarantees `__NR_xxx` availability even if the
  kernel headers don't define it yet.

The key distinction: a "newer" syscall needs guards only during the
transition period when it's being wired up to different architectures. Once
it's in all syscallent tables, `scno.h` provides the definition and guards
are no longer needed.

For syscalls that might be unavailable, use `SKIP_MAIN_UNDEFINED()`:

```c
#ifdef __NR_openat2

# include <stdio.h>
/* ... test code ... */

#else

SKIP_MAIN_UNDEFINED("__NR_openat2")

#endif
```

This macro expands to:
```c
int main(void) { error_msg_and_skip("undefined: %s", arg); }
```

The test exits with code 77, which the framework treats as "skipped".

For runtime availability checks (e.g., kernel feature support), use
`error_msg_and_skip()` or `perror_msg_and_skip()`:

```c
int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_CRYPTO);
if (fd < 0)
	perror_msg_and_skip("NETLINK_CRYPTO socket");
```

### Memory Allocation: TAIL_ALLOC_OBJECT_*

For testing strace's handling of partially accessible memory, use
tail-allocated objects:

```c
#include "tests.h"

TAIL_ALLOC_OBJECT_CONST_PTR(struct iovec, iov);
/* iov is a (struct iovec *const) pointing to memory at the end
   of a mapped page, with unmapped pages on both sides */

iov->iov_base = tail_alloc(42);
iov->iov_len = 42;

/* Passing iov+1 to a syscall will trigger EFAULT */
```

Available macros:
- `TAIL_ALLOC_OBJECT_CONST_PTR(type, ptr)` — const pointer
- `TAIL_ALLOC_OBJECT_VAR_PTR(type, ptr)` — variable pointer
- `TAIL_ALLOC_OBJECT_CONST_ARR(type, ptr, count)` — const array pointer
- `TAIL_ALLOC_OBJECT_VAR_ARR(type, ptr, count)` — variable array pointer

All use `tail_alloc()` internally, which allocates memory ending at a page
boundary with guard pages.

### Common Helper Functions and Macros

From `tests/tests.h`:

**Skip/fail functions**:
```c
void error_msg_and_skip(const char *fmt, ...);
void perror_msg_and_skip(const char *fmt, ...);
void error_msg_and_fail(const char *fmt, ...);
void perror_msg_and_fail(const char *fmt, ...);
```

**Errno/signal name conversion**:
```c
const char *errno2name(void);     /* Returns "ENOENT", etc. */
const char *signal2name(int sig); /* Returns "SIGINT", etc. */
```

**Argument printing helpers**:
```c
#define ARG_STR(arg) (arg), #arg
/* Usage: printxval(xlat, ARG_STR(O_RDONLY)) */

#define ARG_ULL_STR(arg) (arg##ULL), #arg
```

**Return value string macros**:
```c
#define RVAL_EBADF    " = -1 EBADF (%m)\n"
#define RVAL_EFAULT   " = -1 EFAULT (%m)\n"
#define RVAL_EINVAL   " = -1 EINVAL (%m)\n"
/* %m is glibc extension for strerror(errno) */
```

**File/directory helpers**:
```c
int get_dir_fd(const char *dir_path);
void create_and_enter_subdir(const char *subdir);
void leave_and_remove_subdir(void);
```

## Test Variants

### Verbose Tests (-v / VERBOSE)

Create a wrapper file that defines `VERBOSE`:

**tests/bpf-v.c**:
```c
#define VERBOSE 1
#include "bpf.c"
```

The main test uses:
```c
#if VERBOSE
	/* Print full structure contents */
#else
	/* Print abbreviated output */
#endif
```

Register in `gen_tests.in`:
```
bpf-v -a20 -v -e trace=bpf
```

### Path Tracing Tests (-P / PATH_TRACING)

Create a wrapper that defines `PATH_TRACING`:

**tests/fsconfig-P.c**:
```c
#define PATH_TRACING
#include "fsconfig.c"
```

The main test uses:
```c
#ifndef PATH_TRACING
	/* This syscall involves a non-traced path, don't print it */
#endif
```

Register in `gen_tests.in`:
```
fsconfig-P -a37 -P $NAME.sample -e trace=fsconfig
```

Some tests define `PATH_TRACING_FD` instead to specify which fd is traced:
```c
#define PATH_TRACING_FD 9
#include "select.c"
```

### Injection Tests (-success / INJECT_RETVAL)

Create a wrapper that defines `INJECT_RETVAL`:

**tests/bpf-success.c**:
```c
#define INJECT_RETVAL 42
#include "bpf.c"
```

The main test uses:
```c
#ifdef INJECT_RETVAL
	printf("bpf(...) = %d (INJECTED)\n", INJECT_RETVAL);
#else
	printf("bpf(...) = %s\n", sprintrc(rc));
#endif
```

Register in `gen_tests.in`:
```
bpf-success -einject=bpf:retval=42 -etrace=bpf -a20
```

### Output Format Tests (-Xraw, -Xverbose / XLAT_RAW, XLAT_VERBOSE)

The `XLAT_RAW` and `XLAT_VERBOSE` preprocessor macros control xlat output
format testing.

**tests/bpf-Xabbrev.c**:
```c
/* No XLAT defines — this is the default/abbreviated mode */
#include "bpf.c"
```

**tests/bpf-Xraw.c**:
```c
#define XLAT_RAW 1
#include "bpf.c"
```

**tests/bpf-Xverbose.c**:
```c
#define XLAT_VERBOSE 1
#include "bpf.c"
```

The main test uses the `XLAT_KNOWN()`, `XLAT_UNKNOWN()`, and related
macros to print different output based on the mode:

```c
printf("bpf(BPF_MAP_CREATE" XLAT_FMT ", ...) = ...\n",
       XLAT_ARGS(BPF_MAP_CREATE));
```

In raw mode: `bpf(0x0, ...)`
In verbose mode: `bpf(0x0 /* BPF_MAP_CREATE */, ...)`
In abbreviated mode: `bpf(BPF_MAP_CREATE, ...)`

## Test Framework Shell Helpers

All helper functions are defined in `tests/init-once.sh`, which is sourced
by `tests/init.sh`.

### Test Execution Helpers

#### run_prog()

```bash
run_prog [program] [args...]
```

Runs a test program. Defaults to `../$NAME` if no arguments given.
Handles exit codes:
- Exit 77 → calls `skip_` (test skipped)
- Exit 0 → success
- Any other → calls `fail_` (test failed)

Does NOT capture output.

#### run_strace()

```bash
run_strace [strace_args...] [program] [program_args...]
```

Runs strace with `-o $LOG` and the given arguments. Writes strace output
to the `$LOG` file. Fails the test if strace exits with non-zero.

Example:
```bash
run_strace -e trace=open -a32 ../open
# Runs: strace -o log -e trace=open -a32 ../open
# Strace output goes to 'log' file
```

#### run_strace_merge()

```bash
run_strace_merge [strace_args...] [program] [program_args...]
```

For multi-threaded tests with `-ff` (per-thread logs). Runs strace with
`-ff -tt`, then merges the per-thread log files using
`strace-log-merge`.

### Combined Run and Compare Helpers

#### run_strace_match_diff()

```bash
run_strace_match_diff [strace_args...] [QUIRK:...] [program_args...]
```

The workhorse of the test framework. Does:

1. Auto-adds `-e trace=$NAME` if no trace expression is present in the
   args
2. Processes `QUIRK:` pseudo-arguments (see below)
3. Runs test program `../$NAME` standalone, captures stdout to `/dev/null`
4. Runs strace with the given args, capturing test program's stdout to
   `$EXP`
5. Filters strace log with sed
6. Applies `filter_vdso_calls()` to remove spurious vDSO calls on 32-bit
   platforms
7. Compares filtered log against `$EXP` using `match_diff()`

Example:
```bash
run_strace_match_diff -a36 -P /tmp/test
# Auto-adds: -e trace=$NAME
# Runs test program to generate expected output in $EXP
# Runs strace, compares log against $EXP
```

#### run_strace_match_grep()

```bash
run_strace_match_grep [strace_args...]
```

Like `run_strace_match_diff()` but uses `match_grep()` for pattern-based
comparison instead of `match_diff()`.

### QUIRK Pseudo-Arguments

`run_strace_match_diff()` supports special `QUIRK:` arguments for advanced
filtering:

**QUIRK:START-OF-TEST-OUTPUT:string** — Filter strace log to start from
the first line containing `string`:

```bash
run_strace_match_diff QUIRK:START-OF-TEST-OUTPUT:"execve(" -e trace=execve
```

**QUIRK:START-OF-TEST-OUTPUT-REGEX:pattern** — Same but uses a regex
pattern:

```bash
run_strace_match_diff QUIRK:START-OF-TEST-OUTPUT-REGEX:"^[0-9]+ execve"
```

**QUIRK:PROG-ARGS:args** — Pass extra arguments to the test program:

```bash
run_strace_match_diff QUIRK:PROG-ARGS:"arg1 arg2" -a32
# Runs: ../test_program arg1 arg2
```

### Output Comparison Helpers

#### match_diff()

```bash
match_diff [actual] [expected] [error_message]
```

Compares two files using `diff -u`. Fails the test if they differ.

Defaults:
- `actual` = `$LOG` (strace output)
- `expected` = `$srcdir/$NAME.expected`
- `error_message` = "$STRACE $args output mismatch"

Does plain `diff -u` with no special handling for timestamps, PIDs, or
addresses.

#### match_grep()

```bash
match_grep [output] [patterns] [error_message]
```

Reads patterns from `patterns` file (one per line), checks each against
`output` using `grep -E -x`. Fails if any pattern doesn't match.

Patterns can be negated with `!` prefix:
```
execve(.*) = 0
!nosuchsyscall
```

Defaults:
- `output` = `$LOG`
- `patterns` = `$srcdir/$NAME.expected`
- `error_message` = "$STRACE $args output mismatch"

#### match_awk()

```bash
match_awk [output] [awk_program] [error_message]
```

Validates `output` against a gawk program. The program receives
`output` via stdin and should exit 0 on success, non-zero on failure.

Defaults:
- `output` = `$LOG`
- `awk_program` = `$srcdir/$NAME.awk`
- `error_message` = "$STRACE $args output mismatch"

### Skip and Fail Helpers

#### skip_()

```bash
skip_ "reason"
```

Skips the test with the given reason. Exits with code 77.

Example:
```bash
[ -f /dev/full ] || skip_ "/dev/full is not available"
```

#### fail_()

```bash
fail_ "error message"
```

Fails the test with the given message. Exits with code 1.

Example:
```bash
[ $rc -eq 0 ] || fail_ "unexpected return code: $rc"
```

#### framework_failure_()

```bash
framework_failure_ "error message"
```

Reports a framework error (not a test failure). Exits with code 99.

### Other Helpers

#### filter_vdso_calls()

```bash
filter_vdso_calls [program] [output] [expected]
```

Removes spurious `clock_gettime64` vDSO calls from strace output on 32-bit
platforms without vDSO support. Called automatically by
`run_strace_match_diff()`.

#### Advanced Test Functions

**test_trace_expr()** — Tests trace expressions by verifying positive
matches from `$NAME.in` and negative matches from a random sample of
`pure_executables.list`.

**test_pidns()** — Tests PID namespace translation (`--decode-pids=pidns`).

**check_scno_tampering()** — Checks if the current architecture and kernel
support syscall number tampering (required for injection tests). Skips the
test if unsupported.

## Test Registration

### Registering Generated Tests (gen_tests.in)

Add a line to `tests/gen_tests.in`:

```
my_syscall -a25
my_syscall-P -e trace=my_syscall -P /dev/full
my_syscall-success -einject=my_syscall:retval=42 -etrace=my_syscall
```

The `gen_tests.sh` script processes this file and generates:
- Individual `.gen.test` shell scripts for each line
- `gen_tests.am` with the `GEN_TESTS` variable listing all generated tests

The generated `.gen.test` files are already covered by the glob pattern
`*.gen.test` in `tests/.gitignore`, so no `.gitignore` updates are needed.

### Registering Hand-Written Tests (Makefile.am)

Add your `.test` file to one of the test lists in `tests/Makefile.am`.
Both lists must be kept in **lexicographical order**.

**For decoder tests**, add to `DECODER_TESTS`:
```makefile
DECODER_TESTS = \
	...
	my_syscall.test \
	...
```

**For non-decoder tests**, add to `MISC_TESTS`:
```makefile
MISC_TESTS = \
	...
	my_feature.test \
	...
```

**Important**: Maintain alphabetical order when adding tests to these lists.

### Test Program Registration (check_PROGRAMS and pure_executables.list)

Every compiled test program executable must be registered in
`tests/Makefile.am`. The registration determines which programs are built
when running `make check`.

**All test programs must appear in one of two places:**

1. **`tests/pure_executables.list`** — Test programs that can be safely
   invoked concurrently by multiple tests without interfering with each
   other or with other test executables

2. **`check_PROGRAMS` variable in `tests/Makefile.am`** — Test programs
   that require exclusive execution or have special build requirements

**The relationship:**
```makefile
check_PROGRAMS = $(PURE_EXECUTABLES) $(secontext_EXECUTABLES) \
	additional test programs
```

where `PURE_EXECUTABLES` is generated from `pure_executables.list`.

**When to use `pure_executables.list`:**

Add your test program here if it can be safely run concurrently by multiple
tests. "Pure" means safe for concurrent execution, not necessarily
side-effect-free.

```
my_syscall
```

Pure executables are allowed to:
- Create temporary files in a manner that doesn't interfere with concurrent
  invocations (e.g., using unique names, proper locking, or process-specific
  directories)
- Perform operations that don't conflict with parallel test execution

Pure executables should NOT:
- Create files with fixed names that would conflict across concurrent
  invocations
- Use shared resources (files, network ports, etc.) in ways that cause
  conflicts
- Spawn processes that interfere with other tests

Programs in this list are used by `test_trace_expr()` for negative
matching: when testing a trace expression like `-e trace=open`,
`test_trace_expr()` verifies that syscalls from unrelated programs
(randomly sampled from `pure_executables.list`) do NOT appear in the
output. This is why they must be safe for concurrent execution.

**When to add directly to `check_PROGRAMS`:**

Add your test program directly to the `check_PROGRAMS` variable in
`tests/Makefile.am` if it:
- Requires exclusive execution (cannot run safely in parallel with other
  tests)
- Requires special build flags or libraries
- Is a test infrastructure helper that shouldn't be used by
  `test_trace_expr()`

Example additions already in `check_PROGRAMS`:
```makefile
check_PROGRAMS = $(PURE_EXECUTABLES) \
	attach-f-p \
	filter-unavailable \
	...
```

### libtests.a (Test Helper Library)

All test programs link against `libtests.a`, which is built from helper
source files in `tests/Makefile.am`:

```makefile
libtests_a_SOURCES = \
	sprintrc.c \
	tprintf.c \
	errno2name.c \
	error_msg.c \
	fill_memory.c \
	signal2name.c \
	tail_alloc.c \
	...
```

All test executables have `LDADD = libtests.a` (defined in
`tests/Makefile.am`), so they automatically link against the helper
library.

You don't need to modify `libtests.a` registration unless you're adding a
new helper source file to the framework itself.

## Running Tests

### Run full test suite

```bash
make -j$(nproc) check
```

### Run specific test

```bash
make -j$(nproc) check TESTS=openat.gen.test
```

### Run multiple tests

```bash
make -j$(nproc) check TESTS='openat.gen.test read.gen.test'
```

### Verbose output

```bash
make -j$(nproc) check VERBOSE=1 TESTS=openat.gen.test
```

Shows test output and strace invocation details.

### Test artifacts

After running a test, `make check` creates artifacts in the `tests/`
directory:

**For a test named `sample.gen.test`:**

- **`sample.gen.log`** — Test framework log showing pass/fail status and
  exit code:
  ```
  PASS sample.gen.test (exit status: 0)
  ```
  or
  ```
  FAIL sample.gen.test (exit status: 1)
  ```

- **`sample.dir/`** — Working directory containing test execution files:
  - `log` — Actual strace output
  - `exp` — Expected output (from test program's stdout)
  - `out` — Filtered strace output (after sed/filter_vdso_calls, this is
    what gets compared against `exp`)

- **`sample.gen.trs`** — Automake test results in internal format

The `sample.dir/` directory is created by the test framework and persists
after the test completes, allowing inspection of test artifacts for
debugging.

## Debugging Test Failures

### Check test log and artifacts

After a test failure:

```bash
cd tests
cat failing_test.gen.log    # Check test status
ls failing_test.dir/         # List test artifacts
cat failing_test.dir/log     # Actual strace output
cat failing_test.dir/exp     # Expected output
cat failing_test.dir/out     # Filtered output (what was compared)
```

### Compare outputs

```bash
diff -u tests/failing_test.dir/exp tests/failing_test.dir/out
```

Shows the difference between expected and actual output.

## Common Pitfalls

1. **Forgetting `+++ exited with 0 +++`**: Tests must end with this line
   because strace prints it when a traced process exits. Since tests
   compare their stdout against strace's log, the test must print it to
   match.

2. **Using `skip()` instead of `error_msg_and_skip()`**: There is no C
   function named `skip()`. Use `error_msg_and_skip("reason")` or
   `SKIP_MAIN_UNDEFINED("__NR_syscall")`.

3. **Confusing `run_strace()` output**: `run_strace -e trace=open open`
   writes strace output to `$LOG` via `-o`, NOT to stdout. The test
   program's stdout goes to `$EXP` in `run_strace_match_diff()`.

4. **Hardcoded PIDs/addresses**: Use `%p` format for addresses and avoid
   printing PIDs unless testing multi-process scenarios.

5. **Syscalls not yet wired up to all architectures**: Guard with `#ifdef
   __NR_syscall_name` only for syscalls not yet present in syscallent
   tables for all architectures. Once a syscall is wired up everywhere,
   `scno.h` guarantees `__NR_xxx` is defined even if kernel headers don't
   provide it. Check `src/linux/*/syscallent*.h` files to determine if a
   syscall is wired up to all architectures.

6. **Timing-dependent tests**: Avoid tests that depend on wall-clock time
   or sleep durations.

7. **Incorrect alignment**: The `-a` flag sets the column at which return
   values are aligned. Choose a value appropriate for the syscall name
   length to ensure output matches character-for-character.

8. **Assuming `--trace` is added by gen_tests.sh**: The `--trace=<name>`
   auto-add happens at runtime in `run_strace_match_diff()`, not during
   test script generation.

## Advanced Topics

### Custom Test Scripts

For complex scenarios, write a `.test` file:

```bash
#!/bin/sh
. "${srcdir=.}/init.sh"

# Run test program
run_prog

# Run strace
run_strace -e trace=mysyscall ../mysyscall

# Custom validation
check_output() {
	grep -q "mysyscall(.*) = 0" "$LOG" ||
		fail_ "mysyscall not found in output"
}
check_output

exit 0
```

### Multiple Execution Phases

Some tests require multiple strace invocations:

```bash
#!/bin/sh
. "${srcdir=.}/init.sh"

# First phase
run_prog
run_strace -e trace=open ../open_test > "$EXP"
match_diff "$LOG" "$EXP"

# Second phase with different flags
run_strace -e trace=open -v ../open_test > exp2
match_diff "$LOG" exp2

exit 0
```

### Conditional Test Execution

Skip tests on platforms without support:

```c
#include "tests.h"

int
main(void)
{
#if !defined(__NR_io_uring_setup)
	error_msg_and_skip("io_uring syscalls not available");
#endif
	/* ... test code ... */
}
```

## See Also

- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) — Includes test creation as
  part of syscall addition workflow
- [HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md) — Testing ioctl decoders
- [DECODER_API.md](DECODER_API.md) — Print helper API reference for
  formatting output
- `tests/init-once.sh` — Primary source of shell helper functions
- `tests/gen_tests.sh` — Test generation script
- `tests/tests.h` — C helper functions and macros
- `tests/Makefile.am` — Test registration and build rules
