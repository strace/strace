/*
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch_interface.h"
#include "defs.h"
#include "macros.h"
#include "xmalloc.h"

/* Define these shorthand notations to simplify the syscallent files. */
#include "sysent_shorthand_defs.h"

/* For the current functionality there is no need
   to use sen and (*sys_func)() fields in sysent struct */
#define SEN(syscall_name) 0, NULL

/* Generated file based on arch_definitions.h */
#include "arch_includes.h"

/* Now undef them since short defines cause wicked namespace pollution. */
#include "sysent_shorthand_undefs.h"

#define PASS(...) __VA_ARGS__
#define ARCH_DESC_DEFINE(arch, mode, comp_pers, arch_aliases) \
	[ARCH_##arch##_##mode] = { \
		.pers			= ARCH_##arch##_##mode, \
		.arch_name		= arch_aliases, \
		.abi_mode		= #mode, \
		.abi_mode_len		= ARRAY_SIZE(#arch) - 1, \
		.compat_pers		= comp_pers, \
		.max_scn		= ARRAY_SIZE(arch##_##mode##_sysent), \
		.syscall_list		= arch##_##mode##_sysent, \
		.user_num1		= &arch##_##mode##_usr1, \
		.user_num2		= &arch##_##mode##_usr2, \
	}

/* Generate array of arch_descriptors for each personality */
const struct arch_descriptor architectures[] = {
	#include "arch_definitions.h"
};

#undef ARCH_DESC_DEFINE
#undef PASS

struct arch_service *
al_create(unsigned capacity)
{
	ARCH_LIST_DEFINE(as) = NULL;

	if (!capacity)
		return NULL;
	as = xcalloc(sizeof(*as), 1);
	as->arch_list = xcalloc(sizeof(*(as->arch_list)), capacity);
	as->flag = xcalloc(sizeof(*(as->flag)), capacity);
	as->in_aname = xcalloc(sizeof(*(as->in_aname)), capacity);
	as->err = es_create();
	as->capacity = capacity;
	as->next_free = 0;
	return as;
}

int
al_push(struct arch_service *m, const struct arch_descriptor *element)
{
	if (m->next_free >= m->capacity)
		return -1;
	m->arch_list[m->next_free] = element;
	m->flag[m->next_free] = AD_FLAG_EMPTY;
	m->next_free++;
	return 0;
}

static inline int
al_is_index_ok(struct arch_service *m, unsigned index)
{
	if (index >= m->next_free)
		return -1;
	return 0;
}

int
al_set_flag(struct arch_service *m, unsigned index, int flag)
{
	if (al_is_index_ok(m, index) == 0) {
		m->flag[index] = flag;
		return 0;
	}
	return -1;
}

int
al_add_flag(struct arch_service *m, unsigned index, int flag)
{
	if (al_is_index_ok(m, index) == 0) {
		m->flag[index] = m->flag[index] | flag;
		return 0;
	}
	return -1;
}

int
al_sub_flag(struct arch_service *m, unsigned index, int flag)
{
	if (al_is_index_ok(m, index) == 0) {
		m->flag[index] = m->flag[index] & ~flag;
		return 0;
	}
	return -1;
}

const struct arch_descriptor *
al_get(struct arch_service *m, unsigned index)
{
	if (al_is_index_ok(m, index) != 0)
		return NULL;
	return m->arch_list[index];
}

unsigned int
al_size(struct arch_service *m)
{
	return m->next_free;
}

void
al_free(struct arch_service *m)
{
	int i;
	int size = al_size(m);

	for (i = 0; i < size; i++)
		if (al_in_aname(m, i) != NULL)
			free(al_in_aname(m, i));
	free(m->arch_list);
	free(m->flag);
	free(m->in_aname);
	es_free(m->err);
	free(m);
}

struct error_service *al_err(struct arch_service *m)
{
	return m->err;
}

enum arch_pers
al_pers(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);

	return (elem ? elem->pers : ARCH_no_pers);
}

const char **
al_arch_name(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);

	return (elem ? (const char **)elem->arch_name : NULL);
}

enum arch_pers *
al_cpers(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);

	return (elem ? (enum arch_pers *)elem->compat_pers : NULL);
}

const char *
al_abi_mode(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);

	return (elem ? elem->abi_mode : NULL);
}

