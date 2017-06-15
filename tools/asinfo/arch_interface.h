/*
 * The arch_interface.h is purposed to interact with the basic data structure
 * based on arch_descriptor struct. Mainly this set of methods are used by
 * arch_dispatcher.
 *
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedv@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef ASINFO_ARCH_INTERFACE_H
#define ASINFO_ARCH_INTERFACE_H

#include "error_interface.h"
#include "sysent.h"

/* Type implementaion of syscall, internal means as a subcall,
   external means a separate syscall, this enum is purposed for
   well-known ipc and socket subcall group */
enum impl_type {
	IMPL_none = 0,
	IMPL_ext = 1,
	IMPL_int = 2,
	IMPL_int_ext = IMPL_ext | IMPL_int
};

/* Names of personalities
 * arch_pers = ARCH_ + kernel_kernel/other_name + abi_mode */
enum arch_pers {
	#include "arch_personalities.h"
};

#define MAX_ALIASES 6
#define MAX_ALT_ABIS 3

struct arch_descriptor {
	enum arch_pers pers;
	const char *arch_name[MAX_ALIASES];
	const char *abi_mode;
	const int abi_mode_len;
	enum arch_pers compat_pers[MAX_ALT_ABIS];
	const unsigned int max_scn;
	const struct_sysent *syscall_list;
	/* In the most cases these fields are purposed to store specific for
	   given arch constants, for instance, ARM_FIRST_SHUFFLED_SYSCALL */
	const int *user_num1;
	const int *user_num2;
};

#define AD_FLAG_EMPTY 0
/* to hide some abi modes belonging to one architecture */
#define AD_FLAG_PRINT	(1 << 0)
/* main personality, like x86_64 64bit */
#define AD_FLAG_MPERS	(1 << 1)

/* To provide push-back interface with arch_list */
struct arch_service {
	/* immutable field */
	const struct arch_descriptor **arch_list;
	/* User flags for each arch_descriptor */
	int *flag;
	/* To support conformity between ABI and ARCH */
	char **in_aname;
	struct error_service *err;
	unsigned capacity;
	unsigned next_free;
};

#define ARCH_LIST_DEFINE(name) \
	struct arch_service *(name)

/* Push-back interface is purposed to simplify interaction with
   arch_service struct
   NOTE: al - architecture list */

/* base methods */
struct arch_service *al_create(unsigned capacity);

int al_push(struct arch_service *m, const struct arch_descriptor *element);

int al_set_flag(struct arch_service *m, unsigned index, int flag);

int al_add_flag(struct arch_service *m, unsigned index, int flag);

int al_sub_flag(struct arch_service *m, unsigned index, int flag);

const struct arch_descriptor *al_get(struct arch_service *m, unsigned index);

unsigned int al_size(struct arch_service *m);

void al_free(struct arch_service *m);

struct error_service *al_err(struct arch_service *m);

/* methods returning fields with error check */
enum arch_pers al_pers(struct arch_service *m, unsigned index);

const char **al_arch_name(struct arch_service *m, unsigned index);

enum arch_pers *al_cpers(struct arch_service *m, unsigned index);

const char *al_abi_mode(struct arch_service *m, unsigned index);

int al_abi_mode_len(struct arch_service *m, unsigned index);

int al_flag(struct arch_service *m, unsigned index);

int al_set_in_aname(struct arch_service *m, unsigned index, char *aname);

char *al_in_aname(struct arch_service *m, unsigned index);

/* calculating methods */
int al_psize(struct arch_service *m);

int al_arch_name_len(struct arch_service *m, unsigned index, int delim_len);

int al_syscall_impl(struct arch_service *m, unsigned index);

int al_get_abi_modes(struct arch_service *m, unsigned index);

const char *al_next_alias(struct arch_service *m, unsigned index);

enum arch_pers al_next_cpers(struct arch_service *m, unsigned index);

enum impl_type al_ipc_syscall(struct arch_service *m, unsigned index);

enum impl_type al_sck_syscall(struct arch_service *m, unsigned index);

struct arch_service *al_create_filled(void);

int al_mark_matches(struct arch_service *m, char *arch_str);

struct arch_service *al_join_print(struct arch_service *f,
				   struct arch_service *s);

void al_unmark_all(struct arch_service *m, int flag);

int al_mark_pers4arch(struct arch_service *m, unsigned index,
		      const char *abi_mode);

void al_dump(struct arch_service *m, int is_raw);

#endif /* !ASINFO_ARCH_INTERFACE_H */
