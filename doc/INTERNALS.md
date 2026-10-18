# strace Internals

This document provides an architectural overview of how strace works
internally. It's intended for developers who want to understand the
codebase structure, contribute to strace, or debug complex issues.

For the decoder API reference, see [DECODER_API.md](DECODER_API.md).
For step-by-step tutorials, see [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md)
and [HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md). For term definitions, see
[GLOSSARY.md](GLOSSARY.md).

## High-Level Architecture

strace is built around the Linux ptrace(2) facility, which allows one
process (the tracer) to observe and control another process (the
tracee). The basic operation:

1. **Attach and/or Launch**: strace can attach to existing processes
   using PTRACE_SEIZE/PTRACE_ATTACH and/or launch a new child process
   (using PTRACE_TRACEME on old kernels or PTRACE_SEIZE on modern ones)
2. **Wait**: strace waits for ptrace events using wait4()
3. **Inspect**: When a tracee stops (syscall-enter, syscall-exit,
   signal, etc.), strace reads registers to determine the syscall number
   and arguments
4. **Decode**: strace decodes syscall numbers and arguments into
   human-readable format; decoders read tracee memory as needed to
   dereference pointers and fetch structure contents
5. **Resume**: strace resumes the tracee with PTRACE_SYSCALL or
   PTRACE_CONT to continue until the next stop

The main loop is remarkably clean:

```c
int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	init(argc, argv);
	exit_code = !nprocs;
	while (dispatch_event(next_event()))
		;
	terminate();
}
```

Initialization, event loop, cleanup. The event loop is a
producer-consumer pattern: `next_event()` gathers events from wait4(),
`dispatch_event()` acts on them.

## Startup

### Command-Line Processing

The `init()` function (`src/strace.c`) handles all command-line parsing
and initialization:

- Parses options via `getopt_long`
- Sets up signal handlers
- Configures ptrace options (PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC |
  PTRACE_O_TRACEEXIT by default)
- Either calls `startup_child()` if a command is given, or
  `startup_attach()` if attaching to existing PIDs

### Process Attachment (PTRACE_SEIZE vs PTRACE_ATTACH)

`startup_attach()` attaches to existing processes. Modern kernels
support PTRACE_SEIZE, which provides better control than the legacy
PTRACE_ATTACH. The `test_ptrace_seize()` function probes the kernel at
startup; if PTRACE_SEIZE works, the `use_seize` macro evaluates to true.

**PTRACE_SEIZE advantages:**
- Does not send SIGSTOP to the tracee
- Allows PTRACE_INTERRUPT for clean stop requests
- Enables PTRACE_LISTEN for group-stop handling

When attaching with `-f` (follow forks), strace enumerates existing
threads via `/proc/<pid>/task/` and attaches to each one.

### Child Launch

`startup_child()` forks. The child (tracee) behavior depends on whether
the kernel supports PTRACE_SEIZE:

- **If `use_seize` is false** (old kernels): child calls
  `ptrace(PTRACE_TRACEME, ...)` to request tracing by its parent
- **If `use_seize` is true** (modern kernels): child does not call
  PTRACE_TRACEME; parent uses PTRACE_SEIZE instead

After ptrace setup, the child execs the target program. The parent
establishes the trace relationship via `ptrace_attach_or_seize()`.

The `-D`/`--daemonize` option causes strace to run as a grandchild
process (double-fork) to detach from the controlling terminal.

### Following Forks and Clones

The `-f`/`--follow-forks` option enables multi-process tracing. When
enabled:

- `ptrace_setoptions` includes PTRACE_O_TRACECLONE |
  PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK
- New child processes are automatically detected in `next_event()` and
  added via `maybe_allocate_tcb()`
- With `-ff`, each process gets its own output file named
  `<outfname>.<pid>`

## Core Components

### Trace Control Block (struct tcb)

Defined in `src/defs.h`, `struct tcb` represents a single traced
process or thread. Key fields (in declaration order):

