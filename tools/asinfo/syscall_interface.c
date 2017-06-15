/*
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedv@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "arch_interface.h"
#include "error_interface.h"
#include "filter.h"
#include "number_set.h"
#include "arch_defs.h"
#include "syscall_interface.h"
#include "sysent.h"
#include "request_msgs.h"
#include "xmalloc.h"

/* We shouldn't include defs.h here, because the following definitions
   cannot be with const qualifier */
const struct_sysent *sysent_vec[SUPPORTED_PERSONALITIES] = {NULL};
unsigned int nsyscall_vec[SUPPORTED_PERSONALITIES] = {0};

const char *const personality_designators[] =
# if defined X86_64
	{ "64", "32", "x32" }
# elif defined X32
	{ "x32", "32" }
# elif SUPPORTED_PERSONALITIES == 2
	{ "64", "32" }
# else
	{ STRINGIFY_VAL(__WORDSIZE) }
# endif
	;

struct syscall_service *
ss_create(struct arch_service *m, int request_type)
{
	int i;
	int ss_count = 0;
	int ssize = al_psize(m);
	int asize = al_size(m);
	struct syscall_service *ss = NULL;
	int scn = 0;

	ss = xcalloc(sizeof(*ss), 1);
	ss->err = al_err(m);
	/* If we are in arch/abi mode, but we need syscall_service to pass
	   check for errors */
	if (!(request_type & SD_REQ_MASK) || ssize == 0)
		return ss;
	ss->aws = xcalloc(sizeof(*(ss->aws)), ssize);
	ss->narch = ssize;
	for (i = 0; i < asize; i++)
		if (al_flag(m, i) & AD_FLAG_PRINT) {
			ss->aws[ss_count].arch = al_get(m, i);
			scn = ss->aws[ss_count].arch->max_scn;
			ss->aws[ss_count].flag = xcalloc(sizeof(int), scn);
			ss->aws[ss_count].real_snum = xcalloc(sizeof(int), scn);
			ss->aws[ss_count].a_name = al_in_aname(m, i);
			al_set_in_aname(m, i, NULL);
			ss_count++;
		}
	ss->request_type = request_type;
	return ss;
}

int
ssa_is_ok(struct syscall_service *s, int arch, int num)
{
	if (s == NULL || arch > s->narch || arch < 0 || num < 0 ||
	    (unsigned int) num >= s->aws[arch].arch->max_scn)
		return 0;
	return 1;
}

struct error_service *
ss_err(struct syscall_service *s)
{
	return s->err;
}

int
ss_size(struct syscall_service *s)
{
	return s->narch;
}

int
ssa_max_scn(struct syscall_service *s, int arch)
{
	if (!ssa_is_ok(s, arch, 0))
		return -1;
	return s->aws[arch].arch->max_scn;
}

const struct_sysent *
ssa_sysc_list(struct syscall_service *s, int arch)
{
	if (!ssa_is_ok(s, arch, 0))
		return NULL;
	return s->aws[arch].arch->syscall_list;
}

int
ssa_flag(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	return s->aws[arch].flag[num];
}

int
ssa_set_flag(struct syscall_service *s, int arch, int num, int flag)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	s->aws[arch].flag[num] = flag;
	return 0;
}

int
ssa_real_num(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	return s->aws[arch].real_snum[num];
}

int
ssa_set_real_num(struct syscall_service *s, int arch, int num, int real_num)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	s->aws[arch].real_snum[num] = real_num;
	return 0;
}

const char *
ssa_syscall_name(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return NULL;
	return s->aws[arch].arch->syscall_list[num].sys_name;
}

int
ssa_syscall_flag(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	return s->aws[arch].arch->syscall_list[num].sys_flags;
}

int
ssa_syscall_nargs(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return -1;
	return (int)s->aws[arch].arch->syscall_list[num].nargs;
}

int
ssa_user_num1(struct syscall_service *s, int arch)
{
	if (!ssa_is_ok(s, arch, 0))
		return -1;
	return *(s->aws[arch].arch->user_num1);
}

int
ssa_user_num2(struct syscall_service *s, int arch)
{
	if (!ssa_is_ok(s, arch, 0))
		return -1;
	return *(s->aws[arch].arch->user_num2);
}

