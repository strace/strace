#ifndef STRACE_GDBSERVER_PROTOCOL_H
#define STRACE_GDBSERVER_PROTOCOL_H

/* Simple interface of a GDB remote protocol client.
 *
 * Copyright (c) 2015 Red Hat Inc.
 * Copyright (c) 2015 Josh Stone <cuviper@gmail.com>
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct gdb_conn;

/**
 * Structure containing response for vFile requests[1].
 *
 * [1] https://sourceware.org/gdb/onlinedocs/gdb/Host-I_002fO-Packets.html
 */
struct vfile_response {
	/** The whole reply, callee-allocated. */
	char *reply;
	/** Result, returned from the target */
	int64_t result;
	/** Error number returned from the target */
	int64_t errnum;
	/** Size of attachment, in bytes */
	uint64_t attachment_size;
	/**
	 * Pointer to the attachment inside reply. The attachment is hex-encoded
	 * (should be subjected to gdb_decode_hex* before usage).
	 */
	const char *attachment;
};

void gdb_encode_hex(uint8_t byte, char *out);
uint16_t gdb_decode_hex(char msb, char lsb);
uint64_t gdb_decode_hex_n(const char *bytes, size_t n);
uint64_t gdb_decode_hex_str(const char *bytes);
int gdb_decode_hex_buf(const char *bytes, size_t n, char *out);

struct gdb_conn *gdb_begin_command(const char *command);
struct gdb_conn *gdb_begin_tcp(const char *node, const char *service);
struct gdb_conn *gdb_begin_path(const char *path);

void gdb_end(struct gdb_conn *conn);

void gdb_send(struct gdb_conn *conn, const char *command, size_t size);
void gdb_send_str(struct gdb_conn *conn, const char *command);

#define gdb_send_cstr(_conn, _str) \
	gdb_send((_conn), _str, sizeof(_str) + MUST_BE_ARRAY(_str) - 1)

char *gdb_recv(struct gdb_conn *conn, /* out */ size_t *size, bool want_stop);

bool gdb_start_noack(struct gdb_conn *conn);

void gdb_set_non_stop(struct gdb_conn *conn, bool val);

bool gdb_has_non_stop(struct gdb_conn *conn);

char* pop_notification(size_t *size);

void push_notification(char *packet, size_t packet_size);

bool have_notification();

/* Read complete qXfer data, returned as binary with the size.
 * On error, returns NULL with size set to the error code.
 */
char *gdb_xfer_read(struct gdb_conn *conn,
        const char *object, const char *annex,
        /* out */ size_t *size);

int gdb_readlink(struct gdb_conn *conn, const char *linkpath,
        char *buf, unsigned bufsize);

#endif /* !STRACE_GDBSERVER_PROTOCOL_H */
