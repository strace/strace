/*
 * This file contains error printing functions.
 * These functions can be used by various binaries included in the strace
 * package.  Variable 'program_invocation_name' and function 'die()'
 * have to be defined globally.
 *
 * Copyright (c) 2001-2017 The strace developers.
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

#ifndef STRACE_ERROR_PRINTS_H
#define STRACE_ERROR_PRINTS_H

#include "gcc_compat.h"

void die(void) ATTRIBUTE_NORETURN;

void error_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void error_msg_and_help(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void error_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;

#endif /* !STRACE_ERROR_PRINTS_H */