void
ss_free(struct syscall_service *s)
{
	int i;

	es_free(ss_err(s));
	for (i = 0; i < s->narch; i++ ) {
		free(s->aws[i].flag);
		free(s->aws[i].real_snum);
		if (s->aws[i].a_name)
			free(s->aws[i].a_name);
	}
	free(s);
}

int
ssa_print_size(struct syscall_service *s, int arch)
{
	int i;
	int max_scn = ssa_max_scn(s, arch);
	int psize = 0;

	for (i = 0; i < max_scn; i++)
		if (ssa_flag(s, arch, i) & SS_FLAG_PRINT)
			psize++;
	return psize;
}

int
ssa_is_syscall_valid(struct syscall_service *s, int arch, int num)
{
	if (!ssa_is_ok(s, arch, num))
		return 0;
	return ssa_syscall_name(s, arch, num) &&
	       !(ssa_syscall_flag(s, arch, num) & TRACE_INDIRECT_SUBCALL);
}

int
ss_mark_matches(struct syscall_service *s, int arch, char *arg)
{
	int i = 0;
	int scount = 0;
	int max_scn = ssa_max_scn(s, arch);
	struct number_set *trace_set;

	/* Init global variables */
	nsyscall_vec[0] = ssa_max_scn(s, arch);
	sysent_vec[0] =  ssa_sysc_list(s, arch);

	trace_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(arg, trace_set);
	for (i = 0; i < max_scn; i++)
		if (ssa_is_syscall_valid(s, arch, i) &&
		    ssa_real_num(s, arch, i) != HIDDEN_SYSC &&
		    is_number_in_set_array(i, trace_set, 0)) {
			ssa_set_flag(s, arch, i, SS_FLAG_PRINT);
			scount++;
		}
	if (scount == 0) {
		es_set_error(ss_err(s), SD_NO_MATCHES_FND);
		return SD_NO_MATCHES_FND;
	}
	return 0;
}

int
ss_update_sc_num(struct syscall_service *s, int arch)
{
	int i = 0;
	int max_scn = ssa_max_scn(s, arch);
	int usr1n = ssa_user_num1(s, arch);
	int usr2n = ssa_user_num2(s, arch);

	for (i = 0; i < max_scn; i++) {
		if (!ssa_is_syscall_valid(s, arch, i)) {
			ssa_set_real_num(s, arch, i, HIDDEN_SYSC);
			continue;
		}
		switch (s->aws[arch].arch->pers) {
		case ARCH_x86_64_x32:
			/* Pure x32 specific syscalls without X32_SYSCALL_BIT */
			if (strstr(ssa_syscall_name(s, arch, i), "64:"))
				ssa_set_real_num(s, arch, i, HIDDEN_SYSC);
			else
				ssa_set_real_num(s, arch, i, i);
			break;
		case ARCH_arm_oabi:
		case ARCH_arm_eabi:
			/* Do not deal with private ARM syscalls */
			if (i == usr1n)
				ssa_set_real_num(s, arch, i, HIDDEN_SYSC);
			if ((i >= usr1n + 1) && (i <= usr1n + usr2n + 1))
				ssa_set_real_num(s, arch, i, HIDDEN_SYSC);
			if (i < usr1n)
				ssa_set_real_num(s, arch, i, i);
			break;
		case ARCH_sh64_64bit:
			ssa_set_real_num(s, arch, i, i & 0xffff);
			break;
		default:
			ssa_set_real_num(s, arch, i, i);
		}
	}
	return 0;
}

static int
sysccmp(const void *arg1, const void *arg2)
{
	const char *str1 = ((struct in_sysc *)arg1)->sys_name;
	const char *str2 = ((struct in_sysc *)arg2)->sys_name;

	return strcmp(str1, str2);
}

