/*
 * Auxiliary children support implementation.
 *
 * Copyright (c) 2017 The strace developers.
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

#include "defs.h"

#include <sys/types.h>
#include <sys/wait.h>

#include "list.h"

#include "aux_children.h"

/* We store children as a linked list since there is not a lot of them and in
 * order to ease removal */

struct aux_child {
	pid_t pid;
	const struct aux_child_handlers *handlers;
	void *signal_fn_data;
	void *exit_notify_fn_data;
	void *exit_wait_fn_data;
	struct list_item list;
};

static EMPTY_LIST(children);


void
register_aux_child_ex(pid_t pid, const struct aux_child_handlers *h,
		      void *signal_fn_data, void *exit_notify_fn_data,
		       void *exit_wait_fn_data)
{
	static const struct aux_child_handlers default_handlers;

	struct aux_child *child;

	list_foreach(child, &children, list) {
		if (child->pid == pid)
			error_func_msg_and_die("Duplicate auxiliary child pid");
	}

	child = xcalloc(1, sizeof(*child));

	child->pid = pid;
	child->handlers = h ?: &default_handlers;
	child->signal_fn_data = signal_fn_data;
	child->exit_notify_fn_data = exit_notify_fn_data;
	child->exit_wait_fn_data = exit_wait_fn_data;

	list_append(&children, &child->list);
}

void
remove_aux_child(pid_t pid)
{
	struct aux_child *child;

	list_foreach(child, &children, list) {
		if (child->pid == pid) {
			list_remove(&child->list);
			free(child);
		}
	}
}

bool
have_aux_children(void)
{
	return !list_is_empty(&children);
}

enum aux_child_sig
aux_children_signal(pid_t pid, int status)
{
	struct aux_child *child;

	list_foreach(child, &children, list) {
		if (child->pid == pid) {
			aux_child_signal_fn sig_handler =
				child->handlers->signal_fn;

			if (!sig_handler)
				sig_handler = aux_child_sig_handler;

			return sig_handler(pid, status, child->signal_fn_data);
		}
	}

	return ACS_NONE;
}

void
aux_children_exit_notify(int exit_code)
{
	struct aux_child *cur;
	struct aux_child *tmp;

	list_foreach_safe(cur, &children, list, tmp) {
		if (cur->handlers->exit_notify_fn)
			cur->handlers->exit_notify_fn(cur->pid, exit_code,
						      cur->exit_notify_fn_data);
	}
}

int
aux_children_exit_wait(int exit_code)
{
	int cnt = 0;
	struct aux_child *cur;
	struct aux_child *tmp;

	list_foreach_safe(cur, &children, list, tmp) {
		aux_child_exit_fn wait_handler = cur->handlers->exit_wait_fn;


		if (!wait_handler)
			wait_handler = aux_child_wait_handler;

		if (wait_handler(cur->pid, exit_code, cur->exit_wait_fn_data)
		    == ACR_REMOVE_ME) {
			list_remove(&cur->list);
			free(cur);
		} else {
			cnt++;
		}
	}

	return cnt;
}

enum aux_child_sig
aux_child_sig_handler(pid_t pid, int status, void *data)
{
	if (!WIFSTOPPED(status))
		remove_aux_child(pid);

	return ACS_CONTINUE;
}

enum aux_child_ret
aux_child_wait_handler(pid_t pid, int exit_code, void *data)
{
	while (waitpid(pid, NULL, 0) < 0 && errno == EINTR)
		;

	return ACR_REMOVE_ME;
}
