/*
 * Copyright (c) 2018 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "mmap_notify.h"

struct mmap_notify_client {
	mmap_notify_fn fn;
	void *data;
	struct mmap_notify_client *next;
};

static struct mmap_notify_client *clients;

void
mmap_notify_register_client(mmap_notify_fn fn, void *data)
{
	struct mmap_notify_client *client = xmalloc(sizeof(*client));
	client->fn = fn;
	client->data = data;
	client->next = clients;
	clients = client;
}

void
mmap_notify_report(struct tcb *tcp)
{
	struct mmap_notify_client *client;

	for (client = clients; client; client = client->next)
		client->fn(tcp, client->data);
}
