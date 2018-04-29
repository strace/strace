/*
 * This file is based on a patch submitted by Mark Wielaard <mjw@redhat.com>
 * to ltrace project:
 * https://anonscm.debian.org/cgit/collab-maint/ltrace.git/commit/?id=dfefa9f057857735a073ea655f5cb34351032c8e
 *
 * It was re-licensed for strace by the original author:
 * https://lists.strace.io/pipermail/strace-devel/2018-March/008063.html
 *
 * Copyright (c) 2014-2018 Mark Wielaard <mjw@redhat.com>
 * Copyright (c) 2018 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2018 The strace developers.
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
#include "unwind.h"
#include "mmap_notify.h"
#include <elfutils/libdwfl.h>

struct ctx {
	Dwfl *dwfl;
	unsigned int last_proc_updating;
};

static unsigned int mapping_generation;

static void
update_mapping_generation(struct tcb *tcp, void *unused)
{
	mapping_generation++;
}

static void
init(void)
{
	mmap_notify_register_client(update_mapping_generation, NULL);
}

static void *
tcb_init(struct tcb *tcp)
{
	static const Dwfl_Callbacks proc_callbacks = {
		.find_elf = dwfl_linux_proc_find_elf,
		.find_debuginfo = dwfl_standard_find_debuginfo
	};

	Dwfl *dwfl = dwfl_begin(&proc_callbacks);
	if (dwfl == NULL) {
		error_msg("dwfl_begin: %s", dwfl_errmsg(-1));
		return NULL;
	}

	int r = dwfl_linux_proc_attach(dwfl, tcp->pid, true);
	if (r) {
		const char *msg = NULL;

		if (r < 0)
			msg = dwfl_errmsg(-1);
		else if (r > 0)
			msg = strerror(r);

		error_msg("dwfl_linux_proc_attach returned an error"
			  " for process %d: %s", tcp->pid, msg);
		dwfl_end(dwfl);
		return NULL;
	}

	struct ctx *ctx = xmalloc(sizeof(*ctx));
	ctx->dwfl = dwfl;
	ctx->last_proc_updating = 0;
	return ctx;
}

static void
tcb_fin(struct tcb *tcp)
{
	struct ctx *ctx = tcp->unwind_ctx;
	if (ctx) {
		dwfl_end(ctx->dwfl);
		free(ctx);
	}
}

static void
flush_cache_maybe(struct tcb *tcp)
{
	struct ctx *ctx = tcp->unwind_ctx;
	if (!ctx)
		return;

	if (ctx->last_proc_updating == mapping_generation)
		return;

	int r = dwfl_linux_proc_report(ctx->dwfl, tcp->pid);

	if (r < 0)
		error_msg("dwfl_linux_proc_report returned an error"
			  " for pid %d: %s", tcp->pid, dwfl_errmsg(-1));
	else if (r > 0)
		error_msg("dwfl_linux_proc_report returned an error"
			  " for pid %d", tcp->pid);
	else if (dwfl_report_end(ctx->dwfl, NULL, NULL) != 0)
		error_msg("dwfl_report_end returned an error"
			  " for pid %d: %s", tcp->pid, dwfl_errmsg(-1));

	ctx->last_proc_updating = mapping_generation;
}

struct frame_user_data {
	unwind_call_action_fn call_action;
	unwind_error_action_fn error_action;
	void *data;
	int stack_depth;
};

static int
frame_callback(Dwfl_Frame *state, void *arg)
{
	struct frame_user_data *user_data = arg;
	Dwarf_Addr pc;
	bool isactivation;

	if (!dwfl_frame_pc(state, &pc, &isactivation)) {
		/* Propagate the error to the caller.  */
		return -1;
	}

	if (!isactivation)
		pc--;

	Dwfl *dwfl = dwfl_thread_dwfl(dwfl_frame_thread(state));
	Dwfl_Module *mod = dwfl_addrmodule(dwfl, pc);
	GElf_Off off = 0;

	if (mod != NULL) {
		const char *modname = NULL;
		const char *symname = NULL;
		GElf_Sym sym;
		Dwarf_Addr true_offset = pc;

		modname = dwfl_module_info(mod, NULL, NULL, NULL, NULL,
					   NULL, NULL, NULL);
		symname = dwfl_module_addrinfo(mod, pc, &off, &sym,
					       NULL, NULL, NULL);
		dwfl_module_relocate_address(mod, &true_offset);
		user_data->call_action(user_data->data, modname, symname,
				       off, true_offset);
	}
	/* Max number of frames to print reached? */
	if (user_data->stack_depth-- == 0)
		return DWARF_CB_ABORT;

	return DWARF_CB_OK;
}

static void
tcb_walk(struct tcb *tcp,
	 unwind_call_action_fn call_action,
	 unwind_error_action_fn error_action,
	 void *data)
{
	struct ctx *ctx = tcp->unwind_ctx;
	if (!ctx)
		return;

	struct frame_user_data user_data = {
		.call_action = call_action,
		.error_action = error_action,
		.data = data,
		.stack_depth = 256,
	};

	flush_cache_maybe(tcp);

	int r = dwfl_getthread_frames(ctx->dwfl, tcp->pid, frame_callback,
				      &user_data);
	if (r)
		error_action(data,
			     r < 0 ? dwfl_errmsg(-1) : "too many stack frames",
			     0);
}

const struct unwind_unwinder_t unwinder = {
	.name = "libdw",
	.init = init,
	.tcb_init = tcb_init,
	.tcb_fin = tcb_fin,
	.tcb_walk = tcb_walk,
};
