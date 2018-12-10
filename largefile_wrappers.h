/*
 * Wrappers for handling discrepancies in LF64-themed syscalls availability and
 * necessity between verious architectures and kernel veriosns.
 *
 * Copyright (c) 2012-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LARGEFILE_WRAPPERS_H
#define STRACE_LARGEFILE_WRAPPERS_H

#include "defs.h"

#ifdef _LARGEFILE64_SOURCE
# ifdef HAVE_FOPEN64
#  define fopen_stream fopen64
# else
#  define fopen_stream fopen
# endif
# define struct_stat struct stat64
# define stat_file stat64
# define struct_dirent struct dirent64
# define read_dir readdir64
# define struct_rlimit struct rlimit64
# define set_rlimit setrlimit64
#else
# define fopen_stream fopen
# define struct_stat struct stat
# define stat_file stat
# define struct_dirent struct dirent
# define read_dir readdir
# define struct_rlimit struct rlimit
# define set_rlimit setrlimit
#endif

#endif /* STRACE_LARGEFILE_WRAPPERS_H */
