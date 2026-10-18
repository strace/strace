# Bundled Linux UAPI Headers

The `bundled/` directory contains Linux kernel User-space API (UAPI) headers
bundled with strace for build portability and forward compatibility.

## Purpose

strace bundles a subset of Linux kernel UAPI headers to ensure:

1. **Build portability** — strace can be built on systems with older kernel
   headers that lack definitions for newer syscalls, ioctls, or constants
2. **Consistency** — builds produce consistent results across different
   distributions and kernel header versions
3. **Forward compatibility** — strace can decode newer kernel features even
   when built on older systems

The build selects either the bundled or the system kernel headers at
configure time. When bundled headers are selected, their `-isystem` paths
precede the compiler's default system include paths; they are not a
per-header fallback for missing definitions.

## Directory Structure

```
bundled/linux/
├── COPYING                 GPL-2.0 with Linux syscall exception
├── GPL-2.0                 Full GPL-2.0 license text
├── Linux-syscall-note      Syscall exception text
├── Makefile.am             Build system integration
├── include/uapi/
│   ├── asm-generic/        Generic architecture headers
│   ├── linux/              Main Linux UAPI headers
│   │   └── version.h       Kernel version tracking
│   ├── linux/io_uring/     io_uring subsystem headers
│   ├── linux/netfilter/    Netfilter headers
│   ├── linux/sched/        Scheduler headers
│   ├── mtd/                MTD device headers
│   └── rdma/               RDMA headers
└── arch/                   Architecture-specific headers
```

At distribution time, `bundled/Makefile.am` creates a symlink:
`include/uapi/asm → asm-generic` so that architectures using generic
definitions can find them.

## Build System Integration

### Configuration

The bundled headers are controlled by the `--enable-bundled` configure
option:

- `--enable-bundled=yes` — always use bundled headers
- `--enable-bundled=no` — never use bundled headers (fail if system headers
  are too old)
- `--enable-bundled=check` — **(default)** auto-detect: compare system
  `linux/version.h` against bundled `version.h`. Use bundled if system is
  older or equal.

The auto-detection logic compares `LINUX_VERSION_CODE >> 8` from the system
against the bundled version. The bundled version is read from
`bundled/linux/include/uapi/linux/version.h` at autoconf time and exposed as
the `linux_version_code` m4 macro.

When bundled headers are enabled, the build system defines the
`USE_BUNDLED_HEADERS` automake conditional.

### Compiler Flags

When bundled headers are enabled, the build adds `-isystem` flags to
`CPPFLAGS`:

```
-isystem <srcdir>/bundled/linux/arch/<karch>/include/uapi
-isystem <srcdir>/bundled/linux/include/uapi
```

The `-isystem` flag adds bundled directories to the system include search
path, ahead of the compiler's default system directories. Source code uses
standard `#include <linux/foo.h>` directives; when bundled headers are
enabled and the requested header is bundled, it is selected regardless of
whether the system also has that header.

The arch-specific path is searched first, so `#include <asm/fcntl.h>`
resolves to the bundled arch-specific header on those 9 architectures. Other
architectures use `asm-generic/` via the `asm` symlink or fall back to
system headers.

### Version Tracking

The file `bundled/linux/include/uapi/linux/version.h` tracks the bundled
kernel version. Example:

```c
#define LINUX_VERSION_CODE 459520
#define LINUX_VERSION_MAJOR 7
#define LINUX_VERSION_PATCHLEVEL 3
#define LINUX_VERSION_SUBLEVEL 0
```

This version is read by `configure.ac` and compared against the system's
version during auto-detection.

## What Is Bundled

The bundled headers currently track Linux kernel v7.3 (see `version.h`):

- **Main Linux UAPI headers**: syscall structures, ioctl definitions,
  constants and enums for BPF, io_uring, perf_event, prctl, seccomp,
  netfilter, mount, landlock, userfaultfd, futex, and other subsystems
- **Subdirectory headers**: io_uring/, netfilter/, netfilter/ipset/, sched/,
  mtd/, rdma/
- **Generic architecture headers**: `asm-generic/fcntl.h`,
  `asm-generic/hugetlb_encode.h`
- **Architecture-specific headers**: `asm/fcntl.h` for alpha, arm, arm64,
  ia64, m68k, mips, parisc, powerpc, sparc

**Not bundled**: Complete kernel headers, internal kernel APIs, non-UAPI
headers, syscall number tables (`asm/unistd.h`), or architecture-specific
headers beyond `fcntl.h`.

## How to Update Bundled Headers

Updates are done from a `headers_install`-ed kernel tree, not raw kernel
sources. The `headers_install` target sanitizes headers for userspace use.

### Updating Existing Headers

Typical workflow when syncing with a new kernel release:

```bash
# Build a headers_install'd kernel tree
cd /path/to/linux
make headers_install INSTALL_HDR_PATH=$TMPDIR/kernel-headers

# Copy updated headers to strace
cd /path/to/strace
cp $TMPDIR/kernel-headers/include/linux/version.h \
   bundled/linux/include/uapi/linux/
...

# Verify the build
./bootstrap
./configure
make -j$(nproc)
make check
```