static struct sysc_meta *
ss_make_union(struct syscall_service *s,
	      void* (save)(struct syscall_service *, int, int))
{
	struct in_sysc **sysc;
	struct out_sysc *sysc_l, *sysc_r, *sysc_to, *sysc_fr;
	struct sysc_meta *sm = xcalloc(sizeof(*sm), 1);
	int size = ss_size(s);
	int max_scn;
	int psize;
	int out_size = 0;
	int i, j, k, l;
	int c = 0;
	int res = 0;
	int eff_size = 1;
	/* Preparation */
	sysc = xcalloc(sizeof(*sysc), size);
	for (i = 0; i < size; i++) {
		max_scn = ssa_max_scn(s, i);
		psize = ssa_print_size(s, i);
		sysc[i] = xcalloc(sizeof(**sysc), psize + 1);
		c = 0;
		for (j = 0; j < max_scn; j++) {
			if (!(ssa_flag(s, i, j) & SS_FLAG_PRINT))
				continue;
			sysc[i][c].sys_name = ssa_syscall_name(s, i, j);
			sysc[i][c].data = save(s, i, j);
			c++;
		}
		qsort(sysc[i], psize, sizeof(struct in_sysc), &sysccmp);
		out_size += psize;
	}
	/* Allocation */
	sysc_l = xcalloc(sizeof(*sysc_l), out_size);
	sysc_r = xcalloc(sizeof(*sysc_r), out_size);
	for (i = 0; i < out_size; i++) {
		sysc_r[i].data = xcalloc(sizeof(void *), size);
		sysc_l[i].data = xcalloc(sizeof(void *), size);
	}
	/* Set with first arch in sysc */
	for (i = 0; sysc[0][i].sys_name != NULL; i++) {
		sysc_r[i].sys_name = sysc[0][i].sys_name;
		sysc_r[i].data[0] = sysc[0][i].data;
	}
	/* Union
	   Main idea is simple:
	   [sysc_to]<->[sysc_fr]<------[sysc1][sysc2][sysc3]...
	   1) [sysc_fr] = [sysc1]
	   2) [sysc_to] = [sysc_fr] | [sysc2]
	   3) [sysc_to] <-> [sysc_fr]
	   4) [sysc_to] = [sysc_fr] | [sysc3]
	   etc. */
	sysc_to = sysc_r;
	sysc_fr = sysc_l;
	for (i = 1; i < size; i++) {
		l = 0; j = 0; k = 0;
		if (sysc[i][j].sys_name != NULL || i == 1) {
			sysc_fr = (eff_size % 2) ? sysc_r : sysc_l;
			sysc_to = (eff_size % 2) ? sysc_l : sysc_r;
			eff_size++;
		}
		while (sysc[i][j].sys_name != NULL && k < out_size) {
			memset(sysc_to[l].data, 0, sizeof(void *) * size);
			if (!sysc_fr[k].sys_name)
				res = -1;
			else
				res = strcmp(sysc[i][j].sys_name,
					     sysc_fr[k].sys_name);
			if (res < 0) {
				sysc_to[l].sys_name = sysc[i][j].sys_name;
				sysc_to[l].data[i] = sysc[i][j].data;
				j++;
			} else if (res > 0) {
				sysc_to[l].sys_name = sysc_fr[k].sys_name;
				memcpy(sysc_to[l].data, sysc_fr[k].data,
				       sizeof(void *) * size);
				k++;
			} else {
				sysc_to[l].sys_name = sysc[i][j].sys_name;
				memcpy(sysc_to[l].data, sysc_fr[k].data,
				       sizeof(void *) * size);
				sysc_to[l].data[i] = sysc[i][j].data;
				k++;
				j++;
			}
			l++;
		}
		while (k < out_size && l < out_size) {
			sysc_to[l].sys_name = sysc_fr[k].sys_name;
			memcpy(sysc_to[l].data, sysc_fr[k].data,
			       sizeof(void *) * size);
			k++;
			l++;
		}
	}
	/* Free */
	for (i = 0; i < out_size; i++)
		free(sysc_fr[i].data);
	free(sysc_fr);
	for (i = 0; i < size; i++)
		free(sysc[i]);
	free(sysc);

	sm->sysc_list = sysc_to;
	sm->size = out_size;
	return sm;
}