int
al_abi_mode_len(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);

	return (elem ? elem->abi_mode_len : -1);
}

int
al_flag(struct arch_service *m, unsigned index)
{
	int status = al_is_index_ok(m, index);

	return (!status ? m->flag[index] : -1);
}

int
al_set_in_aname(struct arch_service *m, unsigned index, char *aname)
{
	int status = al_is_index_ok(m, index);

	if (status)
		return -1;
	m->in_aname[index] = aname;
	return 0;
}

char *
al_in_aname(struct arch_service *m, unsigned index)
{
	int status = al_is_index_ok(m, index);

	return (!status ? m->in_aname[index] : NULL);
}

int
al_psize(struct arch_service *m)
{
	int i;
	int a_size = al_size(m);
	int psize = 0;

	for (i = 0; i < a_size; i++)
		if (al_flag(m, i) & AD_FLAG_PRINT)
			psize++;
	return psize;
}

int
al_arch_name_len(struct arch_service *m, unsigned index, int delim_len)
{
	const char **arch_name = NULL;
	int i;
	int final_len = 0;

	while (!(al_flag(m, index) & AD_FLAG_MPERS))
		index--;
	arch_name = al_arch_name(m, index);
	for (i = 0; (arch_name[i] != NULL) && (i < MAX_ALIASES); i++) {
		final_len += strlen(arch_name[i]);
		final_len += delim_len;
	}
	final_len -= delim_len;
	return final_len;
}

int
al_syscall_impl(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);
	int count = 0;

	if (!elem)
		return -1;
	for (unsigned int i = 0; i < elem->max_scn; ++i) {
		if (elem->syscall_list[i].sys_name &&
		    !(elem->syscall_list[i].sys_flags &
		    TRACE_INDIRECT_SUBCALL))
			count++;
	}
	return count;
}

/* This method is purposed to count the supported ABI modes for the given
   arch */
int
al_get_abi_modes(struct arch_service *m, unsigned index)
{
	const struct arch_descriptor *elem = al_get(m, index);
	int i = 0;
	int abi_count = 1;

	if (!elem)
		return -1;
	for (i = 0; i < MAX_ALT_ABIS; i++)
		if (elem->compat_pers[i] != ARCH_no_pers)
			abi_count++;
	return abi_count;
}

/* This method is purposed to find next one name of the same architecture.
   For instance, x86_64 = amd64 */
const char *
al_next_alias(struct arch_service *m, unsigned index)
{
	static int next_alias = -1;
	static const char **arch_name = NULL;
	static unsigned lindex = 0;

	if (lindex != index) {
		lindex = index;
		next_alias = -1;
	}
	if (al_pers(m, index) == ARCH_no_pers)
		return NULL;
	if (next_alias == -1) {
		next_alias = 0;
		while (!(al_flag(m, index) & AD_FLAG_MPERS))
			index--;
		arch_name = al_arch_name(m, index);
	} else
		next_alias++;
	if (next_alias >= MAX_ALIASES || arch_name[next_alias] == NULL) {
		next_alias = -1;
		return NULL;
	}
	return arch_name[next_alias];
}

/* This method is purposed to return next one compat personality of the
   same architecture */
enum arch_pers
al_next_cpers(struct arch_service *m, unsigned index)
{
	static int next_pers = -1;
	enum arch_pers *a_pers = al_cpers(m, index);
	static unsigned lindex = 0;

	if (al_pers(m, index) == ARCH_no_pers)
		return ARCH_no_pers;
	if (lindex != index) {
		lindex = index;
		next_pers = -1;
	}
	if (next_pers == -1)
		next_pers = 0;
	else
		next_pers++;
	if (next_pers >= MAX_ALT_ABIS ||
	    a_pers[next_pers] == ARCH_no_pers) {
		next_pers = -1;
		return ARCH_no_pers;
	}
	return a_pers[next_pers];
}

static enum impl_type
al_indirect_subcall(struct arch_service *m, unsigned index, const char *name)
{
	const struct arch_descriptor *elem = al_get(m, index);
	unsigned int impl = IMPL_none;

	for (unsigned int i = 0; i < elem->max_scn; ++i) {
		if (!elem->syscall_list[i].sys_name)
			continue;
		if (!strcmp(elem->syscall_list[i].sys_name, name)) {
			if (!(elem->syscall_list[i].sys_flags &
			     TRACE_INDIRECT_SUBCALL))
				impl |= IMPL_ext;
			else
				impl |= IMPL_int;
		}
	}
	return impl;
}