For architecture-specific headers:

```bash
# Example: updating asm/fcntl.h for mips, after installing mips headers
cp $TMPDIR/mips-kernel-headers/include/asm/fcntl.h \
   bundled/linux/arch/mips/include/uapi/asm/
```

**Important**: Always use `headers_install`-ed headers, not raw kernel
sources. The installation process sanitizes headers for userspace by removing
kernel-internal definitions and ensuring proper include guards.

### Bulk Update Example

For a kernel version update affecting multiple files:

```bash
cd /path/to/linux
git checkout vX.Y
make headers_install INSTALL_HDR_PATH=$TMPDIR/headers-vX.Y

cd /path/to/strace
cd bundled/linux/include/uapi/linux
for f in *.h; do
    cp $TMPDIR/headers-vX.Y/include/linux/$f .
done
cd -

# Test
make -j$(nproc) && make -j$(nproc) check
```

## Adding a New Header

When adding a header that strace previously didn't bundle:

1. **Copy from headers_install**:
   ```bash
   cp $TMPDIR/kernel-headers/include/linux/pidfd.h \
      bundled/linux/include/uapi/linux/
   ```

2. **Register in `bundled/Makefile.am`**:
   ```makefile
   EXTRA_DIST = \
       linux/COPYING \
       ...
       linux/include/uapi/linux/pidfd.h \
       ...
   ```
   Add the new file to the `EXTRA_DIST` list in alphabetical order within
   its section.

3. **Test**:
   ```bash
   ./bootstrap   # Regenerate Makefile.in
   ./configure
   make -j$(nproc)
   ```

The `EXTRA_DIST` list ensures the header is included in distribution
tarballs.

## Commit Message Format

Follow strace's ChangeLog-style commit messages (see
[COMMIT-MESSAGES.md](COMMIT-MESSAGES.md)).

### Updating existing headers

```
bundled: update linux UAPI headers to vX.Y

* bundled/linux/include/uapi/linux/io_uring.h: Update to
headers_install'ed Linux kernel vX.Y.
* bundled/linux/include/uapi/linux/prctl.h: Likewise.
* bundled/linux/include/uapi/linux/version.h: Likewise.
```

For kernel RC versions, reference the specific RC (e.g., `v7.1-rc7`).

When updating for a specific feature or kernel commit:

```
bundled: update linux/io_uring.h from linux vX.Y

Pulls in IORING_OP_WAITID from kernel commit 1234abcd
(io_uring: add IORING_OP_WAITID support).

* bundled/linux/include/uapi/linux/io_uring.h: Update to
headers_install'ed Linux kernel vX.Y.
```

### Adding a new header

```
bundled: add uapi/linux/pidfd.h

* bundled/linux/include/uapi/linux/pidfd.h: New file, imported from
headers_install'ed Linux kernel vX.Y.
* bundled/Makefile.am (EXTRA_DIST): Add linux/include/uapi/linux/pidfd.h.
```

## Relationship to Xlat Files

Many xlat files in `src/xlat/` have `#From` directives that reference Linux
kernel headers:

```
#From include/uapi/linux/bpf.h
BPF_MAP_TYPE_HASH
BPF_MAP_TYPE_ARRAY
...
```

The `#From` directive uses kernel-relative paths (e.g.,
`include/uapi/linux/bpf.h`), not `bundled/` paths. It documents which
upstream kernel header is the source of truth for these constants.

Some xlat files are auto-generated from enum definitions in bundled headers:

```
#Generated by maint/enum2xlat.sh from "enum bpf_map_type" in bundled/linux/include/uapi/linux/bpf.h; do not edit.
```

The `maint/update-xlat.sh` script can regenerate these by parsing the
`#Generated by` line and re-running `maint/enum2xlat.sh` against the
referenced bundled header.

**Workflow**: When updating bundled headers, check if any xlat files need
updating. See [HOWTO_UPDATE_XLAT_AFTER_UAPI.md](HOWTO_UPDATE_XLAT_AFTER_UAPI.md)
for the complete xlat update workflow.

## License

All bundled kernel headers are licensed under **GPL-2.0 WITH
Linux-syscall-note**, which permits their use in userspace programs.

The syscall exception (see `bundled/linux/Linux-syscall-note`) states:

> NOTE! This copyright does *not* cover user programs that use kernel
> services by normal system calls - this is merely considered normal use
> of the kernel, and does *not* fall under the heading of "derived work".

This exception allows strace (LGPL-2.1-or-later) to use these GPL-2.0 headers
without license incompatibility.

The full license text is in `bundled/linux/COPYING` and `bundled/linux/GPL-2.0`.

## See Also

- [HOWTO_UPDATE_XLAT_AFTER_UAPI.md](HOWTO_UPDATE_XLAT_AFTER_UAPI.md) —
  Xlat update workflow after kernel UAPI changes
- [README-xlat.md](README-xlat.md) — Xlat file format, `#From`
  directive, and constant translation
- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) — May require
  bundled header updates for new syscall structures
- [HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md) — Often requires
  bundled header updates for ioctl command definitions
- [COMMIT-MESSAGES.md](COMMIT-MESSAGES.md) — Commit message format
  requirements