```c
struct tcb {
	int flags;                   /* TCB_* state flags */
	int pid;                     /* Process ID, 0 = free */
	int qual_flg;                /* QUAL_* qualifier flags */
	unsigned int currpers;       /* Current personality */
	unsigned long u_error;       /* Error code */
	kernel_ulong_t scno;         /* Syscall number (shuffled) */
	kernel_ulong_t true_scno;    /* Raw syscall number */
	kernel_ulong_t u_arg[MAX_ARGS]; /* Syscall arguments */
	kernel_long_t u_rval;        /* Syscall return value */
	int curcol;                  /* Output column position */
	FILE *outf;                  /* Output file */
	const char *auxstr;          /* Auxiliary info (RVAL_STR) */
	const struct_sysent *s_ent;  /* Pointer to sysent entry */
	/* ... timing, injection, mmap cache, etc. */
};
```

The `entering()` and `exiting()` macros test the `TCB_INSYSCALL` flag
to determine whether strace is at syscall-enter-stop or
syscall-exit-stop.

### TCB Flags

Important flags in `tcp->flags`:

- **TCB_STARTUP** -- Attached but not yet seen stopping (initial state)
- **TCB_INSYSCALL** -- Between syscall-enter and syscall-exit (set in
  `syscall_entering_finish`, cleared in `syscall_exiting_finish`)