/* This method is purposed to create extended list of architectures */
struct arch_service *
al_create_filled(void)
{
	static const int architectures_size = ARRAY_SIZE(architectures) - 1;
	ARCH_LIST_DEFINE(as) = al_create(architectures_size);
	ARCH_LIST_DEFINE(f_as);
	enum arch_pers cpers;
	int esize = 0;
	const char **arch_name = NULL;
	int i;

	/* Push and calculate size of extended table */
	for (i = 0; i < architectures_size; i++) {
		al_push(as, &(architectures[i + 1]));
		arch_name = al_arch_name(as, i);
		if (arch_name[0] != NULL)
			esize += al_get_abi_modes(as, i);
	}
	f_as = al_create(esize);
	/* Fill extended teble */
	for (i = 0; i < architectures_size; i++) {
		arch_name = al_arch_name(as, i);
		if (arch_name[0] == NULL)
			continue;
		al_push(f_as, al_get(as, i));
		al_add_flag(f_as, al_size(f_as) - 1, AD_FLAG_MPERS);
		while ((cpers = al_next_cpers(as, i)) != ARCH_no_pers)
			al_push(f_as, &(architectures[cpers]));
	}
	free(as);
	return f_as;
}

/* To look up arch in arch_descriptor array */
int
al_mark_matches(struct arch_service *m, char *arch_str)
{
	int arch_match = -1;
	char *match_pointer = NULL;
	const char *a_name = NULL;
	int al_size_full = al_size(m);
	unsigned prev_arch_len = 0;
	int i;
	int a_abi;
	char *in_aname;

	if (arch_str == NULL)
		return -1;
	/* Here we find the best match for arch_str in architecture list.
	   Best match means here that we have to find the longest name of
	   architecture in a_full_list with arch_str substring, beginning
	   from the first letter */
	for (i = 0; i < al_size_full; i++) {
		if (!(al_flag(m, i) & AD_FLAG_MPERS))
			continue;
		while ((a_name = al_next_alias(m, i)) != NULL) {
			match_pointer = strstr(arch_str, a_name);
			if (match_pointer == NULL || match_pointer != arch_str)
				continue;
			if (arch_match == -1 ||
			    strlen(a_name) > prev_arch_len) {
				prev_arch_len = strlen(a_name);
				arch_match = i;
			}
		}
	}
	if (arch_match == -1)
		return -1;
	/* Now we find all ABI modes related to the architecture */
	if ((a_abi = al_get_abi_modes(m, arch_match)) == -1)
		return -1;
	for (i = arch_match; i < (arch_match + a_abi); i++) {
		al_add_flag(m, i, AD_FLAG_PRINT);
		in_aname = xcalloc(sizeof(*in_aname), strlen(arch_str) + 1);
		strcpy(in_aname, arch_str);
		al_set_in_aname(m, i, in_aname);
	}
	return 0;
}

/* Join all architectures from 'f' and architectures with AD_FLAG_PRINT
   from 's' arch_service structures */
struct arch_service *
al_join_print(struct arch_service *f, struct arch_service *s)
{
	int size1 = (f ? al_size(f) : 0);
	int psize2 = al_psize(s);
	int size2 = al_size(s);
	int i;
	int start_point = 0;
	ARCH_LIST_DEFINE(final) = al_create(size1 + psize2);

	for (i = 0; i < size2; i++)
		if (al_flag(s, i) & AD_FLAG_PRINT) {
			start_point = i;
			break;
		}
	for (i = 0; i < size1; i++) {
		al_push(final, al_get(f, i));
		al_set_flag(final, i, al_flag(f, i));
		al_set_in_aname(final, i, al_in_aname(f, i));
		al_set_in_aname(f, i, NULL);
	}
	for (i = 0; i < psize2; i++) {
		al_push(final, al_get(s, start_point + i));
		al_set_flag(final, size1 + i , al_flag(s, start_point + i));
		al_set_in_aname(final, size1 + i,
				al_in_aname(s, start_point + i));
		al_set_in_aname(s, start_point + i, NULL);
		al_sub_flag(s, start_point + i, AD_FLAG_PRINT);
	}
	if (f)
		al_free(f);
	return final;
}

