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
#include <string.h>
#include <stdio.h>

#include "error_interface.h"
#include "xmalloc.h"

static const char *errors[] = {
	[AD_UNSUP_ARCH_BIT]	= "architecture '%s' is unsupported",
	[ABI_CANNOT_DETECT_BIT]	= "ABI mode cannot be automatically "
				  "detected for non-target architecture '%s'",
	[ABI_WRONG4ARCH_BIT]	= "architecture '%s' does not have ABI mode "
				  "'%s'",
	[SD_NO_MATCHES_FND_BIT]	= "no matches found",
};

struct error_service *
es_create(void)
{
	struct error_service *err = xcalloc(sizeof(*err), 1);

	return err;
}

enum common_error
es_error(struct error_service *e)
{
	return e->last_error;
}

void
es_set_error(struct error_service *e, enum common_error error)
{
	e->last_error = error;
}

void
es_set_option(struct error_service *e, char *arch, char *abi, char *sc)
{
	if (arch) {
		if (e->last_arch)
			free(e->last_arch);
		e->last_arch = xcalloc(sizeof(*(e->last_arch)),
				       strlen(arch) + 1);
		strcpy(e->last_arch, arch);
	}
	if (abi) {
		if (e->last_abi)
			free(e->last_abi);
		e->last_abi = xcalloc(sizeof(*(e->last_abi)), strlen(abi) + 1);
		strcpy(e->last_abi, abi);
	}
	if (sc) {
		if (e->last_sc)
			free(e->last_sc);
		e->last_sc = xcalloc(sizeof(*(e->last_sc)), strlen(sc) + 1);
		strcpy(e->last_sc, sc);
	}
}

const char *
es_get_serror(struct error_service *e)
{
	int err = 1 << e->last_error;
	if (err & ERROR_ARCH_MASK)
		sprintf(e->string, errors[e->last_error], e->last_arch);
	else if (err & ERROR_NO_ARG_MASK)
		sprintf(e->string, "%s", errors[e->last_error]);
	else if (err & ERROR_ARCH_ABI_MASK)
		sprintf(e->string, errors[e->last_error], e->last_arch,
			e->last_abi);
	return (const char *)(e->string);
}

void
es_free(struct error_service *e)
{
	free(e);
}