static struct sysc_meta *
ss_make_enumeration(struct syscall_service *s,
		    void* (save)(struct syscall_service *, int, int))
{
	struct out_sysc *sysc_out;
	struct sysc_meta *sm = xcalloc(sizeof(*sm), 1);
	int size = ss_size(s);
	int max_scn = 0;
	int i, j, k;
	int flag;
	bool clear = true;

	for (i = 0; i < size; i++)
		if (max_scn < ssa_max_scn(s, i))
			max_scn = ssa_max_scn(s, i);

	sysc_out = xcalloc(sizeof(*sysc_out), max_scn);
	for (i = 0; i < max_scn; i++)
		sysc_out[i].data = xcalloc(sizeof(void *), size);
	for (i = 0; i < size; i++)
		for (j = 0; j < max_scn; j++) {
			clear = true;
			flag = ssa_flag(s, i, j);
			if ((flag != -1) && (flag & SS_FLAG_PRINT)) {
				sysc_out[j].sys_num = j;
				sysc_out[j].data[i] = save(s, i, j);
			}
			for (k = 0; k <= i; k++)
				if (sysc_out[j].data[k])
					clear = 0;
			if (clear)
				sysc_out[j].sys_num = -1;
		}
	sm->sysc_list = sysc_out;
	sm->size = max_scn;
	return sm;
}

static void *
ssa_save_snum(struct syscall_service *s, int arch, int snum)
{
	return (void *)&(s->aws[arch].real_snum[snum]);
}

static void *
ssa_save_nargs(struct syscall_service *s, int arch, int snum)
{
	return (void *)&(s->aws[arch].arch->syscall_list[snum].nargs);
}

static void *
ssa_save_sname(struct syscall_service *s, int arch, int snum)
{
	return (void *)(s->aws[arch].arch->syscall_list[snum].sys_name);
}

static unsigned
fast_len(int num)
{
	int i;
	unsigned count = 0;

	for (i = 1; num/i != 0; i *= 10)
		count++;
	return count;
}

static unsigned *
ss_get_width_sname(struct syscall_service *s, struct sysc_meta *sm, int narch)
{
	/* '2' hereinafter takes into account first two default columns */
	unsigned *width = xcalloc(sizeof(*width), narch + 2);
	int i, j;
	unsigned len;
	unsigned max;
	int N = 1;
	struct out_sysc *psysc = sm->sysc_list;

	/* Calculate length of 'N' and sname columns */
	for (i = 0; i < sm->size; i++) {
		if (!psysc[i].sys_name)
			continue;
		len = strlen(psysc[i].sys_name);
		if (len > width[1])
			width[1] = len;
		N++;
	}
	width[0] = fast_len(N);
	for (i = 0; i < narch; i++) {
		for (j = 0; j < sm->size; j++) {
			if (!psysc[j].data[i])
				continue;
			max = *((int *)(psysc[j].data[i]));
			if (max > width[i + 2])
				width[i + 2] = max;
		}
		width[i + 2] = fast_len(width[i + 2]);
		max = 0;
	}
	return width;
}

static unsigned *
ss_get_width_snum(struct syscall_service *s, struct sysc_meta *sm, int narch)
{
	unsigned *width = xcalloc(sizeof(*width), narch + 2);
	int i, j;
	unsigned len;
	unsigned max;
	int N = 1;
	struct out_sysc *psysc = sm->sysc_list;


	/* Calculate length of 'N' and snum columns */
	for (i = 0; i < sm->size; i++) {
		if (psysc[i].sys_num == -1)
			continue;
		max = fast_len(psysc[i].sys_num);
		if (max > width[1])
			width[1] = max;
		N++;
	}
	width[1] = fast_len(width[1]);
	width[0] = fast_len(N);
	for (i = 0; i < narch; i++) {
		for (j = 0; j < sm->size; j++) {
			if (!psysc[j].data[i])
				continue;
			if (s->request_type & SD_REQ_NARGS)
				len = *((int *)(psysc[j].data[i]));
			else
				len = strlen((char *)(psysc[j].data[i]));
			if (len > width[i + 2])
				width[i + 2] = len;
		}
		if (s->request_type & SD_REQ_NARGS)
			width[i + 2] = fast_len(width[i + 2]);
		len = 0;
	}
	return width;
}

