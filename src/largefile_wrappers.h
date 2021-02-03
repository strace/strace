/*
 * Wrappers for handling discrepancies in LF64-themed syscalls availability and
 * necessity between various architectures and kernel versions.
 *
 * Copyright (c) 2012-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LARGEFILE_WRAPPERS_H
# define STRACE_LARGEFILE_WRAPPERS_H

# include "defs.h"

# ifdef _LARGEFILE64_SOURCE
#  ifdef HAVE_OPEN64
#   define open_file open64
#  else
#   define open_file open
#  endif
#  ifdef HAVE_FOPEN64
#   define fopen_stream fopen64
#  else
#   define fopen_stream fopen
#  endif
#  ifdef HAVE_FCNTL64
#   define fcntl_fd fcntl64
#  else
#   define fcntl_fd fcntl
#  endif
#  define fstat_fd fstat64
#  define strace_stat_t struct stat64
#  define stat_file stat64
#  define struct_dirent struct dirent64
#  define read_dir readdir64
#  define struct_rlimit struct rlimit64
#  define set_rlimit setrlimit64
# else
#  define open_file open
#  define fopen_stream fopen
#  define fcntl_fd fcntl
#  define fstat_fd fstat
#  define strace_stat_t struct stat
#  define stat_file stat
#  define struct_dirent struct dirent
#  define read_dir readdir
#  define struct_rlimit struct rlimit
#  define set_rlimit setrlimit
# endif

#endif /* STRACE_LARGEFILE_WRAPPERS_H */
