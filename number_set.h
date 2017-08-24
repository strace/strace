/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_NUMBER_SET_H
#define STRACE_NUMBER_SET_H

#include "gcc_compat.h"

struct number_set;

extern bool
number_set_array_is_empty(const struct number_set *, unsigned int idx);

extern bool
is_number_in_set(unsigned int number, const struct number_set *);

extern bool
is_number_in_set_array(unsigned int number, const struct number_set *, unsigned int idx);

extern void
add_number_to_set(unsigned int number, struct number_set *);

extern void
add_number_to_set_array(unsigned int number, struct number_set *, unsigned int idx);

extern void
clear_number_set_array(struct number_set *, unsigned int nmemb);

extern void
invert_number_set_array(struct number_set *, unsigned int nmemb);

extern struct number_set *
alloc_number_set_array(unsigned int nmemb) ATTRIBUTE_MALLOC;

extern void
free_number_set_array(struct number_set *, unsigned int nmemb);

extern struct number_set *read_set;
extern struct number_set *write_set;
extern struct number_set *signal_set;

#endif /* !STRACE_NUMBER_SET_H */