void
ss_dump(struct syscall_service *s, int is_raw)
{
	static const char *title[] = {
		"N",
		"Syscall name",
		"Snum",
	};
	struct sysc_meta *sm;
	struct out_sysc *psysc;
	int ncolumn = ss_size(s);
	unsigned *title_len;
	int N = 1;
	int i, j;

	/* Main work */
	if (s->request_type & SD_REQ_GET_SNAME) {
		if (s->request_type & SD_REQ_NARGS)
			sm = ss_make_union(s, ssa_save_nargs);
		else
			sm = ss_make_union(s, ssa_save_snum);
		title_len = ss_get_width_sname(s, sm, ncolumn);
		if (strlen(title[1]) > title_len[1])
			title_len[1] = strlen(title[1]);
	} else {
		if (s->request_type & SD_REQ_NARGS)
			sm = ss_make_enumeration(s, ssa_save_nargs);
		else
			sm = ss_make_enumeration(s, ssa_save_sname);
		title_len = ss_get_width_snum(s, sm, ncolumn);
		if (strlen(title[2]) > title_len[1])
			title_len[1] = strlen(title[2]);
	}
	psysc = sm->sysc_list;
	if (is_raw)
		goto skip_format;
	/* Adjust width */
	for (i = 0; i < ncolumn; i++) {
		if (strlen(s->aws[i].a_name) > title_len[i + 2])
			title_len[i + 2] = strlen(s->aws[i].a_name);
		if (strlen(s->aws[i].arch->abi_mode) > title_len[i + 2])
			title_len[i + 2] = strlen(s->aws[i].arch->abi_mode);
	}
	/* Print out title */
	printf("| %*s | %*s |", title_len[0], "", title_len[1], "");
	for (i = 0; i < ncolumn; i++)
		printf(" %*s |", title_len[i + 2], s->aws[i].a_name);
	puts("");
	printf("| %*s |", title_len[0], title[0]);
	if (s->request_type & SD_REQ_GET_SNAME)
		printf(" %*s |", title_len[1], title[1]);
	else
		printf(" %*s |", title_len[1], title[2]);
	for (i = 0; i < ncolumn; i++)
		printf(" %*s |", title_len[i + 2], s->aws[i].arch->abi_mode);
	puts("");
	/* Syscalls */
	for (i = 0; i < sm->size; i++) {
		if (s->request_type & SD_REQ_GET_SNAME) {
			if (psysc[i].sys_name == NULL)
				continue;
			printf("| %*d |", title_len[0], N);
			printf(" %*s |", title_len[1], psysc[i].sys_name);
		} else {
			if (psysc[i].sys_num == -1)
				continue;
			printf("| %*d |", title_len[0], N);
			printf(" %*d |", title_len[1], psysc[i].sys_num);
		}
		for (j = 0; j < ncolumn; j++) {
			if (!psysc[i].data[j]) {
				printf(" %*s |", title_len[j + 2], "-");
				continue;
			}
			if (s->request_type & SD_REQ_GET_SNAME ||
			    s->request_type & SD_REQ_NARGS)
				printf(" %*d |", title_len[j + 2],
				       *((int *)(psysc[i].data[j])));
			else
				printf(" %*s |", title_len[j + 2],
				       (char *)(psysc[i].data[j]));
		}
		puts("");
		N++;
	}
	goto out;
skip_format:
	for (i = 0; i < sm->size; i++) {
		if (s->request_type & SD_REQ_GET_SNAME) {
			if (psysc[i].sys_name == NULL)
				continue;
			printf("%d;%s;", N, psysc[i].sys_name);
		} else {
			if (psysc[i].sys_num == -1)
				continue;
			printf("%d;%d;", N, psysc[i].sys_num);
		}
		for (j = 0; j < ncolumn; j++) {
			if (!psysc[i].data[j]) {
				printf("%c;", '-');
				continue;
			}
			if (s->request_type & SD_REQ_GET_SNAME ||
			    s->request_type & SD_REQ_NARGS)
				printf("%d;", *((int *)(psysc[i].data[j])));
			else
				printf("%s;", (char *)(psysc[i].data[j]));
		}
		puts("");
		N++;
	}
out:
	/* free, exit */
	for (i = 0; i < sm->size; i++) {
		free(psysc[i].data);
	}
	free(psysc);
	free(sm);
	free(title_len);
}
