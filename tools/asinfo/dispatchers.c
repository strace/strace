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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

#include "arch_interface.h"
#include "dispatchers.h"
#include "macros.h"
#include "request_msgs.h"
#include "syscall_interface.h"
#include "sysent.h"
#include "xmalloc.h"

struct arch_service *
arch_dispatcher(unsigned request_type, char *arch[])
{
	struct utsname info_uname;
	int i;
	ARCH_LIST_DEFINE(arch_list) = al_create_filled();
	ARCH_LIST_DEFINE(arch_final) = NULL;

	/* If user don't type any option in ARCH_REQ group, it means
	   get current arch */
	if ((request_type & AD_REQ_GET_ARCH) ||
	    (!(request_type & AD_REQ_MASK))) {
		uname(&info_uname);
		if (al_mark_matches(arch_list, info_uname.machine) == -1) {
			es_set_error(al_err(arch_list), AD_UNSUP_ARCH);
			es_set_option(al_err(arch_list), info_uname.machine,
				      NULL, NULL);
			goto fail;
		}
		/* Cut off useless archs */
		arch_final = al_join_print(arch_final, arch_list);
		al_unmark_all(arch_final, AD_FLAG_PRINT);
		free(arch_list);
		goto done;
	}

	if (request_type & AD_REQ_SET_ARCH) {
		for (i = 0; arch[i] != NULL; i++) {
			if (al_mark_matches(arch_list, arch[i]) == -1) {
				es_set_error(al_err(arch_list), AD_UNSUP_ARCH);
				es_set_option(al_err(arch_list), arch[i],
					      NULL, NULL);
				goto fail;
			}
			arch_final = al_join_print(arch_final, arch_list);
		}
		al_unmark_all(arch_final, AD_FLAG_PRINT);
		al_free(arch_list);
		goto done;
	}

	if ((request_type & AD_REQ_LIST_ARCH)) {
		int a_size = al_size(arch_list);
		for (i = 0; i < a_size; i++) {
			al_add_flag(arch_list, i, AD_FLAG_PRINT);
		}
		arch_final = arch_list;
		goto done;
	}
fail:
	return arch_list;
done:
	return arch_final;
}

int
abi_dispatcher(struct arch_service *a_serv, unsigned request_type,
	       char *abi[])
{
	int i = 0;
	enum arch_pers pers;
	int arch_size = 0;
	int a_pos = 0;

	arch_size = al_size(a_serv);
	/* The strace package could be compiled as 32bit app on 64bit
	   architecture, therefore asinfo has to detect it and print out
	   corresponding personality. Frankly speaking, it is necessary to
	   detect strace package personality when it is possible */
	if (!(request_type & ABD_REQ_MASK) &&
	    !(request_type & AD_REQ_LIST_ARCH)) {
		pers = al_pers(a_serv, a_pos);
		switch (pers) {
#if defined(MIPS)
		case ARCH_mips_o32:
		case ARCH_mips64_n64:
			al_mark_pers4arch(a_serv, a_pos,
#if defined(LINUX_MIPSO32)
					  "o32"
#elif defined(LINUX_MIPSN32)
					  "n32"
#elif defined(LINUX_MIPSN64)
					  "n64"
#endif
					 );
			break;
#endif
#if defined(ARM)
		case ARCH_arm_oabi:
			al_mark_pers4arch(a_serv, a_pos,
#if defined(__ARM_EABI__) || !defined(ENABLE_ARM_OABI)
					  "eabi"
#else
					  "oabi"
#endif
					 );
			break;
#endif
#if defined(AARCH64)
		case ARCH_aarch64_64bit:
			al_mark_pers4arch(a_serv, a_pos,
#if defined(__ARM_EABI__)
					  "eabi"
#else
					  "64bit"
#endif
					 );
			break;
#endif
#if defined(X86_64) || defined(X32)
		case ARCH_x86_64_64bit:
			al_mark_pers4arch(a_serv, a_pos,
#if defined(X86_64)
					  "64bit"
#elif defined(X32)
					  "x32"
#endif
				    );
			break;
#endif
/* Especially for x86_64 32bit ABI, because configure.ac defines it
   as I386 arch */
#if defined(I386)
		case ARCH_x86_64_64bit:
			al_mark_pers4arch(a_serv, a_pos, "32bit");
			break;
#endif
#if defined(POWERPC) && !defined(POWERPC64LE)
		case ARCH_ppc64_64bit:
			al_mark_pers4arch(a_serv, a_pos,
# if defined(POWERPC64)
					  "64bit"
# else
					  "32bit"
# endif
				    );
			break;
#endif
#if defined(TILE)
		case ARCH_tile_64bit:
			al_mark_pers4arch(a_serv, a_pos,
#if defined(__tilepro__)
					  "32bit"
#else
					  "64bit"
#endif
				    );
			break;
#endif
		default:
			if (arch_size == 1) {
				al_add_flag(a_serv, a_pos, AD_FLAG_PRINT);
				goto done;
			}
			es_set_error(al_err(a_serv), ABI_CANNOT_DETECT);
			es_set_option(al_err(a_serv),
				      al_in_aname(a_serv, a_pos), NULL, NULL);
		}
		goto done;
	}

	if (request_type & ABD_REQ_LIST_ABI) {
		while (a_pos != arch_size) {
			if (!al_mark_pers4arch(a_serv, a_pos, "all")) {
				a_pos += al_get_abi_modes(a_serv, a_pos);
				continue;
			}
			break;
		}
		goto done;
	}

	if (request_type & ABD_REQ_SET_ABI) {
		for (i = 0; abi[i] != NULL; i++) {
			if (!al_mark_pers4arch(a_serv, a_pos, abi[i])) {
				a_pos += al_get_abi_modes(a_serv, a_pos);
				continue;
			}
			es_set_error(al_err(a_serv), ABI_WRONG4ARCH);
			es_set_option(al_err(a_serv),
				      al_in_aname(a_serv, a_pos),
				      abi[i], NULL);
			break;
		}
	}
done:
	return 0;
}

struct syscall_service *
syscall_dispatcher(struct arch_service *arch, int request_type, char *sysc)
{
	SYSCALL_LIST_DEFINE(sysc_serv) = ss_create(arch, request_type);
	int narch = ss_size(sysc_serv);
	int i = 0;
	int ret = 0;
	int count = 0;

	if (request_type & SD_REQ_MASK) {
		for (i = 0; i < narch; i++) {
			ss_update_sc_num(sysc_serv, i);
			ret = ss_mark_matches(sysc_serv, i, sysc);
			if (ret == SD_NO_MATCHES_FND)
				count++;
		}
	}
	/* Clear error if we are in multiarch mode */
	if (count != narch && narch != 1)
		es_set_error(ss_err(sysc_serv), NO_ERROR);

	return sysc_serv;
}
