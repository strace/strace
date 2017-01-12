/*
 * Copyright (c) 2017 Alexey Neyman <stilor@att.net>
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

/*
 * This file is *PREPROCESSED*, not *COMPILED* for host and the result
 * is included into ioctlsort (which is compiled for build). Since some
 * of these values are used in structure initializers, they cannot be
 * defined as 'const unsigned int' - instead, they have to be macros.
 * Hence, the result of preprocessing will be run through sed to change
 * 'DEFINE' into '#define'
 */
#include <linux/ioctl.h>

DEFINE HOST_IOC_NONE _IOC_NONE
DEFINE HOST_IOC_READ _IOC_READ
DEFINE HOST_IOC_WRITE _IOC_WRITE

DEFINE HOST_IOC_SIZESHIFT _IOC_SIZESHIFT
DEFINE HOST_IOC_DIRSHIFT _IOC_DIRSHIFT