/* To avoid duplication of for(;;) construction */
void
al_unmark_all(struct arch_service *m, int flag)
{
	int a_size = al_size(m);
	int i;

	for (i = 0; i < a_size; i++)
		al_sub_flag(m, i, flag);
}

/* Select one compatible personality in range of one architecture */
int
al_mark_pers4arch(struct arch_service *m, unsigned index, const char *abi_mode)
{
	unsigned i = index;

	while (!(al_flag(m, i) & AD_FLAG_MPERS) || (i == index)) {
		if (strcmp(abi_mode, "all") == 0) {
			al_add_flag(m, i, AD_FLAG_PRINT);
			i++;
			if ((al_is_index_ok(m, i)) ||
			    (al_flag(m, i) & AD_FLAG_MPERS))
				return 0;
			else
				continue;
		}
		if (strcmp(al_abi_mode(m, i), abi_mode) == 0) {
			al_add_flag(m, i, AD_FLAG_PRINT);
			return 0;
		}
		i++;
	}
	return -1;
}

void
al_dump(struct arch_service *m, int is_raw)
{
	static const char *title[] = {
		"N",
		"Architecture name",
		"ABI mode",
		/* Implemented syscalls */
		"IMPL syscalls",
		/* IPC implementation */
		"IPC IMPL",
		/* SOCKET implementation */
		"SOCKET IMPL"
	};
	int title_len[] = {
		0,
		strlen(title[1]),
		strlen(title[2]),
		strlen(title[3]),
		strlen(title[4]),
		strlen(title[5]),
	};
	static const char *impl_st[] = {
		"none",
		"external",
		"internal",
		"int/ext"
	};
	static const char *delim = "/";
	int i = 0;
	int N = 0;
	int temp_len = 0;
	int arch_size = al_size(m);
	int arch_psize = al_psize(m);
	const char *next_alias = NULL;
	char *whole_arch_name;

	/* Calculate length of the column with the number of architectures */
	for (i = 1; arch_psize/i != 0; i *= 10)
		title_len[0]++;
	for (i = 0; i < arch_size; i++) {
		if (!(al_flag(m, i) & AD_FLAG_PRINT))
			continue;
		/* Calculate length of the column with the
		   architectures name */
		temp_len = al_arch_name_len(m, i, strlen(delim));
		if (temp_len > title_len[1])
			title_len[1] = temp_len;
		/* Calculate length of the column with the ABI mode */
		if (al_abi_mode_len(m, i) > title_len[2])
			title_len[2] = al_abi_mode_len(m, i);
	}

	whole_arch_name = xcalloc(title_len[1] + 1, sizeof(*whole_arch_name));
	/* Output title */
	if (!is_raw)
		printf("| %*s | %*s | %*s | %*s | %*s | %*s |\n",
			title_len[0], title[0], title_len[1], title[1],
			title_len[2], title[2], title_len[3], title[3],
			title_len[4], title[4], title_len[5], title[5]);
	/* Output architectures */
	for (i = 0; i < arch_size; i++) {
		if (!(al_flag(m, i) & AD_FLAG_PRINT))
			continue;
		N++;
		memset(whole_arch_name, 0, title_len[1]);
		/* Put all the same arch back together */
		next_alias = al_next_alias(m, i);
		strcat(whole_arch_name, next_alias);
		while ((next_alias = al_next_alias(m, i)) != NULL) {
			strcat(whole_arch_name, delim);
			strcat(whole_arch_name, next_alias);
		}
		if (is_raw) {
			printf("%u;%s;%s;%d;%s;%s;\n", N, whole_arch_name,
			       al_abi_mode(m, i), al_syscall_impl(m, i),
			       impl_st[al_indirect_subcall(m, i, "semctl")],
			       impl_st[al_indirect_subcall(m, i, "socket")]);
			continue;
		}
		printf("| %*u | ", title_len[0], N);
		printf("%*s | ", title_len[1], whole_arch_name);
		printf("%*s | ", title_len[2], al_abi_mode(m, i));
		printf("%*d | ", title_len[3], al_syscall_impl(m, i));
		printf("%*s | ", title_len[4], impl_st[al_indirect_subcall(m, i, "semctl")]);
		printf("%*s |\n", title_len[5], impl_st[al_indirect_subcall(m, i, "socket")]);
	}
	free(whole_arch_name);
}
