# strace Developer Documentation Index

This is the master index of strace's developer documentation. For a general project overview, see the [top-level README](../README.md).

This directory contains documentation for strace developers and contributors. Whether you're adding a new syscall decoder, debugging test failures, or understanding strace internals, this index will help you find the right documentation.

## Getting Started

Start here if you're new to strace development.

- [CONTRIBUTING.md](../CONTRIBUTING.md) — How to contribute: code style, commit messages, testing requirements, and submission process
- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) — Adding a syscall decoder (the most common contributor task)
- [HOWTO_TEST.md](HOWTO_TEST.md) — Writing and running tests
- [GLOSSARY.md](GLOSSARY.md) — Project terminology (tcb, xlat, SEN, MPERS, etc.)

## How-To Guides

Task-oriented step-by-step guides for common development tasks.

- [HOWTO_ADD_SYSCALL.md](HOWTO_ADD_SYSCALL.md) — Adding new syscall decoders
- [HOWTO_ADD_IOCTL.md](HOWTO_ADD_IOCTL.md) — Adding ioctl decoders for device-specific ioctl commands
- [HOWTO_ADD_NETLINK.md](HOWTO_ADD_NETLINK.md) — Adding netlink protocol decoders
- [HOWTO_UPDATE_XLAT_AFTER_UAPI.md](HOWTO_UPDATE_XLAT_AFTER_UAPI.md) — Updating xlat tables after kernel UAPI changes
- [HOWTO_TEST.md](HOWTO_TEST.md) — Writing, running, and debugging tests
- [HOWTO_TEST_UNIT.md](HOWTO_TEST_UNIT.md) — Writing unit tests for internal functions and data structures
- [COMMIT-MESSAGES.md](COMMIT-MESSAGES.md) — Commit message format requirements (required reading for contributors)

## Architecture and Reference

Understanding how strace works internally.

- [INTERNALS.md](INTERNALS.md) — Architecture overview: ptrace, event loop, decoders, syscall dispatch
- [DECODER_API.md](DECODER_API.md) — Print helper reference for syscall argument decoding
- [GLOSSARY.md](GLOSSARY.md) — Terminology definitions

## Subsystem Documentation

Deep dives into specific subsystems and specialized topics.

- [README-mpers.md](README-mpers.md) — Multi-personality (MPERS) support for decoding 32-bit processes on 64-bit systems
- [README-linux-ptrace](README-linux-ptrace) — Linux ptrace API reference and behavior documentation
- [README-xlat.md](README-xlat.md) — Xlat file format for constant translation tables
- [../maint/gen/README.md](../maint/gen/README.md) — Syscall definition language for code generation
- [HACKING-scripts](HACKING-scripts) — Maintenance scripts guide (xlat generation, ioctl tables)
- [README-bundled.md](README-bundled.md) — Bundled Linux UAPI headers

## Build and Installation

- [../INSTALL-git.md](../INSTALL-git.md) — Building from git (bootstrap requirements)
- [INSTALL](../dist/INSTALL) — Build requirements and configure options
- [../README-configure](../README-configure) — Generic GNU Autoconf configuration information

## Project

- [../SECURITY.md](../SECURITY.md) — Security policy and vulnerability reporting
- [../NEWS](../NEWS) — Release notes and changelog
- strace(1) man page — [strace.1.in](strace.1.in)
- strace-log-merge(1) man page — [strace-log-merge.1.in](strace-log-merge.1.in)
