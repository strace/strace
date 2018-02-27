/*
 * Wrappers for handling discrepancies in LF64-themed syscalls availability and
 * necessity between verious architectures and kernel veriosns.
 *
 * Copyright (c) 2012-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
