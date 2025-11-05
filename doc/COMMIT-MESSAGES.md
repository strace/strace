# Commit Message Guide

This guide describes the conventions for writing commit messages in the strace
project.  These conventions ensure that commit messages are properly formatted
for automated ChangeLog generation and provide clear, useful information for
future maintainers.

## Table of Contents

1. [Why Commit Messages Matter](#why-commit-messages-matter)
2. [Basic Structure](#basic-structure)
3. [Title (Summary Line)](#title-summary-line)
4. [Body Format](#body-format)
5. [ChangeLog-Style Entries](#changelog-style-entries)
6. [Common Patterns](#common-patterns)
7. [Examples](#examples)
8. [Commit Trailers](#commit-trailers)
9. [Best Practices](#best-practices)

## Why Commit Messages Matter

The commit message that explains changes is just as important as the
changes themselves.  Code may be clearly written with in-code comments
that explain how it works with the surrounding code, but those who need
to fix or enhance code in the future need to know why the code does what
it does.

### Future Maintainability

When adding a new decoder for a syscall or ioctl, updating xlat
constants, or fixing a bug, the implementation may be clear from reading
the code.  The commit message must explain why the change was necessary,
what it accomplishes, and how it relates to other parts of the codebase.

Without this context, future maintainers cannot determine whether the code
is working as intended, why certain design decisions were made, what edge
cases or limitations exist, or how the change fits into the broader
strace architecture.

### Code Review and Quality Assurance

During code review, reviewers assess whether the change solves the right
problem, if the implementation approach is sound, whether the change
conflicts with existing functionality, and if all affected areas have
been updated (tests, NEWS, documentation).

A well-written commit message helps reviewers understand intent without
reverse-engineering it from the code alone, making reviews more efficient
and thorough.

### Debugging and Regression Analysis

When investigating bugs or regressions, commit messages are often the
first place developers look.  A commit message that clearly describes
what the change does, why it was necessary, and what Linux kernel version
or commit it relates to can help quickly determine if a commit is related
to a bug, saving hours of debugging time.

### Automated ChangeLog Generation

The strace project automatically generates the ChangeLog file from commit
messages during the release process.  Commit messages become the official
record of what changed and why.  Users and downstream maintainers rely
on this ChangeLog to understand what new features were added, what bugs
were fixed, what constants were updated, and what syscalls or ioctls are
now decoded.

A poorly written commit message results in a confusing ChangeLog entry
that does not help anyone understand the change.

### Historical Context

Months or years later, developers may need to understand why a particular
approach was chosen, determine if a change can be safely reverted, find
related changes that might have introduced a bug, or learn from past
decisions when implementing similar features.

A well-documented commit message preserves this context long after the
details have faded from memory.

### Example: The Difference It Makes

**Poor commit message:**
```
fix ioctl
```

This provides no information about what was broken, why it was broken, or
how it was fixed.

**Better commit message:**
```
ioctl: fix decoding of SO_DEVMEM_DONTNEED socket option

Unlike SCM_DEVMEM_LINEAR and SCM_DEVMEM_DMABUF, SO_DEVMEM_DONTNEED
is a setsockopt socket option and not a control message type.

* src/xlat/scmvals.in (SO_DEVMEM_DONTNEED): Move ...
* src/xlat/setsock_options.in: ... here.
```

This immediately indicates what was wrong (misplaced constant), why it
was wrong (wrong category), how it was fixed (moved to correct location),
and what files were affected.

### The Goal

The goal of a commit message is to convey the why and what behind a change
to help future developers, reviewers, and users.  Code tells what it does;
a good commit message tells why it does it and what problem it solves.
Both are essential.

## Basic Structure

Each commit message MUST follow this structure:

```
<one-line summary>

<blank line>

<descriptive text explaining the change>
<blank line>

<ChangeLog-style entries>
```

### Requirements

1. **First line**: A concise summary
2. **Second line**: MUST be blank
3. **Descriptive text**: Explains the problem the change tries to solve
   and justifies the way the change solves it (unless the ChangeLog entries
   already provide this information)
4. **Blank line**: Separates descriptive text from ChangeLog entries
5. **ChangeLog-style entries**: Describe all affected files

### Line Length

The 72 character limit applies to both the commit summary and the commit
message body (descriptive text and ChangeLog entries).  This is not a hard
limit; exceptions are acceptable when necessary, for example:

- Code excerpts or snippets that should remain readable
- Command outputs or log excerpts
- File paths or URLs that cannot be reasonably shortened
- Kernel commit references that exceed the limit

When exceeding the limit, prefer readability over strict adherence, but
aim to stay within 72 characters when possible.

### Descriptive Text

The descriptive text between the summary and ChangeLog entries should:

- Explain the problem the change tries to solve (what is wrong with the
  current code without the change)
- Justify the way the change solves the problem (why the result with the
  change is better)
- Mention any alternate solutions considered but discarded, if relevant

For simple changes where the ChangeLog entries already make the purpose
clear (e.g., "xlat: update BR_* constants" with entries listing the new
constants), the descriptive text may be omitted.  However, whenever there
is a design decision, non-obvious rationale, or important context that
cannot be inferred from the ChangeLog entries alone, descriptive text
should be included.

**Example with descriptive text:**

```
xlat: drop numeric constants support

Numeric constant support was introduced by commit v4.9~69 to handle
several corner cases that are no longer relevant.  The last numeric
constant was removed by commit v6.0~27, and there are no plans to use
them again.

* src/xlat/gen.sh: Remove special handling of [0-9]* lines.
```

**Example without descriptive text (simple case):**

```
xlat: update BR_* constants

* src/xlat/rtnl_ifla_br_boolopts.in (BR_BOOLOPT_FDB_LOCAL_VLAN_0):
New constant introduced by Linux kernel commit v6.18-rc1~132^2~195^2~4.
* src/xlat/rtnl_ifla_br_boolopt_flags.in
(1<<BR_BOOLOPT_FDB_LOCAL_VLAN_0): Likewise.
* tests/nlattr_ifla_linkinfo.c (main): Update expected output.
* NEWS: Mention this change.
```

In the second example, the ChangeLog entries already explain what changed
and reference the source (Linux kernel commit), so additional descriptive
text is not needed.

### Whitespace Rules

- Changes MUST NOT introduce whitespace errors (trailing whitespace, spaces
  before tabs, spaces for indentation, blank lines at the end of file)
- Check with:
  `git -c core.whitespace=space-before-tab,trailing-space diff-index \
		--check 4b825dc642cb6eb9a060e54bf8d69288fbee4904`

## Title (Summary Line)

The title should be clear and descriptive.  Common patterns include:

### Format Patterns

- **Category prefix**: `category: brief description`
  - Examples: `xlat: update BR_* constants`, `ioctl: optimize decoding`,
    `tests: check decoding`
- **Component prefix**: `component: action`
  - Examples: `maint: add find-new-xlat-constants.sh script`,
    `bundled: update linux UAPI headers`
- **Direct action**: `action description`
  - Examples: `Implement FS_IOC_GETLBMD_CAP ioctl decoder`,
    `Post-release administrivia`

### Common Categories

- `build:` - Build system changes
- `bundled:` - Updates to bundled headers
- `doc:` - Documentation changes
- `ioctl:` - Changes to ioctl decoders
- `maint:` - Maintenance scripts
- `net:` - Networking-related changes
- `src:` - Generic source code changes
- `tests:` - Test changes
- `xlat:` - Updates to xlat files

### Title Guidelines

- Use imperative mood ("Add feature" not "Added feature")
- Be specific about what changed
- Include constant prefixes or syscall names when relevant
- Keep it concise but descriptive

## Body Format

The body consists of descriptive text (if needed) followed by ChangeLog-style
entries that describe changes to each affected file.  The ChangeLog entries
are used to generate the ChangeLog file automatically, so they must follow a
strict format.

### ChangeLog-Style Entry Format

Each entry follows this pattern:

```
* path/to/file.ext: Description of change.
```

### Detailed Entry Formats

#### File-Level Changes

```
* path/to/file.ext: Description of change.
```

#### Function-Level Changes

```
* path/to/file.ext (function_name): Description of change.
```

#### Constant-Level Changes

```
* path/to/file.ext (CONSTANT_NAME): Description of change.
```

#### Multiple Changes in Same File

```
* path/to/file.ext (function1): First change.
(function2): Second change.
(CONSTANT_NAME): Third change.
```

#### Multiple Files with Same Change

```
* path/to/file1.ext: Description.
* path/to/file2.ext: Likewise.
* path/to/file3.ext: Similarly.
```

### Entry Descriptions

Entry descriptions should:

- Use imperative mood
- Be clear and specific
- Explain *what* changed and often *why*
- Reference kernel commits when adding new constants
- Reference strace commits when fixing issues

## ChangeLog-Style Entries

### File Paths

- Use relative paths from the repository root
- Include file extensions
- Group related files together

### Descriptions

- Start with a capital letter
- End with a period
- Use present tense imperative ("Add", "Update", "Fix", not "Added",
  "Updated", "Fixed")
- Be specific about what changed

### Common Verbs

- `Add` - New functionality
- `Update` - Modify existing functionality
- `Fix` - Correct bugs or errors
- `Remove` - Delete code or files
- `Introduce` - New feature or system
- `Implement` - New decoder or functionality
- `Enhance` - Improve existing functionality
- `Regenerate` - Rebuild generated files
- `Handle` - Support for new constants/options
- `Check` - Test coverage

## Common Patterns

### Xlat Constant Updates

When updating xlat files with new constants:

```
xlat: update CONSTANT_PREFIX constants

* src/xlat/filename.in (NEW_CONSTANT): New constant introduced
by Linux kernel commit v6.18-rc1~132^2~195^2~4.
(ANOTHER_CONSTANT): New constant introduced by Linux kernel
commit v6.18-rc1~132^2~195^2~3.
* tests/test_file.c (main): Update expected output.
* NEWS: Mention this change.
```

References to Linux kernel commits are formatted using
`git describe --match=v\* --contains`.

### Implementing New Decoders

When implementing a new syscall or ioctl decoder:

```
Implement SYSCALL_NAME syscall decoder

* src/xlat/related_constants.in: New file.
* src/syscall_file.c: New file.
* src/Makefile.am (libstrace_a_SOURCES): Add syscall_file.c.
* src/defs.h: Add declaration.
* src/linux/generic/syscallent-common.h [BASE_NR + N]: Wire up syscall.
* tests/syscall_name.c: New file.
* tests/syscall_name-P.c: Likewise.
* tests/pure_executables.list: Add syscall_name and syscall_name-P.
* tests/.gitignore: Add syscall_name and syscall_name-P.
* tests/gen_tests.in (syscall_name, syscall_name-P): New tests.
* NEWS: Mention this change.
```

### Kernel Commit References

When referencing Linux kernel commits:

```
* src/xlat/file.in (CONSTANT): New constant introduced
by Linux kernel commit v6.18-rc1~132^2~195^2~4.
```

### Strace Commit References

When referencing previous strace commits:

```
Fixes: v5.2~45 "Implement decoding of open_tree syscall"
```

Or:

```
This reverts commit 4a80f35bda083abdd4f8be15681e24cc7d7c40c4.
```

Commit references are usually formatted using
`git describe --match=v\* --contains`.

### Regenerating Files

When regenerating xlat files:

```
* src/xlat/file.in: Regenerate using maint/update-xlat.sh.
```

Or:

```
* src/xlat/file1.in: Regenerate using maint/update-xlat.sh.
* src/xlat/file2.in: Likewise.
```

### Bundle Header Updates

When updating bundled Linux headers:

```
bundled: update linux UAPI headers to v6.18-rc1

* bundled/linux/include/uapi/linux/header1.h: Update to
headers_install'ed Linux kernel v6.18-rc1.
* bundled/linux/include/uapi/linux/header2.h: Likewise.
* bundled/Makefile.am (EXTRA_DIST): Add new files.
```

### Test Updates

When updating tests:

```
* tests/test_file.c (main): Update expected output.
```

Or:

```
* tests/test_file.c (main): Check new functionality.
```

### NEWS Updates

Most changes should mention NEWS:

```
* NEWS: Mention this change.
```

### Co-authored Commits

When working with AI assistants:

```
Co-authored-by: name@example.com
```

## Examples

### Example 1: Simple Xlat Update

```
xlat: update BR_* constants

* src/xlat/rtnl_ifla_br_boolopts.in (BR_BOOLOPT_FDB_LOCAL_VLAN_0):
New constant introduced by Linux kernel commit v6.18-rc1~132^2~195^2~4.
* src/xlat/rtnl_ifla_br_boolopt_flags.in
(1<<BR_BOOLOPT_FDB_LOCAL_VLAN_0): Likewise.
* tests/nlattr_ifla_linkinfo.c (main): Update expected output.
* NEWS: Mention this change.
```

### Example 2: New Decoder Implementation

```
Implement FS_IOC_GETLBMD_CAP ioctl decoder

* src/xlat/lbmd_pi_cap_flags.in: New file.
* src/xlat/lbmd_pi_csum_types.in: Likewise.
* src/fs_0x15_ioctl.c: Include "xlat/lbmd_pi_cap_flags.h" and
"xlat/lbmd_pi_csum_types.h".
(decode_logical_block_metadata_cap): New function.
(fs_0x15_ioctl): Use it to handle FS_IOC_GETLBMD_CAP.
* tests/ioctl_fs_0x15.c (main): Check FS_IOC_GETLBMD_CAP decoding.
* NEWS: Mention this change.
```

### Example 3: Bug Fix with Descriptive Text

```
xlat: fix decoding of SO_DEVMEM_DONTNEED socket option

Unlike SCM_DEVMEM_LINEAR and SCM_DEVMEM_DMABUF, SO_DEVMEM_DONTNEED
is a setsockopt socket option and not a control message type.

* src/xlat/scmvals.in (SO_DEVMEM_DONTNEED): Move ...
* src/xlat/setsock_options.in: ... here.
```

This example includes descriptive text explaining why the constant was
misplaced and what the correct category should be.

### Example 4: Multiple File Changes

```
xlat: convert comments about source of xlat constants into directives

* src/xlat/alg_sockaddr_flags.in: Convert the comment about the source
of the xlat constants into #From directive.
* src/xlat/ax25_protocols.in: Likewise.
* src/xlat/elf_em.in: Likewise.
* src/xlat/kd_ioctl_cmds.in: Likewise.
* src/xlat/key_perms.in: Likewise.
* src/xlat/pr_dumpable.in: Likewise.
* src/xlat/syslog_console_levels.in: Likewise.
* src/xlat/x86_xfeature_bits.in: Likewise.
* src/xlat/x86_xfeatures.in: Likewise.
```

### Example 5: Revert

```
Revert "xlat: update NFT_MSG_* constants"

The Linux kernel commit v6.16-rc7~37^2~13^2~1 reverted the change
introduced earlier by Linux kernel commit v6.16-rc1~132^2~30^2~1.

This reverts commit 4a80f35bda083abdd4f8be15681e24cc7d7c40c4.

* src/xlat/nf_nftables_msg_types.in (NFT_MSG_NEWDEV, NFT_MSG_DELDEV):
Remove.
* src/netlink.c (decode_nlmsg_flags_netfilter): Remove them.
```

### Example 6: Maintenance Script

```
maint: add list-xlat-linux-headers.sh script

Extract linux kernel header paths from xlat files that contain
the following annotations:
- #From
- #Generated by maint/enum2xlat.sh
- #Generated by maint/gen_xlat_defs.sh

* maint/list-xlat-linux-headers.sh: New file.
```

### Example 7: Framework Implementation

```
Introduce framework for decoding of linux/fs.h 0x15 ioctl commands

* src/fs_0x15_ioctl.c: New file.
* src/Makefile.am (libstrace_a_SOURCES): Add fs_0x15_ioctl.c.
* src/defs.h (DECL_IOCTL): Add fs_0x15.
* src/ioctl.c (ioctl_decode): Handle ioctl type 0x15.
* tests/ioctl_fs_0x15.c: New file.
* tests/ioctl_fs_0x15-Xabbrev.c: Likewise.
* tests/ioctl_fs_0x15-Xraw.c: Likewise.
* tests/ioctl_fs_0x15-Xverbose.c: Likewise.
* tests/ioctl_fs_0x15-success.c: Likewise.
* tests/ioctl_fs_0x15-success-Xabbrev.c: Likewise.
* tests/ioctl_fs_0x15-success-Xraw.c: Likewise.
* tests/ioctl_fs_0x15-success-Xverbose.c: Likewise.
* tests/pure_executables.list: Add ioctl_fs_0x15, ioctl_fs_0x15-Xabbrev,
ioctl_fs_0x15-Xraw, and ioctl_fs_0x15-Xverbose.
* tests/Makefile.am (check_PROGRAMS): Add ioctl_fs_0x15-success,
ioctl_fs_0x15-success-Xabbrev, ioctl_fs_x-success-Xraw,
and ioctl_fs_0x15-success-Xverbose.
* tests/.gitignore: Add ioctl_fs_0x15, ioctl_fs_0x15-Xabbrev,
ioctl_fs_0x15-Xraw, ioctl_fs_0x15-Xverbose, ioctl_fs_0x15-success,
ioctl_fs_0x15-success-Xabbrev, ioctl_fs_0x15-success-Xraw, and
ioctl_fs_0x15-success-Xverbose.
* tests/gen_tests.in (ioctl_fs_0x15, ioctl_fs_0x15-Xabbrev,
ioctl_fs_0x15-Xraw, ioctl_fs_0x15-Xverbose, ioctl_fs_0x15-success,
ioctl_fs_0x15-success-Xabbrev, ioctl_fs_0x15-success-Xraw,
ioctl_fs_0x15-success-Xverbose): New tests.
```

This example shows a commit that introduces a framework and spans multiple
related changes across source files, build system, and test infrastructure.

## Commit Trailers

Commit trailers are metadata lines placed at the end of commit messages.
They provide attribution, references, and relationship information.  All
trailers should be placed after the ChangeLog-style entries, separated by
a blank line.

### Attribution Trailers (-by:)

These trailers provide attribution for contributions:

#### Co-authored-by

Used when multiple authors contributed to a commit.

```
Co-authored-by: Name <name@example.com>
```

**Examples:**
- AI-assisted commits: `Co-authored-by: auto@cursor.ai`
- Human collaborations: `Co-authored-by: Dmitry V. Levin <ldv@strace.io>`

#### Signed-off-by

Used to indicate that the contributor certifies their contribution
according to the Developer Certificate of Origin (DCO).  This is
commonly used for patches from external contributors.

```
Signed-off-by: Name <name@example.com>
```

**Example:**
```
Signed-off-by: Masatake YAMATO <yamato@redhat.com>
```

#### Suggested-by

Used when a change was suggested by someone (but not necessarily
implemented by them).

```
Suggested-by: Name <name@example.com>
```

**Example:**
```
Suggested-by: Dmitry V. Levin <ldv@strace.io>
```

#### Reported-by

Used to credit the person who reported a bug or issue that this commit
addresses.

```
Reported-by: Name <name@example.com>
```

**Example:**
```
Reported-by: Dmitry V. Levin <ldv@strace.io>
```

#### Reviewed-by

Used to indicate that someone reviewed the patch.

```
Reviewed-by: Name <name@example.com>
```

**Example:**
```
Reviewed-by: Eugene Syromyatnikov <evgsyr@gmail.com>
```

#### Originally-by

Used to credit the original author when a commit is derived from or
inspired by earlier work.

```
Originally-by: Name <name@example.com>
```

**Example:**
```
Originally-by: Michael Vogt <mvo@ubuntu.com>
```

### Reference Trailers

These trailers link commits to issues, bugs, or other commits:

#### Fixes

Used to indicate that a commit fixes a bug introduced by a specific
commit.  Can reference either a strace commit or a GitHub issue.

**Format:**
```
Fixes: <commit-spec> "commit title"
Fixes: https://github.com/strace/strace/issues/N
```

**Examples:**
```
Fixes: v5.2~45 "Implement decoding of open_tree syscall"
Fixes: https://github.com/strace/strace/issues/359
```

**Commit reference format:**

Commit references are formatted using `git describe --match=v\* --contains`.

#### Resolves

Used to indicate that a commit resolves an issue or bug report.

```
Resolves: https://github.com/strace/strace/issues/N
Resolves: https://bugzilla.redhat.com/show_bug.cgi?id=NNNNNN
```

**Example:**
```
Resolves: https://github.com/strace/strace/issues/359
```

#### Closes

Used to indicate that a commit closes an issue (similar to Resolves).

```
Closes: https://github.com/strace/strace/issues/N
```

**Example:**
```
Closes: https://github.com/strace/strace/issues/315
```

#### References

Used to reference related issues, bugs, or discussions without
necessarily fixing them.

```
References: https://github.com/strace/strace/issues/N
References: https://bugzilla.redhat.com/show_bug.cgi?id=NNNNNN
```

**Example:**
```
References: https://github.com/strace/strace/issues/303
```

#### Complements

Used to indicate that a commit complements or works together with
another commit.

```
Complements: <commit-spec> "commit title"
```

**Example:**
```
Complements: v6.11-21-gc7e0ea6d712e "syscall: do not use uninitialized
parts of struct ptrace_syscall_info"
```

#### Reverts

Used when reverting a previous commit.  This is typically generated
automatically by `git revert`.

```
This reverts commit <full-hash> "commit title".
```

**Example:**
```
Revert "xlat: update NFT_MSG_* constants"

This reverts commit 4a80f35bda083abdd4f8be15681e24cc7d7c40c4.
```

### Trailer Format

All trailers follow this format:

```
<Trailer-Name>: <value>
```

**Rules:**
- Trailer names are case-sensitive and use hyphenated format
- MUST be placed at the end of the commit message
- Should be separated from ChangeLog entries by a blank line
- Multiple trailers of the same type are allowed (list them separately)
- Order is typically: attribution trailers first, then reference trailers

### Example with Trailers

```
xlat: update BR_* constants

* src/xlat/rtnl_ifla_br_boolopts.in (BR_BOOLOPT_FDB_LOCAL_VLAN_0):
New constant introduced by Linux kernel commit v6.18-rc1~132^2~195^2~4.
* src/xlat/rtnl_ifla_br_boolopt_flags.in
(1<<BR_BOOLOPT_FDB_LOCAL_VLAN_0): Likewise.
* tests/nlattr_ifla_linkinfo.c (main): Update expected output.
* NEWS: Mention this change.

Co-authored-by: auto@cursor.ai
Reported-by: Dmitry V. Levin <ldv@strace.io>
Reviewed-by: Eugene Syromyatnikov <evgsyr@gmail.com>
Resolves: https://github.com/strace/strace/issues/359
```

### Trailer Usage Guidelines

1. **Co-authored-by**: MUST be used when other collaborators, including
   AI tools, contributed to the commit.
2. **Signed-off-by**: Typically required for external contributions
3. **Suggested-by/Reported-by**: Credit people who contributed ideas or
   bug reports
4. **Fixes/Resolves/Closes**: Use to link commits to issues; prefer
   "Fixes" for bug fixes, "Resolves" for feature requests, "Closes" for
   general issues
5. **References**: Use when mentioning related work without directly
   addressing it
6. **Complements**: Use when a commit works with another commit but
   doesn't fix it

## Best Practices

### For Humans

1. **Be specific**: Clearly describe what changed and why
2. **Group logically**: List related files together
3. **Reference sources**: Include kernel commit references when adding constants
4. **Update NEWS**: Mention NEWS updates for user-visible changes
5. **Update tests**: Include test changes when modifying functionality
6. **Be consistent**: Follow existing patterns in the repository

### For AI Assistants

1. **Follow the format strictly**: The ChangeLog generation depends on
   exact formatting
2. **Use "Likewise." appropriately**: Only when the exact same change
   applies
3. **Reference kernel commits**: Always include kernel commit references
   for new constants
4. **Include all affected files**: Don't omit files that were modified
5. **Use proper function/constant names**: Match the exact names in the code
6. **Add Co-authored-by**: AI assistants MUST include the corresponding
   Co-authored-by: trailer when AI assistance was used in creating the commit

### Common Mistakes to Avoid

1. **Missing blank line**: The second line MUST be blank
2. **Wrong indentation**: ChangeLog entries start with `* ` (asterisk and space)
3. **Missing periods**: Each entry description should end with a period
4. **Inconsistent tense**: Use imperative mood (present tense)
5. **Missing file paths**: Don't describe changes without listing the file
6. **Omitted files**: Include all files that were modified, not just the
   main ones

### Checklist

Before committing, verify:

- [ ] Title is clear and descriptive
- [ ] Second line is blank
- [ ] All affected files are listed
- [ ] Each entry ends with a period
- [ ] Kernel commit references are included for new constants
- [ ] NEWS is updated for user-visible changes
- [ ] Tests are updated when functionality changes
- [ ] No whitespace errors introduced
- [ ] Consistent formatting throughout
- [ ] Co-authored-by trailer added if others contributed or AI
  assistance was used

## Related Documentation

- `README-hacking` - General hacking guidelines
- `src/xlat/README.md` - Xlat file format documentation
- `build-aux/gitlog-to-changelog` - Script that generates ChangeLog from commits

## Notes

- The ChangeLog file is automatically generated from commit messages at
  "make dist" time
- Commit messages are the source of truth for the ChangeLog
- Formatting is strict because it's parsed by automated tools
