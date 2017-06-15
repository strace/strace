/*
 * The syscall_interface.h is purposed to interact with the basic data
 * structure based on arch_descriptor struct. Mainly this set of methods are
 * used by syscall_dispatcher.
 *
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedv@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef ASINFO_SYSCALL_INTERFACE_H
#define ASINFO_SYSCALL_INTERFACE_H

#include <limits.h>
#include <stdbool.h>

#include "arch_interface.h"
#include "error_interface.h"
#include "sysent.h"

#define SS_FLAG_EMPTY 0
#define SS_FLAG_PRINT 1

#define HIDDEN_SYSC INT_MIN

/* Complete element type ‘struct number_set’ */
typedef unsigned int number_slot_t;

struct number_set {
	number_slot_t *vec;
	unsigned int nslots;
	bool not;
};

/* To avoid include defs.h */
extern bool is_number_in_set(unsigned int number, const struct number_set *);

struct arch_wrapper {
	const struct arch_descriptor *arch;
	/* Mutable user flags for each syscall */
	int *flag;
	int *real_snum;
	char *a_name;
};

struct syscall_service {
	struct arch_wrapper *aws;
	int narch;
	/* To choose the format while dumping */
	int request_type;
	struct error_service *err;
};

/* These structures are purposed to make union and enumeration with
   syscall list */
struct out_sysc {
	const char *sys_name;
	int sys_num;
	void **data;
};

struct sysc_meta {
	struct out_sysc *sysc_list;
	int size;
};

struct in_sysc {
	const char *sys_name;
	void *data;
};

#define SYSCALL_LIST_DEFINE(name) \
	struct syscall_service *(name)

/* base methods
   ss is related to syscall_service
   ssa is related to arch_wrapper */
struct syscall_service *ss_create(struct arch_service *m, int request_type);

int ssa_is_ok(struct syscall_service *s, int arch, int num);

struct error_service *ss_err(struct syscall_service *s);

int ss_size(struct syscall_service *m);

int ssa_max_scn(struct syscall_service *s, int arch);

const struct_sysent *ssa_sysc_list(struct syscall_service *s, int arch);

int ssa_flag(struct syscall_service *s, int arch, int num);

int ssa_set_flag(struct syscall_service *s, int arch, int num, int flag);

int ssa_real_num(struct syscall_service *s, int arch, int num);

int ssa_set_real_num(struct syscall_service *s, int arch, int num,
		     int real_num);

const char *ssa_syscall_name(struct syscall_service *s, int arch, int num);

int ssa_syscall_flag(struct syscall_service *s, int arch, int num);

int ssa_syscall_nargs(struct syscall_service *s, int arch, int num);

int ssa_user_num1(struct syscall_service *s, int arch);

int ssa_user_num2(struct syscall_service *s, int arch);

void ss_free(struct syscall_service *s);

/* calculating methods */
int ssa_print_size(struct syscall_service *s, int arch);

int ssa_find_snum(struct syscall_service *s, int arch, int real_num);

int ss_mark_matches(struct syscall_service *s, int arch, char *arg);

int ss_update_sc_num(struct syscall_service *s, int arch);

void ss_dump(struct syscall_service *s, int is_raw);

#endif /* !ASINFO_SYSCALL_INTERFACE_H */