- **TCB_ATTACHED** -- Already attached via PTRACE_TRACEME/ATTACH/SEIZE
- **TCB_FILTERED** -- Syscall filtered out (don't print)
- **TCB_TAMPERED** -- Syscall return value was tampered with (--inject)
- **TCB_RECOVERING** -- Recovering from spurious syscall-enter-stop
- **TCB_HIDE_LOG** -- Hide all output until execve (used for strace's
  own child before exec)
- **TCB_SECCOMP_FILTER** -- Process has seccomp filter attached

Qualifier flags in `tcp->qual_flg`:

- **QUAL_TRACE** -- Syscall should be traced
- **QUAL_ABBREV** -- Use abbreviated output
- **QUAL_VERBOSE** -- Use verbose output
- **QUAL_RAW** -- Use raw numeric values
- **QUAL_INJECT** -- Injection rules apply

### Event Types (trace_event enum)

Defined in `src/trace_event.h`, the `trace_event` enum classifies
wait4() status into actionable events:

- **TE_BREAK** -- Break the main loop (end tracing)
- **TE_NEXT** -- Call next_event() again (nothing to do)
- **TE_RESTART** -- Restart tracee with signal 0
- **TE_SYSCALL_STOP** -- Syscall entry or exit stop
- **TE_SIGNAL_DELIVERY_STOP** -- Signal about to be delivered
- **TE_SIGNALLED** -- Tracee killed by signal
- **TE_GROUP_STOP** -- Tracee in group-stop (SIGSTOP, SIGTSTP, etc.)
- **TE_EXITED** -- Tracee exited normally
- **TE_STOP_BEFORE_EXECVE** -- PTRACE_EVENT_EXEC (about to execve)
- **TE_STOP_BEFORE_EXIT** -- PTRACE_EVENT_EXIT (about to exit)
- **TE_SECCOMP** -- SECCOMP_RET_TRACE triggered

## Event Loop

### next_event()

`static const struct tcb_wait_data * next_event(void)` (`src/strace.c`)
is the event-gathering function. It:

1. Checks for interruption and syscall count limit
2. Drains a queue of `pending_tcps` before calling wait4
3. Calls `wait4(-1, &status, __WALL, ...)` to block until a tracee
   stops
4. Loops calling `wait4(..., WNOHANG)` to gather multiple pending
   events
5. Classifies each wait status into a `trace_event` enum value based on
   WIFSIGNALED, WIFEXITED, WIFSTOPPED, and ptrace event codes
6. Queues all stopped processes in the `pending_tcps` list
7. Dequeues one tcb and returns its wait data
8. Calls `startup_tcb()` for first-time-seen tracees
9. Calls `set_current_tcp(tcp)` to set the output file

### dispatch_event()

`static bool dispatch_event(const struct tcb_wait_data *wd)`
(`src/strace.c`) returns true to continue the main loop, false to stop.
It switches on the `trace_event`:

- **TE_BREAK** -- return false (stop tracing)
- **TE_SYSCALL_STOP** -- call `trace_syscall()`
- **TE_SIGNAL_DELIVERY_STOP** -- print signal delivery
- **TE_SIGNALLED** -- print termination by signal, drop tcb
- **TE_GROUP_STOP** -- print stop, use PTRACE_LISTEN if using SEIZE
- **TE_EXITED** -- print exit status, drop tcb
- **TE_STOP_BEFORE_EXECVE** -- handle exec, apply `-b execve` logic
- **TE_STOP_BEFORE_EXIT** -- print event_exit
- **TE_SECCOMP** -- handle seccomp stop

After handling the event, it calls `ptrace_restart()` with
PTRACE_SYSCALL or PTRACE_CONT to resume the tracee.

## Syscall Handling

### Syscall Entry Tables

Each architecture has a syscall table (`syscallent.h`) mapping syscall
numbers to decoders:

```c
/* src/linux/x86_64/syscallent.h */
[0] = { 3, TD,      SEN(read),   "read"  },
[1] = { 3, TD,      SEN(write),  "write" },
[2] = { 3, TD|TF,   SEN(open),   "open"  },
/* ... */
```

Structure: `{ nargs, flags, SEN(decoder), "name" }`

Most architectures share `src/linux/generic/syscallent-common.h` for
common syscalls and override specific entries.

The `SEN()` macro expands to two fields: an enum value (`SEN_read`) and
a function pointer (`SYS_FUNC_NAME(sys_read)`).

### Decoder Lookup

The `get_scno()` function (`src/syscall.c`) decodes the syscall number
from registers, then calls architecture-specific `shuffle_scno()` to
remap the number (eliminates gaps in the syscall table on some
architectures). The sysent table is indexed to get the decoder function.

Decoders are called via `tcp_sysent(tcp)->sys_func(tcp)`.

For decoder implementation details, see
[DECODER_API.md](DECODER_API.md) and
[HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md).

### Syscall Phases

Syscall handling is split into six phases. The `trace_syscall()`
function branches on `entering(tcp)`:

**Entry (syscall-enter-stop):**

1. `syscall_entering_decode()` -- Calls `get_scno()` to decode syscall
   number, `get_syscall_args()` to populate `tcp->u_arg[]`, handles
   subcall decoding
2. `syscall_entering_trace()` -- Applies filtering and path tracing,
   handles injection, calls `printleader()`, invokes the syscall
   decoder
3. `syscall_entering_finish()` -- Sets `TCB_INSYSCALL` flag, records
   entry timestamp

**Tracee executes the syscall**

**Exit (syscall-exit-stop):**

4. `syscall_exiting_decode()` -- Records exit timestamp, calls
   `get_syscall_result()` to retrieve return value and error
5. `syscall_exiting_trace()` -- Handles tamper/inject on exit, prints
   return value and error name
6. `syscall_exiting_finish()` -- Clears `TCB_INSYSCALL` flag

The `TCB_INSYSCALL` flag is the sole state that tracks whether strace
is at entry or exit.

## Filtering

### Qualifier System (-e trace=, -e signal=, etc.)

The filtering system (`src/filter_qualify.c`) processes `-e` options
via the `qualify()` function, which dispatches to specific handlers:

- `-e trace=` -- controls which syscalls are traced (stored in
  `trace_set`)
- `-e abbrev=` -- controls abbreviated output
- `-e verbose=` -- controls verbose output
- `-e raw=` -- controls raw numeric output
- `-e inject=` -- fault injection rules
- `-e signal=` -- signal filtering
- `-e status=` -- status-based output filtering
- `-e read=` / `-e write=` -- data dumping
- `-e decode-fd=` -- fd path/socket decoding
- `-e decode-pid=` -- pid comm name decoding

Each qualifier uses `number_set` arrays (per-personality bitsets). The
`qual_flags()` function converts set membership into QUAL_* bitmask
flags stored in `tcp->qual_flg`.

### Seccomp-Assisted Tracing

The `--seccomp-bpf` option installs a BPF filter in the tracee that
allows untraced syscalls to execute without ptrace stops. The filter
uses:

- `SECCOMP_RET_TRACE` for syscalls that should be traced
- `SECCOMP_RET_ALLOW` for syscalls that should pass through

Implementation (`src/filter_seccomp.c`):

1. Generates BPF program checking syscall number against trace_set
2. Installs via `prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, prog)` in
   the tracee child before exec
3. Traced syscalls still trap via PTRACE_O_TRACESECCOMP

Benefits: Can significantly reduce overhead for filtered workloads.

Limitations: Requires `--follow-forks` (filter must be installed in all
children).

## Output

### Output Formatting

The output system (`src/strace.c`) handles formatting and printing:

- `printleader(tcp)` -- Prints PID prefix (if multi-process), timestamp
  (if `-t`/`-tt`/`-ttt`), handles line resumption
- `tprintf_string()` / `tprints_string()` -- Output to `tcp->outf`
- `line_ended()` -- Resets column counter, clears `printing_tcp`

With `-o <file>`, all output goes to the specified file. With `-ff`,
each process gets its own file named `<outfname>.<pid>`.

### Multi-Process Interleaving

When multiple processes are being traced without `-ff`, their output
interleaves. The `printing_tcp` global tracks which tcb last printed.

When `printleader()` detects a different process wants to print and the
previous line is incomplete (`curcol != 0`), it outputs
`" <unfinished ...>\n"`.

On syscall exit, if the printing process differs from the current tcb,
`print_syscall_resume()` outputs `"<... SYSCALL resumed>"` to indicate
which syscall is being completed.

### Staged Output (-z, -Z)

When status filtering is active (`-z`, `-Z`, `-e status=`), output is
redirected to a memstream buffer (`src/stage_output.c`). On syscall
completion, the buffered output is either published (if the syscall
matches the status filter) or discarded.

## Fault Injection

The `--inject` option (`src/inject.c`) allows tampering with syscalls.
Injection types:

- **INJECT_F_SIGNAL** -- Inject signal instead of syscall
- **INJECT_F_ERROR** -- Inject error return code
- **INJECT_F_RETVAL** -- Inject specific return value
- **INJECT_F_DELAY_ENTER** -- Delay before syscall
- **INJECT_F_DELAY_EXIT** -- Delay after syscall
- **INJECT_F_SYSCALL** -- Replace with different syscall
- **INJECT_F_POKE_ENTER** -- Modify arguments before syscall
- **INJECT_F_POKE_EXIT** -- Modify memory after syscall

Implementation: When a syscall matches injection rules, strace modifies
registers before/after the syscall or uses PTRACE_SETREGS to skip
execution entirely and inject a fake return value.

## Subsystems

### Xlat Translation System

The xlat system converts numeric constants to symbolic names. Input
files (`src/xlat/*.in`) are processed by `gen.sh` to generate C headers
with lookup tables.

Printing functions like `printxval()` (exclusive values) and
`printflags()` (OR-able flags) perform table lookups with support for
multiple output styles (`-Xabbrev`, `-Xverbose`, `-Xraw`).

For comprehensive xlat documentation, see
[README-xlat.md](README-xlat.md). For xlat printing API, see
[DECODER_API.md](DECODER_API.md).

### Multi-Personality (MPERS)

On architectures like x86_64, a single kernel can run binaries of
different ABIs (64-bit, 32-bit i386, 32-bit x32). Struct layouts differ
between these ABIs.

**The Problem:** When decoding a `stat()` syscall from a 32-bit tracee
on 64-bit strace, we must use the 32-bit struct layout, not the native
64-bit layout.

**The Solution:** MPERS compiles certain files multiple times with
different `-m32`/`-mx32` flags. The objects are linked into
`libmpers-m32.a` and `libmpers-mx32.a`. At runtime, strace selects the
correct decoder based on the tracee's personality.

For implementation details, see [README-mpers.md](README-mpers.md).

### Netlink Decoding

Netlink is a Linux IPC mechanism using special sockets. The main
dispatch (`src/netlink.c`) uses a `netlink_decoders[]` array indexed by
netlink family (NETLINK_ROUTE, NETLINK_CRYPTO, NETLINK_GENERIC, etc.).

When strace intercepts sendto() or recvfrom() on a netlink socket:

1. Determines netlink family from the socket
2. Fetches the `struct nlmsghdr` message header
3. Dispatches to family-specific decoder
4. Decodes nested netlink attributes (TLV structures) using helpers
   from `src/nlattr.h`

For adding netlink decoders, see
[HOWTO_ADD_NETLINK.md](HOWTO_ADD_NETLINK.md).

### FD Path Resolution (-y)

The `-y` flag enables file descriptor path decoding. The
`printfd_pid_with_finfo()` function (`src/util.c`) resolves fd paths by
reading `/proc/<pid>/fd/<fd>` symlinks via `getfdpath_pid()`
(`src/pathtrace.c`).

With `--decode-fd=` options, strace can additionally decode socket
details (local/remote addresses), device information, eventfd/pidfd/
signalfd details.

The `-P` option enables path tracing: only syscalls touching specified
paths are traced. The `pathtrace_match_set()` function checks if a
syscall's fd or path arguments match the filter.

### Syscall Definition Language

For some syscalls, strace uses a declarative definition language
(`maint/gen/*.def` files) processed by a C-based generator built from
lex/yacc sources (`maint/gen/lex.l`, `maint/gen/parse.y`,
`maint/gen/codegen.c`).

For details, see [maint/gen/README.md](../maint/gen/README.md).

## Key Files

### Core

- **src/defs.h** -- Core definitions, struct tcb, macros, function
  prototypes
- **src/strace.c** -- Main event loop, startup, option parsing,
  ptrace control
- **src/syscall.c** -- Syscall dispatch, personality handling, syscall
  number decoding
- **src/util.c** -- Memory access helpers (umove, printstr, etc.),
  printfd implementation

### Decoders

Major decoder files include:

- **src/io.c** -- read, write, pread, pwrite, readv, writev
- **src/desc.c** -- dup, fcntl, ioctl dispatching
- **src/net.c** -- socket, bind, connect, sendto, recvfrom
- **src/signal.c** -- sigaction, sigprocmask, kill, rt_sigqueueinfo
- **src/mem.c** -- mmap, mprotect, brk, munmap

Each major subsystem (bpf, io_uring, mount, etc.) has dedicated decoder
files.

### Support

- **src/print_fields.h** -- PRINT_FIELD macros and printer declarations
- **src/print_fields.c** -- Printer function implementations
- **src/xlat.h** -- Xlat system structures
- **src/sysent.h** -- Syscall entry structures
- **src/trace_event.h** -- Event type enum
- **src/filter_qualify.c** -- Filtering system (-e options)
- **src/filter_seccomp.c** -- Seccomp-BPF filter generation
- **src/pathtrace.c** -- FD path resolution, path tracing
- **src/stage_output.c** -- Output buffering for status filtering

- **[DECODER_API.md](DECODER_API.md)** -- Print helper API reference
  for syscall decoder writers (printaddr, PRINT_FIELD_* macros, xlat
  functions, memory access, output formatting)
- **[HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md)** -- Step-by-step
  guide for adding syscall decoders
- **[HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md)** -- Ioctl decoder guide
- **[HOWTO_ADD_NETLINK.md](HOWTO_ADD_NETLINK.md)** -- Netlink decoder
  guide
- **[README-mpers.md](README-mpers.md)** -- Multi-personality (MPERS)
  system documentation
- **[README-xlat.md](README-xlat.md)** -- Xlat file format and
  generation
- **[README-linux-ptrace](README-linux-ptrace)** -- Linux ptrace API
  reference and behavior documentation
- **[GLOSSARY.md](GLOSSARY.md)** -- Term definitions and trace flags
  table
- **[maint/gen/README.md](../maint/gen/README.md)** -- Syscall
  definition language
- **INSTALL-git.md** -- Build from git (bootstrap, configure)
